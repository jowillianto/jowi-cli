module;
#include <algorithm>
#include <expected>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
export module moderna.cli:argument;
import moderna.generic;
import :argument_value;
import :argument_key;
import :raw_argument;
import :argument_parser_error;
import :app_version;

namespace moderna::cli {
  struct parameter_argument {
    argument_key key;
    bool is_required = false;
    bool require_value = true;
    size_t max_size = 1;
    std::optional<std::string> help_text = std::nullopt;

    parameter_argument &required() noexcept {
      is_required = true;
      return *this;
    }
    parameter_argument &optional() noexcept {
      is_required = false;
      return *this;
    }
    parameter_argument &multiple(size_t max_arg_count = -1) noexcept {
      max_size = max_arg_count;
      return *this;
    }
    parameter_argument &singular() noexcept {
      max_size = 1;
      return *this;
    }
    parameter_argument &as_flag() noexcept {
      require_value = false;
      return *this;
    }
    parameter_argument &help(std::string_view v) noexcept {
      help_text = std::string{v};
      return *this;
    }
  };

  export struct argument_option {
    std::string option;
    std::optional<std::string> help_text = std::nullopt;
  };

  struct position_argument {
    std::optional<std::string> help_text;
    std::optional<std::vector<argument_option>> options;

    template <typename... Args>
      requires(std::is_constructible_v<std::vector<argument_option>, Args...>)
    position_argument &set_options(Args &&...args) noexcept {
      options.emplace(std::vector<argument_option>{std::forward<Args>(args)...});
      return *this;
    }
    template <typename... Args>
      requires(std::is_constructible_v<argument_option, Args> && ...)
    position_argument &set_options(Args &&...args) noexcept {
      options.emplace(std::vector<argument_option>{{argument_option{std::forward<Args>(args)}...}});
      return *this;
    }

    position_argument &help(std::string_view v) noexcept {
      help_text = std::string{v};
      return *this;
    }
  };

  template <class T> struct iterative_argument_parser {
    constexpr bool try_parse(const T &arg, raw_argument_iterator &it) = delete;
    constexpr std::expected<std::reference_wrapper<raw_argument_iterator>, argument_parser_error>
    parse(
      const T &argument,
      argument_values &values,
      raw_argument_iterator &cur,
      const raw_argument_iterator &end
    ) = delete;
    constexpr std::optional<argument_parser_error> finalize(
      const T &argument, const argument_tier &values
    ) = delete;
  };

  template <> struct iterative_argument_parser<parameter_argument> {
    std::string_view arg_key;
    std::optional<std::string_view> arg_value = std::nullopt;

    constexpr bool try_parse(const parameter_argument &arg, const raw_argument_iterator &it) {
      std::string_view cur = *it;
      auto equal_it = std::ranges::find(cur, '=');
      arg_key = std::string_view{cur.begin(), equal_it};
      if (equal_it != cur.end()) {
        arg_value = std::string_view{equal_it + 1, cur.end()};
      }
      argument_match_type t = arg.key.match(arg_key);
      return t != argument_match_type::nothing && t != argument_match_type::merged;
    }

    std::expected<std::reference_wrapper<raw_argument_iterator>, argument_parser_error> parse(
      const parameter_argument &arg,
      argument_values &values,
      raw_argument_iterator &cur,
      const raw_argument_iterator &end
    ) {
      if (arg.require_value && !arg_value) {
        cur++;
        if (cur == end) {
          return std::unexpected{argument_parser_error{
            argument_parser_error_type::no_value_given,
            std::format("argument {} have no value", arg.key.value())
          }};
        }
        arg_value = *cur;
      }
      // Now check if value verifies criteria.
      if (arg.require_value) {
        values.add_keyword_argument(parametric_argument{.key{arg.key}, .value{*arg_value}});
      } else {
        values.add_keyword_argument(parametric_argument{.key{arg.key}, .value{""}});
      }
      auto arg_count = values.count(arg.key);
      cur++;
      return std::ref(cur);
    }

    static std::optional<argument_parser_error> finalize(
      const parameter_argument &arg, const argument_values &values
    ) {
      const auto arg_count = values.count(arg.key);
      if (arg_count == 0 && arg.is_required) {
        return argument_parser_error{
          argument_parser_error_type::no_value_given,
          std::format("No value given for argument {}", arg.key.value())
        };
      } else if (arg_count > arg.max_size) {
        return argument_parser_error{
          argument_parser_error_type::too_many_value_given,
          std::format(
            "argument {} can only accept {} values but received {}",
            arg.key.value(),
            arg.max_size,
            arg_count
          )
        };
      }
      return std::nullopt;
    }
  };

  template <> struct iterative_argument_parser<position_argument> {
    constexpr bool try_parse(const position_argument &arg, const raw_argument_iterator &it) {
      std::string_view cur = *it;
      if (cur.starts_with('-') || cur.starts_with("--")) {
        return false;
      }
      return true;
    }
    std::expected<std::reference_wrapper<raw_argument_iterator>, argument_parser_error> parse(
      const position_argument &arg,
      argument_values &values,
      raw_argument_iterator &cur,
      const raw_argument_iterator &end
    ) {
      if (arg.options) {
        auto option = std::ranges::find_if(
          arg.options.value(),
          [&](const std::string &opt) { return opt == *cur; },
          &argument_option::option
        );
        if (option == arg.options->end()) {
          return std::unexpected{
            argument_parser_error{argument_parser_error_type::invalid_value, ""}
          };
        }
      }
      values.add_positional_argument(*cur);
      cur++;
      return std::ref(cur);
    }
    static std::optional<argument_parser_error> finalize(
      const position_argument &arg, argument_values &values
    ) {
      return std::nullopt;
    }
  };
}