#include <iostream>
#include <numeric>
#include <ranges>
#include <vector>

int main() {
    
    std::cout << std::boolalpha;
    
    std::vector<int> vec;
    std::vector<int> vec2;
    
    for (int i: std::views::iota(0, 10)) vec.push_back(i);                      // (1)         
         
    for (int i: std::views::iota(0) | std::views::take(10)) vec2.push_back(i);  // (2)
    
    std::cout << "vec == vec2: " << (vec == vec2) << '\n'; // true
    
    for (int i: vec) std::cout << i << " ";                // 0 1 2 3 4 5 6 7 8 9
                           
}

