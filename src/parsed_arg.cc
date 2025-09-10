module;
#include <algorithm>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>
export module jowi.cli:parsed_arg;
import jowi.generic;
import :raw_args;
import :arg_key;

namespace jowi::cli {

  using param_pair = std::pair<arg_key, std::string_view>;
  struct positional_parsed_arg {
    std::string_view value;
    std::vector<param_pair> params;
  };
  export class parsed_arg {
    std::vector<positional_parsed_arg> __values;
    raw_args_iterator __beg;
    raw_args_iterator __end;

    parsed_arg(raw_args_iterator beg, raw_args_iterator end) : __values{}, __beg{beg}, __end{end} {}

  public:
    // Parameter Control
    parsed_arg(raw_args args) : __values{}, __beg{args.begin()}, __end{args.end()} {}
    parsed_arg &add_argument(arg_key key, std::string_view value) {
      __values.back().params.emplace_back(param_pair{key, value});
      return *this;
    }
    positional_parsed_arg &add_argument(std::string_view argument) {
      return __values.emplace_back(positional_parsed_arg{argument});
    }
    std::string_view arg() const noexcept {
      return __values.back().value;
    }
    auto param_beg() const noexcept {
      return __values.back().params.begin();
    }
    auto param_end() const noexcept {
      return __values.back().params.end();
    }
    auto begin() const noexcept {
      return __values.begin();
    }
    auto end() const noexcept {
      return __values.end();
    }
    auto param_size() const noexcept {
      return __values.size();
    }
    size_t size() const noexcept {
      return __values.size();
    }
    bool empty() const noexcept {
      return __values.empty();
    }

    // Query Functions
    constexpr std::optional<std::string_view> first_of(
      const generic::is_comparable<arg_key> auto &key
    ) const noexcept {
      auto it = std::ranges::find_if(param_beg(), param_end(), [&](const auto &p) {
        return p.first == key;
      });
      if (it == param_end()) return std::nullopt;
      return it->second;
    }
    constexpr bool contains(const generic::is_comparable<arg_key> auto &key) const noexcept {
      return first_of(key).has_value();
    }
    constexpr auto filter(generic::is_comparable<arg_key> auto key) const noexcept {
      return std::ranges::transform_view{
        std::ranges::filter_view{
          std::ranges::subrange{param_beg(), param_end()},
          [key = std::move(key)](const param_pair &p) { return p.first == key; }
        },
        &param_pair::second
      };
    }
    constexpr size_t count(const generic::is_comparable<arg_key> auto &key) const noexcept {
      return std::ranges::count_if(param_beg(), param_end(), [&](const auto &p) {
        return p.first == key;
      });
    }

    // Raw Argument Iterations
    std::optional<std::string_view> raw() const noexcept {
      if (__beg == __end) {
        return std::nullopt;
      }
      return *__beg;
    }

    std::optional<std::string_view> next_raw() noexcept {
      __beg++;
      return raw();
    }
  };
} // namespace jowi::cli