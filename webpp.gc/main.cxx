import standard;

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <print>
#include <stdexcept>
#include <vector>

using namespace std;

struct garbage_collector {
  struct trash {
    void *ptr;
    size_t length;
  };

  std::vector<trash> trashes;

  void push(void *ptr, size_t sz) { trashes.emplace_back(ptr, sz); }

  void remove(void *inp_ptr) {
    for (auto it = trashes.begin(); it != trashes.end(); ++it) {
      auto [ptr, _] = *it;
      if (ptr == inp_ptr) {
        ::operator delete(ptr);
        trashes.erase(it);
        break;
      }
    }
  }

  void cleanup() {
    for (auto [ptr, length] : trashes) {
      println("Remove {} with length {}", ptr, length);
      ::operator delete(ptr);
    }
  }

  ~garbage_collector() { cleanup(); }
} trash_can;

struct CustomClass {
  CustomClass() { println("X::init"); }

  // custom placement new
  static void *operator new(std::size_t sz) {
    println("custom placement new called");
    auto *ptr = ::operator new(sz);
    trash_can.push(ptr, sz);
    return ptr;
  }

  // custom placement delete
  static void operator delete(void *ptr) {
    println("custom placement delete called");
    trash_can.remove(ptr);
    ::operator delete(ptr);
  }

  int get_value() { return 1; }
};

struct router {

  void operator()(auto &application) {
    // imagine a good router that sends this to the actual users :)
    println("{}", application());

    // clean up the trash
    trash_can.cleanup();
  }
};

/// This is what the developer's would write, kinda:
struct user_land {

  string operator()() {
    // unique_ptr is nice, but we live dangerously here :)
    // unique_ptr<X> x = make_unique<X>();
    auto *x = new CustomClass();
    return format("Hello all {} worlds out there.", x->get_value());
    // CustomClass would be garbage collected automagically
  }
};

int main() {

  router r;
  user_land app;

  r(app);
  r(app);
  r(app);

  return 0;
}
