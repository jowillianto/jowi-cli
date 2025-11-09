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
  export struct ArgKey {
  private:
    std::string __value;
    template <typename... Args>
      requires(std::constructible_from<std::string, Args...>)
    explicit constexpr ArgKey(Args &&...args) : __value{std::forward<Args>(args)...} {}

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
    static std::expected<std::pair<ArgKey, std::optional<std::string_view>>, ParseError>
    parse_arg(std::string_view v) {
      if (!is_arg_key(v)) {
        return std::unexpected{ParseError{ParseErrorType::NOT_ARGUMENT_KEY, "{}", v}};
      }
      auto eq_it = std::ranges::find(v, '=');
      if (eq_it == v.end()) {
        return std::pair{ArgKey{v}, std::nullopt};
      } else {
        return std::pair{ArgKey{v.begin(), eq_it}, std::string_view{eq_it + 1, v.end()}};
      }
    }
    static std::expected<ArgKey, ParseError> make(std::string_view v) {
      if (!is_arg_key(v)) {
        return std::unexpected{ParseError{ParseErrorType::NOT_ARGUMENT_KEY, "{}", v}};
      }
      return ArgKey{v};
    }
    /*
      Comparison Operator Overload
    */
    template <class T>
      requires(!std::same_as<ArgKey, T> && generic::is_comparable<std::string, T>)
    friend constexpr bool operator==(const ArgKey &l, const T &r) {
      return l.__value == r;
    }
    friend constexpr bool operator==(const ArgKey &l, const ArgKey &r) {
      return l.__value == r.__value;
    }
  };
}

template <class CharType> struct std::formatter<jowi::cli::ArgKey, CharType> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const jowi::cli::ArgKey &k, auto &ctx) const {
    return std::format_to(ctx.out(), "{}", k.value());
  }
};