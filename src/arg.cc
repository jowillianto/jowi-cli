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
import jowi.tui;
import jowi.generic;
import :app_version;
import :parse_error;
import :parsed_arg;
import :arg_key;
namespace tui = jowi::tui;

namespace jowi::cli {
  namespace validator {
    template <class T>
    concept HasId = requires(const std::decay_t<T> t) {
      { t.id() } -> std::same_as<std::optional<std::string>>;
    };
    template <class T>
    concept HasHelp = requires(const std::decay_t<T> t) {
      { t.help() } -> std::same_as<std::optional<tui::DomNode>>;
    };

    template <class T>
    concept HasValidate = requires(const std::decay_t<T> t, std::optional<std::string_view> value) {
      { t.validate(value) } -> std::same_as<std::expected<void, ParseError>>;
    };

    template <class T>
    concept HasPostValidate = requires(
      const std::decay_t<T> t, std::optional<std::reference_wrapper<ArgKey>> key, ParsedArg &args
    ) {
      { t.post_validate(key, args) } -> std::same_as<std::expected<void, ParseError>>;
    };
  }
  export template <class T>
  concept IsArgValidator = validator::HasId<T> || validator::HasHelp<T> ||
    validator::HasValidate<T> || validator::HasPostValidate<T>;

  template <class T>
    requires(IsArgValidator<T> || std::same_as<T, void>)
  struct ArgValidator;
  template <> struct ArgValidator<void> {
    virtual std::optional<std::string> id() const = 0;
    virtual std::optional<tui::DomNode> help() const = 0;
    virtual std::expected<void, ParseError> validate(
      std::optional<std::string_view> value
    ) const = 0;
    virtual std::expected<void, ParseError> post_validate(
      std::optional<std::reference_wrapper<const ArgKey>> key, ParsedArg &args
    ) const = 0;
    virtual ~ArgValidator() = default;
  };

  template <IsArgValidator Validator>
  struct ArgValidator<Validator> : private Validator, ArgValidator<void> {
    using Validator::Validator;
    ArgValidator(Validator validator) : Validator{std::move(validator)} {}
    std::optional<std::string> id() const override {
      if constexpr (validator::HasId<Validator>) {
        return Validator::id();
      } else {
        return std::nullopt;
      }
    }
    std::optional<tui::DomNode> help() const override {
      if constexpr (validator::HasHelp<Validator>) {
        return Validator::help();
      } else {
        return std::nullopt;
      }
    }
    std::expected<void, ParseError> validate(std::optional<std::string_view> value) const override {
      if constexpr (validator::HasValidate<Validator>) {
        return Validator::validate(value);
      } else {
        return {};
      }
    }
    std::expected<void, ParseError> post_validate(
      std::optional<std::reference_wrapper<const ArgKey>> key, ParsedArg &args
    ) const override {
      if constexpr (validator::HasPostValidate<Validator>) {
        return Validator::post_validate(key, args);
      } else {
        return {};
      }
    }
  };

  export struct ArgOption {
  private:
    std::string __value;
    std::optional<std::string> __help_text;

  public:
    ArgOption(std::string v, std::optional<std::string> help_text = std::nullopt) :
      __value{std::move(v)}, __help_text{std::move(help_text)} {}
    std::string_view value() const noexcept {
      return __value;
    }
    std::optional<std::string_view> help_text() const noexcept {
      return __help_text;
    }
    ArgOption &help(std::string msg) {
      __help_text.emplace(msg);
      return *this;
    }
    template <class... Args> ArgOption &help(std::format_string<Args...> fmt, Args &&...args) {
      __help_text.emplace(std::format(fmt, std::forward<Args>(args)...));
      return *this;
    }
  };

  export struct ArgOptionsValidator {
  private:
    std::vector<ArgOption> __options;

    std::optional<std::reference_wrapper<ArgOption>> __get(std::string_view value) {
      auto it = std::ranges::find(__options, value, &ArgOption::value);
      if (it == __options.end()) {
        return std::nullopt;
      }
      return std::ref(*it);
    }
    std::optional<std::reference_wrapper<const ArgOption>> __get(std::string_view value) const {
      auto it = std::ranges::find(__options, value, &ArgOption::value);
      if (it == __options.end()) {
        return std::nullopt;
      }
      return std::cref(*it);
    }

  public:
    constexpr ArgOptionsValidator() : __options{} {}

    constexpr ArgOptionsValidator &add_option(std::string_view value) {
      if (__get(value)) return *this;
      __options.emplace_back(ArgOption{std::string{value}});
      return *this;
    }
    constexpr ArgOptionsValidator &add_option(std::string_view value, std::string help_text) {
      auto current_option = __get(value);
      if (current_option) {
        current_option.value().get().help(std::move(help_text));
      } else {
        __options.emplace_back(ArgOption{std::string{value}, std::move(help_text)});
      }
      return *this;
    }
    constexpr ArgOptionsValidator move() noexcept {
      return std::move(*this);
    }

    constexpr auto begin() const noexcept {
      return __options.begin();
    }
    constexpr auto end() const noexcept {
      return __options.end();
    }
    uint64_t size() const noexcept {
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
    std::optional<tui::DomNode> help() const {
      if (empty()) {
        return std::nullopt;
      }
      auto layout = tui::Layout{};
      layout.append_child(tui::DomNode::paragraph("Options:"));
      auto entries = tui::Layout{}.style(tui::DomStyle{}.indent(2));
      for (const auto &option : __options) {
        auto help_text = option.help_text();
        auto text = help_text ? std::format("- {}: {}", option.value(), help_text.value())
                              : std::format("- {}: <no-help>", option.value());
        entries.append_child(tui::DomNode::paragraph(std::move(text)));
      }
      layout.append_child(tui::DomNode::vstack(std::move(entries)));
      return tui::DomNode::vstack(std::move(layout));
    }

    std::expected<void, ParseError> validate(std::optional<std::string_view> value) const {
      if (!value) {
        return std::unexpected{ParseError{ParseErrorType::NO_VALUE_GIVEN, ""}};
      }
      if (!__get(value.value())) {
        return std::unexpected{
          ParseError{ParseErrorType::INVALID_VALUE, "{} is not a valid option.", value.value()}
        };
      }
      return {};
    }
  };

  export struct ArgCountValidator {
    uint64_t min_size;
    uint64_t max_size;

    ArgCountValidator(uint64_t min_size, uint64_t max_size) :
      min_size{min_size}, max_size{max_size} {}

    /*
      Factory Functions
    */
    static ArgCountValidator range(uint64_t min_size, uint64_t max_size) {
      return ArgCountValidator{min_size, max_size};
    }

    static ArgCountValidator at_least(uint64_t min_size) {
      return ArgCountValidator::range(min_size, static_cast<uint64_t>(-1));
    }
    static ArgCountValidator at_most(uint64_t max_size) {
      return ArgCountValidator::range(0, max_size);
    }
    static ArgCountValidator equal_to(uint64_t v) {
      return ArgCountValidator::range(v, v);
    }
    static ArgCountValidator one() {
      return ArgCountValidator::range(1, 1);
    }

    /* Validator Implementation */
    std::optional<std::string> id() const {
      return "ArgCountValidator";
    }
    std::optional<tui::DomNode> help() const {
      std::string desc;
      if (min_size == max_size && min_size != 1) {
        desc = std::format("Arg Count: ={}", min_size);
      } else if (min_size == 0 && max_size == 1) {
        desc = "Optional";
      } else if (min_size == 1 && max_size == 1) {
        desc = "Required";
      } else if (min_size == 1 && max_size == static_cast<uint64_t>(-1)) {
        desc = std::format("Arg Count: >= {}", min_size);
      } else {
        desc = std::format("Arg Count: {} <= n <= {}", min_size, max_size);
      }
      auto layout = tui::Layout{};
      layout.append_child(tui::DomNode::paragraph(std::move(desc)));
      return tui::DomNode::vstack(std::move(layout));
    }
    std::expected<void, ParseError> post_validate(
      std::optional<std::reference_wrapper<const ArgKey>> key, ParsedArg &args
    ) const {
      uint64_t arg_count =
        key.transform([&](const ArgKey &k) { return args.count(k); }).value_or(1);
      if (arg_count > max_size || arg_count < min_size) {
        return std::unexpected{ParseError{
          ParseErrorType::TOO_MANY_VALUE_GIVEN,
          "{} not in {} <= x <= {}",
          arg_count,
          min_size,
          max_size
        }};
      }
      return {};
    }
  };

  export struct ArgEmptyValidator {
  private:
    bool __allow;

  public:
    ArgEmptyValidator(bool allow) : __allow{allow} {}

    std::optional<std::string> id() const {
      return "ArgEmptyValidator";
    }
    std::optional<tui::DomNode> help() const {
      if (!__allow) {
        return std::nullopt;
      }
      auto layout = tui::Layout{};
      layout.append_child(tui::DomNode::paragraph("Flag"));
      return tui::DomNode::vstack(std::move(layout));
    }
    std::expected<void, ParseError> validate(std::optional<std::string_view> value) const {
      if (!__allow && !value) {
        return std::unexpected{ParseError{ParseErrorType::NO_VALUE_GIVEN, ""}};
      }
      return {};
    }
    std::expected<void, ParseError> post_validate(
      std::optional<std::reference_wrapper<const ArgKey>> key, ParsedArg &args
    ) const {
      return {};
    }
  };

  export struct ArgDefaultValidator {
  private:
    std::string __v;

  public:
    ArgDefaultValidator(std::string v) : __v{std::move(v)} {}
    template <class T>
      requires(std::invocable<decltype(static_cast<std::string (*)(T)>(std::to_string)), T>)
    ArgDefaultValidator(T v) : __v{std::to_string(v)} {}

    std::optional<std::string> id() const {
      return "ArgDefaultValidator";
    }

    std::optional<tui::DomNode> help() const {
      return tui::DomNode::paragraph("Default: {}", __v);
    }

    std::expected<void, ParseError> post_validate(
      std::optional<std::reference_wrapper<const ArgKey>> key, ParsedArg &args
    ) const {
      if (key.has_value() && !args.contains(key.value().get())) {
        args.add_argument(key.value(), __v);
      }
      return {};
    }
  };

  export struct Arg {
  private:
    std::vector<std::unique_ptr<ArgValidator<void>>> __vtors;
    std::optional<std::string> __help_text;

  public:
    Arg(std::optional<std::string> h = std::nullopt) : __vtors{}, __help_text{std::move(h)} {}

    // Builder Pattern
    Arg &help(std::string help_text) {
      __help_text.emplace(help_text);
      return *this;
    }
    template <class... Args> Arg &help(std::format_string<Args...> fmt, Args &&...args) {
      __help_text.emplace(std::format(fmt, std::forward<Args>(args)...));
      return *this;
    }
    template <IsArgValidator Validator> Arg &add_validator(Validator &&v) {
      std::unique_ptr<ArgValidator<void>> new_validator =
        std::make_unique<ArgValidator<Validator>>(std::forward<Validator>(v));
      auto new_validator_id = new_validator->id();
      auto it =
        std::ranges::find_if(__vtors, [&](const std::unique_ptr<ArgValidator<void>> &validator) {
          auto validator_id = validator->id();
          if (validator_id.has_value() && new_validator_id.has_value()) {
            return validator_id.value() == new_validator_id.value();
          }
          return false;
        });
      if (it != __vtors.end()) {
        __vtors.erase(it);
      }
      __vtors.emplace_back(std::move(new_validator));
      return *this;
    }
    template <class... Args>
      requires(IsArgValidator<Args> && ...)
    Arg &add_validators(Args &&...args) {
      (add_validator(std::forward<Args>(args)), ...);
      return *this;
    }
    template <class... Args>
      requires(std::constructible_from<ArgDefaultValidator, Args...>)
    Arg &with_default(Args &&...args) {
      return add_validator(ArgDefaultValidator{std::forward<Args>(args)...});
    }
    // shortcut build functions
    Arg &n_at_least(uint64_t min_size) {
      return add_validator(ArgCountValidator::at_least(min_size));
    }
    Arg &n_at_most(uint64_t max_size) {
      return add_validator(ArgCountValidator::at_most(max_size));
    }
    Arg &n_equal_to(uint64_t v) {
      return add_validator(ArgCountValidator::equal_to(v));
    }
    Arg &n_range(uint64_t min_size, uint64_t max_size) {
      return add_validator(ArgCountValidator::range(min_size, max_size));
    }
    Arg &require_value() {
      return add_validator(ArgEmptyValidator{false});
    }
    Arg &as_flag() {
      return add_validator(ArgEmptyValidator{true});
    }
    Arg &required() {
      return add_validators(ArgEmptyValidator{false}, ArgCountValidator::equal_to(1));
    }
    Arg &optional() {
      return add_validators(ArgCountValidator::range(0, 1));
    }

    // Validator Attributes
    uint64_t size() const noexcept {
      return __vtors.size();
    }
    bool empty() const noexcept {
      return __vtors.empty();
    }
    Arg move() noexcept {
      return std::move(*this);
    }

    // Factory Functions
    static Arg positional() {
      Arg arg{};
      arg.required();
      return arg;
    }
    static Arg flag() {
      return Arg{};
    }

    // Arg Validator Implementation
    std::optional<tui::DomNode> help() const {
      bool has_content = false;
      auto layout = tui::Layout{};
      if (__help_text) {
        layout.append_child(tui::DomNode::paragraph(__help_text.value()));
        has_content = true;
      }
      for (const auto &vtor : __vtors) {
        if (auto node = vtor->help()) {
          layout.append_child(std::move(node.value()));
          has_content = true;
        }
      }
      if (!has_content) {
        return std::nullopt;
      }
      return tui::DomNode::vstack(std::move(layout));
    }

    std::expected<void, ParseError> validate(std::optional<std::string_view> value) const {
      for (const auto &vtor : __vtors) {
        auto res = vtor->validate(value);
        if (!res) {
          return res;
        }
      }
      return {};
    }
    std::expected<void, ParseError> post_validate(
      std::optional<std::reference_wrapper<const ArgKey>> key, ParsedArg &args
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
