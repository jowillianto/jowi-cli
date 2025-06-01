module;
#include <exception>
#include <format>
#include <string>
export module moderna.cli:argparse_error;

namespace moderna::cli {
  export enum struct argparse_error_type {
    INVALID_VALUE,
    DUPLICATE_ARGUMENT,
    NO_VALUE_GIVEN,
    NOT_REQUIRED_ARGUMENT,
    NOT_ARGUMENT_KEY,
    TOO_MANY_VALUE_GIVEN
  };
}

/*
  Formatter implementation for argparse_error_type
*/
namespace cli = moderna::cli;
template <class char_type> struct std::formatter<cli::argparse_error_type, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::argparse_error_type &v, auto &ctx) const {
    switch (v) {
      case cli::argparse_error_type::INVALID_VALUE:
        std::format_to(ctx.out(), "INVALID VALUE");
        break;
      case cli::argparse_error_type::DUPLICATE_ARGUMENT:
        std::format_to(ctx.out(), "DUPLICATE ARGUMENT");
        break;
      case cli::argparse_error_type::NO_VALUE_GIVEN:
        std::format_to(ctx.out(), "NO VALUE GIVEN");
        break;
      case cli::argparse_error_type::NOT_REQUIRED_ARGUMENT:
        std::format_to(ctx.out(), "ARGUMENT NOT REQUIRED");
        break;
      case cli::argparse_error_type::NOT_ARGUMENT_KEY:
        std::format_to(ctx.out(), "NOT AN ARGUMENT KEY");
        break;
      case cli::argparse_error_type::TOO_MANY_VALUE_GIVEN:
        std::format_to(ctx.out(), "TOO MANY VALUE GIVEN");
        break;
    };
    return ctx.out();
  }
};

/*
  argparse_error
  wraps error and gives it an error type. F
*/
namespace moderna::cli {
  export class argparse_error : public std::exception {
    argparse_error_type __type;
    std::string __message;

  public:
    argparse_error(argparse_error_type t) : __type{t}, __message{std::format("{}", __type)} {}
    argparse_error_type type() const noexcept {
      return __type;
    }
    const char *what() const noexcept {
      return __message.c_str();
    }
  };
}