import standard;

#include <cstdint>
#include <print>

using namespace std;

template <std::size_t Len>
struct json_path_view {
    static_assert(Len != 0, "Empty Strings are not allowed.");

  private:
    // it's impossible to have more segments than half of the length of the
    // string
    std::array<std::string_view, Len / 2> segments{};
    std::size_t length = 0;

  public:
    consteval json_path_view(json_path_view const&) = default;
    consteval json_path_view(json_path_view&&) = default;

    consteval explicit(false) json_path_view(const char (&str)[Len]) {
        auto* it = str;
        auto* beg = str;
        auto* const end = it + Len;
        for (;;) {
            if (it == end || *it == '.') {
                if (it - beg == 0) {
                    throw std::invalid_argument(
                        "Empty segments are not allowed.");
                }
                segments[length++] = std::string_view{beg, it};
                if (it == end) {
                    break;
                }
                beg = it + 1;
            }
            ++it;
        }
    }

    [[nodiscard]] constexpr auto begin() const noexcept {
        return segments.begin();
    }

    [[nodiscard]] constexpr auto end() const noexcept {
        return segments.begin() + length;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return length; }

    [[nodiscard]] constexpr bool is_multi_segment() const noexcept {
        return size() == 1;
    }

    [[nodiscard]] constexpr std::string_view front() const noexcept {
        return segments[0];
    }
};

int main() {

    println("segments: {}", json_path_view{"one.two.three"}.size());

    return 0;
}
