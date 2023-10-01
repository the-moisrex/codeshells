#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
#include <iterator>
template <typename T,
          typename NType = typename std::iterator_traits<T *>::difference_type>
[[nodiscard]] static constexpr auto next_ptr(T *ptr, NType n = 1) noexcept {
  return std::next(
      ptr, static_cast<typename std::iterator_traits<T *>::difference_type>(n));
}
template <typename NType = std::ptrdiff_t>
[[nodiscard]] static inline auto next_ptr(void const *ptr,
                                          NType n = 1) noexcept {
  return std::next(reinterpret_cast<std::intptr_t const *>(ptr),
                   static_cast<std::ptrdiff_t>(n));
}
template <typename NType = std::ptrdiff_t>
[[nodiscard]] static inline auto next_ptr(void *ptr, NType n = 1) noexcept {
  return std::next(reinterpret_cast<std::intptr_t *>(ptr),
                   static_cast<std::ptrdiff_t>(n));
}
#else
#include <stdint.h>
#define next_ptr(ptr, N)                                                       \
  _Generic((ptr),                                                              \
      void *: ((void *)((intptr_t *)ptr + N)),                                 \
      void const *: ((void const *)((intptr_t const *)ptr + N)))/*,               \
      default: ((ptr) + (N)))*/
#endif

#define typename(x)                                                            \
  _Generic(x, /* Get the name of a type */                                     \
      _Bool: "_Bool",                                                          \
      unsigned char: "unsigned char",                                          \
      char: "char",                                                            \
      signed char: "signed char",                                              \
      short int: "short int",                                                  \
      unsigned short int: "unsigned short int",                                \
      int: "int",                                                              \
      unsigned int: "unsigned int",                                            \
      long int: "long int",                                                    \
      unsigned long int: "unsigned long int",                                  \
      long long int: "long long int",                                          \
      unsigned long long int: "unsigned long long int",                        \
      float: "float",                                                          \
      double: "double",                                                        \
      long double: "long double",                                              \
      char *: "pointer to char",                                               \
      void *: "pointer to void",                                               \
      void const *: "pointer to const void",                                   \
      int *: "pointer to int",                                                 \
      default: "other")

#include <stdio.h>

int main(void) {
  int a = 10;
  void const *bc = &a;
  void *b = &a;

  void *d = next_ptr(b, 10);
  void const *dc = next_ptr(bc, 10);

  printf("%s\n", typename(b));
  printf("%s\n", typename(bc));
  printf("%s\n", typename(d));
  printf("%s\n", typename(dc));

  if ((d - (void *)&a) == (dc - (void const *)&a)) {
    printf("Correct\n");
  } else {
    printf("Wrong\n");
  }

  return 0;
}
