module;
#include <optional>
#include <string_view>
export module jowi.cli:raw_args;

namespace jowi::cli {
  struct raw_args_iterator {
  private:
    int __cur;
    int __argc;
    const char **__argv;

  public:
    raw_args_iterator(const char **argv, int cur, int argc) noexcept :
      __argv{argv}, __cur{cur}, __argc{argc} {}

    // Iterator Satisfaction
    raw_args_iterator &operator++() noexcept {
      __cur += 1;
      return *this;
    }
    void operator++(int) noexcept {
      __cur += 1;
    }
    std::string_view operator*() const noexcept {
      return __argv[__cur];
    }
    friend bool operator==(const raw_args_iterator &l, const raw_args_iterator &r) {
      return l.__cur == r.__cur && l.__argv == r.__argv;
    }
  };

  struct raw_args {
  private:
    int __argc;
    const char **__argv;

  public:
    raw_args(int argc, const char **argv) noexcept : __argc{argc}, __argv{argv} {}

    std::optional<std::string_view> operator[](int id) const noexcept {
      if (id >= __argc) return std::nullopt;
      return std::string_view{__argv[id]};
    }
    raw_args_iterator begin() const noexcept {
      return raw_args_iterator{__argv, 0, __argc};
    }
    raw_args_iterator end() const noexcept {
      return raw_args_iterator{__argv, __argc, __argc};
    }
  };
}