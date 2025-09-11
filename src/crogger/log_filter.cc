module;
#include <concepts>
export module jowi.crogger:log_filter;
import :log_metadata;

namespace jowi::crogger {
  export template <typename T>
  concept basic_filter = requires(const T filter, const log_metadata &data) {
    { filter.filter(data) } -> std::same_as<bool>;
  };

  // Void specialization - abstract base class
  export template <typename T = void> struct log_filter;

  export template <> struct log_filter<void> {
    virtual ~log_filter() = default;

    virtual bool filter(const log_metadata &data) const = 0;
  };

  // Template for types that satisfy basic_filter
  export template <basic_filter filter_type>
  struct log_filter<filter_type> : private filter_type, public log_filter<void> {
    using filter_type::filter_type; // Inherit constructors

    // Move constructor
    log_filter(filter_type &&filter) : filter_type(std::move(filter)) {}

    bool filter(const log_metadata &data) const override {
      return filter_type::filter(data);
    }
  };

  export enum struct filter_op { eq, lt, lte, gt, gte };

  export struct level_filter {
    filter_op op;
    unsigned int level;

    bool filter(const log_metadata &data) const {
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
    bool filter(const log_metadata &data) const {
      return true;
    }
  };
}