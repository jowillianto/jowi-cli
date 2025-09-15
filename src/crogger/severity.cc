module;

export module jowi.crogger:severity;
import jowi.generic;

namespace jowi::crogger {
  export struct severity {
    generic::fixed_string<10> name;
    unsigned int level;

    static severity trace() noexcept {
      return {"TRACE", 0};
    }

    static severity debug() noexcept {
      return {"DEBUG", 10};
    }

    static severity info() noexcept {
      return {"INFO", 20};
    }

    static severity warn() noexcept {
      return {"WARN", 30};
    }

    static severity error() noexcept {
      return {"ERROR", 40};
    }

    static severity critical() noexcept {
      return {"CRITICAL", 50};
    }

    // Three-way comparison operator - compare only the level field
    friend auto operator<=>(const severity &lhs, const severity &rhs) noexcept {
      return lhs.level <=> rhs.level;
    }

    // Equality operator - compare only the level field
    friend bool operator==(const severity &lhs, const severity &rhs) noexcept {
      return lhs.level == rhs.level;
    }
  };
}