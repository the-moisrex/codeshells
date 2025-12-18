import standard;

#include <cstdint>
#include <print>
#include <vector>

using namespace std;

struct A{

    string one;
    string two;
    string three;
};

template <typename CharT>
[[nodiscard]] auto operator+(std::basic_string_view<CharT> str) {
    return std::basic_string<CharT>{str};
}

int main() {

    string_view const one = "one";
    string_view const two = "two";
    string_view const three = "three";


    vector<A> as;

    // this won't work
    // as.emplace_back(one, two, three);

    // this works:
    as.emplace_back(string{one}, string{two}, string{three});

    // But this one's is cute:
    as.emplace_back(+one, +two, +three);

    return 0;
}

// view ./build/optimizations.txt
