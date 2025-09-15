module;
#include <concepts>
#include <expected>
#include <filesystem>
#include <memory>
#include <print>
#include <string_view>
export module jowi.crogger:emitter;
import :error;

namespace jowi::crogger {
  namespace fs = std::filesystem;
  export template <class T>
  concept is_emitter = requires(const T emitter, std::string_view data) {
    { emitter.emit(data) } -> std::same_as<std::expected<void, log_error>>;
  };

  export template <class T>
  concept is_stream_emitter = requires(T emitter, char x) {
    { emitter.push_back(x) } -> std::same_as<void>;
    { emitter.flush() } -> std::same_as<std::expected<void, log_error>>;
  };

  export template <class T = void> struct stream_emitter;
  export template <class T = void> struct emitter;

  template <> struct emitter<void> {
    virtual std::expected<void, log_error> emit(std::string_view) const = 0;
    virtual ~emitter() = default;

    template <is_emitter emitter_type, class... Args>
      requires(std::constructible_from<emitter_type, Args...>)
    static std::unique_ptr<emitter<void>> make_unique(Args &&...args) {
      return std::make_unique<emitter<emitter_type>>(std::forward<Args>(args)...);
    }

    template <is_emitter emitter_type, class... Args>
      requires(std::constructible_from<emitter_type, Args...>)
    static std::shared_ptr<emitter<void>> make_shared(Args &&...args) {
      return std::make_shared<emitter<emitter_type>>(std::forward<Args>(args)...);
    }
  };

  export template <is_emitter T> struct emitter<T> : private T, public emitter<void> {
    using T::T;

    emitter(T &&v)
      requires(std::move_constructible<T>)
      : T{std::forward<T &&>(v)} {}

    std::expected<void, log_error> emit(std::string_view d) const override {
      return T::emit(d);
    }
  };
  template <> struct stream_emitter<void> {
    using value_type = char;
    virtual void push_back(char x) = 0;
    virtual std::expected<void, log_error> flush() = 0;
    virtual ~stream_emitter() = default;

    template <is_stream_emitter emitter_type, class... Args>
      requires(std::constructible_from<emitter_type, Args...>)
    std::unique_ptr<stream_emitter<void>> make_unique(Args &&...args) {
      return std::make_unique<stream_emitter<emitter_type>>(std::forward<Args>(args)...);
    }

    template <is_stream_emitter emitter_type, class... Args>
      requires(std::constructible_from<emitter_type, Args...>)
    std::shared_ptr<stream_emitter<void>> make_shared(Args &&...args) {
      return std::make_shared<stream_emitter<emitter_type>>(std::forward<Args>(args)...);
    }
  };

  export template <is_stream_emitter T>
  struct stream_emitter<T> : private T, public stream_emitter<void> {
    using T::T;

    stream_emitter(T &&v)
      requires(std::move_constructible<T>)
      : T{std::forward<T &&>(v)} {}

    void push_back(char x) override {
      T::push_back(x);
    }

    std::expected<void, log_error> flush() override {
      return T::flush();
    }
  };

  export struct ss_emitter {
  private:
    std::unique_ptr<emitter<void>> __emt;
    std::string __buf;
    size_t __cur;

  public:
    ss_emitter(size_t buf_size, is_emitter auto &&emt) :
      __emt{emitter<>::make_unique<std::decay_t<decltype(emt)>>(std::forward<decltype(emt)>(emt))},
      __buf{}, __cur{0} {
      __buf.reserve(buf_size);
      for (size_t i = 0; i < buf_size; i += 1)
        __buf.push_back('\0');
    }
    ss_emitter(is_emitter auto &&emt) : ss_emitter(120, std::forward<decltype(emt)>(emt)) {}
    void push_back(char x) {
      __buf[__cur] = x;
      __cur += 1;
      if (__cur == __buf.length()) {
        auto res = __emt->emit(__buf);
        if (!res) {
          throw res.error();
        }
        __cur = 0;
      };
    }
    std::expected<void, log_error> flush() noexcept(
      noexcept(__emt->emit(std::declval<std::string_view>()))
    ) {
      return __emt->emit(std::string_view{__buf.begin(), __buf.begin() + __cur}).transform([&]() {
        __cur = 0;
      });
    }
  };

  export struct empty_emitter {
    std::expected<void, log_error> emit(std::string_view data) const {
      return {};
    }
  };

  export struct file_emitter {
  private:
    constexpr static auto fcloser = [](FILE *f) { fclose(f); };
    using file_ptr_type = std::unique_ptr<FILE, decltype(fcloser)>;
    file_ptr_type __f;
    fs::path __path;

    file_emitter(file_ptr_type f, fs::path p) : __f{std::move(f)}, __path{std::move(p)} {}

  public:
    std::expected<void, log_error> emit(std::string_view v) const {
      auto res = fwrite(v.data(), sizeof(char), v.length(), __f.get());
      if (res != v.length()) {
        return std::unexpected{
          log_error{log_error_type::io_error, "cannot write to file {}", __path.c_str()}
        };
      }
      return {};
    }

    const fs::path &path() const noexcept {
      return __path;
    }

    static std::expected<file_emitter, log_error> open(const fs::path &p, bool append) {
      FILE *f = fopen(p.c_str(), append ? "a" : "w");
      if (f == nullptr) {
        return std::unexpected{
          log_error{log_error_type::io_error, "cannot open file {}", p.c_str()}
        };
      }
      file_ptr_type ptr{f, fcloser};
      return file_emitter{std::move(ptr), p};
    }
  };

  export struct stdout_emitter {
    std::expected<void, log_error> emit(std::string_view d) const {
      fwrite(d.data(), sizeof(char), d.length(), stdout);
      return {};
    }
  };

  template struct emitter<file_emitter>;
  template struct emitter<stdout_emitter>;
  template struct emitter<empty_emitter>;
}