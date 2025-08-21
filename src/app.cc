module;
#include <cstdlib>
#include <expected>
#include <format>
#include <print>
#include <source_location>
#include <string>
#include <string_view>
export module moderna.cli:app;
import moderna.generic;
import :parse_error;
import :arg_key;
import :arg_parser;
import :parsed_arg;
import :raw_args;
import :app_identity;
import :terminal;

namespace moderna::cli {
  export struct app {
  private:
    parsed_arg __args;
    arg_parser __parser;

  public:
    app_identity id;
    text_format err_fmt;
    text_format help_fmt;
    app(app_identity id, int argc, const char **argv) :
      __args{raw_args{argc, argv}}, __parser{}, id{std::move(id)},
      err_fmt{text_format{}.fg(color::red())}, help_fmt{text_format{}.fg(color::green())} {
      add_help_argument();
    }

    arg &add_argument() {

      arg &arg = __parser.add_argument();
      add_help_argument();
      return arg;
    }
    arg &add_argument(arg_key k, arg arg = arg::flag()) {
      return __parser.add_argument(std::move(k), std::move(arg));
    }
    arg &add_argument(std::string_view k, arg arg = arg::flag()) {
      return __parser.add_argument(std::move(k), std::move(arg));
    }

    const parsed_arg &args() const noexcept {
      return __args;
    }
    const arg_parser &parser() const noexcept {
      return __parser;
    }

    /*
      Formatting with Error and Help
    */
    terminal_nodes help() const {
      auto nodes = terminal_nodes{};
      // Print the identity
      nodes.append_node("{} v{}", id.name, id.version).new_line();
      if (id.description.length() != 0) {
        nodes.append_node("{}", id.description).new_line();
      }
      if (id.author.has_value()) {
        nodes.append_node("By {}", id.author.value()).new_line();
      }
      if (id.license.has_value()) {
        nodes.append_node("License: {}", id.license.value()).new_line();
      }
      nodes.new_line();
      // Help Formatting
      // Print Current Parsed Arguments
      for (auto arg = args().begin(); arg != args().end(); arg += 1) {
        nodes.append_node("{} ", arg->value);
      }
      if (args().size() != 0) {
        nodes.new_line();
      }
      // The actual help arguments, this should start printing from arg_size - 1.
      nodes.begin_format(help_fmt);
      size_t start_id = std::max(0, static_cast<int>(args().size()) - 1);
      for (auto arg = __parser.begin() + start_id; arg != __parser.end(); arg += 1) {
        size_t cur_arg_id = std::distance(__parser.begin(), arg);
        bool print_cur_pos = cur_arg_id > start_id;
        auto sub_nodes =
          terminal_nodes{print_cur_pos ? static_cast<size_t>(2) : static_cast<size_t>(0)};
        if (print_cur_pos) {
          nodes.append_node("arg{}", cur_arg_id).new_line();
          arg->pos.help(sub_nodes);
        }
        if (!arg->empty()) {
          sub_nodes.append_node("Keyword Arguments: ").new_line();
        }
        for (const auto &[k, arg] : (*arg)) {
          sub_nodes.append_node("{}: ", k).new_line();
          auto sub_sub_nodes = terminal_nodes{2};
          arg.help(sub_sub_nodes);
          sub_nodes.append_node(std::move(sub_sub_nodes));
        }
        nodes.append_node(std::move(sub_nodes));
      }
      nodes.end_format();
      return nodes;
    }

    void add_help_argument() {
      __parser.add_argument("-h")
        .help("Show the help message for the application")
        .optional()
        .as_flag();
      __parser.add_argument("--help")
        .help("Show the help message for the application")
        .optional()
        .as_flag();
    }

    void parse_args(bool auto_help = true) {
      auto res = __parser.parse(__args);
      auto help_arg_count = __args.count("-h") + __args.count("--help");
      if (help_arg_count != 0 && auto_help) {
        std::println("{}", help());
        std::exit(0);
      }
      if (!res) {
        error(1, "{}", res.error().what());
      }
    }
    template <
      class T,
      std::convertible_to<generic::error_formatter> E,
      std::invocable<generic::error_formatter> F>
    T expect_or(std::expected<T, E> &&res, F &&f) const {
      if (!res) {
        std::invoke(f, generic::error_formatter{std::move(res.error())});
        std::exit(1);
      } else {
        if constexpr (!std::same_as<T, void>) {
          return std::move(res.value());
        }
      }
    }

    template <class T, std::convertible_to<generic::error_formatter> E, class... Args>
      requires(std::formattable<Args, char> && ...)
    T expect(
      std::expected<T, E> &&res,
      std::format_string<generic::error_formatter &, Args...> fmt,
      Args &&...args
    ) const {
      return expect_or(std::move(res), [&](generic::error_formatter e) {
        error(
          1,
          "{}",
          terminal_nodes{}
            .begin_format(err_fmt)
            .append_node(fmt, e, std::forward<Args>(args)...)
            .end_format()
        );
      });
    }
    template <class T, class E>
    T expect(
      std::expected<T, E> &&res, std::source_location loc = std::source_location::current()
    ) const {
      return expect(std::move(res), "{1:} line {2:}: {0:}", loc.file_name(), loc.line());
    }
    template <class... Args>
    void error(int ret_code, std::format_string<Args...> fmt, Args &&...args) const {
      std::println(
        stderr,
        "{}",
        terminal_nodes{}
          .begin_format(err_fmt)
          .append_node(fmt, std::forward<Args>(args)...)
          .end_format()
      );
      std::exit(ret_code);
    }
    template <class... Args> void out(std::format_string<Args...> fmt, Args &&...args) const {
      std::println(fmt, std::forward<Args>(args)...);
    }
    template <class... Args>
    void out(int ret_code, std::format_string<Args...> fmt, Args &&...args) const {
      std::println(fmt, std::forward<Args>(args)...);
      std::exit(ret_code);
    }
    template <class... Args> void warn(std::format_string<Args...> fmt, Args &&...args) const {
      std::println(stderr, fmt, std::forward<Args>(args)...);
    }
  };
}