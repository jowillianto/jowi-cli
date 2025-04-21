module;
#include <expected>
#include <format>
#include <optional>
#include <print>
#include <string>
export module moderna.cli:app;
import :argument_parser;
import :argument_value;
import :argument_parser_error;
import :app_version;
import :app_identity;
import :argument_key;

namespace moderna::cli {
  export class app {
    struct parse_result {
      std::reference_wrapper<const app> app_ref;
      std::optional<argument_parser_error> err;

      const parse_result &exit_on_error(bool print_err = true, int err_code = 1) const {
        if (err) {
          std::println(stderr, "Error of type {} : {}", err->type(), err->what());
          exit(err_code);
        }
        return *this;
      }
      const parse_result &exit_on_help(int ret_code = 0) const {
        auto args = app_ref.get().arguments();
        if (args.args().contains("--help") || args.args().contains("-h")) {
          std::print(stdout, "{}\n", app_ref.get().identity());
          const parsed_argument &args = app_ref.get().arguments();
          // Print Positional Arguments First
          for (auto arg = args.args().begin(); arg != args.args().end(); arg += 1) {
            if (arg + 1 == args.args().end()) {
              std::print(stdout, "{}\n", arg->positional_argument);
            } else {
              std::print(stdout, "{} ", arg->positional_argument);
            }
          }
          std::print("{}\n\n", app_ref.get().help_text());
          auto &parser = app_ref.get().parser();
          for (auto tier = parser.begin() + args.tier() - 1; tier < parser.end(); tier += 1) {
            if (tier != parser.begin()) {
              auto id = std::distance(parser.begin(), tier);
              std::print(
                "arg{} - {} - [{}]\n",
                id,
                tier->positional.help_text.value_or("<no-help>"),
                tier->positional.options
                  .transform([](const auto &options) {
                    std::string opt;
                    for (auto option = options.begin(); option != options.end(); option += 1) {
                      if (option + 1 == options.end()) {
                        std::format_to(std::back_inserter(opt), "{}", *option);
                      } else {
                        std::format_to(std::back_inserter(opt), "{}, ", *option);
                      }
                    }
                    return opt;
                  })
                  .value_or("<no-choices>")
              );
            }
            for (auto parameter = tier->parameters.begin(); parameter != tier->parameters.end();
                 parameter += 1) {
              if (parameter->help_text) {
                std::print("{:16} : {}\n", parameter->key.value(), parameter->help_text.value());
              } else {
                std::print("{:16}\n", parameter->key.value());
              }
            }
          }
          exit(ret_code);
        }
        return *this;
      }
    };
    app_identity __identity;
    argument_parser __parser;
    parsed_argument __parsed_args;
    std::string __help;

  public:
    app(app_identity id, int argc, const char **argv) :
      __identity{std::move(id)}, __parser{}, __parsed_args{parsed_argument::empty(argc, argv)},
      __help{__identity.description} {}

    std::string_view help_text() const noexcept {
      return __help;
    }
    app &help(std::string v) noexcept {
      __help = std::move(v);
      return *this;
    }
    /*
      Adds argument into the parser. This mirrors the call signature for
      argument_parser::add_argument
    */
    auto &add_argument() {
      return __parser.add_argument();
    }
    auto &add_argument(std::string_view arg_key) {
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
    parse_result parse_argument() {
      return __parser.parse(__parsed_args)
        .transform_error([&](auto &&e) {
          return parse_result{std::cref(*this), std::move(e)};
        })
        .error_or(parse_result{std::cref(*this), std::nullopt});
    }
  };
}