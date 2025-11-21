module;
#include <concepts>
#include <expected>
#include <format>
#include <iterator>
export module jowi.crogger:formatter;
import jowi.tui;
import :error;
import :log_context;
import :emitter;

namespace tui = jowi::tui;
namespace jowi::crogger {
  export template <class T>
  concept IsFormatter = requires(
    const T Formatter, const LogContext &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
  ) {
    { Formatter.format(ctx, it) } -> std::same_as<std::expected<void, LogError>>;
  };

  export template <typename T>
  concept format_checkable = requires(const T Formatter, const LogContext &ctx) {
    { Formatter.check(ctx) } -> std::same_as<std::expected<void, LogError>>;
  };

  export template <class T = void> struct Formatter;

  export template <> struct Formatter<void> {
    virtual ~Formatter() = default;

    virtual std::expected<void, LogError> check(const LogContext &) const = 0;
    virtual std::expected<void, LogError> format(
      const LogContext &, std::back_insert_iterator<StreamEmitter<void>> &
    ) const = 0;
  };
  export template <IsFormatter FormatterType>
  struct Formatter<FormatterType> : private FormatterType, public Formatter<void> {
    using FormatterType::FormatterType; // Inherit constructors

    // Move constructor
    Formatter(FormatterType &&Formatter) : FormatterType(std::move(Formatter)) {}

    std::expected<void, LogError> format(
      const LogContext &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
    ) const override {
      return FormatterType::format(ctx, it);
    }

    std::expected<void, LogError> check(const LogContext &ctx) const override {
      if constexpr (format_checkable<FormatterType>) {
        return FormatterType::check(ctx);
      } else {
        return {}; // Return success (void) as default
      }
    }
  };

  export struct ColorfulFormatter {
    tui::RgbColor get_level_color(unsigned int lvl) const noexcept {
      if (lvl < 10) return tui::RgbColor::cyan();
      if (lvl < 20) return tui::RgbColor::blue();
      if (lvl < 30) return tui::RgbColor::green();
      if (lvl < 40) return tui::RgbColor::yellow();
      if (lvl < 50) return tui::RgbColor::magenta();
      return tui::RgbColor::red();
    }
    template <class T>
    std::expected<void, LogError> format(
      const LogContext &ctx, std::back_insert_iterator<T> &it
    ) const {

      tui::AnsiFormatter<std::decay_t<decltype(it)>>::render(
        tui::Layout{}.append_child(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(get_level_color(ctx.status.level)))
            .append_child(tui::Paragraph("[{}]", ctx.status.name).no_newline())
        ),
        it,
        0,
        std::nullopt
      );
      std::format_to(it, " {:%FT%TZ} ", ctx.time);
      ctx.message.format(it);
      it = '\n';
      return {};
    }
  };

  export struct BwFormatter {
    std::expected<void, LogError> format(
      const LogContext &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
    ) const {
      std::format_to(it, "[{}] {:%FT%TZ} ", ctx.status.name, ctx.time);
      ctx.message.format(it);
      it = '\n';
      return {};
    }
  };

  export struct EmptyFormatter {
    std::expected<void, LogError> format(
      const LogContext &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
    ) const {
      return {};
    }
  };

  export struct PlainFormatter {
    std::expected<void, LogError> format(
      const LogContext &ctx, std::back_insert_iterator<StreamEmitter<void>> &it
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
