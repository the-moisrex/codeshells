import standard;

#include <cstdint>
#include <print>

// This is not the best polyfill; don't use this. Boost has one
namespace std {
  void breakpoint() noexcept {
    #ifdef _WIN32
      __debugbreak();
    #elif defined(__APPLE__)
      __builtin_trap();
    #elif defined(__GNUC__) || defined(__GNUG__)
      asm("int $3");
    #elif defined(__clang__)
      __builtin_debugtrap();
    #else
      // For other platforms, add appropriate code here
    #endif
  }

  bool is_debugger_present() noexcept {
    #ifdef _WIN32
      return IsDebuggerPresent();
    #else
      // For other platforms, return false as a default polyfill
      return false;
    #endif
  }

  void breakpoint_if_debugging() noexcept {
    if (is_debugger_present()) {
      breakpoint();
    }
  }

}

using namespace std;

static constexpr uint8_t worlds = 7;

int main() {

    std::breakpoint();
  println("Hello all {} worlds.", worlds);

  return 0;
}
