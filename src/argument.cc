module;
#include <algorithm>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
export module moderna.cli:argument;
import moderna.generic;
import :argument_value;
import :argument_key;
import :raw_argument;
import :argparse_error;
import :parsed_argument;

namespace moderna::cli {

  export template <class argument_parser_type>
  concept is_argument_parser = requires(
    argument_parser_type parser,
    parsed_argument &values,
    const parsed_argument &const_values,
    const raw_argument_iterator &const_it,
    raw_argument_iterator &it
  ) {
    { parser.use_parser(const_it) } -> std::same_as<bool>;
    { parser.parse(values, it) } -> std::same_as<std::expected<void, argparse_error>>;
    { parser.finalize(const_values) } -> std::same_as<std::expected<void, argparse_error>>;
  };
  export template <class argument_type>
  concept is_parametric_argument = requires(const argument_type argument) {
    { argument.get_key() } -> std::same_as<const argument_key &>;
    { argument.get_help() } -> std::same_as<std::optional<std::string_view>>;
    { argument.get_parser() } -> is_argument_parser;
  };
  export template <class argument_type>
  concept is_positional_argument = requires(const argument_type argument) {
    { argument.get_help() } -> std::same_as<std::optional<std::string_view>>;
    { argument.get_parser() } -> is_argument_parser;
    std::is_default_constructible_v<argument_type>;
  };

  export struct parameter_argument {
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
    template <class... Args>
    parameter_argument &help(std::format_string<Args...> fmt, Args &&...args) {
      help_text = std::format(fmt, std::forward<Args>(args)...);
      return *this;
    }
    /*
      parameter_argument_parser declaration
    */
    struct parser {
      std::reference_wrapper<const parameter_argument> arg;
      std::string_view arg_key;
      std::optional<std::string_view> arg_value = std::nullopt;
      bool use_parser(const raw_argument_iterator &it) {
        std::string_view cur = *it;
        auto equal_it = std::ranges::find(cur, '=');
        arg_key = std::string_view{cur.begin(), equal_it};
        if (equal_it != cur.end()) {
          arg_value = std::string_view{equal_it + 1, cur.end()};
        }
        argument_match_type t = arg.get().key.match(arg_key);
        return t != argument_match_type::nothing && t != argument_match_type::merged;
      }
      std::expected<void, argparse_error> parse(
        parsed_argument &values, raw_argument_iterator &it
      ) {
        if (arg.get().require_value && !arg_value) {
          it++;
          if (it.is_end()) {
            return std::unexpected{argparse_error{argparse_error_type::NO_VALUE_GIVEN}};
          }
          arg_value = *it;
        }
        // Now check if value verifies criteria.
        if (arg.get().require_value) {
          values.add_argument(arg.get().key, arg_value.value());
        } else {
          values.add_argument(arg.get().key, "");
        }
        auto arg_count = values.count(arg.get().key);
        it++;
        return {};
      }
      std::expected<void, argparse_error> finalize(const parsed_argument &values) {
        const auto arg_count = values.count(arg.get().key);
        if (arg_count == 0 && arg.get().is_required) {
          return std::unexpected{argparse_error{argparse_error_type::NO_VALUE_GIVEN}};
        } else if (arg_count > arg.get().max_size) {
          return std::unexpected{argparse_error{argparse_error_type::TOO_MANY_VALUE_GIVEN}};
        }
        return {};
      }
    };

    /*
      Concept Satisfaction
    */
    const argument_key &get_key() const noexcept {
      return key;
    }
    std::optional<std::string_view> get_help() const noexcept {
      return help_text;
    }
    parser get_parser() const noexcept {
      return parser{std::cref(*this)};
    }
  };
  using parameter_argument_parser = parameter_argument::parser;
  static_assert(is_argument_parser<parameter_argument_parser>);
  static_assert(is_parametric_argument<parameter_argument>);

  export struct basic_position_argument {
    std::optional<std::string> help_text;
    basic_position_argument &help(std::string_view v) noexcept {
      help_text = std::string{v};
      return *this;
    }
    template <class... Args>
    basic_position_argument &help(std::format_string<Args...> fmt, Args &&...args) {
      help_text = std::format(fmt, std::forward<Args>(args)...);
      return *this;
    }

    /*
      Parser specification
    */
    struct parser {
      std::reference_wrapper<const basic_position_argument> arg;
      constexpr bool use_parser(const raw_argument_iterator &it) {
        std::string_view cur = *it;
        if (cur.starts_with('-') || cur.starts_with("--")) {
          return false;
        }
        return true;
      }
      std::expected<void, argparse_error> parse(
        parsed_argument &values, raw_argument_iterator &it
      ) {
        values.add_argument(*it);
        it++;
        return {};
      }
      static std::expected<void, argparse_error> finalize(const parsed_argument &values) {
        return {};
      }
    };

    /*
      Concept Satisfaction
    */
    std::optional<std::string_view> get_help() const noexcept {
      return help_text;
    }
    parser get_parser() const noexcept {
      return parser{std::cref(*this)};
    }
  };
  using basic_position_argument_parser = basic_position_argument::parser;
  static_assert(is_argument_parser<basic_position_argument_parser>);
  static_assert(is_positional_argument<basic_position_argument>);

  export template <class option_data> struct argument_option {
    std::string name;
    option_data data;
  };
  export template <class option_data> struct optioned_position_argument {
    std::optional<std::string> help_text;
    std::vector<argument_option<option_data>> options;

    option_data &add_option(std::string name, option_data data) {
      auto option = argument_option<option_data>{std::move(name), std::move(data)};
      auto it = std::ranges::find(options, name, &argument_option<option_data>::name);
      if (it == options.end()) {
        return options.emplace_back(std::move(option)).data;
      } else {
        *it = std::move(option);
        return it->data;
      }
    }
    std::optional<std::reference_wrapper<const option_data>> get_option(
      std::string_view name
    ) const {
      auto it = std::ranges::find(options, name, &argument_option<option_data>::name);
      if (it == options.end()) {
        return std::nullopt;
      } else {
        return std::cref(it->data);
      }
    }
    size_t option_size() const noexcept {
      return options.size();
    }

    optioned_position_argument &help(std::string help) {
      help_text = std::move(help);
      return *this;
    }

    struct parser {
      std::reference_wrapper<const optioned_position_argument> arg;
      constexpr bool use_parser(const raw_argument_iterator &it) {
        std::string_view cur = *it;
        if (cur.starts_with('-') || cur.starts_with("--")) {
          return false;
        }
        return true;
      }
      std::expected<void, argparse_error> parse(
        parsed_argument &values, raw_argument_iterator &it
      ) {
        std::string_view value = *it;
        auto option = arg.get().get_option(value);
        if (arg.get().option_size() != 0 && !option) {
          return std::unexpected{argparse_error{argparse_error_type::INVALID_VALUE}};
        }
        values.add_argument(*it);
        it++;
        return {};
      }
      static std::expected<void, argparse_error> finalize(const parsed_argument &values) {
        return {};
      }
    };

    std::optional<std::string_view> get_help() const noexcept {
      return help_text;
    }
    parser get_parser() const noexcept {
      return parser{std::cref(*this)};
    }
  };
  template <class option_data>
  using optioned_position_argument_parser = optioned_position_argument<option_data>::parser;

  static_assert(is_positional_argument<optioned_position_argument<int>>);
  static_assert(is_argument_parser<optioned_position_argument_parser<int>>);
};