module;
#include <expected>
#include <format>
#include <functional>
#include <vector>
export module moderna.cli:argument_parser;
import moderna.generic;
import :argument;
import :raw_argument;
import :argument_value;
import :argparse_error;
import :argument_key;
import :parsed_argument;

namespace moderna::cli {
  template <
    is_positional_argument position_argument_type,
    is_parametric_argument parameter_argument_type>
  struct argument_declaration {
    position_argument_type positional;
    std::vector<parameter_argument> parameters;
  };

  export template <
    is_positional_argument position_argument_type,
    is_parametric_argument parameter_argument_type>
  class argument_parser {
    std::vector<argument_declaration<position_argument_type, parameter_argument_type>> __tiers;

    // Gets the start of the parsing point
    auto __parse_beg(parsed_argument &prev_result) const {
      if (prev_result.tier() == 0) {
        return __tiers.cbegin();
      }
      return __tiers.cbegin() + prev_result.tier() - 1;
    }
    // Check is the current tier requires parsing.
    bool __parse_tier(
      std::vector<
        argument_declaration<position_argument_type, parameter_argument_type>>::const_iterator tier,
      parsed_argument &prev_result
    ) {
      auto current_tier = std::distance(__tiers.cbegin(), tier) + 1;
      return prev_result.tier() < current_tier;
    }

  public:
    argument_parser() : __tiers{} {}
    parameter_argument_type &add_argument(argument_key key) {
      return __tiers.back().parameters.emplace_back(parameter_argument_type{std::move(key)});
    }
    position_argument_type &add_argument() {
      auto &tier = __tiers.emplace_back(
        argument_declaration<position_argument_type, parameter_argument_type>{
          position_argument_type{}, {}
        }
      );
      return tier.positional;
    }

    position_argument_type &positional() {
      return __tiers.back().positional;
    }

    std::expected<parsed_argument, argument_error> parse(int argc, const char **argv) {
      auto empty_parse_result = parsed_argument::empty(argc, argv);
      return parse(empty_parse_result).transform([&](parsed_argument &r) { return r; });
    }
    std::expected<std::reference_wrapper<parsed_argument>, argument_error> parse(
      parsed_argument &prev_result
    ) {
      // Start parsing from the current tier.
      // Meaning:
      for (auto tier = __parse_beg(prev_result); tier != __tiers.cend(); tier += 1) {
        position_argument_type pos_arg = tier->positional;
        auto pos_id = std::distance(__tiers.cbegin(), tier);
        if (__parse_tier(tier, prev_result)) {
          is_argument_parser auto parser = pos_arg.get_parser();
          bool use_parser = parser.use_parser(prev_result.raw_arg_cur());
          if (!use_parser) {
            return std::unexpected{argument_error{pos_id, argparse_error_type::INVALID_VALUE}};
          }
          auto res = parser.parse(prev_result, prev_result.raw_mut_arg_cur());
          if (!res) {
            return std::unexpected{argument_error{pos_id, res.error().type()}};
          }
        }
        // Iterate over parameters and parse until there is nothing parseable left.
        while (!prev_result.is_end()) {
          bool have_parseable = false;
          for (const parameter_argument_type &parameter : tier->parameters) {
            is_argument_parser auto parser = parameter.get_parser();
            bool use_parser = parser.use_parser(prev_result.raw_arg_cur());
            have_parseable = use_parser || have_parseable;
            if (use_parser) {
              auto res = parser.parse(prev_result, prev_result.raw_mut_arg_cur());
              if (!res) {
                return std::unexpected{
                  argument_error{parameter.get_key().value(), res.error().type()}
                };
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
        for (const parameter_argument_type &parameter : tier->parameters) {
          auto res = parameter.get_parser().finalize(prev_result);
          if (!res) {
            return std::unexpected{argument_error{parameter.get_key().value(), res.error().type()}};
          }
        }

        if (prev_result.is_end()) {
          break;
        }
      }
      return std::ref(prev_result);
    }
    std::expected<parsed_argument, argument_error> parse(parsed_argument &&res) {
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
  };
}