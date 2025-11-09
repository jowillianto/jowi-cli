module;
#include <concepts>
#include <expected>
#include <format>
#include <iterator>
export module jowi.crogger:formatter;
import jowi.tui;
import :error;
import :context;
import :emitter;

namespace tui = jowi::tui;
namespace jowi::crogger {
  export template <class T>
  concept is_formatter = requires(
    const T Formatter, const Context &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
  ) {
    { Formatter.format(ctx, it) } -> std::same_as<std::expected<void, LogError>>;
  };

  export template <typename T>
  concept format_checkable = requires(const T Formatter, const Context &ctx) {
    { Formatter.check(ctx) } -> std::same_as<std::expected<void, LogError>>;
  };

  export template <class T = void> struct Formatter;

  export template <> struct Formatter<void> {
    virtual ~Formatter() = default;

    virtual std::expected<void, LogError> check(const Context &) const = 0;
    virtual std::expected<void, LogError> format(
      const Context &, std::back_insert_iterator<StreamEmitter<void>> &
    ) const = 0;
  };
  export template <is_formatter FormatterType>
  struct Formatter<FormatterType> : private FormatterType, public Formatter<void> {
    using FormatterType::FormatterType; // Inherit constructors

    // Move constructor
    Formatter(FormatterType &&Formatter) : FormatterType(std::move(Formatter)) {}

    std::expected<void, LogError> format(
      const Context &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
    ) const override {
      return FormatterType::format(ctx, it);
    }

    std::expected<void, LogError> check(const Context &ctx) const override {
      if constexpr (format_checkable<FormatterType>) {
        return FormatterType::check(ctx);
      } else {
        return {}; // Return success (void) as default
      }
    }
  };

  export struct ColorfulFormatter {
    tui::TextFormat get_level_color(unsigned int lvl) const noexcept {
      if (lvl < 10) return tui::TextFormat{}.fg(tui::Color::cyan());
      else if (lvl < 20)
        return tui::TextFormat{}.fg(tui::Color::blue());
      else if (lvl < 30)
        return tui::TextFormat{}.fg(tui::Color::green());
      else if (lvl < 40)
        return tui::TextFormat{}.fg(tui::Color::yellow());
      else if (lvl < 50)
        return tui::TextFormat{}.fg(tui::Color::magenta());
      else
        return tui::TextFormat{}.fg(tui::Color::red());
    }
    std::expected<void, LogError> format(
      const Context &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
    ) const {
      std::format_to(
        it,
        "{}[{}]{} {:%FT%TZ} ",
        tui::CliNode::format_begin(get_level_color(ctx.status.level)),
        ctx.status.name,
        tui::CliNode::format_end(),
        ctx.time
      );
      ctx.message.format(it);
      it = '\n';
      return {};
    }
  };

  export struct BwFormatter {
    std::expected<void, LogError> format(
      const Context &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
    ) const {
      std::format_to(it, "[{}] {:%FT%TZ} ", ctx.status.name, ctx.time);
      ctx.message.format(it);
      it = '\n';
      return {};
    }
  };

  export struct EmptyFormatter {
    std::expected<void, LogError> format(
      const Context &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
    ) const {
      return {};
    }
  };

  export struct PlainFormatter {
    std::expected<void, LogError> format(
      const Context &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
    ) const {
      ctx.message.format(it);
      it = '\n';
      return {};
    }
  };

  template struct Formatter<BwFormatter>;
  template struct Formatter<ColorfulFormatter>;
  template struct Formatter<EmptyFormatter>;
};
