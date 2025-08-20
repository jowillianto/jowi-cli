module;
#include <cstdint>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>
export module moderna.cli:terminal;

namespace moderna::cli {
  enum struct text_effect {
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

  enum struct basic_color {
    // Standard 8 colors
    black,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
    // Bright variants (8 more colors)
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

  class color {
  private:
    using color_variant = std::variant<basic_color, indexed_color, rgb_color>;
    color_variant _value;

  public:
    // Universal constructor
    template <typename... Args>
    constexpr color(Args &&...args)
      requires std::constructible_from<color_variant, Args...>
      : _value(std::forward<Args>(args)...) {}

    // Factory functions for basic colors
    static constexpr color black() {
      return color{basic_color::black};
    }
    static constexpr color red() {
      return color{basic_color::red};
    }
    static constexpr color green() {
      return color{basic_color::green};
    }
    static constexpr color yellow() {
      return color{basic_color::yellow};
    }
    static constexpr color blue() {
      return color{basic_color::blue};
    }
    static constexpr color magenta() {
      return color{basic_color::magenta};
    }
    static constexpr color cyan() {
      return color{basic_color::cyan};
    }
    static constexpr color white() {
      return color{basic_color::white};
    }

    // Factory functions for bright variants
    static constexpr color bright_black() {
      return color{basic_color::bright_black};
    }
    static constexpr color bright_red() {
      return color{basic_color::bright_red};
    }
    static constexpr color bright_green() {
      return color{basic_color::bright_green};
    }
    static constexpr color bright_yellow() {
      return color{basic_color::bright_yellow};
    }
    static constexpr color bright_blue() {
      return color{basic_color::bright_blue};
    }
    static constexpr color bright_magenta() {
      return color{basic_color::bright_magenta};
    }
    static constexpr color bright_cyan() {
      return color{basic_color::bright_cyan};
    }
    static constexpr color bright_white() {
      return color{basic_color::bright_white};
    }

    // Comparison operators
    constexpr bool operator==(const color &) const = default;
    constexpr auto operator<=>(const color &) const = default;

    // Friend formatter
    friend struct std::formatter<color, char>;
    friend struct std::formatter<color, wchar_t>;
  };

  class text_format {
  private:
    std::optional<color> _bg;
    std::optional<color> _fg;
    std::unordered_set<text_effect> _effects;

  public:
    // Default constructor
    text_format() = default;

    // Builder methods
    text_format &bg(const color &c) {
      _bg = c;
      return *this;
    }

    text_format &fg(const color &c) {
      _fg = c;
      return *this;
    }

    text_format &effect(text_effect e) {
      _effects.insert(e);
      return *this;
    }

    template <std::ranges::range Range>
      requires std::convertible_to<std::ranges::range_value_t<Range>, text_effect>
    text_format &effects(Range &&range) {
      _effects.insert_range(std::forward<Range>(range));
      return *this;
    }

    // Query methods
    bool has_effect(text_effect e) const {
      return _effects.contains(e);
    }

    std::size_t len_effects() const {
      return _effects.size();
    }

    const std::optional<color> &get_bg() const {
      return _bg;
    }

    const std::optional<color> &get_fg() const {
      return _fg;
    }

    // Comparison operators
    bool operator==(const text_format &) const = default;

    // Friend formatter
    friend struct std::formatter<text_format, char>;
    friend struct std::formatter<text_format, wchar_t>;
  };

  struct begin_node {
    text_format format;

    constexpr bool operator==(const begin_node &) const = default;
  };

  struct end_node {
    constexpr bool operator==(const end_node &) const = default;
  };

  struct text_node {
    std::string content;

    bool operator==(const text_node &) const = default;
  };

  struct new_line_node {
    constexpr bool operator==(const new_line_node &) const = default;
  };

  struct indent_node {
    std::size_t level;

    constexpr bool operator==(const indent_node &) const = default;
  };

  class terminal_node {
  private:
    using node_variant = std::variant<begin_node, end_node, text_node, new_line_node, indent_node>;
    node_variant _value;

  public:
    // Universal constructor
    template <typename... Args>
    constexpr terminal_node(Args &&...args)
      requires std::constructible_from<node_variant, Args...>
      : _value(std::forward<Args>(args)...) {}

    // From text_format
    terminal_node(const text_format &format) : _value(begin_node{format}) {}
    terminal_node(text_format &&format) : _value(begin_node{std::move(format)}) {}

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    terminal_node(std::format_string<Args...> fmt, Args &&...args) :
      _value{text_node{std::format(fmt, std::forward<Args>(args)...)}} {}

    // From string-like types
    template <typename T>
      requires std::convertible_to<T, std::string>
    terminal_node(T &&text) : _value(text_node{std::forward<T>(text)}) {}

    // Type checking method
    bool is_newline() const {
      return std::holds_alternative<new_line_node>(_value);
    }

    // Type checking methods for formatting
    bool is_begin() const {
      return std::holds_alternative<begin_node>(_value);
    }

    bool is_end() const {
      return std::holds_alternative<end_node>(_value);
    }

    bool is_text() const {
      return std::holds_alternative<text_node>(_value);
    }

    bool is_indent() const {
      return std::holds_alternative<indent_node>(_value);
    }

    // Getters for formatting (unsafe - should only be called after type checking)
    const begin_node &get_begin() const {
      return std::get<begin_node>(_value);
    }

    const text_node &get_text() const {
      return std::get<text_node>(_value);
    }

    const indent_node &get_indent() const {
      return std::get<indent_node>(_value);
    }

    // Comparison operators
    bool operator==(const terminal_node &) const = default;

    // Friend formatter
    friend struct std::formatter<terminal_node, char>;
  };

  class terminal_nodes {
  private:
    std::vector<terminal_node> _nodes;
    std::size_t _indent;

  public:
    // Default constructor (equivalent to Default trait)
    terminal_nodes() : terminal_nodes(0) {}

    // Constructor with indent level
    explicit terminal_nodes(std::size_t indent_level) :
      _indent(indent_level), _nodes{indent_node{indent_level}} {}

    // Factory function with format
    template <typename NodeType>
      requires std::convertible_to<NodeType, terminal_node>
    static terminal_nodes with_format(text_format fmt, NodeType &&node, std::size_t indent_level) {
      terminal_nodes result(indent_level);
      result.begin_format(std::move(fmt)).append_node(std::forward<NodeType>(node)).end_format();
      return result;
    }

    // Append node with automatic indentation after newlines
    template <typename NodeType>
      requires std::convertible_to<NodeType, terminal_node>
    terminal_nodes &append_node(NodeType &&node) {
      if (_nodes.empty()) {
        _nodes.emplace_back(std::forward<NodeType>(node));
      } else {
        if (_nodes.back().is_newline()) {
          _nodes.emplace_back(indent_node{_indent});
        }
        _nodes.emplace_back(std::forward<NodeType>(node));
      }
      return *this;
    }

    template <typename... Args>
      requires(std::formattable<Args, char> && ...)
    terminal_nodes &append_node(std::format_string<Args...> fmt, Args &&...args) {
      return append_node(terminal_node{fmt, std::forward<Args>(args)...});
    }

    // Append sub-nodes
    terminal_nodes &append_sub_node(terminal_nodes sub_nodes) {
      for (auto &&node : sub_nodes._nodes) {
        append_node(std::move(node));
      }
      return *this;
    }

    // Begin format
    terminal_nodes &begin_format(text_format fmt) {
      return append_node(terminal_node{std::move(fmt)});
    }

    // End format
    terminal_nodes &end_format() {
      _nodes.emplace_back(end_node{});
      return *this;
    }

    // New line
    terminal_nodes &new_line() {
      return append_node(new_line_node{});
    }

    // Query methods
    std::size_t len() const {
      return _nodes.size();
    }

    std::size_t indent() const {
      return _indent;
    }

    // Iterator access
    auto begin() const {
      return _nodes.begin();
    }
    auto end() const {
      return _nodes.end();
    }
    auto begin() {
      return _nodes.begin();
    }
    auto end() {
      return _nodes.end();
    }

    // Comparison operators
    bool operator==(const terminal_nodes &) const = default;

    // Friend formatter
    friend struct std::formatter<terminal_nodes, char>;
  };
}

template <> struct std::formatter<moderna::cli::text_effect, char> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const moderna::cli::text_effect &effect, auto &ctx) const {
    using moderna::cli::text_effect;
    switch (effect) {
      case text_effect::bold:
        return std::format_to(ctx.out(), "\x1b[1m");
      case text_effect::dim:
        return std::format_to(ctx.out(), "\x1b[2m");
      case text_effect::italic:
        return std::format_to(ctx.out(), "\x1b[3m");
      case text_effect::underline:
        return std::format_to(ctx.out(), "\x1b[4m");
      case text_effect::slow_blink:
        return std::format_to(ctx.out(), "\x1b[5m");
      case text_effect::rapid_blink:
        return std::format_to(ctx.out(), "\x1b[6m");
      case text_effect::reverse:
        return std::format_to(ctx.out(), "\x1b[7m");
      case text_effect::strikethrough:
        return std::format_to(ctx.out(), "\x1b[9m");
      case text_effect::double_underline:
        return std::format_to(ctx.out(), "\x1b[21m");
    }
    return ctx.out();
  }
};

template <> struct std::formatter<moderna::cli::color, char> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const moderna::cli::color &c, auto &ctx) const {
    using moderna::cli::basic_color;
    using moderna::cli::indexed_color;
    using moderna::cli::rgb_color;

    return std::visit(
      [&ctx](const auto &variant) {
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
        return ctx.out();
      },
      c._value
    );
  }
};

template <> struct std::formatter<moderna::cli::text_format, char> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const moderna::cli::text_format &fmt, auto &ctx) const {
    auto out = ctx.out();
    if (fmt._bg.has_value()) {
      out = std::format_to(out, "{}", fmt._bg.value());
    }
    if (fmt._fg.has_value()) {
      out = std::format_to(out, "{}", fmt._fg.value());
    }
    for (const auto &effect : fmt._effects) {
      out = std::format_to(out, "{}", effect);
    }
    return out;
  }
};

template <> struct std::formatter<moderna::cli::terminal_node, char> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const moderna::cli::terminal_node &node, auto &ctx) const {
    if (node.is_begin()) {
      return std::format_to(ctx.out(), "{}", node.get_begin().format);
    } else if (node.is_end()) {
      return std::format_to(ctx.out(), "\x1b[0m");
    } else if (node.is_text()) {
      return std::format_to(ctx.out(), "{}", node.get_text().content);
    } else if (node.is_indent()) {
      return std::format_to(ctx.out(), "{:{}}", "", node.get_indent().level);
    } else if (node.is_newline()) {
      return std::format_to(ctx.out(), "\n");
    }
    return ctx.out();
  }
};

template <> struct std::formatter<moderna::cli::terminal_nodes, char> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const moderna::cli::terminal_nodes &nodes, auto &ctx) const {
    auto out = ctx.out();
    for (const auto &node : nodes._nodes) {
      out = std::format_to(out, "{}", node);
    }
    return out;
  }
};