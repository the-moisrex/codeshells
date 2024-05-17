import standard;
#include <cstdint>
#include <print>

using namespace std;

struct alignas(uint32_t) decomp_index {
  enum struct compatibility_type {
    canonical = 0,
    compatiblity = 1,
  } type : 1;

  uint16_t index : 15;
  uint8_t mask;
  uint8_t shift;

  consteval decomp_index(std::uint32_t val) noexcept
      : type{static_cast<compatibility_type>((val >> 31) & 0b1)},
        index{static_cast<std::uint16_t>((val >> 16U) & (0xFF >> 1U))},
        mask{static_cast<std::uint8_t>(val >> 8U)},
        shift{static_cast<std::uint8_t>(val)} {}
};

int main() {

  decomp_index one{0x0F'FF'00};

  println("Size: {}", sizeof(decomp_index));
  println("index: {}", static_cast<int>(one.index));
  println("mask: {}", one.mask);
  println("shift: {}", one.shift);

  return 0;
}
