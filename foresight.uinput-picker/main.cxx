import standard;

#include <bit>
#include <cstdint>
#include <linux/input-event-codes.h>
#include <print>

using namespace std;

namespace test {
    struct event_code {
        int type;
        int code;
    };

    static constexpr std::uint16_t shift =
        std::bit_width<std::uint16_t>(KEY_MAX) - 1U;

    [[nodiscard]] static consteval std::uint16_t
    hash(event_code const event) noexcept {
        return static_cast<std::uint16_t>(event.type << shift) |
               static_cast<std::uint16_t>(event.code);
    }

    static constexpr std::uint16_t max_hash =
        hash({.type = EV_MAX, .code = KEY_MAX});
} // namespace test

int main() {

    println("shift: {}", test::shift);
    println("max hash: {:x}", test::max_hash);

    return 0;
}

// view ./build/optimizations.txt
