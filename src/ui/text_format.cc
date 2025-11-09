module;
#include <format>
#include <optional>
#include <unordered_set>
export module jowi.tui:text_format;
import :color;

namespace jowi::tui {
  export enum struct TextEffect {
    bold,
    dim,
    italic,
    underline,
    slow_blink,
    rapid_blink,
    reverse,
    strikethrough,
    double_underline
  };

  export class TextFormat {
  private:
    std::optional<Color> __bg;
    std::optional<Color> __fg;
    std::unordered_set<TextEffect> __effects;

  public:
    TextFormat() : __bg{std::nullopt}, __fg{std::nullopt}, __effects{} {}
    // Builder methods
    TextFormat &bg(const Color &c) {
      __bg = c;
      return *this;
    }

    TextFormat &fg(const Color &c) {
      __fg = c;
      return *this;
    }

    TextFormat &effect(TextEffect e) {
      __effects.insert(e);
      return *this;
    }

    template <std::ranges::range Range>
      requires std::convertible_to<std::ranges::range_value_t<Range>, TextEffect>
    TextFormat &effects(Range &&range) {
      __effects.insert_range(std::forward<Range>(range));
      return *this;
    }

    // Effects Query method
    bool has_effect(TextEffect e) const {
      return __effects.contains(e);
    }
    std::uint64_t effects_size() const {
      return __effects.size();
    }
    constexpr auto effects_begin() const {
      return __effects.begin();
    }
    constexpr auto effects_end() const {
      return __effects.end();
    }
    const std::optional<Color> &get_bg() const {
      return __bg;
    }
    const std::optional<Color> &get_fg() const {
      return __fg;
    }

    // Comparison operators
    friend bool operator==(const TextFormat &l, const TextFormat &r) {
      if (l.__bg.has_value() != r.__bg.has_value() || l.__fg.has_value() != l.__fg.has_value() ||
          l.__effects != r.__effects) {
        return false;
      }
      if (l.__bg.has_value() && r.__bg.has_value() && l.__bg.value() != r.__bg.value()) {
        return false;
      }
      if (l.__fg.has_value() && r.__fg.has_value() && l.__fg.value() != r.__fg.value()) {
        return false;
      }
      return true;
    }
  };
}

template <class CharType> struct std::formatter<jowi::tui::TextEffect, CharType> {
  constexpr auto parse(auto &ctx) const {
    return ctx.begin();
  }
  constexpr auto format(const jowi::tui::TextEffect &e, auto &ctx) const {
    switch (e) {
      case jowi::tui::TextEffect::bold:
        std::format_to(ctx.out(), "\x1b[1m");
        break;
      case jowi::tui::TextEffect::dim:
        std::format_to(ctx.out(), "\x1b[2m");
        break;
      case jowi::tui::TextEffect::italic:
        std::format_to(ctx.out(), "\x1b[3m");
        break;
      case jowi::tui::TextEffect::underline:
        std::format_to(ctx.out(), "\x1b[4m");
        break;
      case jowi::tui::TextEffect::slow_blink:
        std::format_to(ctx.out(), "\x1b[5m");
        break;
      case jowi::tui::TextEffect::rapid_blink:
        std::format_to(ctx.out(), "\x1b[6m");
        break;
      case jowi::tui::TextEffect::reverse:
        std::format_to(ctx.out(), "\x1b[7m");
        break;
      case jowi::tui::TextEffect::strikethrough:
        std::format_to(ctx.out(), "\x1b[9m");
        break;
      case jowi::tui::TextEffect::double_underline:
        std::format_to(ctx.out(), "\x1b[21m");
        break;
    }
    return ctx.out();
  }
};

template <class CharType> struct std::formatter<jowi::tui::TextFormat, CharType> {
  constexpr auto parse(auto &ctx) const {
    return ctx.begin();
  }
  constexpr auto format(const jowi::tui::TextFormat &fmt, auto &ctx) const {
    auto bg = fmt.get_bg();
    auto fg = fmt.get_fg();
    if (bg) {
      std::format_to(ctx.out(), "{}", bg.value());
    }
    if (fg) {
      std::format_to(ctx.out(), "{}", fg.value());
    }
    for (const auto &e : std::ranges::subrange{fmt.effects_begin(), fmt.effects_end()}) {
      std::format_to(ctx.out(), "{}", e);
    }
    return ctx.out();
  }
};
