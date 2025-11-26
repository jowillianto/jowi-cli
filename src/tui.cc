module;
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <type_traits>
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

    Paragraph() : text{} {}
    Paragraph(std::string text) : text(std::move(text)) {}

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

  namespace ansi {
    inline constexpr std::array<std::pair<RgbColor, unsigned int>, 16> ANSI_BG_MAP{{
      {RgbColor::black(), 40},
      {RgbColor::red(), 41},
      {RgbColor::green(), 42},
      {RgbColor::yellow(), 43},
      {RgbColor::blue(), 44},
      {RgbColor::magenta(), 45},
      {RgbColor::cyan(), 46},
      {RgbColor::white(), 47},
      {RgbColor::bright_black(), 100},
      {RgbColor::bright_red(), 101},
      {RgbColor::bright_green(), 102},
      {RgbColor::bright_yellow(), 103},
      {RgbColor::bright_blue(), 104},
      {RgbColor::bright_magenta(), 105},
      {RgbColor::bright_cyan(), 106},
      {RgbColor::bright_white(), 107},
    }};

    inline constexpr std::array<std::pair<RgbColor, unsigned int>, 16> ANSI_FG_MAP{{
      {RgbColor::black(), 30},
      {RgbColor::red(), 31},
      {RgbColor::green(), 32},
      {RgbColor::yellow(), 33},
      {RgbColor::blue(), 34},
      {RgbColor::magenta(), 35},
      {RgbColor::cyan(), 36},
      {RgbColor::white(), 37},
      {RgbColor::bright_black(), 90},
      {RgbColor::bright_red(), 91},
      {RgbColor::bright_green(), 92},
      {RgbColor::bright_yellow(), 93},
      {RgbColor::bright_blue(), 94},
      {RgbColor::bright_magenta(), 95},
      {RgbColor::bright_cyan(), 96},
      {RgbColor::bright_white(), 97},
    }};

    inline constexpr std::array<std::pair<TextEffect, unsigned int>, 9> ANSI_EFFECT_MAP{{
      {TextEffect::BOLD, 1},
      {TextEffect::DIM, 2},
      {TextEffect::ITALIC, 3},
      {TextEffect::UNDERLINE, 4},
      {TextEffect::SLOW_BLINK, 5},
      {TextEffect::RAPID_BLINK, 6},
      {TextEffect::REVERSE, 7},
      {TextEffect::STRIKETHROUGH, 8},
      {TextEffect::DOUBLE_UNDERLINE, 9},
    }};

    inline std::optional<std::string> render_style(const DomStyle &style) {
      std::vector<unsigned int> codes;
      if (const auto &effects = style.effects()) {
        for (const auto &effect : *effects) {
          if (const auto it = std::ranges::find_if(
                ANSI_EFFECT_MAP, [&](const auto &entry) { return entry.first == effect; }
              );
              it != ANSI_EFFECT_MAP.end()) {
            codes.push_back(it->second);
          }
        }
      }
      if (const auto &bg = style.bg_color()) {
        if (const auto it = std::ranges::find_if(
              ANSI_BG_MAP, [&](const auto &entry) { return entry.first == *bg; }
            );
            it != ANSI_BG_MAP.end()) {
          codes.push_back(it->second);
        }
      }
      if (const auto &fg = style.fg_color()) {
        if (const auto it = std::ranges::find_if(
              ANSI_FG_MAP, [&](const auto &entry) { return entry.first == *fg; }
            );
            it != ANSI_FG_MAP.end()) {
          codes.push_back(it->second);
        }
      }

      if (codes.empty()) {
        return std::nullopt;
      }

      std::string code_str{"\x1b["};
      for (std::size_t i = 0; i < codes.size(); ++i) {
        if (i != 0) {
          code_str.push_back(';');
        }
        code_str.append(std::to_string(codes[i]));
      }
      code_str.push_back('m');
      return code_str;
    }

    template <class T> T reset_format(T out) {
      std::format_to(out, "\x1b[0m");
      return out;
    }

    template <class T>
    T render_dom(
      const DomNode &dom,
      T out,
      std::size_t indent,
      std::optional<std::reference_wrapper<const std::string>> prev_style
    );

    template <class T> T render_text(const Paragraph &paragraph, T out, std::size_t indent) {
      for (std::size_t i = 0; i < indent; ++i) {
        *out++ = ' ';
      }
      if (paragraph.new_line) {
        std::format_to(out, "{}\n", paragraph.text);

      } else {
        std::format_to(out, "{}", paragraph.text);
      }
      return out;
    }

    template <class T>
    T render_layout(
      const Layout &layout,
      T out,
      std::size_t indent,
      std::optional<std::reference_wrapper<const std::string>> prev_style
    ) {
      auto current_style = render_style(layout.style());
      if (current_style) {
        reset_format(out);
        std::format_to(out, "{}", *current_style);
      }

      for (const auto &child : layout) {
        render_dom(
          child,
          out,
          indent + static_cast<std::size_t>(layout.style().indentation()),
          current_style ? std::optional{std::cref(*current_style)} : prev_style
        );
      }

      if (current_style) {
        reset_format(out);
      }
      if (prev_style) {
        std::format_to(out, "{}", prev_style->get());
      }
      return out;
    }

    template <class T>
    T render_dom(
      const DomNode &dom,
      T out,
      std::size_t indent,
      std::optional<std::reference_wrapper<const std::string>> prev_style
    ) {
      return dom.visit(
        [&](const Paragraph &p) { return render_text(p, out, indent); },
        [&](const Layout &l) { return render_layout(l, out, indent, prev_style); }
      );
    }

    template <class T> T render_dom(const DomNode &dom, T out) {
      return render_dom(dom, out, 0, std::nullopt);
    }
  } // namespace ansi
} // namespace jowi::tui

namespace std {
  template <class CharType> struct formatter<jowi::tui::Paragraph, CharType> {
    static_assert(std::same_as<CharType, char>, "Paragraph formatter only supports char output");

    constexpr auto parse(auto &ctx) {
      return ctx.begin();
    }

    auto format(const jowi::tui::Paragraph &paragraph, auto &ctx) const {
      jowi::tui::ansi::render_text(paragraph, ctx.out(), 0);
      return ctx.out();
    }
  };

  template <class CharType> struct formatter<jowi::tui::Layout, CharType> {
    static_assert(std::same_as<CharType, char>, "Layout formatter only supports char output");

    constexpr auto parse(auto &ctx) {
      return ctx.begin();
    }

    auto format(const jowi::tui::Layout &layout, auto &ctx) const {
      jowi::tui::ansi::render_layout(layout, ctx.out(), 0, std::nullopt);
      return ctx.out();
    }
  };

  template <class CharType> struct formatter<jowi::tui::DomNode, CharType> {

    constexpr auto parse(auto &ctx) {
      return ctx.begin();
    }
    auto format(const jowi::tui::DomNode &dom, auto &ctx) const {
      jowi::tui::ansi::render_dom(dom, ctx.out());
      return ctx.out();
    }
  };
}
