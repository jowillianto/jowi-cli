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
  concept is_emitter = requires(const T Emitter, std::string_view data) {
    { Emitter.emit(data) } -> std::same_as<std::expected<void, LogError>>;
  };

  export template <class T>
  concept is_stream_emitter = requires(T Emitter, char x) {
    { Emitter.push_back(x) } -> std::same_as<void>;
    { Emitter.flush() } -> std::same_as<std::expected<void, LogError>>;
  };

  export template <class T = void> struct StreamEmitter;
  export template <class T = void> struct Emitter;

  template <> struct Emitter<void> {
    virtual std::expected<void, LogError> emit(std::string_view) const = 0;
    virtual ~Emitter() = default;

    template <is_emitter EmitterType, class... Args>
      requires(std::constructible_from<EmitterType, Args...>)
    static std::unique_ptr<Emitter<void>> make_unique(Args &&...args) {
      return std::make_unique<Emitter<EmitterType>>(std::forward<Args>(args)...);
    }

    template <is_emitter EmitterType, class... Args>
      requires(std::constructible_from<EmitterType, Args...>)
    static std::shared_ptr<Emitter<void>> make_shared(Args &&...args) {
      return std::make_shared<Emitter<EmitterType>>(std::forward<Args>(args)...);
    }
  };

  export template <is_emitter T> struct Emitter<T> : private T, public Emitter<void> {
    using T::T;

    Emitter(T &&v)
      requires(std::move_constructible<T>)
      : T{std::forward<T &&>(v)} {}

    std::expected<void, LogError> emit(std::string_view d) const override {
      return T::emit(d);
    }
  };
  template <> struct StreamEmitter<void> {
    using value_type = char;
    virtual void push_back(char x) = 0;
    virtual std::expected<void, LogError> flush() = 0;
    virtual ~StreamEmitter() = default;

    template <is_stream_emitter EmitterType, class... Args>
      requires(std::constructible_from<EmitterType, Args...>)
    std::unique_ptr<StreamEmitter<void>> make_unique(Args &&...args) {
      return std::make_unique<StreamEmitter<EmitterType>>(std::forward<Args>(args)...);
    }

    template <is_stream_emitter EmitterType, class... Args>
      requires(std::constructible_from<EmitterType, Args...>)
    std::shared_ptr<StreamEmitter<void>> make_shared(Args &&...args) {
      return std::make_shared<StreamEmitter<EmitterType>>(std::forward<Args>(args)...);
    }
  };

  export template <is_stream_emitter T>
  struct StreamEmitter<T> : private T, public StreamEmitter<void> {
    using T::T;

    StreamEmitter(T &&v)
      requires(std::move_constructible<T>)
      : T{std::forward<T &&>(v)} {}

    void push_back(char x) override {
      T::push_back(x);
    }

    std::expected<void, LogError> flush() override {
      return T::flush();
    }
  };

  export struct SsEmitter {
  private:
    std::unique_ptr<Emitter<void>> __emt;
    std::string __buf;
    uint64_t __cur;

  public:
    SsEmitter(uint64_t buf_size, is_emitter auto &&emt) :
      __emt{Emitter<>::make_unique<std::decay_t<decltype(emt)>>(std::forward<decltype(emt)>(emt))},
      __buf{}, __cur{0} {
      __buf.reserve(buf_size);
      for (uint64_t i = 0; i < buf_size; i += 1)
        __buf.push_back('\0');
    }
    SsEmitter(is_emitter auto &&emt) : SsEmitter(120, std::forward<decltype(emt)>(emt)) {}
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
    std::expected<void, LogError> flush() noexcept(
      noexcept(__emt->emit(std::declval<std::string_view>()))
    ) {
      return __emt->emit(std::string_view{__buf.begin(), __buf.begin() + __cur}).transform([&]() {
        __cur = 0;
      });
    }
  };

  export struct EmptyEmitter {
    std::expected<void, LogError> emit(std::string_view data) const {
      return {};
    }
  };

  export struct FileEmitter {
  private:
    constexpr static auto fcloser = [](FILE *f) { fclose(f); };
    using FilePtrType = std::unique_ptr<FILE, decltype(fcloser)>;
    FilePtrType __f;
    fs::path __path;

    FileEmitter(FilePtrType f, fs::path p) : __f{std::move(f)}, __path{std::move(p)} {}

  public:
    std::expected<void, LogError> emit(std::string_view v) const {
      auto res = fwrite(v.data(), sizeof(char), v.length(), __f.get());
      if (res != v.length()) {
        return std::unexpected{
          LogError{LogErrorType::io_error, "cannot write to file {}", __path.c_str()}
        };
      }
      return {};
    }

    const fs::path &path() const noexcept {
      return __path;
    }

    static std::expected<FileEmitter, LogError> open(const fs::path &p, bool append) {
      FILE *f = fopen(p.c_str(), append ? "a" : "w");
      if (f == nullptr) {
        return std::unexpected{LogError{LogErrorType::io_error, "cannot open file {}", p.c_str()}};
      }
      FilePtrType ptr{f, fcloser};
      return FileEmitter{std::move(ptr), p};
    }
  };

  export struct StdoutEmitter {
    std::expected<void, LogError> emit(std::string_view d) const {
      fwrite(d.data(), sizeof(char), d.length(), stdout);
      return {};
    }
  };

  template struct Emitter<FileEmitter>;
  template struct Emitter<StdoutEmitter>;
  template struct Emitter<EmptyEmitter>;
}
