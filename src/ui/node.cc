module;
#include <concepts>
#include <cstdint>
#include <format>
#include <ranges>
#include <vector>
export module jowi.cli.ui:node;
import jowi.generic;
import :color;
import :text_format;

namespace jowi::cli::ui {

  enum struct valueless_node { format_end, new_line, text, format_start, indent };

  struct indent_node {
    std::uint8_t level;

    constexpr bool operator==(const indent_node &) const = default;
  };

  constexpr std::array no_value_nodes = {valueless_node::format_end, valueless_node::new_line};

  export struct cli_node {
  private:
    using variant_type = generic::variant<valueless_node, text_format, std::string, indent_node>;
    variant_type __value;

    template <typename... Args>
    constexpr cli_node(Args &&...args)
      requires std::constructible_from<variant_type, Args...>
      : __value(std::forward<Args>(args)...) {}

  public:
    template <valueless_node node_type> bool is() const {
      if constexpr (std::ranges::find(no_value_nodes, node_type) != no_value_nodes.end()) {
        return __value.as<valueless_node>()
          .transform([](const auto &val) { return val == node_type; })
          .value_or(false);
      } else if constexpr (node_type == valueless_node::text) {
        return __value.is<std::string>();
      } else if constexpr (node_type == valueless_node::format_start) {
        return __value.is<text_format>();
      } else if constexpr (node_type == valueless_node::indent) {
        return __value.is<indent_node>();
      }
    }

    /*
      Visit
    */
    template <typename... Args> constexpr auto visit(Args &&...args) const {
      return __value.visit(std::forward<Args>(args)...);
    }

    friend bool operator==(const cli_node &l, const cli_node &r) {
      return l.__value.visit([&](const auto &l) {
        using l_type = std::decay_t<decltype(l)>;
        return r.__value.visit(
          [&](const l_type &r) { return l == r; }, [](auto &&) { return false; }
        );
      });
    }

    static cli_node new_line() noexcept {
      return cli_node{valueless_node::new_line};
    }
    static cli_node format_end() noexcept {
      return cli_node{valueless_node::format_end};
    }
    static cli_node indent(std::uint8_t indent) noexcept {
      return cli_node{indent_node{indent}};
    }
    template <class... Args>
    static cli_node text(std::format_string<Args...> fmt, Args &&...args) noexcept {
      return cli_node{std::format(fmt, std::forward<Args>(args)...)};
    }
    static cli_node format_begin(text_format fmt) {
      return cli_node{std::move(fmt)};
    }
  };

  template <class T>
  concept is_cli_nodes_leaf = std::convertible_to<std::decay_t<T>, cli_node>;
  template <class T>
  concept is_cli_nodes_subtree =
    std::ranges::range<T> && std::same_as<cli_node, std::decay_t<std::ranges::range_value_t<T>>>;

  template <class T>
  concept is_cli_nodes_tree = is_cli_nodes_leaf<T> || is_cli_nodes_subtree<T>;

  export struct cli_nodes {
  private:
    std::vector<cli_node> __nodes;
    std::uint8_t __indent;

    cli_nodes &__append_node(cli_node n) {
      if (__nodes.empty() || __nodes.back().is<valueless_node::new_line>() && __indent != 0) {
        __nodes.emplace_back(cli_node::indent(__indent));
      }
      __nodes.emplace_back(std::move(n));
      return *this;
    }

  public:
    template <class... Args>
      requires(is_cli_nodes_tree<Args> && ...)
    cli_nodes(std::uint8_t indent, Args &&...args) : __indent{indent}, __nodes{} {
      __nodes.reserve(sizeof...(Args));
      append_nodes(std::forward<Args>(args)...);
    }
    template <class... Args>
      requires(is_cli_nodes_tree<Args> && ...)
    cli_nodes(Args &&...args) : cli_nodes(0, std::forward<Args>(args)...) {}

    template <is_cli_nodes_tree T> cli_nodes &append_node(T &&v) {
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
    cli_nodes &append_nodes(Args &&...args) {
      __nodes.reserve(__nodes.size() + sizeof...(args));
      (append_node(std::forward<Args>(args)), ...);
      return *this;
    }

    // Access
    size_t size() const noexcept {
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

template <class char_type> struct std::formatter<jowi::cli::ui::indent_node, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const jowi::cli::ui::indent_node &node, auto &ctx) const {
    for (std::uint8_t i = 0; i != node.level; i += 1) {
      std::format_to(ctx.out(), " ");
    }
    return ctx.out();
  }
};

template <class char_type> struct std::formatter<jowi::cli::ui::cli_node, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const jowi::cli::ui::cli_node &n, auto &ctx) const {
    n.visit(
      [&](const jowi::cli::ui::valueless_node &n) {
        if (n == jowi::cli::ui::valueless_node::new_line) {
          std::format_to(ctx.out(), "\n");
        } else if (n == jowi::cli::ui::valueless_node::format_end) {
          std::format_to(ctx.out(), "\x1b[0m");
        }
      },
      [&](const auto &n) { std::format_to(ctx.out(), "{}", n); }
    );
    return ctx.out();
  }
};

template <class char_type> struct std::formatter<jowi::cli::ui::cli_nodes, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const jowi::cli::ui::cli_nodes &nodes, auto &ctx) const {
    for (const auto &n : nodes) {
      std::format_to(ctx.out(), "{}", n);
    }
    return ctx.out();
  }
};