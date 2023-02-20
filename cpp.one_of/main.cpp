#include <iostream>
#include <concepts>

using namespace std;
namespace stl {
using namespace std;
}

    /*
     * Get the last parameter type from a tuple-like type
     */
    template <typename... T>
    struct last_type {

        static_assert(sizeof...(T) > 0, "last_type won't work with empty type lists.");

        template <typename... FT>
        struct fake_tup {
            template <typename... L>
            using append = fake_tup<FT..., L...>;


            template <template <typename...> typename Tt, typename... Additionals>
            using replace_template = Tt<FT..., Additionals...>;

            static constexpr stl::size_t size = sizeof...(FT);
        };

        template <typename TagT, typename Tup = fake_tup<>, stl::size_t Limit = static_cast<stl::size_t>(-1)>
        struct tag {
            // last type
            using last = TagT;

            // the rest of the types
            using rest = Tup;

            // all of them
            using all = typename rest::template append<last>;

            static constexpr stl::size_t all_size = all::size;

            // use TagT as the last type
            template <typename Tag>
                requires(all_size < Limit)
            constexpr tag<typename Tag::last, all> operator|(Tag&&) const noexcept {
                return {};
            }

            template <typename Tag>
            constexpr tag operator|(Tag&&) const noexcept {
                return {};
            }
        };

        // last type
        using type = typename decltype((tag<T>{}, ...))::last;

        // all except last type (remove the last type)
        template <template <typename...> typename Tt>
        using remove = typename decltype((... | tag<T>{}))::rest::template replace_template<Tt>;

        // remove the last types ao there's only N types in the tuple
        template <template <typename...> typename Tt, stl::size_t N>
        using remove_limit =
          typename decltype((... | tag<T, fake_tup<>, N>{}))::all::template replace_template<Tt>;

        // remove the last tyoe if
        template <template <typename...> typename Tt, template <typename> typename Condition>
        using remove_if =
          stl::conditional_t<Condition<type>::value,
                             typename decltype((... | tag<T>{}))::rest::template replace_template<Tt>,
                             Tt<T...>>;

        // replace last type
        template <template <typename...> typename Tt, typename... Replacements>
        using replace =
          typename decltype((... | tag<T>{}))::rest::template replace_template<Tt, Replacements...>;

        // replace last type if
        template <template <typename...> typename Tt,
                  template <typename>
                  typename Condition,
                  typename... Replacements>
        using replace_if = stl::conditional_t<Condition<type>::value, replace<Tt, Replacements...>, Tt<T...>>;


        // replace if exists, add if it doesnt
        template <template <typename...> typename Tt,
                  template <typename>
                  typename Condition,
                  typename... Replacements>
        using put_if =
          stl::conditional_t<Condition<type>::value, replace<Tt, Replacements...>, Tt<T..., Replacements...>>;
    };

    template <typename... T>
    using last_type_t = typename last_type<T...>::type;

template <typename ...T>
struct fake_tuple {};

template <typename T, typename U>
struct is_one_of;

template <typename T, typename...U>
struct is_one_of<T, fake_tuple<U...>> {
    static constexpr bool value = (same_as<T, U> || ...);
};

/**
 * Same as "same_as" but check against multiple types.
 * @tparam T The type you want to check
 * @tparam U The types that will be checked against
 */
template <typename... T>
concept one_of = (sizeof...(T) > 1) &&
                 is_one_of<last_type_t<T...>, typename last_type<T...>::template remove<fake_tuple>>::value;

template <typename T>
void wow(T&& t) {
    if constexpr (requires (T t) {
        {t.get()} -> one_of<int, double>;
    }) {
      cout << t.get() << endl;
    } else if constexpr (
        requires (T t) {
            {t.get()} -> one_of<string>;
        })
    {
        cout << "String: " << t.get() << endl;
    }
}

template <typename T>
struct test {
    T t;

    T get() {
        return t;
    }
};
auto main() -> int {
    wow(test<int>(23));
    wow(test<string>("Hello world"));
  return 0;
}
