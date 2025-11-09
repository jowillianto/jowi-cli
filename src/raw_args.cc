module;
#include <optional>
#include <string_view>
export module jowi.cli:raw_args;

namespace jowi::cli {
  struct RawArgsIterator {
  private:
    int __cur;
    int __argc;
    const char **__argv;

  public:
    RawArgsIterator(const char **argv, int cur, int argc) noexcept :
      __argv{argv}, __cur{cur}, __argc{argc} {}

    // Iterator Satisfaction
    RawArgsIterator &operator++() noexcept {
      __cur += 1;
      return *this;
    }
    void operator++(int) noexcept {
      __cur += 1;
    }
    std::string_view operator*() const noexcept {
      return __argv[__cur];
    }
    friend bool operator==(const RawArgsIterator &l, const RawArgsIterator &r) {
      return l.__cur == r.__cur && l.__argv == r.__argv;
    }
  };

  struct RawArgs {
  private:
    int __argc;
    const char **__argv;

  public:
    RawArgs(int argc, const char **argv) noexcept : __argc{argc}, __argv{argv} {}

    std::optional<std::string_view> operator[](int id) const noexcept {
      if (id >= __argc) return std::nullopt;
      return std::string_view{__argv[id]};
    }
    RawArgsIterator begin() const noexcept {
      return RawArgsIterator{__argv, 0, __argc};
    }
    RawArgsIterator end() const noexcept {
      return RawArgsIterator{__argv, __argc, __argc};
    }
  };
}