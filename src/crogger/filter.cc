module;
#include <concepts>
#include <memory>
export module jowi.crogger:filter;
import :context;

namespace jowi::crogger {
  export template <typename T>
  concept is_filter = requires(const T filter, const Context &data) {
    { filter.filter(data) } -> std::same_as<bool>;
  };

  // Void specialization - abstract base class
  export template <typename T = void> struct ContextFilter;

  export template <> struct ContextFilter<void> {
    virtual ~ContextFilter() = default;

    virtual bool filter(const Context &data) const = 0;
  };

  // Template for types that satisfy basic_filter
  export template <is_filter FilterType>
  struct ContextFilter<FilterType> : private FilterType, public ContextFilter<void> {
    using FilterType::FilterType; // Inherit constructors

    // Move constructor
    ContextFilter(FilterType &&filter) : FilterType(std::move(filter)) {}

    bool filter(const Context &data) const override {
      return FilterType::filter(data);
    }
  };

  export enum struct FilterOp { eq, lt, lte, gt, gte };

  export struct SeverityFilter {
    FilterOp op;
    unsigned int level;

    bool filter(const Context &data) const {
      unsigned int level_l = data.status.level;
      switch (op) {
        case FilterOp::eq:
          return level_l == level;
        case FilterOp::lt:
          return level_l < level;
        case FilterOp::lte:
          return level_l <= level;
        case FilterOp::gt:
          return level_l > level;
        case FilterOp::gte:
          return level_l >= level;
      };
    }
  };

  export struct NoFilter {
    bool filter(const Context &data) const {
      return true;
    }
  };

  template struct ContextFilter<SeverityFilter>;
  template struct ContextFilter<NoFilter>;
}