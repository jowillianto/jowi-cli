module;
#include <algorithm>
#include <charconv>
#include <concepts>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
export module moderna.cli:arg;
import :terminal;
import :parse_error;
import :parsed_arg;
import :arg_key;

namespace moderna::cli {
  export struct basic_arg_validator {
    virtual std::optional<std::string> validator_id() const {
      return std::nullopt;
    }
    virtual void help(terminal_nodes &nodes) const {}
    virtual std::expected<void, parse_error> validate(std::optional<std::string_view> value) const {
      return {};
    }
    virtual std::expected<void, parse_error> post_validate(
      std::optional<std::reference_wrapper<const arg_key>> key, parsed_arg &args
    ) const {
      return {};
    }
    virtual ~basic_arg_validator() = default;
  };

  export struct arg_option {
  private:
    std::string __value;
    std::optional<std::string> __help_text;

  public:
    arg_option(std::string v, std::optional<std::string> help_text = std::nullopt) :
      __value{std::move(v)}, __help_text{std::move(help_text)} {}
    std::string_view value() const noexcept {
      return __value;
    }
    std::optional<std::string_view> help_text() const noexcept {
      return __help_text;
    }
    arg_option &help(std::string msg) {
      __help_text.emplace(msg);
      return *this;
    }
    template <class... Args> arg_option &help(std::format_string<Args...> fmt, Args &&...args) {
      __help_text.emplace(std::format(fmt, std::forward<Args>(args)...));
      return *this;
    }
  };

  export struct arg_options_validator : public basic_arg_validator {
  private:
    std::vector<arg_option> __options;

    std::optional<std::reference_wrapper<arg_option>> __get(std::string_view value) {
      auto it = std::ranges::find(__options, value, &arg_option::value);
      if (it == __options.end()) {
        return std::nullopt;
      }
      return std::ref(*it);
    }
    std::optional<std::reference_wrapper<const arg_option>> __get(std::string_view value) const {
      auto it = std::ranges::find(__options, value, &arg_option::value);
      if (it == __options.end()) {
        return std::nullopt;
      }
      return std::cref(*it);
    }

  public:
    constexpr arg_options_validator() : __options{} {}

    constexpr arg_options_validator &add_option(std::string_view value) {
      if (__get(value)) return *this;
      __options.emplace_back(arg_option{std::string{value}});
      return *this;
    }
    constexpr arg_options_validator &add_option(std::string_view value, std::string help_text) {
      auto current_option = __get(value);
      if (current_option) {
        current_option.value().get().help(std::move(help_text));
      } else {
        __options.emplace_back(arg_option{std::string{value}, std::move(help_text)});
      }
      return *this;
    }

    constexpr auto begin() const noexcept {
      return __options.begin();
    }
    constexpr auto end() const noexcept {
      return __options.end();
    }
    size_t size() const noexcept {
      return __options.size();
    }
    bool empty() const noexcept {
      return __options.empty();
    }

    /*
      Argument validation
    */
    std::optional<std::string> validator_id() const override {
      return "arg_options";
    }
    void help(terminal_nodes &nodes) const override {
      if (empty()) {
        return;
      }
      nodes.append_node("Options: ");
      for (const auto &option : __options) {
        auto help_text = option.help_text();
        if (help_text) {
          nodes.append_node("- {}: {}", option.value(), help_text.value());
        } else {
          nodes.append_node("- {}: <no-help>", option.value());
        }
      }
      nodes.new_line();
    }

    std::expected<void, parse_error> validate(
      std::optional<std::string_view> value
    ) const override {
      if (!value) {
        return std::unexpected{parse_error{parse_error_type::NO_VALUE_GIVEN, ""}};
      }
      if (!__get(value.value())) {
        return std::unexpected{parse_error{parse_error_type::INVALID_VALUE, "{}", value.value()}};
      }
      return {};
    }
  };

  export struct arg_count_validator : public basic_arg_validator {
    size_t min_size;
    size_t max_size;

    arg_count_validator(size_t min_size, size_t max_size) :
      min_size{min_size}, max_size{max_size} {}

    /*
      Factory Functions
    */
    static arg_count_validator range(size_t min_size, size_t max_size) {
      return arg_count_validator{min_size, max_size};
    }

    static arg_count_validator at_least(size_t min_size) {
      return arg_count_validator::range(min_size, static_cast<size_t>(-1));
    }
    static arg_count_validator at_most(size_t max_size) {
      return arg_count_validator::range(0, max_size);
    }
    static arg_count_validator equal_to(size_t v) {
      return arg_count_validator::range(v, v);
    }
    static arg_count_validator one() {
      return arg_count_validator::range(1, 1);
    }

    /* Validator Implementation */
    std::optional<std::string> validator_id() const override {
      return "arg_count_validator";
    }
    void help(terminal_nodes &nodes) const override {
      if (min_size == max_size && min_size != 1) {
        nodes.append_node("Arg Count: ={}", min_size);
      } else if (min_size == 0 && max_size == 1) {
        nodes.append_node("Optional");
      } else if (min_size == 1 && max_size == 1) {
        nodes.append_node("Required");
      } else if (min_size == 1 && max_size == -1) {
        nodes.append_node("Arg Count: >= {}", min_size);
      } else {
        nodes.append_node("Arg Count: {} <= n <= {}", min_size, max_size);
      }
    }
    std::expected<void, parse_error> post_validate(
      std::optional<std::reference_wrapper<const arg_key>> key, parsed_arg &args
    ) const override {
      size_t arg_count = key.transform([&](const arg_key &k) { return args.count(k); }).value_or(1);
      if (arg_count > max_size || arg_count < min_size) {
        return std::unexpected{parse_error{
          parse_error_type::TOO_MANY_VALUE_GIVEN,
          "{} not in {} <= x <= {}",
          arg_count,
          min_size,
          max_size
        }};
      }
      return {};
    }
  };

  export struct arg_empty_validator : public basic_arg_validator {
    arg_empty_validator() {}

    std::optional<std::string> validator_id() const override {
      return "arg_empty_validator";
    }
    void help(terminal_nodes &nodes) const override {
      nodes.append_node("allow_empty: false");
    }
    std::expected<void, parse_error> validate(
      std::optional<std::string_view> value
    ) const override {
      if (!value) {
        return std::unexpected{parse_error{parse_error_type::NO_VALUE_GIVEN, ""}};
      }
      return {};
    }
    std::expected<void, parse_error> post_validate(
      std::optional<std::reference_wrapper<const arg_key>> key, parsed_arg &args
    ) const override {
      return {};
    }
  };

  export struct arg : public basic_arg_validator {
  private:
    std::vector<std::unique_ptr<basic_arg_validator>> __vtors;
    std::optional<std::string> __help_text;

  public:
    arg(std::optional<std::string> h = std::nullopt) : __vtors{}, __help_text{std::move(h)} {}

    // Builder Pattern
    arg &help(std::string help_text) {
      __help_text.emplace(help_text);
      return *this;
    }
    template <class... Args> arg &help(std::format_string<Args...> fmt, Args &&...args) {
      __help_text.emplace(std::format(fmt, std::forward<Args>(args)...));
      return *this;
    }
    template <std::derived_from<basic_arg_validator> T>
      requires(std::constructible_from<std::decay_t<T>, T>)
    arg &add_validator(T &&v) {
      std::unique_ptr<basic_arg_validator> new_vtor = std::make_unique<T>(std::move(v));
      auto new_vtor_id = new_vtor->validator_id();
      if (new_vtor_id) {
        auto it =
          std::ranges::find_if(__vtors, [&](const std::unique_ptr<basic_arg_validator> &vtor) {
            auto vtor_id = vtor->validator_id();
            return vtor_id && vtor_id.value() == new_vtor_id.value();
          });
        if (it == __vtors.end()) {
          __vtors.emplace_back(std::move(new_vtor));
        } else {
          *it = std::move(new_vtor);
        }
      } else {
        __vtors.emplace_back(std::move(new_vtor));
      }
      return *this;
    }
    // shortcut build functions
    arg &n_at_least(size_t min_size) {
      return add_validator(arg_count_validator::at_least(min_size));
    }
    arg &n_at_most(size_t max_size) {
      return add_validator(arg_count_validator::at_most(max_size));
    }
    arg &n_equal_to(size_t v) {
      return add_validator(arg_count_validator::equal_to(v));
    }
    arg &n_range(size_t min_size, size_t max_size) {
      return add_validator(arg_count_validator::range(min_size, max_size));
    }
    arg &require_value() {
      return add_validator(arg_empty_validator{});
    }
    arg &required() {
      return require_value().n_equal_to(1);
    }
    arg &optional() {
      return n_range(0, 1);
    }

    // Validator Attributes
    size_t size() const noexcept {
      return __vtors.size();
    }
    bool empty() const noexcept {
      return __vtors.empty();
    }

    // Factory Functions
    static arg positional() {
      return std::move(arg{}.n_equal_to(1));
    }
    static arg flag() {
      return arg{};
    }

    // Arg Validator Implementation
    void help(terminal_nodes &nodes) const override {
      if (__help_text) {
        nodes.append_node(__help_text.value());
      }
      for (const auto &vtor : __vtors) {
        vtor->help(nodes);
        nodes.new_line();
      }
    }

    std::expected<void, parse_error> validate(
      std::optional<std::string_view> value
    ) const override {
      for (const auto &vtor : __vtors) {
        auto res = vtor->validate(value);
        if (!res) {
          return res;
        }
      }
      return {};
    }
    std::expected<void, parse_error> post_validate(
      std::optional<std::reference_wrapper<const arg_key>> key, parsed_arg &args
    ) const override {
      for (const auto &vtor : __vtors) {
        auto res = vtor->post_validate(key, args);
        if (!res) {
          return res;
        }
      }
      return {};
    }
  };

  /*
    Shortcut Parse Functions (to number)
  */
  export template <class num_type>
    requires(std::floating_point<num_type> || std::integral<num_type>)
  std::expected<num_type, parse_error> parse_num(std::string_view v) {
    num_type num;
    auto res = std::from_chars(v.begin(), v.end(), num);
    if (res.ec == std::errc{}) {
      return num;
    } else {
      return std::unexpected{parse_error{parse_error_type::INVALID_VALUE, "{} not a number", v}};
    }
  }

  /*
    Instantiate
  */
  template std::expected<int, parse_error> parse_num(std::string_view);
  template std::expected<long long int, parse_error> parse_num(std::string_view);
  template std::expected<float, parse_error> parse_num(std::string_view);
  template std::expected<double, parse_error> parse_num(std::string_view);
}