#include "/data/data/com.termux/files/home/webpp/core/include/webpp/std/type_traits.hpp"
#include <concepts>
#include <iostream>
#include <tuple>
#include <type_traits>

using namespace std;
using namespace webpp;
using namespace webpp::istl;

template <std::size_t N>
struct size_holder : std::integral_constant<std::size_t, N> {};

template <typename T> struct is_size_holder : std::false_type {
  static constexpr std::size_t size = 0;
};

template <std::size_t N>
struct is_size_holder<size_holder<N>> : std::true_type {
  static constexpr std::size_t size = N;
};

template <typename... T>
struct mituple : last_type<T...>::template remove_if<tuple, is_size_holder> {
  using last = last_type<T...>;
  using this_tuple =
      typename last_type<T...>::template remove_if<tuple, is_size_holder>;
  static constexpr std::size_t native_tuple_size =
      std::tuple_size_v<this_tuple>;
  static constexpr std::size_t tuple_size =
      native_tuple_size + is_size_holder<typename last::type>::size;

  // using typename last_type<T...>::template remove<tuple>::tuple;

  template <typename... Args>
  constexpr mituple(Args &&...args) : this_tuple{std::forward<Args>(args)...} {}

  constexpr this_tuple as_tuple() const noexcept {
    return *static_cast<this_tuple const *>(this);
  }

  template <std::size_t NewSize>
  [[nodiscard]] auto structured() const noexcept {
    if constexpr (NewSize > native_tuple_size) {
      return typename last_type<T...>::template put_if<
          mituple, is_size_holder, size_holder<NewSize - native_tuple_size>>{
          as_tuple()};
    } else if constexpr (NewSize < native_tuple_size) {
      // todo
    } else {
      return *this;
    }
  }

  template <std::size_t I> auto get() const noexcept {
    if constexpr (I >= native_tuple_size) {
      return nothing_type{};
    } else {
      return std::get<I>(as_tuple());
    }
  }
};

template <std::size_t I, class... T>
requires(
    I < mituple<T...>::native_tuple_size) struct tuple_element<I, mituple<T...>>
    : tuple_element<I, typename mituple<T...>::this_tuple> {
};

template <std::size_t I, class... T>
requires(
    I >=
    mituple<T...>::native_tuple_size) struct tuple_element<I, mituple<T...>> {
  using type = nothing_type;
};

template <class... T>
struct tuple_size<mituple<T...>>
    : std::integral_constant<std::size_t, mituple<T...>::tuple_size> {};

auto main() -> int {

  auto const tup = mituple<int, double>{1.3, 1.1};
  auto const tup3 = tup.structured<3>();
  auto const [mi_int, mi_double, mi_nothing] = tup3;

  static_assert(is_same_v<int, remove_cvref_t<decltype(mi_int)>>,
                "it should be int");
  static_assert(is_same_v<double, remove_cvref_t<decltype(mi_double)>>,
                "it should be double");
  static_assert(is_same_v<nothing_type, remove_cvref_t<decltype(mi_nothing)>>,
                "it should be nothing");

  cout << get<0>(tup) << endl;
  cout << get<1>(tup) << endl;
  cout << get<1>(tup3) << endl;
  cout << get<1>(tup3) << endl;
  cout << get<0>(tup3) << endl;
  cout << get<0>(tup3) << endl;
  cout << mi_int << endl << mi_double << endl;
  cout << mi_int << endl << mi_double << endl;

  return 0;
}
