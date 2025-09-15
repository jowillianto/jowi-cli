#include <chrono>
#include <memory>
#include <string_view>
#include <thread>
import jowi.crogger;
import jowi.cli;
import jowi.test_lib;
namespace crogger = jowi::crogger;
namespace cli = jowi::cli;
namespace test_lib = jowi::test_lib;

auto crogger_id = cli::app_identity{.name = "Crogger Benchmarker"};

crogger::logger create_logger(std::string_view formatter, std::string_view emitter) {
  crogger::logger logger;
  if (formatter == "bw") {
    logger.set_formatter(crogger::bw_formatter());
  } else if (formatter == "empty") {
    logger.set_formatter(crogger::empty_formatter{});
  } else if (formatter == "plain") {
    logger.set_formatter(crogger::plain_formatter{});
  }
  if (emitter == "empty") {
    logger.set_emitter(crogger::empty_emitter{});
  }
  return logger;
}

template <class F, class... Args>
  requires(std::invocable<F, Args...>)
std::pair<std::invoke_result_t<F, Args...>, std::chrono::system_clock::duration> invoke_bench(
  F &&f, Args &&...args
) {
  auto beg = std::chrono::system_clock::now();
  auto res = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
  auto end = std::chrono::system_clock::now();
  return std::pair{std::move(res), end - beg};
}

auto log_messages(const crogger::logger &logger, std::string_view msg, unsigned count) {
  for (size_t i = 0; i < count; i += 1) {
    crogger::info(logger, crogger::message{"{} - {}", i, msg});
  }
  return count;
}

int main(int argc, const char **argv) {
  cli::app app{crogger_id, argc, argv};
  app.add_argument("--count")
    .help("The Amount of Log Message to benchmark on")
    .require_value()
    .optional();
  app.add_argument("--msg_length")
    .help("The length of messages to print per log, the default is to print: 80")
    .require_value()
    .optional();
  app.add_argument("--emit")
    .help("The output emitter to use. The default is stdout")
    .require_value()
    .add_validator(
      cli::arg_options_validator{}
        .add_option("stdout", "emit logs to stdout")
        .add_option("empty", "do not emit anywhere")
        .move()
    )
    .optional();
  app.add_argument("--format")
    .help("The format for the logger")
    .require_value()
    .optional()
    .add_validator(
      cli::arg_options_validator{}
        .add_option("empty", "no format")
        .add_option("bw", "black and white formatting with date and formatted severity")
        .add_option("color", "colorful formatting with date and formatted severity")
        .add_option("plain", "format message only")
        .move()
    );
  app.parse_args();
  auto count = app.expect(
    app.args().first_of("--count").transform(cli::parse_arg<unsigned int>).value_or(1000000)
  );
  auto log_msg_length = app.expect(
    app.args().first_of("--msg_length").transform(cli::parse_arg<unsigned int>).value_or(80)
  );
  auto formatter =
    app.args().first_of("--format").transform(cli::parse_arg<std::string>).value_or("color");
  auto emitter =
    app.args().first_of("--emit").transform(cli::parse_arg<std::string>).value_or("stdout");
  auto rnd_msg = test_lib::random_string(log_msg_length);
  crogger::warn({"Begin: Logger Init"});
  auto [logger, logger_init_time] = invoke_bench(create_logger, formatter, emitter);
  crogger::warn(crogger::message{"End: Logger Init ({})", logger_init_time});
  crogger::warn(crogger::message{"Begin: Log Message"});
  auto [log_count, logger_log_time] = invoke_bench(log_messages, logger, rnd_msg, count);
  crogger::warn(
    crogger::message{
      "End: Log Message ({})",
      std::chrono::duration_cast<std::chrono::milliseconds>(logger_log_time)
    }
  );
  std::this_thread::sleep_for(std::chrono::seconds{1});
}