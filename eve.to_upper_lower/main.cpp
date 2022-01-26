#include <eve/algo/as_range.hpp>
#include <eve/algo/equal.hpp>
#include <eve/algo/transform.hpp>
#include <eve/function/add.hpp>
#include <eve/function/sub.hpp>
#include <iostream>

using namespace std;

inline void eve_to_lower(auto &str) noexcept {
  using value_type = typename remove_cvref_t<decltype(str)>::value_type;
  using char_type = make_unsigned_t<value_type>;
  static constexpr char_type alphabet_length = 'z' - 'a';
  static constexpr char_type a_A_offset = 'a' - 'A';
  auto start = reinterpret_cast<char_type *>(str.data());
  const auto finish = start + str.size();
  eve::algo::transform_inplace(eve::algo::as_range(start, finish), [](auto c) {
    return eve::add[(c - 'A') <= alphabet_length](c, a_A_offset);
  });
}

inline void eve_to_upper(auto &str) noexcept {
  using value_type = typename remove_cvref_t<decltype(str)>::value_type;
  using char_type = make_unsigned_t<value_type>;
  static constexpr char_type alphabet_length = 'z' - 'a';
  static constexpr char_type a_A_offset = 'a' - 'A';
  auto start = reinterpret_cast<char_type *>(str.data());
  const auto finish = start + str.size();
  eve::algo::transform_inplace(eve::algo::as_range(start, finish), [](auto c) {
    return eve::sub[(c - 'a') <= alphabet_length](c, a_A_offset);
  });
}

auto main() -> int {

  std::wstring str = L"Hello World!";
  eve_to_upper(str);
  wcout << str << endl;
  eve_to_lower(str);
  wcout << str << endl;

  return 0;
}
