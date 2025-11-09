module;

export module jowi.crogger:severity;
import jowi.generic;

namespace jowi::crogger {
  export struct Severity {
    generic::FixedString<10> name;
    unsigned int level;

    static Severity trace() noexcept {
      return {"TRACE", 0};
    }

    static Severity debug() noexcept {
      return {"DEBUG", 10};
    }

    static Severity info() noexcept {
      return {"INFO", 20};
    }

    static Severity warn() noexcept {
      return {"WARN", 30};
    }

    static Severity error() noexcept {
      return {"ERROR", 40};
    }

    static Severity critical() noexcept {
      return {"CRITICAL", 50};
    }

    // Three-way comparison operator - compare only the level field
    friend auto operator<=>(const Severity &lhs, const Severity &rhs) noexcept {
      return lhs.level <=> rhs.level;
    }

    // Equality operator - compare only the level field
    friend bool operator==(const Severity &lhs, const Severity &rhs) noexcept {
      return lhs.level == rhs.level;
    }
  };
}