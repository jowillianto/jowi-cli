module;
#include <concepts>
#include <cstdint>
#include <format>
#include <ranges>
#include <vector>
export module jowi.tui:node;
import jowi.generic;
import :color;
import :text_format;

namespace jowi::tui {

  enum struct ValuelessNode { format_end, new_line, text, format_start, indent };

  struct IndentNode {
    std::uint8_t level;

    constexpr bool operator==(const IndentNode &) const = default;
  };

  constexpr std::array no_value_nodes = {ValuelessNode::format_end, ValuelessNode::new_line};

  export struct CliNode {
  private:
    using VariantType = generic::Variant<ValuelessNode, TextFormat, std::string, IndentNode>;
    VariantType __value;

    template <typename... Args>
    constexpr CliNode(Args &&...args)
      requires std::constructible_from<VariantType, Args...>
      : __value(std::forward<Args>(args)...) {}

  public:
    template <ValuelessNode node_type> bool is() const {
      if constexpr (std::ranges::find(no_value_nodes, node_type) != no_value_nodes.end()) {
        return __value.as<ValuelessNode>()
          .transform([](const auto &val) { return val == node_type; })
          .value_or(false);
      } else if constexpr (node_type == ValuelessNode::text) {
        return __value.is<std::string>();
      } else if constexpr (node_type == ValuelessNode::format_start) {
        return __value.is<TextFormat>();
      } else if constexpr (node_type == ValuelessNode::indent) {
        return __value.is<IndentNode>();
      }
    }

    /*
      Visit
    */
    template <typename... Args> constexpr auto visit(Args &&...args) const {
      return __value.visit(std::forward<Args>(args)...);
    }

    friend bool operator==(const CliNode &l, const CliNode &r) {
      return l.__value.visit([&](const auto &l) {
        using LType = std::decay_t<decltype(l)>;
        return r.__value.visit(
          [&](const LType &r) { return l == r; }, [](auto &&) { return false; }
        );
      });
    }

    static CliNode new_line() noexcept {
      return CliNode{ValuelessNode::new_line};
    }
    static CliNode format_end() noexcept {
      return CliNode{ValuelessNode::format_end};
    }
    static CliNode indent(std::uint8_t indent) noexcept {
      return CliNode{IndentNode{indent}};
    }
    template <class... Args>
    static CliNode text(std::format_string<Args...> fmt, Args &&...args) noexcept {
      return CliNode{std::format(fmt, std::forward<Args>(args)...)};
    }
    template <class... Args>
    static CliNode vtext(std::string_view fmt, std::format_args args) noexcept {
      return CliNode{std::vformat(fmt.data(), std::forward<std::format_args>(args))};
    }
    static CliNode format_begin(TextFormat fmt) {
      return CliNode{std::move(fmt)};
    }
  };

  template <class T>
  concept is_cli_nodes_leaf = std::convertible_to<std::decay_t<T>, CliNode>;
  template <class T>
  concept is_cli_nodes_subtree =
    std::ranges::range<T> && std::same_as<CliNode, std::decay_t<std::ranges::range_value_t<T>>>;

  template <class T>
  concept is_cli_nodes_tree = is_cli_nodes_leaf<T> || is_cli_nodes_subtree<T>;

  export struct CliNodes {
  private:
    std::vector<CliNode> __nodes;
    std::uint8_t __indent;

    CliNodes &__append_node(CliNode n) {
      if (__nodes.empty() || __nodes.back().is<ValuelessNode::new_line>() && __indent != 0) {
        __nodes.emplace_back(CliNode::indent(__indent));
      }
      __nodes.emplace_back(std::move(n));
      return *this;
    }

  public:
    template <class... Args>
      requires(is_cli_nodes_tree<Args> && ...)
    CliNodes(std::uint8_t indent, Args &&...args) : __indent{indent}, __nodes{} {
      __nodes.reserve(sizeof...(Args));
      append_nodes(std::forward<Args>(args)...);
    }
    template <class... Args>
      requires(is_cli_nodes_tree<Args> && ...)
    CliNodes(Args &&...args) : CliNodes(0, std::forward<Args>(args)...) {}

    template <is_cli_nodes_tree T> CliNodes &append_node(T &&v) {
      if constexpr (is_cli_nodes_leaf<T>) {
        return __append_node(std::forward<T>(v));
      } else {
        if constexpr (std::ranges::sized_range<T>) {
          __nodes.reserve(__nodes.size() + std::ranges::size(v));
        }
        for (auto &&n : std::forward<T>(v)) {
          if constexpr (std::is_rvalue_reference_v<T>) {
            __append_node(std::move(n));
          } else {
            __append_node(n);
          }
        }
        return *this;
      }
    }

    template <class... Args>
      requires(is_cli_nodes_tree<Args> && ...)
    CliNodes &append_nodes(Args &&...args) {
      __nodes.reserve(__nodes.size() + sizeof...(args));
      (append_node(std::forward<Args>(args)), ...);
      return *this;
    }

    // Access
    uint64_t size() const noexcept {
      return __nodes.size();
    }
    bool empty() const noexcept {
      return __nodes.empty();
    }
    std::uint8_t indent() const noexcept {
      return __indent;
    }
    constexpr auto begin() const noexcept {
      return __nodes.begin();
    }
    constexpr auto end() const noexcept {
      return __nodes.end();
    }
  };
}

template <class CharType> struct std::formatter<jowi::tui::IndentNode, CharType> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const jowi::tui::IndentNode &node, auto &ctx) const {
    for (std::uint8_t i = 0; i != node.level; i += 1) {
      std::format_to(ctx.out(), " ");
    }
    return ctx.out();
  }
};

template <class CharType> struct std::formatter<jowi::tui::CliNode, CharType> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const jowi::tui::CliNode &n, auto &ctx) const {
    n.visit(
      [&](const jowi::tui::ValuelessNode &n) {
        if (n == jowi::tui::ValuelessNode::new_line) {
          std::format_to(ctx.out(), "\n");
        } else if (n == jowi::tui::ValuelessNode::format_end) {
          std::format_to(ctx.out(), "\x1b[0m");
        }
      },
      [&](const auto &n) { std::format_to(ctx.out(), "{}", n); }
    );
    return ctx.out();
  }
};

template <class CharType> struct std::formatter<jowi::tui::CliNodes, CharType> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const jowi::tui::CliNodes &nodes, auto &ctx) const {
    for (const auto &n : nodes) {
      std::format_to(ctx.out(), "{}", n);
    }
    return ctx.out();
  }
};
