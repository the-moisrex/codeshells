import standard;

#include <cstdint>
#include <print>

using namespace std;

int main() {

    using flag_type = std::uint8_t;
    static constexpr flag_type dot_flag = 0b100'0000U;
    static constexpr flag_type x_flag = 0b1U;
    static constexpr flag_type n_flag = 0b10U;
    static constexpr flag_type dash_flag = 0b100U;
    static constexpr flag_type ascii_flag = 0b1000U;
    static constexpr auto xnd_flag =
        x_flag | n_flag | dash_flag | ascii_flag;
    static constexpr flag_type clean_flag = static_cast<flag_type>(~dot_flag);

    static_assert(same_as<decltype(xnd_flag), const int>, "God damn it");
    println("xnd:   {}", xnd_flag);
    println("clean: {}", clean_flag);

    return 0;
}

// view ./build/optimizations.txt
