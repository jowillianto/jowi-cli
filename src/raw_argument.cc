module;
#include <optional>
#include <string_view>
export module moderna.cli:raw_argument;

namespace moderna::cli {
  class raw_argument_iterator {
    int __ptr;
    const char **__argv;

    raw_argument_iterator(const char **argv, int ptr) : __argv{argv}, __ptr{ptr} {}

  public:
    static raw_argument_iterator begin(const char **argv) {
      return raw_argument_iterator{argv, 0};
    }
    static raw_argument_iterator end(int argc, const char **argv) {
      return raw_argument_iterator{argv, argc};
    }
    raw_argument_iterator &operator++() noexcept {
      __ptr += 1;
      return *this;
    }
    raw_argument_iterator operator+(int num) const noexcept {
      return raw_argument_iterator(__argv, __ptr + num);
    }
    void operator++(int) noexcept {
      __ptr += 1;
    }
    std::string_view operator*() const noexcept {
      return __argv[__ptr];
    }
    raw_argument_iterator reset() const noexcept {
      return raw_argument_iterator::begin(__argv);
    }
    friend bool operator==(const raw_argument_iterator &l, const raw_argument_iterator &r) {
      return l.__argv == r.__argv && l.__ptr == r.__ptr;
    }
  };

  class raw_arguments {
    int __argc;
    const char **__argv;

  public:
    raw_arguments(int argc, const char **argv) : __argc{argc}, __argv{argv} {}
    raw_argument_iterator begin() const noexcept {
      return raw_argument_iterator::begin(__argv);
    }
    raw_argument_iterator end() const noexcept {
      return raw_argument_iterator::end(__argc, __argv);
    }
    std::optional<std::string_view> operator[](int id) const noexcept {
      if (id < __argc) {
        return __argv[id];
      }
      return std::nullopt;
    }
  };
}