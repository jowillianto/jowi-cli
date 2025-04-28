module;
#include <expected>
#include <format>
#include <functional>
#include <optional>
#include <vector>
export module moderna.cli:argument_parser;
import moderna.generic;
import :argument;
import :raw_argument;
import :argument_value;
import :argument_parser_error;
import :argument_key;
import :parsed_argument;

namespace moderna::cli {
  struct argument_declaration {
    std::vector<parameter_argument> parameters;
    position_argument positional;
  };

  export class argument_parser {
    std::vector<argument_declaration> __tiers;

    // Gets the start of the parsing point
    auto __parse_beg(parsed_argument &prev_result) const {
      if (prev_result.tier() == 0) {
        return __tiers.cbegin();
      }
      return __tiers.cbegin() + prev_result.tier() - 1;
    }
    // Check is the current tier requires parsing.
    bool __parse_tier(
      std::vector<argument_declaration>::const_iterator tier, parsed_argument &prev_result
    ) {
      auto current_tier = std::distance(__tiers.cbegin(), tier) + 1;
      return prev_result.tier() < current_tier;
    }

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
      // Start parsing from the current tier.
      // Meaning:
      for (auto tier = __parse_beg(prev_result); tier != __tiers.cend(); tier += 1) {
        // If the previous tier have already been parsed
        if (__parse_tier(tier, prev_result)) {
          auto parser = iterative_argument_parser<position_argument>{};
          bool use_parser = parser.try_parse(tier->positional, prev_result.raw_mut_arg_cur());
          if (!use_parser) {
            return std::unexpected{argument_parser_error{
              argument_parser_error_type::invalid_value,
              std::format("Expected a positional argument at : {}", *prev_result.raw_mut_arg_cur())
            }};
          }
          auto res = parser.parse(
            tier->positional,
            prev_result.raw_mut_args(),
            prev_result.raw_mut_arg_cur(),
            prev_result.raw_mut_arg_end()
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
            bool use_parser = parser.try_parse(parameter, prev_result.raw_mut_arg_cur());
            have_parseable = use_parser || have_parseable;
            if (use_parser) {
              auto res = parser.parse(
                parameter,
                prev_result.raw_mut_args(),
                prev_result.raw_mut_arg_cur(),
                prev_result.raw_mut_arg_end()
              );
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