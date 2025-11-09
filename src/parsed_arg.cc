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

  using ParamPair = std::pair<ArgKey, std::string_view>;
  struct PositionalParsedArg {
    std::string_view value;
    std::vector<ParamPair> params;
  };
  export class ParsedArg {
    std::vector<PositionalParsedArg> __values;
    RawArgsIterator __beg;
    RawArgsIterator __end;

    ParsedArg(RawArgsIterator beg, RawArgsIterator end) : __values{}, __beg{beg}, __end{end} {}

  public:
    // Parameter Control
    ParsedArg(RawArgs args) : __values{}, __beg{args.begin()}, __end{args.end()} {}
    ParsedArg &add_argument(ArgKey key, std::string_view value) {
      __values.back().params.emplace_back(ParamPair{key, value});
      return *this;
    }
    PositionalParsedArg &add_argument(std::string_view argument) {
      return __values.emplace_back(PositionalParsedArg{argument});
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
    uint64_t size() const noexcept {
      return __values.size();
    }
    bool empty() const noexcept {
      return __values.empty();
    }

    // Query Functions
    constexpr std::optional<std::string_view> first_of(
      const generic::is_comparable<ArgKey> auto &key
    ) const noexcept {
      auto it = std::ranges::find_if(param_beg(), param_end(), [&](const auto &p) {
        return p.first == key;
      });
      if (it == param_end()) return std::nullopt;
      return it->second;
    }
    constexpr bool contains(const generic::is_comparable<ArgKey> auto &key) const noexcept {
      return first_of(key).has_value();
    }
    constexpr auto filter(generic::is_comparable<ArgKey> auto key) const noexcept {
      return std::ranges::transform_view{
        std::ranges::filter_view{
          std::ranges::subrange{param_beg(), param_end()},
          [key = std::move(key)](const ParamPair &p) { return p.first == key; }
        },
        &ParamPair::second
      };
    }
    constexpr uint64_t count(const generic::is_comparable<ArgKey> auto &key) const noexcept {
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
