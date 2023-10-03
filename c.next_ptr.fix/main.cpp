#include <iostream>
#include <cstdint>

using namespace std;

#ifdef __cplusplus
#    include <cstdint>
#    include <iterator>
//// This is commented out because the C implementation's non-void-pointers implementation yields warning
//// when it's actually void(const)*.
// template <typename T,
//           typename NType = typename std::iterator_traits<T *>::difference_type>
// [[nodiscard]] static constexpr auto io_uring_next_void_ptr(T *ptr, NType n = 1) noexcept {
//     return std::next(
//       ptr, static_cast<typename std::iterator_traits<T *>::difference_type>(n));
// }
template <typename NType = std::ptrdiff_t>
[[nodiscard]] static inline auto io_uring_next_void_ptr(void const* ptr, NType n = 1) noexcept {
    return reinterpret_cast<void const*>(reinterpret_cast<intptr_t const>(ptr) + static_cast<std::ptrdiff_t>(n));
    // return std::next(reinterpret_cast<char const*>(ptr), static_cast<std::ptrdiff_t>(n));
}
template <typename NType = std::ptrdiff_t>
[[nodiscard]] static inline auto io_uring_next_void_ptr(void* ptr, NType n = 1) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<intptr_t>(ptr) + static_cast<std::ptrdiff_t>(n));
    // return std::next(reinterpret_cast<char*>(ptr), static_cast<std::ptrdiff_t>(n));
}
#else
#    include <stdint.h>
#    define io_uring_next_void_ptr(ptr, N)        \
        _Generic((ptr),                           \
          void*: ((void*) ((char*) ptr + N)), \
          void const*: ((void const*) ((char const*) ptr + N)))
#endif

extern "C" void* calcit(void* a) {
  return a + 10;
}

#include <iterator>
#include <cassert>
auto main() -> int {

  cout << "char: " << sizeof(char) << endl;
  cout << "char*: " << sizeof(char*) << endl;
  cout << "void*: " << sizeof(void*) << endl;
  cout << "unsigned int: " << sizeof(unsigned) << endl;

  void* a = (void*) 10;
  void* b = calcit(a);
  void* c = std::next((char*)a, 10);
  void* d = a;

  char* dd = (char*)a;
  std::advance(dd, 10);
  d = (void*)dd;
  a = io_uring_next_void_ptr(a, 10);
  if (a == (void*)20) {
    cout << "first: yes" << endl;
  } else {
    cout << "first: no " << (intptr_t)a << endl;
  }
  if(b == (void*)20) {
    cout << "second: yes" << endl;
  } else {
    cout << "second: no " << (intptr_t)b << endl;
  }

  if(c == (void*)20) {
    cout << "third: yes" << endl;
  } else {
    cout << "third: no " << (intptr_t)c << endl;
  }

  if(d == (void*)20) {
    cout << "forth: yes" << endl;
  } else {
    cout << "forth: no " << (intptr_t)d << endl;
  }


  return 0;
}
