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
  struct help_formatter {
    bool include_app_id = true;
    std::string operator()(
      const argument_parser &parser, const parsed_argument &args, const app_identity &id
    ) const {
      std::string buf;
      auto it = std::back_insert_iterator(buf);
      if (include_app_id) {
        std::format_to(it, "{}\n", id);
      }
      (*this)(it, parser, args);
      return buf;
    }

    std::back_insert_iterator<std::string> &operator()(
      std::back_insert_iterator<std::string> &it,
      const argument_parser &parser,
      const parsed_argument &args
    ) const {
      // Print The currently inputted positional arguments
      for (auto arg = args.arg_begin(); arg != args.arg_end(); arg++) {
        if (arg + 1 == args.arg_end()) {
          std::format_to(it, "{}\n", arg->positional_argument);
        } else {
          std::format_to(it, "{} ", arg->positional_argument);
        }
      }
      // Print the parameters help starting from the currently inputted tier.
      for (auto tier = parser.begin() + args.tier() - 1; tier != parser.end(); tier++) {
        // Print argument help
        auto tier_id = std::distance(parser.begin(), tier);
        if (tier != parser.begin() && tier > parser.begin() + args.tier() - 1) {
          std::format_to(
            it, "arg{} - {}\n", tier_id, tier->positional.help_text.value_or("<no-help>")
          );
          if (tier->positional.options) {
            std::format_to(it, "Options: \n");
            for (auto option = tier->positional.options->begin();
                 option != tier->positional.options->end();
                 option++) {
              if (option->help_text) {
                std::format_to(it, "{:16} : {}\n", option->option, option->help_text.value());
              } else {
                std::format_to(it, "{:16}\n", option->option);
              }
            }
          }
        }
        for (auto parameter = tier->parameters.begin(); parameter != tier->parameters.end();
             parameter += 1) {
          if (parameter->help_text) {
            std::format_to(
              it, "{:16} : {}\n", parameter->key.value(), parameter->help_text.value()
            );
          } else {
            std::format_to(it, "{:16}\n", parameter->key.value());
          }
        }
      }
      return it;
    }
  }; 
  
  export class app {
    struct parse_result {
      std::reference_wrapper<const app> app_ref;
      std::optional<argument_parser_error> err;

      const parse_result &exit_on_error(bool print_err = true, int err_code = 1) const {
        if (err) {
          app_ref.get().error(err_code, "Error of type {} : {}", err->type(), err->what());
        }
        return *this;
      }
      const parse_result &exit_on_help(
        int ret_code = 0, help_formatter format = help_formatter{}
      ) const {
        auto args = app_ref.get().arguments();
        if (args.contains("--help") || args.contains("-h")) {
          std::println(
            stdout,
            "{}",
            format(app_ref.get().parser(), app_ref.get().arguments(), app_ref.get().identity())
          );
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
    app_identity __identity;
    argument_parser __parser;
    parsed_argument __parsed_args;
    std::string __help;

  public:
    app(app_identity id, int argc, const char **argv, bool auto_help = true) :
      __identity{std::move(id)}, __parser{}, __parsed_args{parsed_argument::empty(argc, argv)},
      __help{__identity.description} {
      add_help_argument();
    }

    std::string_view help_text() const noexcept {
      return __help;
    }
    app &help(std::string v) noexcept {
      __help = std::move(v);
      return *this;
    }

    /*
      add a positional argument, accepts one boolean parameter to indicate if help information
      should automatically be added.
    */
    template <class T>
      requires(std::same_as<T, bool>)
    position_argument &add_argument(T v = true) {
      auto &arg = __parser.add_argument();
      if (v) add_help_argument();
      return arg;
    }
    /*
      Adds argument into the parser. This mirrors the call signature for
      argument_parser::add_argument
    */
    parameter_argument &add_argument(std::string_view arg_key) {
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
    const argument_parser &parser() const noexcept {
      return __parser;
    }

    /*
      Get the name of the app
    */
    std::string_view name() const noexcept {
      return __identity.name;
    }

    /*
      Get the description of the application
    */
    std::string_view description() const noexcept {
      return __identity.description;
    }

    /*
      Get the current application version.
    */
    const app_version &version() const noexcept {
      return __identity.version;
    }
    const app_identity &identity() const noexcept {
      return __identity;
    }
    /*
      parses arguments from the command line, this saves the command line arguments inside the
      function itself. Call this on subsequent parses after parsing for the first time with argc and
      argv.
    */
    struct parse_argument_arg {
      bool auto_help = true;
      bool auto_exit = true;
      bool is_final = true;
    };
    parse_result parse_argument(parse_argument_arg arg = parse_argument_arg{true, true, true}) {
      auto res =
        __parser.parse(__parsed_args)
          .transform_error([&](auto &&e) { return parse_result{std::cref(*this), std::move(e)}; })
          .error_or(parse_result{std::cref(*this), std::nullopt});
      if (arg.auto_help) res.exit_on_help();
      if (arg.auto_exit) res.exit_on_error();
      if (arg.is_final) res.assert_final();
      return res;
    }
    template <class... Args>
    void error(int return_code, std::format_string<Args...> fmt, Args &&...args) const {
      std::println(stderr, fmt, std::forward<Args>(args)...);
      exit(return_code);
    }
    template <class T, generic::is_whatable_error E, typename... Args>
    T test_expected(
      std::expected<T, E> &&res,
      int return_code,
      std::format_string<std::string_view, Args...> fmt,
      Args &&...args
    ) const {
      if (res) {
        if constexpr (std::same_as<T, void>) {
          return;
        } else {
          return res.value();
        }
      } else {
        error(
          return_code, fmt, generic::make_error_message(res.error()), std::forward<Args>(args)...
        );
        throw;
      }
    }
    template <class... Args> void warn(std::format_string<Args...> fmt, Args &&...args) {
      std::println(stderr, fmt, std::forward<Args>(args)...);
    }
    template <class... Args> void out(std::format_string<Args...> fmt, Args &&...args) {
      std::println(stdout, fmt, std::forward<Args>(args)...);
    }
  };
}