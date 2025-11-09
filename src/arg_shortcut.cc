module;
#include <charconv>
#include <concepts>
#include <expected>
#include <filesystem>
#include <string_view>
export module jowi.cli:arg_shortcut;
import :parse_error;

namespace jowi::cli {
  /*
    Shortcut Parse Functions (to number)
  */
  export template <class T> struct ParseShortcut {
    std::expected<T, ParseError> from(std::string_view v) = delete;
  };

  template <class NumType>
    requires(std::floating_point<NumType> || std::integral<NumType>)
  struct ParseShortcut<NumType> {
    std::expected<NumType, ParseError> from(std::string_view v) {
      NumType num;
      auto res = std::from_chars(v.begin(), v.end(), num);
      if (res.ec == std::errc{}) {
        return num;
      } else {
        return std::unexpected{ParseError{ParseErrorType::INVALID_VALUE, "{} not a number", v}};
      }
    }
  };

  template <std::constructible_from<std::string_view> T> struct ParseShortcut<T> {
    T from(std::string_view v) {
      return T{v};
    }
  };

  template <class T>
  concept has_shortcut = requires(std::string_view v, ParseShortcut<T> parser) {
    { parser.from(v) };
  };

  export template <has_shortcut T>
    requires(std::constructible_from<ParseShortcut<T>>)
  auto parse_arg(std::string_view v) {
    return ParseShortcut<T>{}.from(v);
  }

  /*
    Instantiate
  */
  template auto parse_arg<int>(std::string_view);
  template auto parse_arg<long long int>(std::string_view);
  template auto parse_arg<float>(std::string_view);
  template auto parse_arg<double>(std::string_view);
  template auto parse_arg<std::string>(std::string_view);
  template auto parse_arg<std::filesystem::path>(std::string_view);
}