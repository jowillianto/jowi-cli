module;

export module jowi.crogger:log_status;
import jowi.generic;

namespace jowi::crogger {
  export struct log_status {
    generic::fixed_string<10> name;
    unsigned int level;

    static log_status trace() noexcept {
      return {"TRACE", 0};
    }

    static log_status debug() noexcept {
      return {"DEBUG", 10};
    }

    static log_status info() noexcept {
      return {"INFO", 20};
    }

    static log_status warn() noexcept {
      return {"WARN", 30};
    }

    static log_status error() noexcept {
      return {"ERROR", 40};
    }

    static log_status critical() noexcept {
      return {"CRITICAL", 50};
    }

    // Three-way comparison operator - compare only the level field
    friend auto operator<=>(const log_status &lhs, const log_status &rhs) noexcept {
      return lhs.level <=> rhs.level;
    }

    // Equality operator - compare only the level field
    friend bool operator==(const log_status &lhs, const log_status &rhs) noexcept {
      return lhs.level == rhs.level;
    }
  };
}