module;
#include <chrono>
#include <cstdlib>
#include <expected>
#include <format>
#include <iterator>
#include <memory>
#include <source_location>
#include <string>
export module jowi.crogger:logger;
import :log_error;
import :log_metadata;
import :log_filter;
import :log_formatter;
import :log_emitter;

namespace jowi::crogger {

  export template <typename T>
  concept basic_logger = requires(const T logger, const log_metadata &data) {
    { logger.log(data) } -> std::same_as<std::expected<void, log_error>>;
  };

  export template <typename T = void> struct logger;

  template <> struct logger<void> {
    virtual std::expected<void, log_error> log(const log_metadata &metadata) const = 0;
    virtual ~logger() = default;
  };

  template <basic_logger logger_type>
  struct logger<logger_type> : private logger_type, public logger<void> {
    using logger_type::logger_type;
    logger(logger_type l) : logger_type{std::move(l)} {}

    std::expected<void, log_error> log(const log_metadata &d) const {
      return logger_type::log(d);
    }
  };

  export struct log_pipeline {
  private:
    std::unique_ptr<log_formatter<void>> __formatter;
    std::unique_ptr<log_filter<void>> __filter;
    std::unique_ptr<log_emitter<void>> __emitter;

  public:
    template <basic_formatter formatter_type, basic_filter filter_type, basic_emitter emitter_type>
    log_pipeline(formatter_type &&fmt, filter_type &&flt, emitter_type &&emt) :
      __formatter{std::make_unique<log_formatter<std::decay_t<formatter_type>>>(
        std::forward<formatter_type>(fmt)
      )},
      __filter{
        std::make_unique<log_filter<std::decay_t<filter_type>>>(std::forward<filter_type>(flt))
      },
      __emitter{
        std::make_unique<log_emitter<std::decay_t<emitter_type>>>(std::forward<emitter_type>(emt))
      } {}

    std::expected<void, log_error> log(const log_metadata &metadata) const {
      if (__filter->filter(metadata)) {
        std::string msg;
        auto it = std::back_inserter(msg);
        return __formatter->format(metadata, it).and_then([&]() { return __emitter->emit(msg); });
      }
      return {};
    }
  };

  export struct root_logger {
  private:
    static std::unique_ptr<logger<void>> root;

  public:
    static const logger<void> &get_logger() {
      return *root;
    }
    template <basic_logger logger_type> static void set_logger(logger_type &&l) {
      root = std::make_unique<std::decay_t<logger_type>>(std::forward<logger_type>(l));
    }
  };

  std::unique_ptr<logger<void>> root_logger::root = std::make_unique<logger<log_pipeline>>(
    formatter_tuple{
      ' ', colorful_level_formatter{}, time_formatter{}, file_loc_formatter{}, message_formatter{}
    },
    no_filter{},
    stdout_emitter{}
  );

  export template <basic_logger logger, class... Args>
  std::expected<void, log_error> log(
    const logger &l,
    log_status status,
    std::source_location loc,
    std::format_string<Args...> fmt,
    Args &&...args
  ) {
    return l.log(
      log_metadata{
        .status = std::move(status),
        .message = std::format(fmt, std::forward<Args>(args)...),
        .loc = loc,
        .time = std::chrono::system_clock::now()
      }
    );
  }

  export template <class... Args>
  std::expected<void, log_error> log(
    log_status status, std::source_location loc, std::format_string<Args...> fmt, Args &&...args
  ) {
    return log(root_logger::get_logger(), status, loc, fmt, std::forward<Args>(args)...);
  }
}