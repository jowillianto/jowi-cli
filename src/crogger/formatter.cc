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
  concept IsFormatter = requires(const T Formatter, const LogContext &ctx) {
    { Formatter.format(ctx) } -> std::same_as<std::expected<std::string, LogError>>;
  };

  export template <class T = void> struct Formatter;

  export template <> struct Formatter<void> {
    virtual ~Formatter() = default;

    virtual std::expected<std::string, LogError> format(const LogContext &) const = 0;
  };
  export template <IsFormatter FormatterType>
  struct Formatter<FormatterType> : private FormatterType, public Formatter<void> {
    using FormatterType::FormatterType; // Inherit constructors

    // Move constructor
    Formatter(FormatterType &&Formatter) : FormatterType(std::move(Formatter)) {}

    std::expected<std::string, LogError> format(const LogContext &ctx) const override {
      return FormatterType::format(ctx);
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
    std::expected<std::string, LogError> format(const LogContext &ctx) const {
      std::expected<std::string, LogError> buf{std::string{}};
      std::back_insert_iterator<std::string> it = std::back_inserter(buf.value());
      std::format_to(
        it,
        "{} {:%FT%TZ} ",
        tui::DomNode::vstack(
          tui::Layout{}
            .style(tui::DomStyle{}.fg(get_level_color(ctx.status.level)))
            .append_child(tui::Paragraph("[{}]", ctx.status.name).no_newline())
        ),
        ctx.time
      );
      ctx.message.format(it);
      it = '\n';
      return buf;
    }
  };

  export struct BwFormatter {
    std::expected<std::string, LogError> format(const LogContext &ctx) const {
      std::expected<std::string, LogError> buf{std::string{}};
      auto it = std::back_inserter(buf.value());
      std::format_to(it, "[{}] {:%FT%TZ} ", ctx.status.name, ctx.time);
      ctx.message.format(it);
      it = '\n';
      return buf;
    }
  };

  export struct EmptyFormatter {
    std::expected<std::string, LogError> format(const LogContext &ctx) const {
      return {};
    }
  };

  export struct PlainFormatter {
    std::expected<std::string, LogError> format(const LogContext &ctx) const {
      std::expected<std::string, LogError> buf{std::string{}};
      std::back_insert_iterator<std::string> it = std::back_inserter(buf.value());
      ctx.message.format(it);
      it = '\n';
      return buf;
    }
  };

  template struct Formatter<BwFormatter>;
  template struct Formatter<ColorfulFormatter>;
  template struct Formatter<EmptyFormatter>;
};
