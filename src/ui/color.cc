module;
#include <concepts>
#include <cstdint>
#include <format>
#include <variant>
export module jowi.tui:color;

namespace jowi::tui {
  enum struct BasicColor {
    black,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
    bright_black,
    bright_red,
    bright_green,
    bright_yellow,
    bright_blue,
    bright_magenta,
    bright_cyan,
    bright_white
  };

  struct IndexedColor {
    std::uint8_t value;

    constexpr bool operator==(const IndexedColor &) const = default;
    constexpr auto operator<=>(const IndexedColor &) const = default;
  };

  struct RgbColor {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;

    constexpr bool operator==(const RgbColor &) const = default;
    constexpr auto operator<=>(const RgbColor &) const = default;
  };

  export class Color {
  private:
    using VariantType = std::variant<BasicColor, IndexedColor, RgbColor>;
    VariantType __value;

    // Private constructor
    template <typename... Args>
      requires std::constructible_from<VariantType, Args...>
    constexpr Color(Args &&...args) noexcept : __value(std::forward<Args>(args)...) {}

  public:
    // Factory functions for basic colors
    static constexpr Color black() noexcept {
      return Color{BasicColor::black};
    }
    static constexpr Color red() noexcept {
      return Color{BasicColor::red};
    }
    static constexpr Color green() noexcept {
      return Color{BasicColor::green};
    }
    static constexpr Color yellow() noexcept {
      return Color{BasicColor::yellow};
    }
    static constexpr Color blue() noexcept {
      return Color{BasicColor::blue};
    }
    static constexpr Color magenta() noexcept {
      return Color{BasicColor::magenta};
    }
    static constexpr Color cyan() noexcept {
      return Color{BasicColor::cyan};
    }
    static constexpr Color white() noexcept {
      return Color{BasicColor::white};
    }

    // Factory functions for bright variants
    static constexpr Color bright_black() noexcept {
      return Color{BasicColor::bright_black};
    }
    static constexpr Color bright_red() noexcept {
      return Color{BasicColor::bright_red};
    }
    static constexpr Color bright_green() noexcept {
      return Color{BasicColor::bright_green};
    }
    static constexpr Color bright_yellow() noexcept {
      return Color{BasicColor::bright_yellow};
    }
    static constexpr Color bright_blue() noexcept {
      return Color{BasicColor::bright_blue};
    }
    static constexpr Color bright_magenta() noexcept {
      return Color{BasicColor::bright_magenta};
    }
    static constexpr Color bright_cyan() noexcept {
      return Color{BasicColor::bright_cyan};
    }
    static constexpr Color bright_white() noexcept {
      return Color{BasicColor::bright_white};
    }

    static constexpr Color indexed(std::uint8_t c) noexcept {
      return Color{IndexedColor{c}};
    }

    static constexpr Color rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
      return Color{RgbColor{r, g, b}};
    }

    // Comparison operators
    constexpr bool operator==(const Color &) const = default;
    constexpr auto operator<=>(const Color &) const = default;

    // Visit function instead of exposing variant
    template <typename F>
      requires(
        std::invocable<F, BasicColor> && std::invocable<F, IndexedColor> &&
        std::invocable<F, RgbColor>
      )
    constexpr auto visit(F &&f) const {
      return std::visit(std::forward<F>(f), __value);
    }
  };
}

template <class CharType> struct std::formatter<jowi::tui::Color, CharType> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const jowi::tui::Color &c, auto &ctx) const {
    using jowi::tui::BasicColor;
    using jowi::tui::IndexedColor;
    using jowi::tui::RgbColor;

    c.visit([&ctx](const auto &variant) {
      using T = std::decay_t<decltype(variant)>;
      if constexpr (std::same_as<T, BasicColor>) {
        switch (variant) {
          case BasicColor::black:
            return std::format_to(ctx.out(), "\x1b[30m");
          case BasicColor::red:
            return std::format_to(ctx.out(), "\x1b[31m");
          case BasicColor::green:
            return std::format_to(ctx.out(), "\x1b[32m");
          case BasicColor::yellow:
            return std::format_to(ctx.out(), "\x1b[33m");
          case BasicColor::blue:
            return std::format_to(ctx.out(), "\x1b[34m");
          case BasicColor::magenta:
            return std::format_to(ctx.out(), "\x1b[35m");
          case BasicColor::cyan:
            return std::format_to(ctx.out(), "\x1b[36m");
          case BasicColor::white:
            return std::format_to(ctx.out(), "\x1b[37m");
          case BasicColor::bright_black:
            return std::format_to(ctx.out(), "\x1b[90m");
          case BasicColor::bright_red:
            return std::format_to(ctx.out(), "\x1b[91m");
          case BasicColor::bright_green:
            return std::format_to(ctx.out(), "\x1b[92m");
          case BasicColor::bright_yellow:
            return std::format_to(ctx.out(), "\x1b[93m");
          case BasicColor::bright_blue:
            return std::format_to(ctx.out(), "\x1b[94m");
          case BasicColor::bright_magenta:
            return std::format_to(ctx.out(), "\x1b[95m");
          case BasicColor::bright_cyan:
            return std::format_to(ctx.out(), "\x1b[96m");
          case BasicColor::bright_white:
            return std::format_to(ctx.out(), "\x1b[97m");
        }
      } else if constexpr (std::same_as<T, IndexedColor>) {
        return std::format_to(ctx.out(), "\x1b[38;5;{}m", variant.value);
      } else if constexpr (std::same_as<T, RgbColor>) {
        return std::format_to(ctx.out(), "\x1b[38;2;{};{};{}m", variant.r, variant.g, variant.b);
      }
    });
    return ctx.out();
  }
};
