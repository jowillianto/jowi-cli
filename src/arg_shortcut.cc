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
  export template <class T> struct parse_shortcut {
    std::expected<T, parse_error> from(std::string_view v) = delete;
  };

  template <class num_type>
    requires(std::floating_point<num_type> || std::integral<num_type>)
  struct parse_shortcut<num_type> {
    std::expected<num_type, parse_error> from(std::string_view v) {
      num_type num;
      auto res = std::from_chars(v.begin(), v.end(), num);
      if (res.ec == std::errc{}) {
        return num;
      } else {
        return std::unexpected{parse_error{parse_error_type::INVALID_VALUE, "{} not a number", v}};
      }
    }
  };

  template <std::constructible_from<std::string_view> T> struct parse_shortcut<T> {
    T from(std::string_view v) {
      return T{v};
    }
  };

  template <class T>
  concept has_shortcut = requires(std::string_view v, parse_shortcut<T> parser) {
    { parser.from(v) };
  };

  export template <has_shortcut T>
    requires(std::constructible_from<parse_shortcut<T>>)
  auto parse_arg(std::string_view v) {
    return parse_shortcut<T>{}.from(v);
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