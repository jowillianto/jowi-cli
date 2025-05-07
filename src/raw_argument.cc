module;
#include <optional>
#include <string_view>
export module moderna.cli:raw_argument;

namespace moderna::cli {
  class raw_argument_iterator {
    int __ptr;
    int __argc;
    const char **__argv;

    raw_argument_iterator(const char **argv, int ptr, int argc) :
      __argv{argv}, __ptr{ptr}, __argc{argc} {}

  public:
    /*
      Gets an iterator pointing to the start of the argument values
    */
    static raw_argument_iterator begin(int argc, const char **argv) {
      return raw_argument_iterator{argv, 0, argc};
    }
    /*
      Gets an iterator pointing to the end of the argument values
    */
    static raw_argument_iterator end(int argc, const char **argv) {
      return raw_argument_iterator{argv, argc, argc};
    }
    /*
      Increments the current iterator
    */
    raw_argument_iterator &operator++() noexcept {
      __ptr += 1;
      return *this;
    }
    /*
      Increments the current iterator by the value specified in int
    */
    raw_argument_iterator operator+(int num) const noexcept {
      return raw_argument_iterator(__argv, __ptr + num, __argc);
    }
    /*
      Increments the current iterator by int
    */
    void operator++(int) noexcept {
      __ptr += 1;
    }
    /*
      Dereferences the current iterator into a string view
    */
    std::string_view operator*() const noexcept {
      return __argv[__ptr];
    }
    /*
      Reset the current iterator to the beginning
    */
    raw_argument_iterator reset() const noexcept {
      return raw_argument_iterator::begin(__argc, __argv);
    }
    /*
      Check if the current iterator is equal to another iterator
    */
    friend bool operator==(const raw_argument_iterator &l, const raw_argument_iterator &r) {
      return l.__argv == r.__argv && l.__ptr == r.__ptr;
    }
    /*
      Checks if the current iterator has reached the end, this eliminates the use of passing and
      end iterator
    */
    bool is_end() const noexcept {
      return __argc == __ptr;
    }
  };

  class raw_arguments {
    int __argc;
    const char **__argv;

  public:
    raw_arguments(int argc, const char **argv) : __argc{argc}, __argv{argv} {}
    raw_argument_iterator begin() const noexcept {
      return raw_argument_iterator::begin(__argc, __argv);
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