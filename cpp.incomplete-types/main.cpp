#include <concepts>
#include <iostream>
#include <memory>
#include <type_traits>

namespace stl {
using namespace std;
}

/// Constraint for a non-explicit constructor.
template <typename T, typename Arg>
concept implicitly_constructible =
    stl::constructible_from<T, Arg> && stl::convertible_to<Arg, T>;

/// Constraint for a non-explicit constructor.
template <typename T, typename Arg>
concept explicitly_constructible =
    stl::constructible_from<T, Arg> && !stl::convertible_to<Arg, T>;

namespace details {
struct do_is_implicitly_default_constructible_impl {
  template <typename T> static void helper([[maybe_unused]] T const &inp) {}

  template <typename T>
  static stl::true_type
  test([[maybe_unused]] T const &inp,
       [[maybe_unused]] decltype(helper<T const &>({})) *arg = nullptr) {
    return {};
  }

  static stl::false_type test(...) { // NOLINT(*-dcl50-cpp)
    return {};
  }
};

template <typename T>
struct is_implicitly_default_constructible_impl
    : do_is_implicitly_default_constructible_impl {
  using type = decltype(test(stl::declval<T>()));
};

template <typename T>
struct is_implicitly_default_constructible_safe
    : is_implicitly_default_constructible_impl<T>::type {};

} // namespace details

template <typename T>
concept implicitly_default_constructible =
    stl::is_default_constructible_v<T> &&
    details::is_implicitly_default_constructible_safe<T>::value;

template <typename T>
concept explicitly_default_constructible =
    stl::is_default_constructible_v<T> && !implicitly_default_constructible<T>;

struct incomplete_type;

using namespace std;

auto main() -> int {

  cout << boolalpha;
  cout << movable<incomplete_type> << endl;
  cout << copyable<incomplete_type> << endl;
  // cout << is_copy_constructible_v<incomplete_type> << endl;
  cout << constructible_from<incomplete_type, incomplete_type> << endl;
  cout << is_copy_assignable_v<incomplete_type> << endl;
  cout << is_copy_constructible_v<unique_ptr<incomplete_type>> << endl;
  cout << is_copy_assignable_v<unique_ptr<incomplete_type>> << endl;
  cout << movable<unique_ptr<incomplete_type>> << endl;
  cout << copyable<unique_ptr<incomplete_type>> << endl;
  cout << constructible_from<unique_ptr<incomplete_type>,
                             incomplete_type> << endl;

  cout << "----" << endl;
  // cout << implicitly_default_constructible<incomplete_type> << endl;
  cout << implicitly_default_constructible<unique_ptr<incomplete_type>> << endl;
  // cout << explicitly_default_constructible<incomplete_type> << endl;
  cout << explicitly_default_constructible<unique_ptr<incomplete_type>> << endl;
  return 0;
}
