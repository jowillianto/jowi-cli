module;
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <expected>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
export module jowi.tui;
import jowi.generic;

namespace jowi::tui {

  export struct RgbColor {
    std::uint8_t r{0};
    std::uint8_t g{0};
    std::uint8_t b{0};

    constexpr RgbColor() = default;
    constexpr RgbColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue) :
      r{red}, g{green}, b{blue} {}

    constexpr bool operator==(const RgbColor &) const = default;
    constexpr auto operator<=>(const RgbColor &) const = default;

    static constexpr RgbColor black() {
      return {0, 0, 0};
    }
    static constexpr RgbColor red() {
      return {205, 0, 0};
    }
    static constexpr RgbColor green() {
      return {0, 205, 0};
    }
    static constexpr RgbColor yellow() {
      return {205, 205, 0};
    }
    static constexpr RgbColor blue() {
      return {0, 0, 205};
    }
    static constexpr RgbColor magenta() {
      return {205, 0, 205};
    }
    static constexpr RgbColor cyan() {
      return {0, 205, 205};
    }
    static constexpr RgbColor white() {
      return {229, 229, 229};
    }
    static constexpr RgbColor bright_black() {
      return {127, 127, 127};
    }
    static constexpr RgbColor bright_red() {
      return {255, 0, 0};
    }
    static constexpr RgbColor bright_green() {
      return {0, 255, 0};
    }
    static constexpr RgbColor bright_yellow() {
      return {255, 255, 0};
    }
    static constexpr RgbColor bright_blue() {
      return {92, 92, 255};
    }
    static constexpr RgbColor bright_magenta() {
      return {255, 0, 255};
    }
    static constexpr RgbColor bright_cyan() {
      return {0, 255, 255};
    }
    static constexpr RgbColor bright_white() {
      return {255, 255, 255};
    }
  };
}

namespace std {
  template <> struct hash<jowi::tui::RgbColor> {
    std::size_t operator()(const jowi::tui::RgbColor &color) const noexcept {
      return (static_cast<std::size_t>(color.r) << 16) ^ (static_cast<std::size_t>(color.g) << 8) ^
        static_cast<std::size_t>(color.b);
    }
  };
}

namespace jowi::tui {

  export enum class TextEffect {
    BOLD,
    DIM,
    ITALIC,
    UNDERLINE,
    SLOW_BLINK,
    RAPID_BLINK,
    REVERSE,
    STRIKETHROUGH,
    DOUBLE_UNDERLINE
  };

  export class Paragraph {
  public:
    std::string text;
    bool new_line = true;

    template <class... Args>
      requires std::constructible_from<std::string, Args...>
    Paragraph(Args &&...args) : text(std::forward<Args>(args)...) {}

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    Paragraph(std::format_string<Args...> fmt, Args &&...args) :
      text{std::format(fmt, std::forward<Args>(args)...)} {}

    Paragraph &no_newline() {
      new_line = false;
      return *this;
    }
  };

  export class DomStyle {
  private:
    std::uint32_t __indentation{0};
    std::optional<std::unordered_set<TextEffect>> __effects;
    std::optional<RgbColor> __bg;
    std::optional<RgbColor> __fg;

  public:
    DomStyle &indent(std::uint32_t value) {
      __indentation = value;
      return *this;
    }

    DomStyle &effect(TextEffect effect) {
      if (!__effects) {
        __effects.emplace();
      }
      __effects->insert(effect);
      return *this;
    }

    template <std::ranges::input_range Range>
      requires std::convertible_to<std::ranges::range_value_t<Range>, TextEffect>
    DomStyle &effects(Range &&range) {
      for (auto &&effect : range) {
        this->effect(effect);
      }
      return *this;
    }

    DomStyle &bg(RgbColor color) {
      __bg = color;
      return *this;
    }

    DomStyle &fg(RgbColor color) {
      __fg = color;
      return *this;
    }

    std::uint32_t indentation() const noexcept {
      return __indentation;
    }
    const std::optional<std::unordered_set<TextEffect>> &effects() const noexcept {
      return __effects;
    }
    const std::optional<RgbColor> &bg_color() const noexcept {
      return __bg;
    }
    const std::optional<RgbColor> &fg_color() const noexcept {
      return __fg;
    }
  };

  export class DomNode {
  public:
    struct Layout {
    private:
      std::vector<DomNode> __children;
      DomStyle __style{};

    public:
      Layout() = default;

      Layout &style(DomStyle style) {
        __style = std::move(style);
        return *this;
      }
      template <class Node>
        requires std::convertible_to<Node, DomNode>
      Layout &append_child(Node &&child) {
        __children.emplace_back(std::forward<Node>(child));
        return *this;
      }
      template <std::ranges::input_range Range>
        requires std::
          convertible_to<std::remove_cvref_t<std::ranges::range_reference_t<Range>>, DomNode>
        Layout &append_children(Range &&children) {
        for (auto &&child : children) {
          append_child(std::forward<decltype(child)>(child));
        }
        return *this;
      }
      std::size_t size() const noexcept {
        return __children.size();
      }
      bool empty() const noexcept {
        return __children.empty();
      }
      const DomStyle &style() const noexcept {
        return __style;
      }
      auto begin() const noexcept {
        return __children.begin();
      }
      auto end() const noexcept {
        return __children.end();
      }
    };

  private:
    using VariantType = generic::Variant<Paragraph, Layout>;
    VariantType __value;

  public:
    template <class... Args>
      requires std::constructible_from<VariantType, Args...>
    DomNode(Args &&...args) : __value(std::forward<Args>(args)...) {}

    template <class T>
      requires std::constructible_from<Paragraph, T>
    static DomNode paragraph(T &&value) {
      return DomNode{Paragraph{std::forward<T>(value)}};
    }
    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static DomNode paragraph(std::format_string<Args...> fmt, Args &&...args) {
      return DomNode{Paragraph{std::format(fmt, std::forward<Args>(args)...)}};
    }
    static DomNode vstack(Layout layout) {
      return DomNode{std::move(layout)};
    }

    template <class T> bool is() const noexcept {
      return __value.template is<T>();
    }
    template <class T> auto as() noexcept {
      return __value.template as<T>();
    }
    template <class T> auto as() const noexcept {
      return __value.template as<T>();
    }

    template <class... Functions> decltype(auto) visit(Functions &&...functions) & {
      return __value.visit(std::forward<Functions>(functions)...);
    }
    template <class... Functions> decltype(auto) visit(Functions &&...functions) const & {
      return __value.visit(std::forward<Functions>(functions)...);
    }
    template <class... Functions> decltype(auto) visit(Functions &&...functions) && {
      return std::move(__value).visit(std::forward<Functions>(functions)...);
    }
  };

  export using Layout = DomNode::Layout;

  export class RenderError : public std::exception {
  private:
    generic::FixedString<64> __msg;

  public:
    explicit RenderError(generic::FixedString<64> msg) noexcept : __msg{std::move(msg)} {}
    explicit RenderError(std::string_view msg) noexcept : __msg{msg} {}

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static RenderError format(std::format_string<Args...> fmt, Args &&...args) {
      generic::FixedString<64> buf;
      buf.emplace_format(fmt, std::forward<Args>(args)...);
      return RenderError{buf};
    }

    const char *what() const noexcept override {
      return __msg.c_str();
    }
  };

  using RenderResult = std::expected<void, RenderError>;

  export template <class Renderer>
  concept IsDomRenderer = requires(Renderer renderer, const DomNode &node) {
    { renderer.render(node) } -> std::same_as<RenderResult>;
    { renderer.clear() } -> std::same_as<RenderResult>;
  };

  export struct FileCloser {
    bool is_closer{true};
    void operator()(std::FILE *file) const noexcept {
      if (file && is_closer) {
        std::fclose(file);
      }
    }
  };
  export using FileHandle = std::unique_ptr<std::FILE, FileCloser>;

  export template <class Iterator> struct AnsiFormatter {
  private:
    using ColorMap = std::unordered_map<RgbColor, std::pair<std::uint8_t, std::uint8_t>>;
    using EffectMap = std::unordered_map<TextEffect, std::uint8_t>;

    inline static const ColorMap COLOR_MAP{
      {RgbColor::black(), {30, 40}},
      {RgbColor::red(), {31, 41}},
      {RgbColor::green(), {32, 42}},
      {RgbColor::yellow(), {33, 43}},
      {RgbColor::blue(), {34, 44}},
      {RgbColor::magenta(), {35, 45}},
      {RgbColor::cyan(), {36, 46}},
      {RgbColor::white(), {37, 47}},
      {RgbColor::bright_black(), {90, 100}},
      {RgbColor::bright_red(), {91, 101}},
      {RgbColor::bright_green(), {92, 102}},
      {RgbColor::bright_yellow(), {93, 103}},
      {RgbColor::bright_blue(), {94, 104}},
      {RgbColor::bright_magenta(), {95, 105}},
      {RgbColor::bright_cyan(), {96, 106}},
      {RgbColor::bright_white(), {97, 107}}
    };

    inline static const EffectMap EFFECT_MAP{
      {TextEffect::BOLD, 1},
      {TextEffect::DIM, 2},
      {TextEffect::ITALIC, 3},
      {TextEffect::UNDERLINE, 4},
      {TextEffect::SLOW_BLINK, 5},
      {TextEffect::RAPID_BLINK, 6},
      {TextEffect::REVERSE, 7},
      {TextEffect::STRIKETHROUGH, 8},
      {TextEffect::DOUBLE_UNDERLINE, 9}
    };

  public:
    static void render(
      const DomNode &node,
      Iterator &it,
      std::uint32_t indent,
      std::optional<std::reference_wrapper<const DomStyle>> prev_style
    ) {
      node.visit(
        [&](const Paragraph &p) { render(p, it, indent, prev_style); },
        [&](const Layout &l) { render(l, it, indent, prev_style); }
      );
    }

    static void render(
      const Paragraph &p,
      Iterator &it,
      std::uint32_t indent,
      std::optional<std::reference_wrapper<const DomStyle>> prev_style
    ) {
        while (indent) {
            std::format_to(it, " ");
            indent -= 1;
        }
      if (p.new_line) {
        std::format_to(it, "{}\n", p.text);
      } else {
        std::format_to(it, "{}", p.text);
      }
    }

    static void reset_format(Iterator &it) {
      std::format_to(it, "\x1b[0m");
    }

    static void render(
      const Layout &layout,
      Iterator &it,
      std::uint32_t indent,
      std::optional<std::reference_wrapper<const DomStyle>> prev_style
    ) {
      bool is_applied = apply_style(it, layout.style());
      for (const auto &c : layout) {
        render(c, it, indent + layout.style().indentation(), std::ref(layout.style()));
      }
      if (is_applied) reset_format(it);
      if (prev_style) apply_style(it, prev_style.value());
    }

    static bool apply_style(Iterator &it, const DomStyle &style) {
      std::vector<uint8_t> codes;
      if (const auto &effects = style.effects()) {
        for (const auto &effect : *effects) {
          if (auto it = EFFECT_MAP.find(effect); it != EFFECT_MAP.end()) {
            codes.emplace_back(it->second);
          }
        }
      }
      if (const auto &bg = style.bg_color()) {
        if (auto it = COLOR_MAP.find(*bg); it != COLOR_MAP.end()) {
          codes.emplace_back(it->second.second);
        }
      }
      if (const auto &fg = style.fg_color()) {
        if (auto it = COLOR_MAP.find(*fg); it != COLOR_MAP.end()) {
          codes.emplace_back(it->second.first);
        }
      }
      // Now apply styles
      if (!codes.empty()) {
        reset_format(it);
        std::format_to(it, "\x1b[");
        for (auto i = codes.begin(); i != codes.end(); i += 1) {
          if (i + 1 == codes.end()) {
            std::format_to(it, "{}m", std::to_string(*i));
          } else {
            std::format_to(it, "{};", std::to_string(*i));
          }
        }
        return true;
      }
      return false;
    }
  };

  export class StringRenderer {
  private:
    std::optional<std::string> __buffer;

  public:
    RenderResult render(const DomNode &dom) {
      __buffer.emplace("");
      auto it = std::back_inserter(__buffer.value());
      AnsiFormatter<std::back_insert_iterator<std::string>>::render(dom, it, 0, std::nullopt);
      return {};
    }

    RenderResult clear() {
      __buffer.reset();
      return {};
    }

    std::string read() {
      return std::move(__buffer).value_or("");
    }
  };

  export class AnsiTerminal {
  private:
    FileHandle __file;

  public:
    explicit AnsiTerminal(FileHandle file) : __file{std::move(file)} {}

    static AnsiTerminal stdout_terminal() {
      return AnsiTerminal{FileHandle{stdout, FileCloser{false}}};
    }

    static AnsiTerminal stderr_terminal() {
      return AnsiTerminal{FileHandle{stderr, FileCloser{false}}};
    }

    RenderResult render(const DomNode &dom) {
      std::string buffer{};
      auto it = std::back_inserter(buffer);
      AnsiFormatter<std::back_insert_iterator<std::string>>::render(dom, it, 0, std::nullopt);
      std::print(__file.get(), "{}", buffer);
      return {};
    }

    RenderResult clear() {
      std::print(__file.get(), "{}", "\x1b[2J");
      return {};
    }
  };

  export inline AnsiTerminal out_terminal = AnsiTerminal::stdout_terminal();

  export inline AnsiTerminal err_terminal = AnsiTerminal::stderr_terminal();
} // namespace jowi::tui
