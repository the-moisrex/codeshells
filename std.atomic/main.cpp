#include <array>
#include <atomic>
#include <iostream>
#include <thread>

std::atomic<long long> data{10};
std::array<long long, 5> return_values{};

void do_work(int thread_num) {
  long long val = data.fetch_add(1, std::memory_order_relaxed);
  return_values[thread_num] = val;
}

int main() {
  {
    std::thread th0{do_work, 0};
    std::thread th1{do_work, 1};
    std::thread th2{do_work, 2};
    std::thread th3{do_work, 3};
    std::thread th4{do_work, 4};
  }

  std::cout << "Result : " << data << '\n';

  for (long long val : return_values) {
    std::cout << "Seen return value : " << val << std::endl;
  }
}
