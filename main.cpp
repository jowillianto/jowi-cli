#include <array>
#include <bitset>
#include <expected>
#include <print>
#include <string_view>

enum struct utf8_error { invalid_byte };

struct utf8_char {
private:
  const char *__beg;
  const char *__end;
  utf8_char(const char *beg, const char *end) : __beg{beg}, __end{end} {}
  static std::optional<std::string_view> __check_empty(std::string_view v) {
    if (v.length() == 0) {
      return std::nullopt;
    } else {
      return v;
    }
  }

  /*
    Requires that the length of v >= char_size
  */
  static std::expected<std::pair<utf8_char, std::optional<std::string_view>>, utf8_error>
  __make_iter_pair(std::string_view v, size_t char_size) {
    utf8_char c{v.begin(), v.begin() + char_size};
    for (auto i = c.begin() + 1; i != c.end(); i += 1) {
      if (!is_utf8_cont(*i)) {
        return std::unexpected{utf8_error::invalid_byte};
      }
    }
    return std::pair{c, __check_empty({v.begin() + char_size, v.end()})};
  }

public:
  static std::expected<utf8_char, utf8_error> decode(std::string_view v) {
    return iterative_decode(v).transform([](auto &&p) { return std::move(p.first); });
  }
  static std::expected<std::pair<utf8_char, std::optional<std::string_view>>, utf8_error>
  iterative_decode(std::string_view v) {
    // read first byte
    if (v.length() == 0) {
      return std::unexpected{utf8_error::invalid_byte};
    }
    unsigned char first_b = v[0];
    if ((first_b >> 7) == 0x0) {
      return __make_iter_pair(v, 1);
    } else if ((first_b >> 5) == 0x06 && v.length() >= 2) {
      return __make_iter_pair(v, 2);
    } else if ((first_b >> 4) == 0x0E && v.length() >= 3) {
      return __make_iter_pair(v, 3);
    } else if ((first_b >> 3) == 0xF0 && v.length() >= 4) {
      return __make_iter_pair(v, 4);
    } else {
      return std::unexpected{utf8_error::invalid_byte};
    }
  }
  static bool is_utf8_cont(unsigned char cont) {
    return (cont >> 6) == 0x2;
  }
  // Bitwise iteration
  const char *begin() const noexcept {
    return __beg;
  }
  const char *end() const noexcept {
    return __end;
  }
  std::string_view to_string_view() const noexcept {
    return std::string_view{begin(), end()};
  }
  size_t size() const noexcept {
    return __end - __beg;
  }
};

struct utf8_reader {};

int main() {
  // char v[] = "한국어 ㅎㅎㅎ";
  // std::optional<std::string_view> initial_utf8{v};
  // size_t i = 0;
  // while (initial_utf8) {
  //   auto res = utf8_char::iterative_decode(initial_utf8.value());
  //   if (!res) {
  //     std::println("illegal char at {}", initial_utf8.value());
  //     return 1;
  //   }
  //   std::println("{}: {}", i, res.value().first.view());
  //   initial_utf8 = std::move(res.value().second);
  //   i += 1;
  // }
  char raw_str[] = "이것은 UTF-8기반 글입니다!";
  auto str = utf8_view::decode("이것은 UTF-8기반 글입니다!").value();
  std::println("UTF-8");
  std::println("RAW BYTES");
  for (char c : raw_str) {
    std::println("{}", c);
  }
}