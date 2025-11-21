module;

export module jowi.crogger:log_level;
import jowi.generic;

namespace jowi::crogger {
  export struct LogLevel {
    generic::FixedString<10> name;
    unsigned int level;

    static LogLevel trace() noexcept {
      return {"TRACE", 0};
    }

    static LogLevel debug() noexcept {
      return {"DEBUG", 10};
    }

    static LogLevel info() noexcept {
      return {"INFO", 20};
    }

    static LogLevel warn() noexcept {
      return {"WARN", 30};
    }

    static LogLevel error() noexcept {
      return {"ERROR", 40};
    }

    static LogLevel critical() noexcept {
      return {"CRITICAL", 50};
    }

    // Three-way comparison operator - compare only the level field
    friend auto operator<=>(const LogLevel &lhs, const LogLevel &rhs) noexcept {
      return lhs.level <=> rhs.level;
    }

    // Equality operator - compare only the level field
    friend bool operator==(const LogLevel &lhs, const LogLevel &rhs) noexcept {
      return lhs.level == rhs.level;
    }
  };
}