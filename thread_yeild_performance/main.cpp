#include <iostream>
#include <thread>
#include <chrono>

using namespace std;


void little_sleep(std::chrono::microseconds us)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;
    do {
        std::this_thread::yield();
    } while (std::chrono::high_resolution_clock::now() < end);
}
 
int main()
{
    auto start = std::chrono::high_resolution_clock::now();
 
    little_sleep(std::chrono::microseconds(10000000));
 
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "waited for "
              << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()
              << " microseconds\n";
}

