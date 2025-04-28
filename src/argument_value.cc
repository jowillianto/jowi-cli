module;
#include <algorithm>
#include <concepts>
#include <format>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>
export module moderna.cli:argument_value;
import :argument_key;
import moderna.generic;

namespace moderna::cli {
  export struct parametric_argument {
    argument_key key;
    std::string_view value;
  };

  export class parametric_arguments {
    std::vector<parametric_argument> __values;

  public:
    constexpr parametric_arguments() : __values{} {}

    /*
      Add an argument into the parametric argument list. This is reminiscent of the builder pattern.
    */
    constexpr parametric_arguments &add_argument(parametric_argument arg) {
      __values.emplace_back(std::move(arg));
      return *this;
    }
    /*
      Find a parametric argument given the invocable F to filter.
    */
    constexpr std::optional<std::string_view> first_of(
      const is_comparable_with<argument_key> auto &key
    ) const noexcept {
      auto it = std::ranges::find_if(__values, [&](const parametric_argument &arg) {
        return arg.key == key;
      });
      if (it == __values.end()) return std::nullopt;
      else
        return it->value;
    }
    /*
      Filter based on the invocable F
    */
    template <is_comparable_with<argument_key> T>
    constexpr auto filter(const T &key) const noexcept {
      return std::ranges::filter_view(__values, [&](const parametric_argument &arg) {
        return arg.key == key;
      });
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
      return std::ranges::count_if(__values, [&](const parametric_argument &arg) {
        return arg.key == key;
      });
    }
    /*
      Checks if the current argument is empty
    */
    constexpr bool is_empty() const noexcept {
      return __values.empty();
    }
    /*
      Iterators
    */
    constexpr auto begin() const noexcept {
      return __values.begin();
    }
    constexpr auto end() const noexcept {
      return __values.end();
    }
  };

  export struct argument_tier {
    parametric_arguments parameters;
    std::string_view positional_argument;

    static argument_tier make_empty(std::string_view arg) {
      return argument_tier{parametric_arguments{}, arg};
    }
  };

  export class argument_values {
    std::vector<argument_tier> __arguments;

  public:
    argument_values &add_keyword_argument(parametric_argument arg) {
      __arguments.back().parameters.add_argument(std::move(arg));
      return *this;
    }
    argument_values &add_positional_argument(std::string_view v) {
      __arguments.emplace_back(argument_tier::make_empty(v));
      return *this;
    }
    const parametric_arguments &current_parameters() const {
      return __arguments.back().parameters;
    }
    std::string_view current_positional() const {
      return __arguments.back().positional_argument;
    }
    size_t current_tier() const {
      return __arguments.size() - 1;
    }

    /*
      Reading Arguments
    */
    std::optional<std::reference_wrapper<const argument_tier>> operator[](size_t id
    ) const noexcept {
      if (id < __arguments.size()) {
        return std::cref(__arguments[id]);
      } else {
        return std::nullopt;
      }
    }

    /*
      Reading positional arguments
    */
    size_t count(const is_comparable_with<argument_key> auto &k) const noexcept {
      return current_parameters().count(k);
    }
    bool contains(const is_comparable_with<argument_key> auto &k) const noexcept {
      return current_parameters().contains(k);
    }
    std::optional<std::string_view> first_of(const is_comparable_with<argument_key> auto &k
    ) const noexcept {
      return current_parameters().first_of(k);
    };
    template <typename... Args> auto filter(Args &&...args) const noexcept {
      return current_parameters().filter(std::forward<Args>(args)...);
    }

    constexpr auto begin() const noexcept {
      return __arguments.begin();
    }
    constexpr auto end() const noexcept {
      return __arguments.end();
    }
    size_t size() const noexcept {
      return __arguments.size();
    }
  };
}

namespace cli = moderna::cli;

template <class char_type> struct std::formatter<cli::argument_values, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::argument_values &tiers, auto &ctx) const {
    for (auto tier = tiers.begin(); tier != tiers.end(); tier += 1) {
      std::format_to(
        ctx.out(), "{}| {}\n", std::distance(tiers.begin(), tier), tier->positional_argument
      );
      for (auto i = tier->parameters.begin(); i != tier->parameters.end(); i++) {
        if (i->value.empty()) {
          std::format_to(ctx.out(), "   {}\n", i->key.value());
        } else {
          std::format_to(ctx.out(), "   {}={}\n", i->key.value(), i->value);
        }
      }
    }
    return ctx.out();
  }
};