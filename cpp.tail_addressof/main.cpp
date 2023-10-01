#include <cstdint>
#include <iostream>

using namespace std;

struct tail_type {
private:
  uint64_t resv1 = 0;
  uint16_t resv2 = 0;
  uint16_t resv3 = 0;
  uint16_t tail = 0;

public:
  constexpr tail_type() noexcept = default;
  constexpr tail_type(uint16_t inp_tail) noexcept : tail{inp_tail} {}
  constexpr tail_type(tail_type const &) noexcept = default;
  constexpr tail_type(tail_type &&) noexcept = default;
  constexpr tail_type &operator=(tail_type const &) noexcept = default;
  constexpr tail_type &operator=(tail_type &&) noexcept = default;
  constexpr tail_type &operator=(uint16_t inp_tail) noexcept {
    tail = inp_tail;
    return *this;
  }

  constexpr operator uint16_t() const noexcept { return tail; }

  friend constexpr void const *operator&(tail_type const &tail) noexcept {
    return &tail.tail;
  }
};
auto main() -> int {

  tail_type tail;

  cout << "addressof: " << addressof(tail) << endl;
  cout << "&tail:     " << &tail << endl;

  return 0;
}
