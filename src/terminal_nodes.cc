module;
#include <concepts>
#include <cstdint>
#include <format>
#include <ranges>
#include <vector>
export module moderna.cli:terminal_nodes;
import moderna.generic;
import :terminal_color;
import :terminal_format;

namespace moderna::cli {

  enum struct valueless_node { format_end, new_line, text, format_start, indent };

  struct indent_node {
    std::uint8_t level;

    constexpr bool operator==(const indent_node &) const = default;
  };

  constexpr std::array no_value_nodes = {valueless_node::format_end, valueless_node::new_line};

  export struct terminal_node {
  private:
    using variant_type = generic::variant<valueless_node, text_format, std::string, indent_node>;
    variant_type __value;

    template <typename... Args>
    constexpr terminal_node(Args &&...args)
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
    template <typename F> constexpr auto visit(F &&f) const {
      return __value.visit(std::forward<F>(f));
    }

    friend bool operator==(const terminal_node &l, const terminal_node &r) {
      return l.__value.visit([&](const auto &l) {
        return r.__value.visit([&](const auto &r) {
          using r_type = std::decay_t<decltype(r)>;
          using l_type = std::decay_t<decltype(l)>;
          if constexpr (std::same_as<r_type, l_type>) {
            return l == r;
          } else {
            return false;
          }
        });
      });
    }

    static terminal_node new_line() noexcept {
      return terminal_node{valueless_node::new_line};
    }
    static terminal_node format_end() noexcept {
      return terminal_node{valueless_node::format_end};
    }
    static terminal_node indent(std::uint8_t indent) noexcept {
      return terminal_node{indent_node{indent}};
    }
    template <class... Args>
    static terminal_node text(std::format_string<Args...> fmt, Args &&...args) noexcept {
      return terminal_node{std::format(fmt, std::forward<Args>(args)...)};
    }
    static terminal_node format_begin(text_format fmt) {
      return terminal_node{std::move(fmt)};
    }
  };

  template <class T>
  concept is_terminal_nodes_leaf = std::convertible_to<std::decay_t<T>, terminal_node>;
  template <class T>
  concept is_terminal_nodes_subtree = std::ranges::range<T> &&
    std::same_as<terminal_node, std::decay_t<std::ranges::range_value_t<T>>>;

  template <class T>
  concept is_terminal_nodes_tree = is_terminal_nodes_leaf<T> || is_terminal_nodes_subtree<T>;

  export struct terminal_nodes {
  private:
    std::vector<terminal_node> __nodes;
    std::uint8_t __indent;

    terminal_nodes &__append_node(terminal_node n) {
      if (__nodes.empty() || __nodes.back().is<valueless_node::new_line>()) {
        __nodes.emplace_back(terminal_node::indent(__indent));
      }
      __nodes.emplace_back(std::move(n));
      return *this;
    }

  public:
    template <class... Args>
      requires(is_terminal_nodes_tree<Args> && ...)
    terminal_nodes(std::uint8_t indent, Args &&...args) : __indent{indent}, __nodes{} {
      __nodes.reserve(sizeof...(Args));
      append_nodes(std::forward<Args>(args)...);
    }
    template <class... Args>
      requires(is_terminal_nodes_tree<Args> && ...)
    terminal_nodes(Args &&...args) : terminal_nodes(0, std::forward<Args>(args)...) {}

    template <is_terminal_nodes_tree T> terminal_nodes &append_node(T &&v) {
      if constexpr (is_terminal_nodes_leaf<T>) {
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
      requires(is_terminal_nodes_tree<Args> && ...)
    terminal_nodes &append_nodes(Args &&...args) {
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

template <class char_type> struct std::formatter<moderna::cli::indent_node, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const moderna::cli::indent_node &node, auto &ctx) const {
    for (std::uint8_t i = 0; i != node.level; i += 1) {
      std::format_to(ctx.out(), " ");
    }
    return ctx.out();
  }
};

template <class char_type> struct std::formatter<moderna::cli::terminal_node, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const moderna::cli::terminal_node &n, auto &ctx) const {
    n.visit([&](const auto &n) {
      using node_type = std::decay_t<decltype(n)>;
      if constexpr (std::same_as<node_type, moderna::cli::valueless_node>) {
        if (n == moderna::cli::valueless_node::new_line) {
          std::format_to(ctx.out(), "\n");
        } else if (n == moderna::cli::valueless_node::format_end) {
          std::format_to(ctx.out(), "\x1b[0m");
        }
      } else {
        std::format_to(ctx.out(), "{}", n);
      }
    });
    return ctx.out();
  }
};

template <class char_type> struct std::formatter<moderna::cli::terminal_nodes, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(const moderna::cli::terminal_nodes &nodes, auto &ctx) const {
    for (const auto &n : nodes) {
      std::format_to(ctx.out(), "{}", n);
    }
    return ctx.out();
  }
};