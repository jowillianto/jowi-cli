module;
#include <algorithm>
#include <exception>
#include <format>
#include <string>
#include <string_view>
export module moderna.cli:parse_error;

namespace moderna::cli {
  export enum struct parse_error_type {
    INVALID_VALUE,
    DUPLICATE_ARGUMENT,
    NO_VALUE_GIVEN,
    NOT_REQUIRED_ARGUMENT,
    NOT_ARGUMENT_KEY,
    TOO_MANY_VALUE_GIVEN,
    NOT_POSITIONAL
  };
}

/*
  Formatter implementation for parse_error_type
*/
namespace cli = moderna::cli;
template <class char_type> struct std::formatter<cli::parse_error_type, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::parse_error_type &v, auto &ctx) const {
    switch (v) {
      case cli::parse_error_type::INVALID_VALUE:
        std::format_to(ctx.out(), "INVALID VALUE");
        break;
      case cli::parse_error_type::DUPLICATE_ARGUMENT:
        std::format_to(ctx.out(), "DUPLICATE ARGUMENT");
        break;
      case cli::parse_error_type::NO_VALUE_GIVEN:
        std::format_to(ctx.out(), "NO VALUE GIVEN");
        break;
      case cli::parse_error_type::NOT_REQUIRED_ARGUMENT:
        std::format_to(ctx.out(), "ARGUMENT NOT REQUIRED");
        break;
      case cli::parse_error_type::NOT_ARGUMENT_KEY:
        std::format_to(ctx.out(), "NOT AN ARGUMENT KEY");
        break;
      case cli::parse_error_type::TOO_MANY_VALUE_GIVEN:
        std::format_to(ctx.out(), "TOO MANY VALUE GIVEN");
        break;
      case cli::parse_error_type::NOT_POSITIONAL:
        std::format_to(ctx.out(), "NOT POSITIONAL");
        break;
    };
    return ctx.out();
  }
};

/*
  parse_error
  wraps error and gives it an error type. F
*/
namespace moderna::cli {
  export struct parse_error : public std::exception {
    parse_error_type __t;
    std::string __what;
    std::string_view __msg;

  public:
    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    parse_error(parse_error_type t, std::format_string<Args...> fmt, Args &&...args) :
      __t{t}, __what{} {
      auto it = std::back_inserter(__what);
      std::format_to(it, "{}: ", t);
      auto type_end = __what.length();
      std::format_to(it, fmt, std::forward<Args>(args)...);
      __msg = std::string_view{__what.begin() + type_end, __what.end()};
    }
    parse_error(parse_error_type t, std::string_view msg) : parse_error(t, "{}", msg) {}

    const char *what() const noexcept {
      return __what.c_str();
    }

    std::string_view msg() const noexcept {
      return __msg;
    }

    parse_error_type err_type() const noexcept {
      return __t;
    }

    template <class... Args>
    parse_error &reformat(
      std::format_string<parse_error_type &, std::string_view &, Args...> fmt, Args &&...args
    ) {
      __what = std::format(fmt, __t, __msg, std::forward<Args>(args)...);
      __msg = __what;
      return *this;
    }

    // Shortcut Factory Functions
    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static parse_error invalid_value(std::format_string<Args...> fmt, Args &&...args) {
      return parse_error(parse_error_type::INVALID_VALUE, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static parse_error duplicate_argument(std::format_string<Args...> fmt, Args &&...args) {
      return parse_error(parse_error_type::DUPLICATE_ARGUMENT, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static parse_error no_value_given(std::format_string<Args...> fmt, Args &&...args) {
      return parse_error(parse_error_type::NO_VALUE_GIVEN, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static parse_error not_required_argument(std::format_string<Args...> fmt, Args &&...args) {
      return parse_error(parse_error_type::NOT_REQUIRED_ARGUMENT, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static parse_error not_argument_key(std::format_string<Args...> fmt, Args &&...args) {
      return parse_error(parse_error_type::NOT_ARGUMENT_KEY, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static parse_error too_many_value_given(std::format_string<Args...> fmt, Args &&...args) {
      return parse_error(parse_error_type::TOO_MANY_VALUE_GIVEN, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static parse_error not_positional(std::format_string<Args...> fmt, Args &&...args) {
      return parse_error(parse_error_type::NOT_POSITIONAL, fmt, std::forward<Args>(args)...);
    }
  };
}