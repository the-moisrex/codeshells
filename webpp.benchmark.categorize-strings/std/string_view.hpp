// Created by moisrex on 4/23/20.

#ifndef WEBPP_STD_STRING_VIEW_HPP
#define WEBPP_STD_STRING_VIEW_HPP

#include "string_concepts.hpp"

#include <string_view>

namespace webpp::istl {


    template <typename T>
    concept StringView = requires(stl::remove_cvref_t<T> str) {
        typename stl::remove_cvref_t<T>::value_type;
        requires requires(typename stl::remove_cvref_t<T>::value_type a_char) {
            // { T{"str"} };
            str.empty();
            str.at(0);
            str.data();
            str.size();
            str.remove_suffix(1);
            str.remove_prefix(1);
            str.starts_with(a_char);
            str.ends_with(a_char);
            str.substr(a_char);
            str.begin();
            str.end();
            str.cbegin();
            str.cend();
            stl::remove_cvref_t<T>::npos;
        };
    } && !requires(stl::remove_cvref_t<T> str) {
        // the things that std::string has
        str.clear();
        str.shrink_to_fit();
        str.capacity();

        // NOLINTNEXTLINE(*-avoid-c-arrays)
        requires requires(typename stl::remove_cvref_t<T>::value_type const char_str[3]) {
            {
                str = char_str
            };
        };
    };

    namespace details::string_view {
        /**
         * Due to a GCC bug in 10.2.0, we're doing this to deduce the template type, because GCC doesn't
         * seem to be able to deduce a template type in a concept, but it can do it from here.
         */
        template <template <typename...> typename TT, typename... T>
        using deduced_type = decltype(TT{stl::declval<T>()...});

    } // namespace details::string_view

    /// Get the basic_string_view<...> type based on std::basic_string<...> template parameters
    template <typename T>
    using string_view_type_of = stl::
      conditional_t<StringView<T>, T, stl::basic_string_view<char_type_of_t<T>, char_traits_type_of_t<T>>>;

    namespace details {
        template <typename StrViewType, typename T>
        concept StringViewifiableOf =
          !stl::is_void_v<StrViewType> && !stl::is_void_v<char_type_of_t<T>> && requires(T str) {
              typename StrViewType::value_type;
              requires stl::is_trivial_v<typename StrViewType::value_type>;
              requires stl::is_standard_layout_v<typename StrViewType::value_type>;
              requires !stl::is_pointer_v<typename StrViewType::value_type>; // exclude array<char const*, N>
              requires requires { StrViewType{str}; } || requires {
                  str.data();
                  str.size();
                  StrViewType{str.data(), str.size()};
              } || requires {
                  str.c_str();
                  str.size();
                  StrViewType{str.c_str(), str.size()};
              };
          };
    } // namespace details

    /**
     * Check if T is a "string view" of type "StringViewType"
     */
    template <typename StrViewType, typename T>
    concept StringViewifiableOf =
      details::StringViewifiableOf<stl::remove_cvref_t<StrViewType>, stl::remove_cvref_t<T>>;

    template <template <typename...> typename StrViewType, typename T>
    concept StringViewifiableOfTemplate =
      StringViewifiableOf<details::string_view::deduced_type<StrViewType, T>, T>;

    template <typename T>
    concept StringViewifiable = StringViewifiableOf<string_view_type_of<T>, T>;

    /**
     * Convert the string value specified to a "string view" of type StrViewT
     */
    template <typename StrViewT, typename StrT>
        requires(StringViewifiableOf<StrViewT, StrT>)
    [[nodiscard]] constexpr StrViewT string_viewify_of(StrT&& str) noexcept {
        if constexpr (stl::is_convertible_v<StrT, StrViewT>) {
            return stl::forward<StrT>(str);
        } else if constexpr (requires { StrViewT{str}; }) {
            return StrViewT{str};
        } else if constexpr (requires {
                                 str.c_str();
                                 str.size();
                                 StrViewT{str.c_str(), str.size()};
                             })
        {
            return StrViewT{str.c_str(), str.size()};
        } else if constexpr (requires {
                                 str.data();
                                 str.size();
                                 StrViewT{str.data(), str.size()};
                             })
        {
            return StrViewT{str.data(), str.size()};
        } else if constexpr (requires { str.string_view(); }) {
            return string_viewify_of<StrViewT>(str.string_view());
        } else if constexpr (requires { str.str(); }) {
            return string_viewify_of<StrViewT>(str.str());
        } else if constexpr (requires { str.string(); }) {
            return string_viewify_of<StrViewT>(str.string());
        } else if constexpr (requires { str.template to_string<StrViewT>(); }) {
            return string_viewify_of<StrViewT>(str.template to_string<StrViewT>());
        } else if constexpr (requires { str.to_string(); }) {
            return string_viewify_of<StrViewT>(str.to_string());
        } else {
            static_assert(false && sizeof(StrT), "The specified input is not convertible to string view");
        }
    }

    /**
     * Convert to string view of the specified template type
     * @example
     * @code
     *      string_viewify_of<std::basic_string_view>("convert to string view")
     * @endcode
     */
    template <template <typename...> typename StrViewT, typename StrT>
        requires(StringViewifiableOfTemplate<StrViewT, StrT>)
    [[nodiscard]] constexpr auto string_viewify_of(StrT&& str) noexcept {
        using deduced_type = details::string_view::deduced_type<StrViewT, StrT>;
        return string_viewify_of<deduced_type, StrT>(stl::forward<StrT>(str));
    }

    /**
     * Convert to string view (if itself is one, return itself, otherwise get one of the basic_string_view)
     */
    template <StringViewifiable StrT>
    [[nodiscard]] constexpr auto string_viewify(StrT&& str) noexcept {
        using str_view_t = string_view_type_of<StrT>;
        return string_viewify_of<str_view_t>(stl::forward<StrT>(str));
    }

    template <StringViewifiable T>
    [[nodiscard]] static constexpr auto to_std_string_view(T&& str) noexcept {
        using str_t     = stl::remove_cvref_t<T>;
        using char_type = char_type_of_t<str_t>;
        using str_v     = stl::basic_string_view<char_type>;
        if constexpr (stl::is_same_v<str_t, str_v>) {
            return str;
        } else {
            return string_viewify_of<str_v>(stl::forward<T>(str));
        }
    }

} // namespace webpp::istl

#endif // WEBPP_STD_STRING_VIEW_HPP
