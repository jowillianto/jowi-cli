module;
#include <concepts>
#include <expected>
#include <string>
export module moderna.cli:arg_key;
import :argparse_error;

namespace moderna::cli {
  export struct arg_key {
  private:
    std::string __value;
    template <typename... Args>
      requires(std::constructible_from<std::string, Args...>)
    constexpr arg_key(Args &&...args) : __value{std::forward<Args>(args)...} {}

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
    static std::expected<arg_key, parse_error> make(std::string_view v) {
      if (v.starts_with("--") && v.size() > 2) {
        return arg_key{v};
      } else if (v.starts_with("-") && v.size() == 2) {
        return arg_key{v};
      } else {
        return std::unexpected{parse_error{argparse_error_type::NOT_ARGUMENT_KEY, "{}", v}};
      }
    }
    /*
      Comparison Operator Overload
    */
    template <class T>
      requires(!std::same_as<arg_key, T> && std::equality_comparable_with<arg_key, T>)
    friend constexpr bool operator==(const arg_key &l, const T &r) {
      return l.__value == r;
    }
    friend constexpr bool operator==(const arg_key &l, const arg_key &r) {
      return l.__value == r.__value;
    }
  };
}