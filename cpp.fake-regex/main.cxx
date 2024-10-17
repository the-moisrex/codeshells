import standard;

#include <cstdint>
#include <utility>
#include <algorithm>
#include <string_view>
#include <cctype>
#include <array>
#include <print>

template <std::size_t Len>
struct const_string_view : std::array<char, Len> {
    using arr_type = std::array<char, Len>;

    consteval const_string_view(char const (&str)[Len]) noexcept {
        std::copy(str, str + Len, this->begin());
    }

    using arr_type::operator[];
    constexpr std::size_t size() const noexcept {
        return arr_type::size() - 1;
    }
};

template <std::size_t Len>
const_string_view(char const[Len]) -> const_string_view<Len>;

template <const_string_view Format = "dddd-dd-dd">
constexpr auto for_all(auto func, std::string_view date) noexcept {
    return ([=]<std::size_t... I>(std::index_sequence<I...>) {
        if (date.size() < Format.size()) {
            return false;
        }
        return (func.template operator()<Format[I]>(date[I]) && ...);
    })(std::make_index_sequence<Format.size()>{});
}

constexpr bool verify_date_format(std::string_view date) noexcept {
    return for_all<"dddd-dd-dd">(
        []<char FMChar>(char cur) {
            if constexpr (FMChar == 'd') {
                if (!isdigit(cur)) {
                    return false;
                }
            } else if constexpr (FMChar == '-') {
                if (cur != '-') {
                    return false;
                }
            } else {
                std::unreachable();
            }
            return true;
        },
        date);
}

bool test(std::string_view date) {
    bool const res = verify_date_format(date);
    std::println("{}: {}", res, date);
    return res;
}

int main() {

    return static_cast<int>(
        test("1111-11-11") &&
        test("1110-21-31") &&
        test("111-21-31") &&
        true
    );
}

