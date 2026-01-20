#include <type_traits>
import standard;

#include <concepts>
#include <cstdint>
#include <memory_resource>
#include <print>

using namespace std;

namespace stl {
    using namespace std;

    namespace tag_invoke_fn_ns {
        void tag_invoke();

        struct tag_invoke_fn {
            template <typename Tag, typename... Args>
                requires requires(Tag tag, Args&&... args) {
                    tag_invoke(stl::forward<Tag>(tag),
                               stl::forward<Args>(args)...);
                }
            constexpr auto operator()(Tag tag, Args&&... args) const
                noexcept(noexcept(tag_invoke(stl::forward<Tag>(tag),
                                             stl::forward<Args>(args)...)))
                    -> decltype(tag_invoke(stl::forward<Tag>(tag),
                                           stl::forward<Args>(args)...)) {
                return tag_invoke(stl::forward<Tag>(tag),
                                  stl::forward<Args>(args)...);
            }
        };
    } // namespace tag_invoke_fn_ns

    inline namespace tag_invoke_ns {
        inline constexpr tag_invoke_fn_ns::tag_invoke_fn tag_invoke = {};
    } // namespace tag_invoke_ns

    template <typename Tag, typename... Args>
    concept tag_invocable = requires(Tag tag, Args... args) {
        tag_invoke(stl::forward<Tag>(tag), stl::forward<Args>(args)...);
    };

    template <typename Tag, typename... Args>
    concept nothrow_tag_invocable =
        tag_invocable<Tag, Args...> && requires(Tag tag, Args... args) {
            {
                tag_invoke(stl::forward<Tag>(tag), stl::forward<Args>(args)...)
            } noexcept;
        };

    template <typename Tag, typename... Args>
    using tag_invoke_result = invoke_result<decltype(tag_invoke), Tag, Args...>;

    template <typename Tag, typename... Args>
    using tag_invoke_result_t =
        invoke_result_t<decltype(tag_invoke), Tag, Args...>;

    template <auto& Tag>
    using tag_t = decay_t<decltype(Tag)>;

} // namespace stl

// Name requirement: https://en.cppreference.com/w/cpp/named_req/Allocator
template <typename A>
concept AllocatorType = requires {
    typename A::value_type;
    typename stl::allocator_traits<A>::pointer;
    typename stl::allocator_traits<A>::size_type;
    typename stl::allocator_traits<A>::const_pointer;
    typename stl::allocator_traits<A>::void_pointer;
    typename stl::allocator_traits<A>::const_void_pointer;
    requires requires(
        A alloc, A alloc2, typename stl::allocator_traits<A>::size_type n,
        typename stl::allocator_traits<A>::pointer ptr,
        typename stl::allocator_traits<A>::const_pointer const_ptr,
        typename stl::allocator_traits<A>::void_pointer void_ptr,
        typename stl::allocator_traits<A>::const_void_pointer cvp,
        decltype(*ptr) res) {
        alloc.allocate(n);
        *ptr;
        *const_ptr;
        // {p->m};
        // {cp->m};

        static_cast<typename stl::allocator_traits<A>::pointer>(void_ptr);
        static_cast<typename stl::allocator_traits<A>::const_pointer>(cvp);
        stl::pointer_traits<
            typename stl::allocator_traits<A>::pointer>::pointer_to(res);
        alloc == alloc2;
        alloc != alloc2;
        A(alloc);
        // {A a1 = a};
        // A a(b)};

        A(stl::move(alloc));
        // {A a1 = stl::move(a)};
        // {A a(stl::move(b))};
    };
};

template <typename A>
concept Allocator = AllocatorType<stl::remove_cvref_t<A>>;

template <typename T, int priority = 1>
struct default_allocator {
    using type = default_allocator<T, priority - 1>;
};

/// Default, Default Allocator (if the user doesn't specify the default
/// allocator, this would be used)
template <typename T>
struct default_allocator<T, 0> {
    using type = stl::allocator<T>;
};

static constexpr struct [[nodiscard]] alloc_tag {

    template <typename T = stl::byte>
    using allocator_type = typename default_allocator<T>::type;

    /// Customization Point
    template <typename T>
        requires stl::tag_invocable<alloc_tag, stl::type_identity<T>>
    [[nodiscard]] constexpr stl::tag_invoke_result_t<alloc_tag,
                                                     stl::type_identity<T>>
    operator()(stl::type_identity<T> ident) const noexcept {
        return stl::tag_invoke(*this, ident);
    }

    /// default impl: default construct
    template <Allocator T>
        requires stl::is_default_constructible_v<T>
    [[nodiscard]] friend constexpr T
    tag_invoke([[maybe_unused]] alloc_tag tag, stl::type_identity<T>) noexcept {
        return {};
    }

    template <Allocator T>
    [[nodiscard]] constexpr explicit(false) operator T() const noexcept {
        return operator()(stl::type_identity<T>{});
    }

    template <typename T>
    [[nodiscard]] constexpr explicit(false)
    operator allocator_type<T>() const noexcept {
        return operator()(stl::type_identity<allocator_type<T>>{});
    }

    template <typename T = stl::byte>
    [[nodiscard]] constexpr decltype(auto) of() const noexcept {
        static_assert(!Allocator<T>,
                      "Don't pass an allocator, pass the value type you need.");
        return operator()(stl::type_identity<allocator_type<T>>{});
    }
} alloc;

// Change the default value
template <typename T>
struct default_allocator<T> {
    using type = stl::pmr::polymorphic_allocator<T>;
};

// Custom alllocator
template <typename T>
std::pmr::polymorphic_allocator<T>
tag_invoke(alloc_tag,
           std::type_identity<std::pmr::polymorphic_allocator<T>>) noexcept {
    std::println("pmr allocator");
    return {};
}

// Custom allocator 2
template <typename T>
std::allocator<T> tag_invoke(alloc_tag,
                             std::type_identity<stl::allocator<T>>) noexcept {
    std::println("std::allocator");
    return {};
}

int main() {

    string str{"str", alloc};
    pmr::string pmr_str{"pmr", alloc};
    basic_string str3{"default", alloc.of<char>()};

    println("Value: {}", str);
    println("Value: {}", pmr_str);
    println("Value: {}", str3);

    return 0;
}

// view ./build/optimizations.txt
