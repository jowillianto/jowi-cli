module;
#include <exception>
#include <format>
#include <string_view>
export module jowi.cli:parse_error;
import jowi.generic;

namespace jowi::cli {
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
namespace cli = jowi::cli;
template <class char_type> struct std::formatter<cli::parse_error_type, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::parse_error_type &v, auto &ctx) const {
    switch (v) {
      case cli::parse_error_type::INVALID_VALUE:
        std::format_to(ctx.out(), "invalid value");
        break;
      case cli::parse_error_type::DUPLICATE_ARGUMENT:
        std::format_to(ctx.out(), "duplicate argument");
        break;
      case cli::parse_error_type::NO_VALUE_GIVEN:
        std::format_to(ctx.out(), "no value given");
        break;
      case cli::parse_error_type::NOT_REQUIRED_ARGUMENT:
        std::format_to(ctx.out(), "argument not required");
        break;
      case cli::parse_error_type::NOT_ARGUMENT_KEY:
        std::format_to(ctx.out(), "not an argument key");
        break;
      case cli::parse_error_type::TOO_MANY_VALUE_GIVEN:
        std::format_to(ctx.out(), "too many value given");
        break;
      case cli::parse_error_type::NOT_POSITIONAL:
        std::format_to(ctx.out(), "not positional");
        break;
    };
    return ctx.out();
  }
};

/*
  parse_error
  wraps error and gives it an error type. F
*/
namespace jowi::cli {
  export struct parse_error : public std::exception {
    parse_error_type __t;
    generic::fixed_string<255> __msg;
    size_t __type_msg_size;

  public:
    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    parse_error(parse_error_type t, std::format_string<Args...> fmt, Args &&...args) noexcept :
      __t{t}, __msg{} {
      __msg.emplace_format("{}: ", t);
      __type_msg_size = __msg.size();
      __msg.emplace_format(fmt, std::forward<Args>(args)...);
    }
    parse_error(parse_error_type t, std::string_view msg) noexcept : parse_error(t, "{}", msg) {}

    const char *what() const noexcept {
      return __msg.c_str();
    }

    std::string_view msg_only() const noexcept {
      return std::string_view{__msg.begin() + __type_msg_size, __msg.end()};
    }

    parse_error_type err_type() const noexcept {
      return __t;
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