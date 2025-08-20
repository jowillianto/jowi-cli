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
      err_fmt{text_format{}.fg(color::red())}, help_fmt{text_format{}.fg(color::green())} {}

    arg &add_argument() {
      return __parser.add_argument();
    }
    arg &add_argument(arg_key k) {
      return __parser.add_argument(std::move(k));
    }
    arg &add_argument(std::string_view k) {
      return __parser.add_argument(std::move(k));
    }

    const parsed_arg &args() const noexcept {
      return __args;
    }

    /*
      Formatting with Error and Help
    */
    terminal_nodes help() const {
      auto nodes = terminal_nodes{}.begin_format(help_fmt);
      auto beg_id = args().size();
      for (auto arg = __parser.begin() + args().size() - 1; arg != __parser.end(); arg += 1) {
        if (beg_id == args().size()) {
          nodes.append_node("arg{}", beg_id).new_line();
        }
        if (!arg->empty()) {
          nodes.append_node("Keyword Arguments: ").new_line();
        }
        for (const auto &[k, arg] : (*arg)) {
          nodes.append_node("{}: ", k);
          auto sub_nodes = terminal_nodes{2};
          arg.help(sub_nodes);
          nodes.append_sub_node(std::move(sub_nodes));
        }
        beg_id += 1;
      }
      return nodes;
    }

    void add_help_argument() {
      __parser.add_argument("-h").help("Show the help message for the application").optional();
      __parser.add_argument("--help").help("Show the help message for the application").optional();
    }

    void parse_args(bool auto_help = true) {
      auto res = __parser.parse(__args);
      auto help_arg_count = __args.count("-h") + __args.count("--help");
      if (help_arg_count != 0 && auto_help) {
        std::println("{}", help());
        std::exit(0);
      }
      if (!res) {
        std::println("{}", terminal_nodes::with_format(err_fmt, res.error().what(), 0));
        std::exit(1);
      }
    }
    template <class T, class E, std::invocable<generic::error_formatter, std::string_view, int> F>
      requires(std::constructible_from<generic::error_formatter, E>)
    T expect_or(
      std::expected<T, E> &&res, F &&f, std::source_location loc = std::source_location::current()
    ) {
      if (!res) {
        std::invoke(
          f,
          generic::error_formatter{std::move(res.error())},
          std::string_view{loc.file_name()},
          loc.line()
        );
        std::exit(1);
      } else {
        return std::move(res.value());
      }
    }
    template <class T, class E, class... Args>
      requires(std::constructible_from<generic::error_formatter, E>)
    T expect(
      std::expected<T, E> &&res,
      std::format_string<generic::error_formatter &, std::string_view &, int &, Args...> fmt,
      Args &&...args,
      std::source_location loc = std::source_location::current()
    ) {
      return expect_or(
        std::move(res),
        [&](auto e, auto fname, auto line) {
          std::println(
            stderr,
            "{}",
            terminal_nodes{}
              .begin_format(err_fmt)
              .append_node(fmt, e, fname, line, std::forward<Args>(args)...)
              .end_format()
          );
        },
        loc
      );
    }
    template <class... Args>
    void error(int ret_code, std::format_string<Args...> fmt, Args &&...args) {
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
  };
}