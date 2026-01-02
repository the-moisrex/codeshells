import standard;

#include <cstdint>
#include <map>
#include <print>
#include <string>

using namespace std;

////////////////////////////// [[lifetimebound]] //////////////////////////////
#ifndef __has_cpp_attribute
#define webpp_lifetimebound
#elif __has_cpp_attribute(msvc::lifetimebound)
#define webpp_lifetimebound [[msvc::lifetimebound]]
#elif __has_cpp_attribute(clang::lifetimebound)
#define webpp_lifetimebound [[clang::lifetimebound]]
#elif __has_cpp_attribute(lifetimebound)
#define webpp_lifetimebound [[lifetimebound]]
#else
#define webpp_lifetimebound
#endif


struct test {
    std::string str;

    std::string_view view() const webpp_lifetimebound {
        return std::string_view{str.data(), str.size()};
    }
};

int main() {
    string_view str = "";
    {
        test t;
        t.str = "nice";
        str = t.view();
    }
    println("str: {}", str);

    return 0;
}

