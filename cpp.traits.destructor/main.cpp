#include <type_traits>

template <class T>
concept integral = std::is_integral_v<T>;

struct parser {

  virtual ~parser() requires(std::is_same_v<decltype(*this), parser> || requires {
    { this->get_int() } -> integral;
  }) = default;
};

struct new_parser : parser {
  auto get_int() -> int { return 20; }
};

auto main() -> int { return 0; }
