module;
#include <exception>
#include <format>
#include <string>
export module moderna.cli:argument_parser_error;

namespace moderna::cli {
  export enum class argument_parser_error_type {
    invalid_value,
    duplicate_argument,
    no_value_given,
    argument_not_required,
    not_an_argument_key,
    too_many_value_given
  };

  /*
    argument_parser_error
    Wraps errors produced by the argument parser, this allows the argument parser to print errors
    in a format that the user can read and understand such that they can fix the arguments given
    to the application.
  */
  export class argument_parser_error : public std::exception {
    argument_parser_error_type __type;
    std::string __message;

  public:
    template <typename T>
      requires(std::is_constructible_v<std::string, T>)
    argument_parser_error(argument_parser_error_type t, T &&message) :
      __type{t}, __message{std::forward<T>(message)} {}
    argument_parser_error_type type() const noexcept {
      return __type;
    }
    const char *what() const noexcept {
      return __message.c_str();
    }
  };
}

namespace cli = moderna::cli;

template <class char_type> struct std::formatter<cli::argument_parser_error_type, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::argument_parser_error_type &v, auto &ctx) const {
    switch (v) {
      case cli::argument_parser_error_type::invalid_value:
        std::format_to(ctx.out(), "INVALID VALUE");
        break;
      case cli::argument_parser_error_type::duplicate_argument:
        std::format_to(ctx.out(), "DUPLICATE ARGUMENT");
        break;
      case cli::argument_parser_error_type::no_value_given:
        std::format_to(ctx.out(), "NO VALUE GIVEN");
        break;
      case cli::argument_parser_error_type::argument_not_required:
        std::format_to(ctx.out(), "ARGUMENT NOT REQUIRED");
        break;
      case cli::argument_parser_error_type::not_an_argument_key:
        std::format_to(ctx.out(), "NOT AN ARGUMENT KEY");
        break;
      case cli::argument_parser_error_type::too_many_value_given:
        std::format_to(ctx.out(), "TOO MANY VALUE GIVEN");
        break;
    };
    return ctx.out();
  }
};