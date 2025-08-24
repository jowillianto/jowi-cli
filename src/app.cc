module;
#include <cstdlib>
#include <expected>
#include <format>
#include <print>
#include <ranges>
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
import :terminal_nodes;

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
      nodes.append_nodes(
        terminal_node::text("{} v{}", id.name, id.version),
        terminal_node::new_line(),
        id.description.length() != 0
          ? terminal_nodes{terminal_node::text("{}", id.description), terminal_node::new_line()}
          : terminal_nodes{},
        id.author
          ? terminal_nodes{terminal_node::text("By {}", id.author.value()), terminal_node::new_line()}
          : terminal_nodes{},
        id.license
          ? terminal_nodes{terminal_node::text("License: {}", id.license.value()), terminal_node::new_line()}
          : terminal_nodes{},
        terminal_node::new_line(),
        std::ranges::transform_view{
          args(), [](auto &&arg) { return terminal_node::text("{}", arg.value); }
        },
        args().size() == 0 ? terminal_nodes{} : terminal_nodes{terminal_node::new_line()},
        terminal_node::format_begin(help_fmt)
      );
      size_t start_id = std::max(0, static_cast<int>(args().size()) - 1);
      for (auto arg = __parser.begin() + start_id; arg != __parser.end(); arg += 1) {
        size_t cur_arg_id = std::distance(__parser.begin(), arg);
        bool print_cur_pos = cur_arg_id > start_id;
        auto sub_nodes = terminal_nodes{
          print_cur_pos ? static_cast<std::uint8_t>(2) : static_cast<std::uint8_t>(0)
        };
        if (print_cur_pos) {
          nodes.append_nodes(terminal_node::text("arg{}", cur_arg_id), terminal_node::new_line());
          arg->pos.help(sub_nodes);
        }
        nodes.append_nodes(
          !(arg->empty())
            ? terminal_nodes{terminal_node::text("Keyword Arguments: "), terminal_node::new_line()}
            : terminal_nodes{}
        );
        for (const auto &[k, arg] : (*arg)) {
          sub_nodes.append_nodes(terminal_node::text("{}: ", k), terminal_node::new_line());
          auto sub_sub_nodes = terminal_nodes{2};
          arg.help(sub_sub_nodes);
          sub_nodes.append_nodes(std::move(sub_sub_nodes));
        }
        nodes.append_nodes(std::move(sub_nodes));
      }
      nodes.append_nodes(terminal_node::format_end());
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

    std::expected<void, parse_error> parse_args(bool auto_help = true, bool auto_exit = true) {
      auto res = __parser.parse(__args);
      auto help_arg_count = __args.count("-h") + __args.count("--help");
      if (help_arg_count != 0 && auto_help) {
        std::println("{}", help());
        std::exit(0);
      }
      if (!res && auto_exit) {
        error(1, "{}", res.error().what());
      }
      if (!res) {
        return std::unexpected{std::move(res.error())};
      }
      return {};
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
          terminal_nodes{
            terminal_node::format_begin(err_fmt),
            terminal_node::text(fmt, std::forward<Args>(args)...),
            terminal_node::format_end()
          }
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
        terminal_nodes{
          terminal_node::format_begin(err_fmt),
          terminal_node::text(fmt, std::forward<Args>(args)...),
          terminal_node::format_end()
        }
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