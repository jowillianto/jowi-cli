module;
#include <algorithm>
#include <concepts>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <string_view>
export module jowi.cli:arg_key;
import jowi.generic;
import :parse_error;

namespace jowi::cli {
  export struct arg_key {
  private:
    std::string __value;
    template <typename... Args>
      requires(std::constructible_from<std::string, Args...>)
    explicit constexpr arg_key(Args &&...args) : __value{std::forward<Args>(args)...} {}

  public:
    /*
      Key Access
    */
    std::string_view value() const noexcept {
      return __value;
    }
    /*
      Factory Functions
    */
    static bool is_arg_key(std::string_view v) {
      return (v.starts_with("--") && v.size() > 2) || (v.starts_with("-") && v.size() == 2);
    }
    static std::expected<std::pair<arg_key, std::optional<std::string_view>>, parse_error>
    parse_arg(std::string_view v) {
      if (!is_arg_key(v)) {
        return std::unexpected{parse_error{parse_error_type::NOT_ARGUMENT_KEY, "{}", v}};
      }
      auto eq_it = std::ranges::find(v, '=');
      if (eq_it == v.end()) {
        return std::pair{arg_key{v}, std::nullopt};
      } else {
        return std::pair{arg_key{v.begin(), eq_it}, std::string_view{eq_it + 1, v.end()}};
      }
    }
    static std::expected<arg_key, parse_error> make(std::string_view v) {
      if (!is_arg_key(v)) {
        return std::unexpected{parse_error{parse_error_type::NOT_ARGUMENT_KEY, "{}", v}};
      }
      return arg_key{v};
    }
    /*
      Comparison Operator Overload
    */
    template <class T>
      requires(!std::same_as<arg_key, T> && generic::is_comparable<std::string, T>)
    friend constexpr bool operator==(const arg_key &l, const T &r) {
      return l.__value == r;
    }
    friend constexpr bool operator==(const arg_key &l, const arg_key &r) {
      return l.__value == r.__value;
    }
  };
}

template <class char_type> struct std::formatter<jowi::cli::arg_key, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const jowi::cli::arg_key &k, auto &ctx) const {
    return std::format_to(ctx.out(), "{}", k.value());
  }
};