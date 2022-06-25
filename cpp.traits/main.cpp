#include <type_traits>

template <class T>
concept integral = std::is_integral_v<T>;

struct parser {

  template <typename T>
  requires requires(T self) {
    { self.get_int() } -> integral;
  }
  using traits = parser;
};

struct new_parser : parser {
  auto get_int() -> int { return 20; }
  new_parser() : parser::traits<new_parser>{} {}
};

auto main() -> int {
  return 0;
}
