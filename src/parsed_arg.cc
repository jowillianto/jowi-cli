module;
#include <optional>
#include <string_view>
#include <vector>
export module moderna.cli:parsed_arg;
import moderna.generic;
import :raw_argument;
import :arg_key;

namespace moderna::cli {
  struct positional_parsed_arg {
    std::string_view value;
    generic::key_vector<arg_key, std::string_view> params;
  };
  export class parsed_arg {
    std::vector<positional_parsed_arg> __values;
    raw_argument_iterator __beg;
    raw_argument_iterator __end;

    parsed_arg(raw_argument_iterator beg, raw_argument_iterator end) :
      __values{}, __beg{beg}, __end{end} {}

  public:
    /*
      Add a parameter argument
    */
    constexpr parsed_arg &add_argument(arg_key key, std::string_view value) {}
    constexpr position_argument_value &add_argument(std::string_view argument) {
      return __values.emplace_back(position_argument_value{argument_value{argument}});
    }
    const std::vector<position_argument_value> &args() const noexcept {
      return __values;
    }
    std::string_view current_positional() const noexcept {
      return __values.back().value();
    }
    auto arg_begin() const noexcept {
      return __values.begin();
    }
    auto arg_end() const noexcept {
      return __values.end();
    }
    /*
      get current argument position iterator
    */
    auto raw_arg_cur() const noexcept {
      return __beg;
    }
    /*
      get the end of the current argument
    */
    auto raw_arg_end() const noexcept {
      return __end;
    }
    /*
      get the beginning of the current argument
    */
    auto raw_arg_beg() const noexcept {
      return __beg.reset();
    }
    bool is_end() const noexcept {
      return raw_arg_end() == raw_arg_cur();
    }
    /*
      Returns the amount of positionals currently contained.
    */
    size_t tier() const noexcept {
      return __values.size();
    }
    /*
      Getting an argument
    */
    constexpr std::optional<argument_value> first_of(
      const std::equality_comparable_with<arg_key> auto &key
    ) const noexcept {
      return __values.back().first_of(key);
    }
    /*
      Checking existence
    */
    constexpr bool contains(const std::equality_comparable_with<arg_key> auto &key) const noexcept {
      return __values.back().contains(key);
    }
    /*
      Counting amount of modules
    */
    constexpr size_t count(const std::equality_comparable_with<arg_key> auto &key) const noexcept {
      return __values.back().count(key);
    }
    /*
      Filter
    */
    constexpr auto filter(const std::equality_comparable_with<arg_key> auto &key) const noexcept {
      return __values.back().filter(key);
    }
    position_argument_value &raw_mut_args() noexcept {
      return __values.back();
    }
    auto &raw_mut_arg_cur() noexcept {
      return __beg;
    }
    auto &raw_mut_arg_end() noexcept {
      return __end;
    }

    /*
      Creates a new parsed_argument that is empty.
    */
    static parsed_arg empty(int argc, const char **argv) noexcept {
      return parsed_arg{
        raw_argument_iterator::begin(argc, argv), raw_argument_iterator::end(argc, argv)
      };
    }
  };
} // namespace moderna::cli