module;
#include <concepts>
#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
export module jowi.crogger:emitter;
import :error;

namespace jowi::crogger {
  namespace fs = std::filesystem;
  export template <class T>
  concept IsEmitter = requires(const T Emitter, std::string_view data) {
    { Emitter.emit(data) } -> std::same_as<std::expected<void, LogError>>;
  };

  export template <class T = void> struct Emitter;

  template <> struct Emitter<void> {
    virtual std::expected<void, LogError> emit(std::string_view) const = 0;
    virtual ~Emitter() = default;

    template <IsEmitter EmitterType, class... Args>
      requires(std::constructible_from<EmitterType, Args...>)
    static std::unique_ptr<Emitter<void>> make_unique(Args &&...args) {
      return std::make_unique<Emitter<EmitterType>>(std::forward<Args>(args)...);
    }

    template <IsEmitter EmitterType, class... Args>
      requires(std::constructible_from<EmitterType, Args...>)
    static std::shared_ptr<Emitter<void>> make_shared(Args &&...args) {
      return std::make_shared<Emitter<EmitterType>>(std::forward<Args>(args)...);
    }
  };

  export template <IsEmitter T> struct Emitter<T> : private T, public Emitter<void> {
    using T::T;

    Emitter(T &&v)
      requires(std::move_constructible<T>)
      : T{std::forward<T &&>(v)} {}

    std::expected<void, LogError> emit(std::string_view d) const override {
      return T::emit(d);
    }
  };

  export struct EmptyEmitter {
    std::expected<void, LogError> emit(std::string_view data) const {
      return {};
    }
  };

  struct FileCloser {
    void operator()(std::FILE *f) {
      if (f != nullptr) {
        std::fclose(f);
      }
    }
  };

  export struct FileEmitter {
  private:
    using FilePtrType = std::unique_ptr<FILE, FileCloser>;
    FilePtrType __f;
    fs::path __path;

    FileEmitter(FilePtrType f, fs::path p) : __f{std::move(f)}, __path{std::move(p)} {}

  public:
    std::expected<void, LogError> emit(std::string_view v) const {
      auto res = fwrite(v.data(), sizeof(char), v.length(), __f.get());
      if (res != v.length()) {
        return std::unexpected{LogError::io_error("cannot write to file {}", __path.c_str())};
      }
      return {};
    }

    const fs::path &path() const noexcept {
      return __path;
    }

    static std::expected<FileEmitter, LogError> open(const fs::path &p, bool append) {
      FILE *f = fopen(p.c_str(), append ? "a" : "w");
      if (f == nullptr) {
        return std::unexpected{LogError::io_error("cannot open file {}", p.c_str())};
      }
      FilePtrType ptr{f, FileCloser{}};
      return FileEmitter{std::move(ptr), p};
    }
  };

  export struct StdoutEmitter {
    std::expected<void, LogError> emit(std::string_view d) const noexcept {
      fwrite(d.data(), sizeof(char), d.length(), stdout);
      return {};
    }
  };

  export struct StderrEmitter {
    std::expected<void, LogError> emit(std::string_view d) const noexcept {
      fwrite(d.data(), sizeof(char), d.length(), stderr);
      return {};
    }
  };

  template struct Emitter<FileEmitter>;
  template struct Emitter<StdoutEmitter>;
  template struct Emitter<StderrEmitter>;
  template struct Emitter<EmptyEmitter>;
}
