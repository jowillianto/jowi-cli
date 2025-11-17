module;
#include <algorithm>
#include <cstdlib>
#include <expected>
#include <format>
#include <iterator>
#include <print>
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
    App(AppIdentity id, int argc, const char **argv, const char **envp = nullptr) :
      __args{RawArgs{argc, argv}}, __parser{}, id{std::move(id)} {
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
    tui::DomNode help_dom() const {
      auto layout =
        tui::Layout{}.style(help_style).append_child(tui::Paragraph("{} v{}", id.name, id.version));
      if (!id.description.empty()) {
        layout.append_child(tui::Paragraph(id.description));
      }
      if (id.author) {
        layout.append_child(tui::Paragraph("By {}", id.author.value()));
      }
      if (id.license) {
        layout.append_child(tui::Paragraph("License: {}", id.license.value()));
      }
      layout.append_child(tui::Paragraph(""));
      if (!args().empty()) {
        std::string parsed_args;
        for (const auto &arg_value : args()) {
          parsed_args.append(arg_value.value);
          parsed_args.push_back(' ');
        }
        if (!parsed_args.empty()) {
          layout.append_child(tui::Paragraph(parsed_args));
          layout.append_child(tui::Paragraph(""));
        }
      }
      uint64_t start_id = std::max(0, static_cast<int>(args().size()) - 1);
      for (auto arg = __parser.begin() + start_id; arg != __parser.end(); arg += 1) {
        uint64_t cur_arg_id = std::distance(__parser.begin(), arg);
        bool print_cur_pos = cur_arg_id > start_id;
        auto section = tui::Layout{}.style(tui::DomStyle{}.indent(2));
        if (print_cur_pos) {
          section.append_child(tui::Paragraph("arg{}", cur_arg_id));
          if (auto node = arg->pos.help()) {
            auto detail = tui::Layout{}.style(tui::DomStyle{}.indent(2));
            detail.append_child(std::move(node.value()));
            section.append_child(tui::DomNode::vstack(std::move(detail)));
          }
        }
        if (!arg->empty()) {
          section.append_child(tui::Paragraph("Keyword Arguments:"));
          auto keyword_layout = tui::Layout{}.style(tui::DomStyle{}.indent(2));
          for (const auto &[k, validator_arg] : (*arg)) {
            auto entry = tui::Layout{};
            entry.append_child(tui::Paragraph("{}:", k));
            if (auto validator_help = validator_arg.help()) {
              auto validator_layout = tui::Layout{}.style(tui::DomStyle{}.indent(2));
              validator_layout.append_child(std::move(validator_help.value()));
              entry.append_child(tui::DomNode::vstack(std::move(validator_layout)));
            } else {
              entry.append_child(tui::Paragraph("<no help>"));
            }
            keyword_layout.append_child(tui::DomNode::vstack(std::move(entry)));
          }
          section.append_child(tui::DomNode::vstack(std::move(keyword_layout)));
        }
        layout.append_child(tui::DomNode::vstack(std::move(section)));
        layout.append_child(tui::Paragraph(""));
      }
      return tui::DomNode::vstack(std::move(layout));
    }

    void print_help() const {
      tui::out_terminal.render(help_dom());
      std::exit(0);
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
        print_help();
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
      std::convertible_to<generic::ErrorFormatter> E,
      std::invocable<generic::ErrorFormatter> F>
    static T expect_or(std::expected<T, E> &&res, F &&f) {
      if (!res) {
        std::invoke(f, generic::ErrorFormatter{res.error()});
        std::exit(1);
      } else {
        if constexpr (!std::same_as<T, void>) {
          return std::move(res.value());
        }
      }
    }

    template <class T, std::convertible_to<generic::ErrorFormatter> E, class... Args>
      requires(std::formattable<Args, char> && ...)
    static T expect(
      std::expected<T, E> &&res,
      std::format_string<generic::ErrorFormatter &, Args...> fmt,
      Args &&...args
    ) {
      return expect_or(std::forward<std::expected<T, E> &&>(res), [&](generic::ErrorFormatter e) {
        tui::err_terminal.render(
          tui::Layout{}
            .style(error_style)
            .append_child(tui::Paragraph(fmt, e, std::forward<Args>(args)...))
        );
        std::exit(1);
      });
    }
    template <class T, class E>
    static T expect(
      std::expected<T, E> &&res, std::source_location loc = std::source_location::current()
    ) {
      return expect(
        std::forward<std::expected<T, E> &&>(res),
        "{1:} line {2:}: {0:}",
        loc.file_name(),
        loc.line()
      );
    }
    template <class... Args>
    static void error(int ret_code, std::format_string<Args...> fmt, Args &&...args) {
      tui::err_terminal.render(
        tui::Layout{}
          .style(error_style)
          .append_child(tui::Paragraph(fmt, std::forward<Args>(args)...))
      );
      std::exit(ret_code);
    }

    inline static tui::DomStyle error_style = tui::DomStyle{}.fg(tui::RgbColor::bright_yellow());
    inline static tui::DomStyle help_style = tui::DomStyle{}.fg(tui::RgbColor::bright_green());
  };
}
