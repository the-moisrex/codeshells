import standard;

#include <cstdint>
#include <print>

static constexpr long Inf = std::numeric_limits<long>::max();

struct StupidString {
    std::string_view str;

    auto operator[](long start, long end, long step) {
        std::string out;
        auto const safe_end = end > str.size() ? str.size() : end;
        out.reserve((safe_end - start) / step + 1);
        for (long index = start; index < safe_end; index += step) {
            out += str[index];
        }
        return out;
    }
};

StupidString operator"" _ssv(const char* const str, std::size_t length) {
    return StupidString{{str, length}};
}

// A dare from https://x.com/i_bogosavljevic/status/1864700256161554654
auto even_or_odd(long num) {
    // Yeah, C++ can do that too if we wanted to be stupid.
    return "eovdedn"_ssv[num % 2, Inf, 2];
}

int main() {

    std::println("1: {}", even_or_odd(1));
    std::println("20: {}", even_or_odd(20));
    std::println("35: {}", even_or_odd(35));

    return 0;
}
