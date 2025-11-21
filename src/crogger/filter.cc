module;
#include <concepts>
#include <memory>
export module jowi.crogger:filter;
import :log_context;

namespace jowi::crogger {
  export template <typename T>
  concept IsFilter = requires(const T filter, const LogContext &data) {
    { filter.filter(data) } -> std::same_as<bool>;
  };

  // Void specialization - abstract base class
  export template <typename T = void> struct ContextFilter;

  export template <> struct ContextFilter<void> {
    virtual ~ContextFilter() = default;

    virtual bool filter(const LogContext &data) const = 0;
  };

  // Template for types that satisfy basic_filter
  export template <IsFilter FilterType>
  struct ContextFilter<FilterType> : private FilterType, public ContextFilter<void> {
    using FilterType::FilterType; // Inherit constructors

    // Move constructor
    ContextFilter(FilterType &&filter) : FilterType(std::move(filter)) {}

    bool filter(const LogContext &data) const override {
      return FilterType::filter(data);
    }
  };

  enum struct FilterOp { EQ, LT, LTE, GT, GTE };

  export struct LevelFilter {
    FilterOp op;
    unsigned int level;

    bool filter(const LogContext &data) const {
      unsigned int level_l = data.status.level;
      switch (op) {
        case FilterOp::EQ:
          return level_l == level;
        case FilterOp::LT:
          return level_l < level;
        case FilterOp::LTE:
          return level_l <= level;
        case FilterOp::GT:
          return level_l > level;
        case FilterOp::GTE:
          return level_l >= level;
      };
    }

    static LevelFilter equal_to(unsigned int level) {
      return LevelFilter{FilterOp::EQ, level};
    }

    static LevelFilter less_than(unsigned int level) {
      return LevelFilter{FilterOp::LT, level};
    }

    static LevelFilter less_than_or_equal_to(unsigned int level) {
      return LevelFilter{FilterOp::LTE, level};
    }
    static LevelFilter greater_than(unsigned int level) {
      return LevelFilter{FilterOp::GT, level};
    }
    static LevelFilter greater_than_or_equal_to(unsigned int level) {
      return LevelFilter{FilterOp::GTE, level};
    }
  };

  export struct NoFilter {
    bool filter(const LogContext &data) const {
      return true;
    }
  };

  template struct ContextFilter<LevelFilter>;
  template struct ContextFilter<NoFilter>;
}