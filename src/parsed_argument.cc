module;
#include <optional>
#include <string>
export module moderna.cli:parsed_argument;
import :argument_value;
import :raw_argument;
import :argument_key;

namespace moderna::cli {
  class parsed_argument {
    argument_values __values;
    raw_argument_iterator __beg;
    raw_argument_iterator __end;

    parsed_argument(argument_values values, raw_argument_iterator beg, raw_argument_iterator end) :
      __values{std::move(values)}, __beg{beg}, __end{end} {}

    friend class argument_parser;

  protected:
    /*
      These are for friend class access
    */
    argument_values &raw_mut_args() noexcept {
      return __values;
    }
    auto &raw_mut_arg_cur() noexcept {
      return __beg;
    }
    auto &raw_mut_arg_end() noexcept {
      return __end;
    }

  public:
    /*
      Get internal argument values
    */
    const argument_values &args() const noexcept {
      return __values;
    }
    std::string_view current_positional() const noexcept {
      return __values.current_positional();
    }
    const parametric_arguments &parameters() const noexcept {
      return __values.current_parameters();
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
    constexpr std::optional<std::string_view> first_of(
      const is_comparable_with<argument_key> auto &key
    ) const noexcept {
      return args().first_of(key);
    }
    /*
      Checking existence
    */
    constexpr bool contains(const is_comparable_with<argument_key> auto &key) const noexcept {
      return args().contains(key);
    }
    /*
      Counting amount of modules
    */
    constexpr size_t count(const is_comparable_with<argument_key> auto &key) const noexcept {
      return args().count(key);
    }

    /*
      Creates a new parsed_argument that is empty.
    */
    static parsed_argument empty(int argc, const char **argv) noexcept {
      return parsed_argument{
        argument_values{},
        raw_argument_iterator::begin(argv),
        raw_argument_iterator::end(argc, argv)
      };
    }
  };
}