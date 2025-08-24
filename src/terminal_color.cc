module;
#include <concepts>
#include <cstdint>
#include <format>
#include <variant>
export module moderna.cli:terminal_color;

namespace moderna::cli {
#include <concepts>
#include <cstdint>
#include <variant>

  enum struct basic_color {
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

  struct indexed_color {
    std::uint8_t value;

    constexpr bool operator==(const indexed_color &) const = default;
    constexpr auto operator<=>(const indexed_color &) const = default;
  };

  struct rgb_color {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;

    constexpr bool operator==(const rgb_color &) const = default;
    constexpr auto operator<=>(const rgb_color &) const = default;
  };

  export class color {
  private:
    using variant_type = std::variant<basic_color, indexed_color, rgb_color>;
    variant_type __value;

    // Private constructor
    template <typename... Args>
      requires std::constructible_from<variant_type, Args...>
    constexpr color(Args &&...args) noexcept : __value(std::forward<Args>(args)...) {}

  public:
    // Factory functions for basic colors
    static constexpr color black() noexcept {
      return color{basic_color::black};
    }
    static constexpr color red() noexcept {
      return color{basic_color::red};
    }
    static constexpr color green() noexcept {
      return color{basic_color::green};
    }
    static constexpr color yellow() noexcept {
      return color{basic_color::yellow};
    }
    static constexpr color blue() noexcept {
      return color{basic_color::blue};
    }
    static constexpr color magenta() noexcept {
      return color{basic_color::magenta};
    }
    static constexpr color cyan() noexcept {
      return color{basic_color::cyan};
    }
    static constexpr color white() noexcept {
      return color{basic_color::white};
    }

    // Factory functions for bright variants
    static constexpr color bright_black() noexcept {
      return color{basic_color::bright_black};
    }
    static constexpr color bright_red() noexcept {
      return color{basic_color::bright_red};
    }
    static constexpr color bright_green() noexcept {
      return color{basic_color::bright_green};
    }
    static constexpr color bright_yellow() noexcept {
      return color{basic_color::bright_yellow};
    }
    static constexpr color bright_blue() noexcept {
      return color{basic_color::bright_blue};
    }
    static constexpr color bright_magenta() noexcept {
      return color{basic_color::bright_magenta};
    }
    static constexpr color bright_cyan() noexcept {
      return color{basic_color::bright_cyan};
    }
    static constexpr color bright_white() noexcept {
      return color{basic_color::bright_white};
    }

    static constexpr color indexed(std::uint8_t c) noexcept {
      return color{indexed_color{c}};
    }

    static constexpr color rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
      return color{rgb_color{r, g, b}};
    }

    // Comparison operators
    constexpr bool operator==(const color &) const = default;
    constexpr auto operator<=>(const color &) const = default;

    // Visit function instead of exposing variant
    template <typename F>
      requires(
        std::invocable<F, basic_color> && std::invocable<F, indexed_color> &&
        std::invocable<F, rgb_color>
      )
    constexpr auto visit(F &&f) const {
      return std::visit(std::forward<F>(f), __value);
    }
  };
}

template <class char_type> struct std::formatter<moderna::cli::color, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const moderna::cli::color &c, auto &ctx) const {
    using moderna::cli::basic_color;
    using moderna::cli::indexed_color;
    using moderna::cli::rgb_color;

    c.visit([&ctx](const auto &variant) {
      using T = std::decay_t<decltype(variant)>;
      if constexpr (std::same_as<T, basic_color>) {
        switch (variant) {
          case basic_color::black:
            return std::format_to(ctx.out(), "\x1b[30m");
          case basic_color::red:
            return std::format_to(ctx.out(), "\x1b[31m");
          case basic_color::green:
            return std::format_to(ctx.out(), "\x1b[32m");
          case basic_color::yellow:
            return std::format_to(ctx.out(), "\x1b[33m");
          case basic_color::blue:
            return std::format_to(ctx.out(), "\x1b[34m");
          case basic_color::magenta:
            return std::format_to(ctx.out(), "\x1b[35m");
          case basic_color::cyan:
            return std::format_to(ctx.out(), "\x1b[36m");
          case basic_color::white:
            return std::format_to(ctx.out(), "\x1b[37m");
          case basic_color::bright_black:
            return std::format_to(ctx.out(), "\x1b[90m");
          case basic_color::bright_red:
            return std::format_to(ctx.out(), "\x1b[91m");
          case basic_color::bright_green:
            return std::format_to(ctx.out(), "\x1b[92m");
          case basic_color::bright_yellow:
            return std::format_to(ctx.out(), "\x1b[93m");
          case basic_color::bright_blue:
            return std::format_to(ctx.out(), "\x1b[94m");
          case basic_color::bright_magenta:
            return std::format_to(ctx.out(), "\x1b[95m");
          case basic_color::bright_cyan:
            return std::format_to(ctx.out(), "\x1b[96m");
          case basic_color::bright_white:
            return std::format_to(ctx.out(), "\x1b[97m");
        }
      } else if constexpr (std::same_as<T, indexed_color>) {
        return std::format_to(ctx.out(), "\x1b[38;5;{}m", variant.value);
      } else if constexpr (std::same_as<T, rgb_color>) {
        return std::format_to(ctx.out(), "\x1b[38;2;{};{};{}m", variant.r, variant.g, variant.b);
      }
    });
    return ctx.out();
  }
};