import standard;

#include <cassert>
#include <cstdint>
#include <print>
#include <type_traits>
#include <utility>

using namespace std;
template <typename T, typename Allocator = std::allocator<T>>
struct [[nodiscard]] nullable_indirect {
    using element_type = T;
    using allocator_type = Allocator;
    using allocator_traits = std::allocator_traits<allocator_type>;

  private:
    [[no_unique_address]] allocator_type alloc;
    T* ptr = nullptr;

    // Type-erased operation pointers
    void (*destroy_fn)(T*, allocator_type&) = nullptr;
    T* (*clone_fn)(T const*, allocator_type&) = nullptr;

    // Helper to generate the type-erased functions when T is complete
    static void destroy_impl(T* p, allocator_type& alloc) {
        if (p) {
            allocator_traits::destroy(alloc, p);
            allocator_traits::deallocate(alloc, p, 1);
        }
    }

    static T* clone_impl(T const* p, allocator_type& alloc) {
        if (!p) {
            return nullptr;
        }
        T* new_ptr = allocator_traits::allocate(alloc, 1);
        try {
            allocator_traits::construct(alloc, new_ptr, *p);
            return new_ptr;
        } catch (...) {
            allocator_traits::deallocate(alloc, new_ptr, 1);
            throw;
        }
    }

    static constexpr T* (*clone_fn_for())(T const*, allocator_type&) {
        if constexpr (std::is_copy_constructible_v<T>) {
            return &clone_impl;
        }
        return nullptr;
    }

    // Private constructor used by `make`
    nullable_indirect(T* inp_ptr, allocator_type const& inp_alloc) noexcept
        : alloc(inp_alloc), ptr(inp_ptr), destroy_fn(&destroy_impl),
          clone_fn(clone_fn_for()) {}

  public:
    // --- Constructors ---
    constexpr nullable_indirect() noexcept = default;

    constexpr explicit nullable_indirect(std::nullptr_t) noexcept
        : nullable_indirect() {}

    // Factory function to create an instance (T MUST be complete here)
    template <typename... Args>
    static nullable_indirect make(Args&&... args) {
        return make_allocated(allocator_type{}, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static nullable_indirect make_allocated(allocator_type const& alloc,
                                            Args&&... args) {
        allocator_type a = alloc;
        T* p = allocator_traits::allocate(a, 1);
        try {
            allocator_traits::construct(a, p, std::forward<Args>(args)...);
            return nullable_indirect(p, a);
        } catch (...) {
            allocator_traits::deallocate(a, p, 1);
            throw;
        }
    }

    // --- Rule of Five ---

    // Destructor: T can be incomplete here
    constexpr ~nullable_indirect() noexcept { reset(); }

    // Copy Constructor: T can be incomplete here (performs deep copy)
    constexpr nullable_indirect(nullable_indirect const& other)
        : alloc(allocator_traits::select_on_container_copy_construction(
              other.alloc)),
          destroy_fn(other.destroy_fn), clone_fn(other.clone_fn) {
        assert(other.ptr == nullptr || other.clone_fn != nullptr);
        if (other.ptr && other.clone_fn) {
            ptr = clone_fn(other.ptr, alloc);
        }
    }

    // Move Constructor: T can be incomplete here
    constexpr nullable_indirect(nullable_indirect&& other) noexcept
        : alloc(std::move(other.alloc)), ptr(std::exchange(other.ptr, nullptr)),
          destroy_fn(std::exchange(other.destroy_fn, nullptr)),
          clone_fn(std::exchange(other.clone_fn, nullptr)) {}

    // Copy Assignment
    constexpr nullable_indirect& operator=(nullable_indirect const& other) {
        if (this != &other) {
            nullable_indirect temp(other);
            swap(temp);
        }
        return *this;
    }

    // Move Assignment
    constexpr nullable_indirect& operator=(nullable_indirect&& other) noexcept(
        allocator_traits::propagate_on_container_move_assignment::value ||
        allocator_traits::is_always_equal::value) {
        if (this != &other) {
            reset();
            if constexpr (allocator_traits::
                              propagate_on_container_move_assignment::value) {
                alloc = std::move(other.alloc);
            }
            ptr = std::exchange(other.ptr, nullptr);
            destroy_fn = std::exchange(other.destroy_fn, nullptr);
            clone_fn = std::exchange(other.clone_fn, nullptr);
        }
        return *this;
    }

    // --- Modifiers ---
    constexpr void reset() noexcept {
        if (ptr && destroy_fn) {
            destroy_fn(ptr, alloc);
        }
        ptr = nullptr;
        destroy_fn = nullptr;
        clone_fn = nullptr;
    }

    constexpr void swap(nullable_indirect& other) noexcept {
        using std::swap;
        if constexpr (allocator_traits::propagate_on_container_swap::value) {
            swap(alloc, other.alloc);
        }
        swap(ptr, other.ptr);
        swap(destroy_fn, other.destroy_fn);
        swap(clone_fn, other.clone_fn);
    }

    // --- Observers ---
    [[nodiscard]] constexpr T* get() const noexcept { return ptr; }

    [[nodiscard]] constexpr T* operator->() const noexcept {
        assert(ptr != nullptr);
        return ptr;
    }

    [[nodiscard]] constexpr T& operator*() const noexcept {
        assert(ptr != nullptr);
        return *ptr;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return ptr != nullptr;
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept {
        return alloc;
    }
};

template <typename T, typename Alloc>
constexpr void swap(nullable_indirect<T, Alloc>& lhs,
                    nullable_indirect<T, Alloc>& rhs) noexcept {
    lhs.swap(rhs);
}

// struct [[nodiscard]] consteval_copyable {
//     // Still consteval so the type can only be default-constructed
//     // in a constant-expression context.
//     consteval consteval_copyable() noexcept = default;

//     // Compile-time copy OK, runtime copy rejected.
//     constexpr consteval_copyable(consteval_copyable const&) noexcept {
//         if consteval {
//             // nothing to do
//         } else {
//             // throw "consteval_copyable may not be copied at runtime";
//             std::fprintf(stderr, "Failed.");
//             std::abort();
//         }
//     }

//     constexpr consteval_copyable(consteval_copyable&&) noexcept = default;

//     constexpr consteval_copyable&
//     operator=(consteval_copyable&&) noexcept = default;

//     constexpr consteval_copyable&
//     operator=(consteval_copyable const&) noexcept {
//         if consteval {
//             // nothing to do
//         } else {
//             // throw "consteval_copyable may not be copy-assigned at
//             runtime"; std::fprintf(stderr, "Failed."); std::abort();
//         }
//         return *this;
//     }

//     constexpr ~consteval_copyable() noexcept = default;
// };

struct [[nodiscard]] consteval_copyable {
    // Still consteval so the type can only be default-constructed
    // in a constant-expression context.
    consteval consteval_copyable() noexcept = default;

    // Compile-time copy OK, runtime copy rejected.
    constexpr consteval_copyable(consteval_copyable const&) noexcept {
        if !consteval {
            std::println(stderr,
                         "This object is only copyable at compile time.");
            std::abort();
        }
    }

    constexpr consteval_copyable(consteval_copyable&&) noexcept = default;

    constexpr consteval_copyable&
    operator=(consteval_copyable&&) noexcept = default;

    constexpr consteval_copyable&
    operator=(consteval_copyable const&) noexcept {
        if consteval {
            std::println(stderr,
                         "This object is only copyable at compile time.");
            std::abort();
        }
        return *this;
    }

    constexpr ~consteval_copyable() noexcept = default;
};

// struct [[nodiscard]] consteval_copyable {
//     consteval consteval_copyable() noexcept = default;
//     consteval consteval_copyable(consteval_copyable const&) noexcept {
//         if !consteval {
//             std::terminate();
//         }
//     }
//     constexpr consteval_copyable(consteval_copyable&&) noexcept = default;
//     constexpr consteval_copyable&
//     operator=(consteval_copyable&&) noexcept = default;
//     consteval consteval_copyable&
//     operator=(consteval_copyable const&) noexcept {
//         if !consteval {
//             std::terminate();
//         }
//         return *this;
//     }
//     constexpr ~consteval_copyable() noexcept = default;
// };

template <typename>
struct [[nodiscard]] pimpl_idiom : consteval_copyable {
    using consteval_copyable::consteval_copyable;

    // [[nodiscard]] constexpr bool has_impl() const noexcept {
    //     return static_cast<bool>(pimpl);
    // }

    // [[nodiscard]] constexpr impl* get_impl() noexcept {
    //     return pimpl.get();
    // }

    // [[nodiscard]] constexpr impl const* get_impl() const noexcept {
    //     return pimpl.get();
    // }

  protected:
    struct [[nodiscard]] impl;

    template <typename... Args>
    constexpr void init_impl(Args&&... args) {
        assert(pimpl.get() == nullptr);
        pimpl = nullable_indirect<impl>::make(std::forward<Args>(args)...);
    }

    nullable_indirect<impl> pimpl{};
};

constexpr struct [[nodiscard]] basic_device_registry
    : pimpl_idiom<basic_device_registry> {
    using pimpl_idiom<basic_device_registry>::pimpl_idiom;
    int a = 20;
} device_registry;

constexpr struct [[nodiscard]] test : consteval_copyable {
    using consteval_copyable::consteval_copyable;
    int a = 20;
} kk;

constexpr struct [[nodiscard]] wtest {

    consteval wtest() noexcept = default;
    consteval wtest(wtest const&) noexcept = default;
    constexpr wtest(wtest&&) noexcept = default;
    constexpr wtest& operator=(wtest&&) noexcept = default;
    consteval wtest& operator=(wtest const&) noexcept = default;
    constexpr ~wtest() noexcept = default;

    int a = 20;
} kkk;

int main() {

    // auto reg = auto{device_registry};
    // auto reg2 = new test{kk};
    // println("Number: {}", reg2->a);
    auto reg4 = new wtest(kkk);
    reg4->a = 15;
    auto reg3 = new wtest{*reg4};
    println("Number: {}", reg3->a);

    return 0;
}

// view ./build/optimizations.txt
