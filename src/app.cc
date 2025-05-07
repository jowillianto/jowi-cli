module;
#include <algorithm>
#include <expected>
#include <format>
#include <functional>
#include <optional>
#include <print>
#include <string>
#include <string_view>
export module moderna.cli:app;
import moderna.generic;
import :argument_parser;
import :argument_value;
import :argument_parser_error;
import :app_version;
import :app_identity;
import :argument;
import :argument_key;

namespace moderna::cli {
  struct app_parser_arg {
    bool auto_help = true;
    bool auto_exit = true;
    bool is_final = true;
  };
  export class app {
  public:
    struct action {
      std::function<void(app &)> action;
      std::string help_text;
    };
    struct action_builder {
      std::reference_wrapper<app> app_ref;

      template <std::invocable<app &> F>
      action_builder &add_action(std::string action_name, std::string help, F &&f) {
        app_ref.get().argument_parser().positional().add_option(
          std::move(action_name), action{std::forward<F>(f), std::move(help)}
        );
        return *this;
      }
      void run() {
        app_ref.get().parse_argument({.is_final = false});
        auto selected_option = app_ref.get().arguments().current_positional();
        auto option = app_ref.get().argument_parser().positional().get_option(selected_option);
        if (!option) {
          app_ref.get().error(1, "Error: No such action \"{}\"", selected_option);
        } else {
          option->get().action(app_ref.get());
        }
      }
    };
    struct parse_result {
      std::reference_wrapper<const app> app_ref;
      std::optional<argument_parser_error> err;

      const parse_result &exit_on_error(bool print_err = true, int err_code = 1) const {
        if (err) {
          app_ref.get().error(err_code, "Error of type {}: {}", err->type(), err->what());
        }
        return *this;
      }
      const parse_result &exit_on_help(int ret_code = 0) const {
        auto args = app_ref.get().arguments();
        if (args.contains("--help") || args.contains("-h")) {
          app_ref.get().print_help(true);
          exit(ret_code);
        }
        return *this;
      }
      void assert_final() {
        if (!app_ref.get().arguments().is_end()) {
          std::string args;
          auto args_inserter = std::back_inserter(args);
          for (auto arg = app_ref.get().arguments().raw_arg_cur();
               arg != app_ref.get().arguments().raw_arg_end();
               arg++) {
            std::format_to(args_inserter, "{} ", *arg);
          }
          app_ref.get().error(1, "Error: Unknown Arguments : {}", args);
        }
      }
    };
    using position_argument_type = optioned_position_argument<action>;
    using parameter_argument_type = parameter_argument;

  private:
    app_identity __id;
    argument_parser<optioned_position_argument<action>, parameter_argument> __parser;
    parsed_argument __parsed_args;

  public:
    app(app_identity id, int argc, const char **argv, bool auto_help = true) :
      __id{std::move(id)}, __parser{}, __parsed_args{parsed_argument::empty(argc, argv)} {
      add_argument(true);
    }

    /*
      Argument Adding parsing
    */
    position_argument_type &add_argument(std::same_as<bool> auto auto_help = true) {
      auto &arg = __parser.add_argument();
      if (auto_help) add_help_argument();
      return arg;
    }
    parameter_argument_type &add_argument(std::string_view arg_key) {
      return __parser.add_argument(argument_key::make_arg(arg_key).value());
    }
    app &add_help_argument(std::string_view help_text = "Show help information") {
      add_argument("-h").optional().as_flag().help(help_text);
      add_argument("--help").optional().as_flag().help(help_text);
      return *this;
    }
    const parsed_argument &arguments() const noexcept {
      return __parsed_args;
    }
    argument_parser<optioned_position_argument<action>, parameter_argument> &argument_parser(
    ) noexcept {
      return __parser;
    }

    /*
      Application Identity
    */
    std::string_view name() const noexcept {
      return __id.name;
    }
    std::string_view description() const noexcept {
      return __id.description;
    }
    const app_version &version() const noexcept {
      return __id.version;
    }
    const app_identity &id() const noexcept {
      return __id;
    }

    /*
      CLI IO
    */
    template <class... Args> void warn(std::format_string<Args...> fmt, Args &&...args) const {
      std::println(stderr, fmt, std::forward<Args>(args)...);
    }
    template <class... Args>
    void error(int return_code, std::format_string<Args...> fmt, Args &&...args) const {
      std::println(stderr, fmt, std::forward<Args>(args)...);
      exit(return_code);
    }
    template <class... Args> void out(std::format_string<Args...> fmt, Args &&...args) const {
      std::println(stdout, fmt, std::forward<Args>(args)...);
    }
    template <class... Args>
    void inline_out(std::format_string<Args...> fmt, Args &&...args) const {
      std::print(stdout, fmt, std::forward<Args>(args)...);
    }
    template <class T, generic::is_whatable_error E, typename... Args>
    T test_expected(
      std::expected<T, E> &&res,
      int return_code,
      std::format_string<std::string_view, Args...> fmt,
      Args &&...args
    ) const {
      if (res.has_value()) {
        if constexpr (std::same_as<T, void>) {
          return;
        } else {
          return std::move(res.value());
        }
      } else {
        error(return_code, fmt, std::string_view{res.error().what()}, std::forward<Args>(args)...);
        throw;
      }
    }

    /*
      Parsing and running
    */
    parse_result parse_argument(app_parser_arg arg = app_parser_arg{true, true, true}) {
      auto res =
        __parser.parse(__parsed_args)
          .transform_error([&](auto &&e) { return parse_result{std::cref(*this), std::move(e)}; })
          .error_or(parse_result{std::cref(*this), std::nullopt});
      if (arg.auto_help) res.exit_on_help();
      if (arg.auto_exit) res.exit_on_error();
      if (arg.is_final) res.assert_final();
      return res;
    }

    /*
      Action Builder
    */
    action_builder build_action(std::string help_text = "") noexcept {
      add_argument(true).help(std::move(help_text));
      return action_builder{std::ref(*this)};
    }

    /*
      Printing Help Information
    */
    void print_help(bool include_id) const {
      if (include_id) {
        out("{}\n", id());
      }
      for (auto arg = __parsed_args.arg_begin(); arg != __parsed_args.arg_end(); arg++) {
        if (arg + 1 == __parsed_args.arg_end()) {
          out("{}", arg->value());
        } else {
          inline_out("{} ", arg->value());
        }
      }
      for (auto tier = __parser.begin() + __parsed_args.tier() - 1; tier != __parser.end();
           tier++) {
        auto tier_id = std::distance(__parser.begin(), tier);
        if (tier != __parser.begin() && tier > __parser.begin() + __parsed_args.tier() - 1) {
          out("arg{} - {}", tier_id, tier->positional.get_help().value_or("<no-help>"));
          if (tier->positional.option_size() != 0) {
            out("Options: ");
            for (auto option = tier->positional.options.begin();
                 option != tier->positional.options.end();
                 option++) {
              out("{:16} : {}", option->name, option->data.help_text);
            }
          }
        }
        if (tier->parameters.size() != 0) {
          out("Parameters : ");
        }
        for (const auto &param : tier->parameters) {
          out("{:16} : {}", param.get_key().value(), param.get_help().value_or("<no-help>"));
        }
      }
    }
  };
  export using app_action = app::action;
  export using app_parse_result = app::parse_result;
  export using app_action_builder = app::action_builder;
  export using app_position_argument = app::position_argument_type;
  export using app_parameter_argument = app::parameter_argument_type;
}
