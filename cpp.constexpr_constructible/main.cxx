import standard;

#include <cstdint>
#include <print>
#include <iostream>

using namespace std;

// from: https://x.com/supahvee1234/status/1999899526883053921?s=20


// concept to check if type T has a constexpr constructor 
// and thus can be used in constexpr contexts, in another words, 
// the type T can be constructed in compiletime.
template<typename T>
concept constexpr_constructible =   // requires will just check if the expression is valid.

 requires { std::array<int, (T{}, 1)>{}; }; // I just used a lambda here, because c++ don't accept                                        // to initialize an object in a not allowed place 
                                            // but inside a lambda it will allow it. "return a;" is 
                                            // just to make sure the object is used and avoid the
                                            // compile to think it can be discarded.

template<constexpr_constructible T>
void showValue(T obj) 
{
    std::cout << "Value of obj: " << obj.get() << std::endl;
}


class Teste 
{
public:
     Teste() { }
    Teste(int n) : data{n} {}
    
    int get() {
        return data;
    }

private:
    int data{};
};

static_assert(constexpr_constructible<int>);
static_assert(!constexpr_constructible<Teste>);




int main() {
    return 0;
}

// view ./build/optimizations.txt
