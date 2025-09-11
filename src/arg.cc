module;
#include <algorithm>
#include <concepts>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
export module jowi.cli:arg;
import jowi.cli.ui;
import jowi.generic;
import :app_version;
import :parse_error;
import :parsed_arg;
import :arg_key;

namespace jowi::cli {
  template <class T>
  concept unique_arg_validator = requires(const std::decay_t<T> t) {
    { t.id() } -> std::same_as<std::optional<std::string>>;
  };

  template <class T>
  concept help_providing_arg_validator =
    requires(const std::decay_t<T> t, ui::cli_nodes &a, ui::cli_nodes &b) {
      { t.help(a, b) } -> std::same_as<void>;
    };

  template <class T>
  concept value_arg_validator =
    requires(const std::decay_t<T> t, std::optional<std::string_view> sv) {
      { t.validate(sv) } -> std::same_as<std::expected<void, parse_error>>;
    };

  template <class T>
  concept post_validate_arg_validator = requires(
    const std::decay_t<T> t,
    std::optional<std::reference_wrapper<const arg_key>> key,
    parsed_arg &pa
  ) {
    { t.post_validate(key, pa) } -> std::same_as<std::expected<void, parse_error>>;
  };

  template <class validator_type>
  concept is_arg_validator =
    unique_arg_validator<validator_type> || help_providing_arg_validator<validator_type> ||
    value_arg_validator<validator_type> || post_validate_arg_validator<validator_type>;

  template <class T>
    requires(is_arg_validator<T> || std::same_as<T, void>)
  struct arg_validator;

  template <> struct arg_validator<void> {
    virtual std::optional<std::string> id() const = 0;
    virtual ui::cli_nodes &help(ui::cli_nodes &nodes) const = 0;
    virtual std::expected<void, parse_error> validate(
      std::optional<std::string_view> value
    ) const = 0;
    virtual std::expected<void, parse_error> post_validate(
      std::optional<std::reference_wrapper<const arg_key>> key, parsed_arg &args
    ) const = 0;
    virtual ~arg_validator() = default;
  };
  template <is_arg_validator validator_type>
  struct arg_validator<validator_type> : private validator_type, arg_validator<void> {
    using validator_type::validator_type;
    arg_validator(validator_type validator) : validator_type{std::move(validator)} {}
    std::optional<std::string> id() const override {
      if constexpr (unique_arg_validator<validator_type>) {
        return validator_type::id();
      } else {
        return std::nullopt;
      }
    }
    ui::cli_nodes &help(ui::cli_nodes &nodes) const override {
      if constexpr (help_providing_arg_validator<validator_type>) {
        return validator_type::help(nodes);
      } else {
        return nodes;
      }
    }
    std::expected<void, parse_error> validate(
      std::optional<std::string_view> value
    ) const override {
      if constexpr (value_arg_validator<validator_type>) {
        return validator_type::validate(value);
      } else {
        return {};
      }
    }
    std::expected<void, parse_error> post_validate(
      std::optional<std::reference_wrapper<const arg_key>> key, parsed_arg &args
    ) const override {
      if constexpr (post_validate_arg_validator<validator_type>) {
        return validator_type::post_validate(key, args);
      } else {
        return {};
      }
    }
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

  export struct arg_options_validator {
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
    constexpr arg_options_validator move() noexcept {
      return std::move(*this);
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
    std::optional<std::string> id() const {
      return "arg_options";
    }
    ui::cli_nodes &help(ui::cli_nodes &nodes) const {
      if (empty()) {
        return nodes;
      }

      nodes.append_nodes(ui::cli_node::text("Options: "), ui::cli_node::new_line());
      for (const auto &option : __options) {
        auto help_text = option.help_text();
        if (help_text) {
          nodes.append_nodes(
            ui::cli_node::text("- {}: {}", option.value(), help_text.value()),
            ui::cli_node::new_line()
          );
        } else {
          nodes.append_nodes(
            ui::cli_node::text("- {}: <no-help>", option.value()), ui::cli_node::new_line()
          );
        }
      }
      return nodes;
    }

    std::expected<void, parse_error> validate(std::optional<std::string_view> value) const {
      if (!value) {
        return std::unexpected{parse_error{parse_error_type::NO_VALUE_GIVEN, ""}};
      }
      if (!__get(value.value())) {
        return std::unexpected{
          parse_error{parse_error_type::INVALID_VALUE, "{} is not a valid option.", value.value()}
        };
      }
      return {};
    }
  };

  export struct arg_count_validator {
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
    std::optional<std::string> id() const {
      return "arg_count_validator";
    }
    ui::cli_nodes &help(ui::cli_nodes &nodes) const {
      if (min_size == max_size && min_size != 1) {
        nodes.append_nodes(
          ui::cli_node::text("Arg Count: ={}", min_size), ui::cli_node::new_line()
        );
      } else if (min_size == 0 && max_size == 1) {
        nodes.append_nodes(ui::cli_node::text("Optional"), ui::cli_node::new_line());
      } else if (min_size == 1 && max_size == 1) {
        nodes.append_nodes(ui::cli_node::text("Required"), ui::cli_node::new_line());
      } else if (min_size == 1 && max_size == -1) {
        nodes.append_nodes(
          ui::cli_node::text("Arg Count: >= {}", min_size), ui::cli_node::new_line()
        );
      } else {
        nodes.append_nodes(
          ui::cli_node::text("Arg Count: {} <= n <= {}", min_size, max_size),
          ui::cli_node::new_line()
        );
      }
      return nodes;
    }
    std::expected<void, parse_error> post_validate(
      std::optional<std::reference_wrapper<const arg_key>> key, parsed_arg &args
    ) const {
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

  export struct arg_empty_validator {
  private:
    bool __allow;

  public:
    arg_empty_validator(bool allow) : __allow{allow} {}

    std::optional<std::string> id() const {
      return "arg_empty_validator";
    }
    ui::cli_nodes &help(ui::cli_nodes &nodes) const {
      if (__allow) {
        nodes.append_nodes(ui::cli_node::text("Flag"), ui::cli_node::new_line());
      }
      return nodes;
    }
    std::expected<void, parse_error> validate(std::optional<std::string_view> value) const {
      if (!__allow && !value) {
        return std::unexpected{parse_error{parse_error_type::NO_VALUE_GIVEN, ""}};
      }
      return {};
    }
    std::expected<void, parse_error> post_validate(
      std::optional<std::reference_wrapper<const arg_key>> key, parsed_arg &args
    ) const {
      return {};
    }
  };

  export struct arg {
  private:
    std::vector<std::unique_ptr<arg_validator<void>>> __vtors;
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
    template <is_arg_validator validator_type> arg &add_validator(validator_type &&v) {
      std::unique_ptr<arg_validator<void>> new_vtor =
        std::make_unique<arg_validator<validator_type>>(std::forward<validator_type>(v));
      auto new_vtor_id = new_vtor->id();
      if (new_vtor_id) {
        auto it =
          std::ranges::find_if(__vtors, [&](const std::unique_ptr<arg_validator<void>> &vtor) {
            auto vtor_id = vtor->id();
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
      return add_validator(arg_empty_validator{false});
    }
    arg &as_flag() {
      return add_validator(arg_empty_validator{true});
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
    arg move() noexcept {
      return std::move(*this);
    }

    // Factory Functions
    static arg positional() {
      arg arg{};
      arg.required();
      return arg;
    }
    static arg flag() {
      arg arg{};
      arg.as_flag().optional();
      return arg;
    }

    // Arg Validator Implementation
    ui::cli_nodes &help(ui::cli_nodes &nodes) const {
      nodes.append_nodes(
        __help_text
          ? ui::cli_nodes{ui::cli_node::text("{}", __help_text.value()), ui::cli_node::new_line()}
          : ui::cli_nodes{}
      );
      for (const auto &vtor : __vtors) {
        vtor->help(nodes);
      }
      return nodes;
    }

    std::expected<void, parse_error> validate(std::optional<std::string_view> value) const {
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
    ) const {
      for (const auto &vtor : __vtors) {
        auto res = vtor->post_validate(key, args);
        if (!res) {
          return res;
        }
      }
      return {};
    }
  };
}