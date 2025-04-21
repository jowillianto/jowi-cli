module;
#include <expected>
#include <format>
#include <optional>
#include <print>
#include <string>
#include <vector>
export module moderna.cli:argument_parser;
import moderna.generic;
import :argument;
import :raw_argument;
import :argument_value;
import :argument_parser_error;
import :argument_key;

namespace moderna::cli {
  struct argument_declaration {
    std::vector<parameter_argument> parameters;
    position_argument positional;
  };

  class parsed_argument {
    argument_values __values;
    raw_argument_iterator __beg;
    raw_argument_iterator __end;

    parsed_argument(argument_values values, raw_argument_iterator beg, raw_argument_iterator end) :
      __values{std::move(values)}, __beg{beg}, __end{end} {}

    friend class argument_parser;

  public:
    const argument_values &args() const noexcept {
      return __values;
    }
    std::string_view current_positional() const noexcept {
      return __values.current_positional();
    }
    const parametric_arguments &parameters() const noexcept {
      return __values.current_parameters();
    }
    bool is_end() const noexcept {
      return __beg == __end;
    }
    size_t tier() const noexcept {
      return __values.size();
    }
    static parsed_argument empty(int argc, const char **argv) noexcept {
      return parsed_argument{
        argument_values{},
        raw_argument_iterator::begin(argc, argv),
        raw_argument_iterator::end(argc, argv)
      };
    }
  };

  export class argument_parser {
    std::vector<argument_declaration> __tiers;

  public:
    argument_parser() {
      add_argument();
    }
    parameter_argument &add_argument(argument_key key) {
      return __tiers.back().parameters.emplace_back(parameter_argument{std::move(key)});
    }
    position_argument &add_argument() {
      return __tiers.emplace_back(argument_declaration{{}, position_argument{}}).positional;
    }

    std::expected<parsed_argument, argument_parser_error> parse(int argc, const char **argv) {
      auto empty_parse_result = parsed_argument::empty(argc, argv);
      return parse(empty_parse_result).transform([&](parsed_argument &r) { return r; });
    }
    std::expected<std::reference_wrapper<parsed_argument>, argument_parser_error> parse(
      parsed_argument &prev_result
    ) {
      auto tier = __tiers.cbegin();
      if (prev_result.tier() != 0) {
        tier = tier + prev_result.tier() - 1;
      }
      for (; tier != __tiers.cend(); tier += 1) {
        // If the previous tier have already been parsed
        if (prev_result.tier() == std::distance(__tiers.cbegin(), tier)) {
          auto parser = iterative_argument_parser<position_argument>{};
          bool use_parser = parser.try_parse(tier->positional, prev_result.__beg);
          if (!use_parser) {
            return std::unexpected{argument_parser_error{
              argument_parser_error_type::invalid_value,
              std::format("Expected a positional argument at : {}", *prev_result.__beg)
            }};
          }
          auto res = parser.parse(
            tier->positional, prev_result.__values, prev_result.__beg, prev_result.__end
          );
          if (!res) {
            return std::unexpected{res.error()};
          }
        }
        // Iterate over parameters and parse until there is nothing parseable left.
        while (!prev_result.is_end()) {
          bool have_parseable = false;
          for (const auto &parameter : tier->parameters) {
            auto parser = iterative_argument_parser<parameter_argument>{};
            bool use_parser = parser.try_parse(parameter, prev_result.__beg);
            have_parseable = use_parser || have_parseable;
            if (use_parser) {
              auto res =
                parser.parse(parameter, prev_result.__values, prev_result.__beg, prev_result.__end);
              if (!res) {
                return std::unexpected{res.error()};
              }
            }
            if (prev_result.is_end()) {
              break;
            }
          }
          if (!have_parseable) {
            break;
          }
        }

        // Perform finalization
        for (const auto &parameter : tier->parameters) {
          auto err =
            iterative_argument_parser<parameter_argument>::finalize(parameter, prev_result.args());
          if (err) {
            return std::unexpected{err.value()};
          }
        }

        if (prev_result.is_end()) {
          break;
        }
      }
      return std::ref(prev_result);
    }
    std::expected<parsed_argument, argument_parser_error> parse(parsed_argument &&res) {
      return parse(res).transform([](parsed_argument &res) { return std::move(res); });
    }

    // Iteration and Parsing
    constexpr auto begin() const noexcept {
      return __tiers.begin();
    }
    constexpr auto end() const noexcept {
      return __tiers.end();
    }
    size_t size() const noexcept {
      return __tiers.size();
    }
    std::optional<std::reference_wrapper<const argument_declaration>> operator[](size_t id) {
      if (id < __tiers.size()) {
        return std::cref(__tiers[id]);
      }
      return std::nullopt;
    }
  };
}