#include <utility>

// source: https://godbolt.org/z/6oxS3d
// info: https://twitter.com/hankadusikova/status/1276828584179642368
// article: https://thephd.dev/freestanding-noexcept-allocators-vector-memory-hole#make-it-noexcept

#ifdef _MSC_VER
#define INVOKE_NOEXCEPT_ALWAYS_INLINE __forceinline
#else
#define INVOKE_NOEXCEPT_ALWAYS_INLINE __attribute__((always_inline))
#endif

template <typename Fnc> struct noexcept_call_helper;

template <typename Ret, typename... Args>
struct noexcept_call_helper<Ret (&)(Args...)> {
  using type = Ret (&)(Args...) noexcept;
};

template <typename Ret, typename... Args>
struct noexcept_call_helper<Ret (&)(Args...) noexcept> {
  using type = Ret (&)(Args...) noexcept;
};

template <typename Ret, typename... Args>
struct noexcept_call_helper<Ret (*)(Args...)> {
  using type = Ret (*)(Args...) noexcept;
};

template <typename Ret, typename... Args>
struct noexcept_call_helper<Ret (*)(Args...) noexcept> {
  using type = Ret (*)(Args...) noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...)> {
  using type = Ret (T::*)(Args...) noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) noexcept> {
  using type = Ret (T::*)(Args...) noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) const> {
  using type = Ret (T::*)(Args...) const noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) const noexcept> {
  using type = Ret (T::*)(Args...) const noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) &> {
  using type = Ret (T::*)(Args...) & noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) & noexcept> {
  using type = Ret (T::*)(Args...) & noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) const &> {
  using type = Ret (T::*)(Args...) const & noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) const & noexcept> {
  using type = Ret (T::*)(Args...) const & noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) &&> {
  using type = Ret (T::*)(Args...) && noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) && noexcept> {
  using type = Ret (T::*)(Args...) && noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) const &&> {
  using type = Ret (T::*)(Args...) const && noexcept;
};

template <typename Ret, typename T, typename... Args>
struct noexcept_call_helper<Ret (T::*)(Args...) const && noexcept> {
  using type = Ret (T::*)(Args...) const && noexcept;
};

template <typename T, typename F> struct detect_which_function_to_call {
  template <typename Y>
  static auto test(Y &&)
      -> decltype(static_cast<F>(&Y::operator()), std::true_type{});
  static std::false_type test(...);
  static constexpr bool value =
      decltype(test(std::declval<std::remove_reference_t<T>>()))::value;
};

template <typename T> static constexpr bool dep_false = false;
template <typename> struct identify;

template <typename Fnc, typename... Args>
INVOKE_NOEXCEPT_ALWAYS_INLINE static constexpr decltype(auto)
invoke_noexcept(Fnc &&fnc, Args &&...args) noexcept
  requires std::is_invocable_v<Fnc, Args...>
{
  // this function exists only if we can be sure Fnc is a callable
  if constexpr (noexcept(std::forward<Fnc>(fnc)(std::forward<Args>(args)...))) {
    // it's already noexcept
    return std::forward<Fnc>(fnc)(std::forward<Args>(args)...);
  } else if constexpr (std::is_class_v<std::remove_reference_t<Fnc>>) {
    // it's a callable object

    using return_type =
        decltype(std::forward<Fnc>(fnc)(std::forward<Args>(args)...));

    using type = std::remove_reference_t<Fnc>;
    using function_ptr_lval = return_type (type::*)(Args...) &;
    using function_const_ptr_lval = return_type (type::*)(Args...) const &;
    using function_ptr_rval = return_type (type::*)(Args...) &&;
    using function_const_ptr_rval = return_type (type::*)(Args...) const &&;
    using function_ptr = return_type (type::*)(Args...);
    using function_const_ptr = return_type (type::*)(Args...) const;

    if constexpr ((std::is_rvalue_reference_v<Fnc> ||
                   !std::is_lvalue_reference_v<Fnc>)&&!std::is_const_v<type> &&
                  detect_which_function_to_call<Fnc,
                                                function_ptr_rval>::value) {

      function_ptr_rval f = &type::operator();

      auto noexcept_f =
          reinterpret_cast<typename noexcept_call_helper<decltype(f)>::type>(f);
      return (std::forward<Fnc>(fnc).*noexcept_f)(std::forward<Args>(args)...);

    } else if constexpr (
        (std::is_rvalue_reference_v<Fnc> ||
         !std::is_lvalue_reference_v<
             Fnc>)&&detect_which_function_to_call<Fnc,
                                                  function_const_ptr_rval>::
            value) {

      function_const_ptr_rval f = &type::operator();

      auto noexcept_f =
          reinterpret_cast<typename noexcept_call_helper<decltype(f)>::type>(f);
      return (std::forward<Fnc>(fnc).*noexcept_f)(std::forward<Args>(args)...);

    } else if constexpr (!std::is_const_v<type> &&
                         detect_which_function_to_call<
                             Fnc, function_ptr_lval>::value) {

      function_ptr_lval f = &type::operator();

      auto noexcept_f =
          reinterpret_cast<typename noexcept_call_helper<decltype(f)>::type>(f);
      return (std::forward<Fnc>(fnc).*noexcept_f)(std::forward<Args>(args)...);

    } else if constexpr (detect_which_function_to_call<
                             Fnc, function_const_ptr_lval>::value) {

      function_const_ptr_lval f = &type::operator();

      auto noexcept_f =
          reinterpret_cast<typename noexcept_call_helper<decltype(f)>::type>(f);
      return (std::forward<Fnc>(fnc).*noexcept_f)(std::forward<Args>(args)...);

    } else if constexpr (!std::is_const_v<type> &&
                         detect_which_function_to_call<Fnc,
                                                       function_ptr>::value) {

      function_ptr f = &type::operator();

      auto noexcept_f =
          reinterpret_cast<typename noexcept_call_helper<decltype(f)>::type>(f);
      return (std::forward<Fnc>(fnc).*noexcept_f)(std::forward<Args>(args)...);

    } else if constexpr (detect_which_function_to_call<
                             Fnc, function_const_ptr>::value) {

      function_const_ptr f = &type::operator();

      auto noexcept_f =
          reinterpret_cast<typename noexcept_call_helper<decltype(f)>::type>(f);
      return (std::forward<Fnc>(fnc).*noexcept_f)(std::forward<Args>(args)...);

    } else {
      static_assert(dep_false<Fnc>);
    }

  } else {
    // it's a pointer or a reference to a function
    return reinterpret_cast<typename noexcept_call_helper<Fnc>::type>(
        std::forward<Fnc>(fnc))(std::forward<Args>(args)...);
  }
}

extern "C" int hello();

struct my_type {
  int operator()(int) const & { return 1; }
  int operator()(int, int) const & { return 2; }
  int operator()(int) const && { return 3; }
  int operator()(int, int) const && { return 4; }
  int operator()(int) & { return 5; }
  int operator()(int, int) & { return 6; }
  int operator()(int) && { return 7; }
  int operator()(int, int) && { return 8; }
};

struct my_second_type {
  int operator()(int v) { return v; }
  int operator()(int v) const { return v + 1; }
};

int standalone_function(int a) { return a + 2; }

int main() {
  my_type a{};
  const my_type b{};

  int result = 0;

  result += invoke_noexcept(b, 1);
  result += invoke_noexcept(b, 1, 2);
  result += invoke_noexcept(std::move(b), 1);
  result += invoke_noexcept(std::move(b), 1, 2);
  result += invoke_noexcept(a, 1);
  result += invoke_noexcept(a, 1, 2);
  result += invoke_noexcept(std::move(a), 1);
  result += invoke_noexcept(std::move(a), 1, 2);

  result += invoke_noexcept([](auto a) noexcept { return a; }, 9);

  result += invoke_noexcept([](auto a) { return a; }, 10);
  result += invoke_noexcept(my_second_type{}, 11);
  const my_second_type x;
  result += invoke_noexcept(x, 11);
  result += invoke_noexcept(standalone_function, 11);

  return result; // should be 91
}
