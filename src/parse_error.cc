module;
#include <exception>
#include <format>
#include <string>
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
namespace jowi::cli {
  export struct parse_error : public std::exception {
    parse_error_type __t;
    generic::fixed_string<64> __msg;
    std::string_view __msg_only;

  public:
    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    parse_error(parse_error_type t, std::format_string<Args...> fmt, Args &&...args) noexcept :
      __t{t}, __msg{} {
      __msg.emplace_format("{}: ", t);
      auto msg_begin = __msg.end();
      __msg.emplace_format(fmt, std::forward<Args>(args)...);
      __msg_only = std::string_view{msg_begin, __msg.end()};
    }
    parse_error(parse_error_type t, std::string_view msg) noexcept : parse_error(t, "{}", msg) {}

    const char *what() const noexcept {
      return __msg.c_str();
    }

    parse_error_type err_type() const noexcept {
      return __t;
    }

    template <class... Args>
    parse_error &reformat(
      std::format_string<parse_error_type &, std::string_view &, Args...> fmt, Args &&...args
    ) {
      generic::fixed_string<64> new_msg;
      new_msg.emplace_format(fmt, __t, __msg_only, std::forward<Args>(args)...);
      __msg_only = std::string_view{new_msg.begin(), new_msg.end()};
      __msg = new_msg;
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