module;
#include <format>
#include <optional>
#include <unordered_set>
export module moderna.cli:terminal_format;
import :terminal_color;

namespace moderna::cli {
  export enum struct text_effect {
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

  export class text_format {
  private:
    std::optional<color> __bg;
    std::optional<color> __fg;
    std::unordered_set<text_effect> __effects;

  public:
    text_format() : __bg{std::nullopt}, __fg{std::nullopt}, __effects{} {}
    // Builder methods
    text_format &bg(const color &c) {
      __bg = c;
      return *this;
    }

    text_format &fg(const color &c) {
      __fg = c;
      return *this;
    }

    text_format &effect(text_effect e) {
      __effects.insert(e);
      return *this;
    }

    template <std::ranges::range Range>
      requires std::convertible_to<std::ranges::range_value_t<Range>, text_effect>
    text_format &effects(Range &&range) {
      __effects.insert_range(std::forward<Range>(range));
      return *this;
    }

    // Effects Query method
    bool has_effect(text_effect e) const {
      return __effects.contains(e);
    }
    std::size_t effects_size() const {
      return __effects.size();
    }
    constexpr auto effects_begin() const {
      return __effects.begin();
    }
    constexpr auto effects_end() const {
      return __effects.end();
    }
    const std::optional<color> &get_bg() const {
      return __bg;
    }
    const std::optional<color> &get_fg() const {
      return __fg;
    }

    // Comparison operators
    friend bool operator==(const text_format &l, const text_format &r) {
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

template <class char_type> struct std::formatter<moderna::cli::text_effect, char_type> {
  constexpr auto parse(auto &ctx) const {
    return ctx.begin();
  }
  constexpr auto format(const moderna::cli::text_effect &e, auto &ctx) const {
    switch (e) {
      case moderna::cli::text_effect::bold:
        std::format_to(ctx.out(), "\x1b[1m");
        break;
      case moderna::cli::text_effect::dim:
        std::format_to(ctx.out(), "\x1b[2m");
        break;
      case moderna::cli::text_effect::italic:
        std::format_to(ctx.out(), "\x1b[3m");
        break;
      case moderna::cli::text_effect::underline:
        std::format_to(ctx.out(), "\x1b[4m");
        break;
      case moderna::cli::text_effect::slow_blink:
        std::format_to(ctx.out(), "\x1b[5m");
        break;
      case moderna::cli::text_effect::rapid_blink:
        std::format_to(ctx.out(), "\x1b[6m");
        break;
      case moderna::cli::text_effect::reverse:
        std::format_to(ctx.out(), "\x1b[7m");
        break;
      case moderna::cli::text_effect::strikethrough:
        std::format_to(ctx.out(), "\x1b[9m");
        break;
      case moderna::cli::text_effect::double_underline:
        std::format_to(ctx.out(), "\x1b[21m");
        break;
    }
    return ctx.out();
  }
};

template <class char_type> struct std::formatter<moderna::cli::text_format, char_type> {
  constexpr auto parse(auto &ctx) const {
    return ctx.begin();
  }
  constexpr auto format(const moderna::cli::text_format &fmt, auto &ctx) const {
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