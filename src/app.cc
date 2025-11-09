module;
#include <cstdlib>
#include <expected>
#include <format>
#include <print>
#include <ranges>
#include <source_location>
#include <string>
#include <string_view>
export module jowi.cli:app;
import jowi.generic;
import jowi.tui;
import :parse_error;
import :arg_key;
import :arg_parser;
import :parsed_arg;
import :raw_args;
import :app_identity;
namespace tui = jowi::tui;

namespace jowi::cli {
  export struct App {
  private:
    ParsedArg __args;
    ArgParser __parser;

  public:
    AppIdentity id;
    tui::TextFormat err_fmt;
    tui::TextFormat help_fmt;
    App(AppIdentity id, int argc, const char **argv, const char **envp = nullptr) :
      __args{RawArgs{argc, argv}}, __parser{}, id{std::move(id)},
      err_fmt{tui::TextFormat{}.fg(tui::Color::red())},
      help_fmt{tui::TextFormat{}.fg(tui::Color::green())} {
      add_help_argument();
    }

    static App create(AppIdentity id, int argc, const char **argv, const char **envp = nullptr) {
      return App{std::move(id), argc, argv, envp};
    }

    Arg &add_argument() {

      Arg &arg = __parser.add_argument();
      add_help_argument();
      return arg;
    }
    Arg &add_argument(ArgKey k, Arg arg = Arg::flag()) {
      return __parser.add_argument(std::move(k), std::move(arg));
    }
    Arg &add_argument(std::string_view k, Arg arg = Arg::flag()) {
      return __parser.add_argument(std::move(k), std::move(arg));
    }

    const ParsedArg &args() const noexcept {
      return __args;
    }
    const ArgParser &parser() const noexcept {
      return __parser;
    }

    /*
      Formatting with Error and Help
    */
    tui::CliNodes help() const {
      auto nodes = tui::CliNodes{};
      // Print the identity
      nodes.append_nodes(
        tui::CliNode::text("{} v{}", id.name, id.version),
        tui::CliNode::new_line(),
        id.description.length() != 0
          ? tui::CliNodes{tui::CliNode::text("{}", id.description), tui::CliNode::new_line()}
          : tui::CliNodes{},
        id.author
          ? tui::CliNodes{tui::CliNode::text("By {}", id.author.value()), tui::CliNode::new_line()}
          : tui::CliNodes{},
        id.license
          ? tui::
              CliNodes{tui::CliNode::text("License: {}", id.license.value()), tui::CliNode::new_line()}
          : tui::CliNodes{},
        tui::CliNode::new_line(),
        std::ranges::transform_view{
          args(), [](auto &&arg) { return tui::CliNode::text("{} ", arg.value); }
        },
        args().size() == 0 ? tui::CliNodes{} : tui::CliNodes{tui::CliNode::new_line()},
        tui::CliNode::format_begin(help_fmt)
      );
      uint64_t start_id = std::max(0, static_cast<int>(args().size()) - 1);
      for (auto arg = __parser.begin() + start_id; arg != __parser.end(); arg += 1) {
        uint64_t cur_arg_id = std::distance(__parser.begin(), arg);
        bool print_cur_pos = cur_arg_id > start_id;
        auto sub_nodes = tui::CliNodes{
          print_cur_pos ? static_cast<std::uint8_t>(2) : static_cast<std::uint8_t>(0)
        };
        if (print_cur_pos) {
          nodes.append_nodes(tui::CliNode::text("arg{}", cur_arg_id), tui::CliNode::new_line());
          arg->pos.help(sub_nodes);
        }
        sub_nodes.append_nodes(
          !(arg->empty())
            ? tui::CliNodes{tui::CliNode::text("Keyword Arguments: "), tui::CliNode::new_line()}
            : tui::CliNodes{}
        );
        for (const auto &[k, arg] : (*arg)) {
          sub_nodes.append_nodes(tui::CliNode::text("{}: ", k), tui::CliNode::new_line());
          auto sub_sub_nodes = tui::CliNodes{2};
          arg.help(sub_sub_nodes);
          sub_nodes.append_nodes(std::move(sub_sub_nodes));
        }
        nodes.append_nodes(std::move(sub_nodes));
      }
      nodes.append_nodes(tui::CliNode::format_end());
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

    std::expected<void, ParseError> parse_args(bool auto_help = true, bool auto_exit = true) {
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
        std::invoke(f, generic::error_formatter{res.error()});
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
      return expect_or(std::forward<std::expected<T, E> &&>(res), [&](generic::error_formatter e) {
        error(
          1,
          "{}",
          tui::CliNodes{
            tui::CliNode::format_begin(err_fmt),
            tui::CliNode::text(fmt, e, std::forward<Args>(args)...),
            tui::CliNode::format_end()
          }
        );
      });
    }
    template <class T, class E>
    T expect(
      std::expected<T, E> &&res, std::source_location loc = std::source_location::current()
    ) const {
      return expect(
        std::forward<std::expected<T, E> &&>(res),
        "{1:} line {2:}: {0:}",
        loc.file_name(),
        loc.line()
      );
    }
    template <class... Args>
    void error(int ret_code, std::format_string<Args...> fmt, Args &&...args) const {
      std::println(
        stderr,
        "{}",
        tui::CliNodes{
          tui::CliNode::format_begin(err_fmt),
          tui::CliNode::text(fmt, std::forward<Args>(args)...),
          tui::CliNode::format_end()
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
