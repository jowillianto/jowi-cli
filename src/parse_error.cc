module;
#include <exception>
#include <format>
#include <string_view>
export module jowi.cli:parse_error;
import jowi.generic;

namespace jowi::cli {
  export enum struct ParseErrorType {
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
  Formatter implementation for ParseErrorType
*/
namespace cli = jowi::cli;
template <class CharType> struct std::formatter<cli::ParseErrorType, CharType> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::ParseErrorType &v, auto &ctx) const {
    switch (v) {
      case cli::ParseErrorType::INVALID_VALUE:
        std::format_to(ctx.out(), "invalid value");
        break;
      case cli::ParseErrorType::DUPLICATE_ARGUMENT:
        std::format_to(ctx.out(), "duplicate argument");
        break;
      case cli::ParseErrorType::NO_VALUE_GIVEN:
        std::format_to(ctx.out(), "no value given");
        break;
      case cli::ParseErrorType::NOT_REQUIRED_ARGUMENT:
        std::format_to(ctx.out(), "argument not required");
        break;
      case cli::ParseErrorType::NOT_ARGUMENT_KEY:
        std::format_to(ctx.out(), "not an argument key");
        break;
      case cli::ParseErrorType::TOO_MANY_VALUE_GIVEN:
        std::format_to(ctx.out(), "too many value given");
        break;
      case cli::ParseErrorType::NOT_POSITIONAL:
        std::format_to(ctx.out(), "not positional");
        break;
    };
    return ctx.out();
  }
};

/*
  ParseError
  wraps error and gives it an error type. F
*/
namespace jowi::cli {
  export struct ParseError : public std::exception {
    ParseErrorType __t;
    generic::FixedString<255> __msg;
    uint64_t __type_msg_size;

  public:
    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    ParseError(ParseErrorType t, std::format_string<Args...> fmt, Args &&...args) noexcept :
      __t{t}, __msg{} {
      __msg.emplace_format("{}: ", t);
      __type_msg_size = __msg.size();
      __msg.emplace_format(fmt, std::forward<Args>(args)...);
    }
    ParseError(ParseErrorType t, std::string_view msg) noexcept : ParseError(t, "{}", msg) {}

    const char *what() const noexcept {
      return __msg.c_str();
    }

    std::string_view msg_only() const noexcept {
      return std::string_view{__msg.begin() + __type_msg_size, __msg.end()};
    }

    ParseErrorType err_type() const noexcept {
      return __t;
    }

    // Shortcut Factory Functions
    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static ParseError invalid_value(std::format_string<Args...> fmt, Args &&...args) {
      return ParseError(ParseErrorType::INVALID_VALUE, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static ParseError duplicate_argument(std::format_string<Args...> fmt, Args &&...args) {
      return ParseError(ParseErrorType::DUPLICATE_ARGUMENT, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static ParseError no_value_given(std::format_string<Args...> fmt, Args &&...args) {
      return ParseError(ParseErrorType::NO_VALUE_GIVEN, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static ParseError not_required_argument(std::format_string<Args...> fmt, Args &&...args) {
      return ParseError(ParseErrorType::NOT_REQUIRED_ARGUMENT, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static ParseError not_argument_key(std::format_string<Args...> fmt, Args &&...args) {
      return ParseError(ParseErrorType::NOT_ARGUMENT_KEY, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static ParseError too_many_value_given(std::format_string<Args...> fmt, Args &&...args) {
      return ParseError(ParseErrorType::TOO_MANY_VALUE_GIVEN, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static ParseError not_positional(std::format_string<Args...> fmt, Args &&...args) {
      return ParseError(ParseErrorType::NOT_POSITIONAL, fmt, std::forward<Args>(args)...);
    }
  };
}
