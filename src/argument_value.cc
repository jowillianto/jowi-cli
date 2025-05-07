module;
#include <algorithm>
#include <charconv>
#include <concepts>
#include <expected>
#include <format>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>
export module moderna.cli:argument_value;
import :argument_key;
import :argument_parser_error;
import :app_version;
import moderna.generic;

namespace moderna::cli {
  export template <class T> struct argument_converter {
    static std::expected<T, argument_parser_error> cast(std::string_view v) = delete;
  };
  template <class T>
  concept has_argument_converter = requires(std::string_view v) {
    { argument_converter<T>::cast(v) } -> std::same_as<std::expected<T, argument_parser_error>>;
  };
  export struct argument_value {
    std::string_view raw_value;

    operator std::string_view() const noexcept {
      return raw_value;
    }
    template <has_argument_converter T> std::expected<T, argument_parser_error> cast() const {
      return cast(argument_converter<T>::cast);
    }
    template <std::invocable<std::string_view> F> auto cast(F &&f) const {
      return std::invoke(std::forward<F>(f), raw_value);
    }
  };
  export struct parameter_argument_value {
    argument_key key;
    argument_value value;
  };

  export class position_argument_value {
    argument_value __value;
    std::vector<parameter_argument_value> __params;

  public:
    position_argument_value(argument_value v) : __value{std::move(v)} {}
    /*
      Get the current positional argument value
    */
    const argument_value &value() const noexcept {
      return __value;
    }
    /*
      Add a parameter argument
    */
    constexpr parameter_argument_value &add_argument(argument_key key, argument_value value) {
      return __params.emplace_back(std::move(key), std::move(value));
    }
    /*
      Get the first entry of a certain key
    */
    constexpr std::optional<argument_value> first_of(
      const is_comparable_with<argument_key> auto &key
    ) const noexcept {
      auto it = std::ranges::find_if(__params, [&](const auto &arg) { return arg.key == key; });
      if (it == __params.end()) return std::nullopt;
      return it->value;
    }
    /*
      Get the boolean value for a flag.
    */
    bool get_flag(const is_comparable_with<argument_key> auto &key) const noexcept {
      return contains(key);
    }
    /*
      Check if the current parametric argument contains argument key
    */
    constexpr bool contains(const is_comparable_with<argument_key> auto &key) const noexcept {
      return first_of(key).has_value();
    }
    /*
      Count the number of arguments under the argument_key
    */
    constexpr size_t count(const is_comparable_with<argument_key> auto &key) const noexcept {
      return std::ranges::count_if(__params, [&](const parameter_argument_value &arg) {
        return arg.key == key;
      });
    }
    /*
      Filter for certain keys
    */
    constexpr auto filter(const is_comparable_with<argument_key> auto &key) const noexcept {
      return std::ranges::transform_view{
        std::ranges::filter_view(__params, [&](const auto &arg) { return arg.key == key; }),
        &parameter_argument_value::value
      };
    }
    /*
      Checks if the current argument is empty
    */
    constexpr bool is_empty() const noexcept {
      return __params.empty();
    }
    /*
      Iterators
    */
    constexpr auto begin() const noexcept {
      return __params.begin();
    }
    constexpr auto end() const noexcept {
      return __params.end();
    }
  };
}

namespace cli = moderna::cli;

template <class T>
  requires(std::is_constructible_v<T, std::string_view> || std::is_constructible_v<T, const char *>)
struct cli::argument_converter<T> {
  static std::expected<T, cli::argument_parser_error> cast(std::string_view v) {
    if constexpr (std::is_constructible_v<T, std::string_view>) {
      return T{v};
    } else {
      return T{v.data()};
    }
  }
};
template <class number_type>
  requires(std::integral<number_type> || std::floating_point<number_type>)
struct cli::argument_converter<number_type> {
  static std::expected<number_type, cli::argument_parser_error> cast(std::string_view v) {
    number_type num;
    auto ec = std::from_chars(v.begin(), v.end(), num);
    if (ec.ec == std::errc{}) {
      return num;
    } else {
      return std::unexpected{cli::argument_parser_error{
        cli::argument_parser_error_type::invalid_value, std::format("{} is not a number", v)
      }};
    }
  }
};

template <> struct cli::argument_converter<cli::app_version> {
  static std::expected<app_version, cli::argument_parser_error> cast(std::string_view v) {
    auto version = cli::app_version::from_string(v);
    if (version) {
      return version.value();
    } else {
      return std::unexpected{cli::argument_parser_error{
        cli::argument_parser_error_type::invalid_value,
        std::format("{} is an invalid version string", v)
      }};
    }
  }
};

template <class char_type> struct std::formatter<cli::argument_value, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::argument_value &v, auto &ctx) const {
    return std::format_to(ctx.out(), "{}", v.raw_value);
  }
};