module;
#include <concepts>
#include <memory>
export module jowi.crogger:filter;
import :context;

namespace jowi::crogger {
  export template <typename T>
  concept is_filter = requires(const T filter, const context &data) {
    { filter.filter(data) } -> std::same_as<bool>;
  };

  // Void specialization - abstract base class
  export template <typename T = void> struct context_filter;

  export template <> struct context_filter<void> {
    virtual ~context_filter() = default;

    virtual bool filter(const context &data) const = 0;
  };

  // Template for types that satisfy basic_filter
  export template <is_filter filter_type>
  struct context_filter<filter_type> : private filter_type, public context_filter<void> {
    using filter_type::filter_type; // Inherit constructors

    // Move constructor
    context_filter(filter_type &&filter) : filter_type(std::move(filter)) {}

    bool filter(const context &data) const override {
      return filter_type::filter(data);
    }
  };

  export enum struct filter_op { eq, lt, lte, gt, gte };

  export struct severity_filter {
    filter_op op;
    unsigned int level;

    bool filter(const context &data) const {
      unsigned int level_l = data.status.level;
      switch (op) {
        case filter_op::eq:
          return level_l == level;
        case filter_op::lt:
          return level_l < level;
        case filter_op::lte:
          return level_l <= level;
        case filter_op::gt:
          return level_l > level;
        case filter_op::gte:
          return level_l >= level;
      };
    }
  };

  export struct no_filter {
    bool filter(const context &data) const {
      return true;
    }
  };

  template struct context_filter<severity_filter>;
  template struct context_filter<no_filter>;
}