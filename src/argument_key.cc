module;
#include <concepts>
#include <optional>
#include <string>
export module moderna.cli:argument_key;

namespace moderna::cli {
  template <class L, class R>
  concept is_comparable_with = requires(const std::decay_t<L> &l, const std::decay_t<R> &r) {
    { l == r } -> std::same_as<bool>;
  };
  export enum struct argument_match_type { merged, single, nothing };
  export class argument_key {
    std::string __value;

    template <typename... Args>
      requires(std::is_constructible_v<std::string, Args...>)
    constexpr argument_key(Args &&...args) : __value{std::forward<Args>(args)...} {}

  public:
    constexpr std::string_view name() const noexcept {
      if (__value.starts_with("--")) {
        return std::string_view{__value.begin() + 2, __value.end()};
      } else {
        return std::string_view{__value.begin() + 1, __value.end()};
      }
    }
    constexpr std::string_view value() const noexcept {
      return __value;
    }
    constexpr operator std::string_view() const noexcept {
      return value();
    }
    constexpr argument_match_type match(std::string_view v) const noexcept {
      auto arg = value();
      auto arg_name = name();
      // Exact argument match
      if (v == arg) return argument_match_type::single;
      // Multiple Arguments.
      else if (v.starts_with("-") && arg_name.length() == 1 && v.contains(arg_name)) {
        return argument_match_type::merged;
      } else {
        return argument_match_type::nothing;
      }
    }
    constexpr static std::optional<argument_key> make_arg(std::string_view v) {
      if (v.starts_with("--") && v.size() > 2) {
        return argument_key{v};
      } else if (v.starts_with("-") && v.size() > 1) {
        return argument_key{v};
      } else {
        return std::nullopt;
      }
    }

    /*
      Over here, template is used to assert that there will be no implicit conversion occurring.
    */
    template <class T> friend constexpr bool operator==(const argument_key &l, const T &r) {
      if constexpr (std::same_as<T, argument_key>) {
        return l.__value == r.__value;
      } else if constexpr (is_comparable_with<T, std::string>) {
        return l.__value == r;
      } else {
        static_assert(false, "No comparison operation available");
      }
    }
  };
}