import standard;

#include <cassert>
#include <concepts>
#include <memory>
#include <print>

/**
 * A binder provides the storage and swap mechanism for an ambient context
 * pointer.
 *
 * Dynamic Scopes (or Locally Bound Globals) act as implicit parameters. They
 * allow a caller to temporarily override a global or thread-local instance of
 * T, which nested functions can then access without modifying their signatures.
 *
 * Contracts:
 * - Must provide a static `instance()` returning a reference to the active
 * pointer.
 * - Must provide an `exchange()` method to atomically or safely swap the
 * pointer.
 */
template <typename T>
concept binder_instance = requires(T obj) {
    typename T::binding;
    typename T::type;
    typename T::pointer;
    { T::instance() } noexcept -> std::same_as<typename T::pointer&>;

    requires requires(typename T::pointer ptr) {
        { obj.operator->() } noexcept -> std::same_as<typename T::pointer>;
        { obj.ptr() } noexcept -> std::same_as<typename T::pointer>;
        { T::exchange(ptr) } noexcept -> std::same_as<typename T::pointer>;
    };
};

/**
 * Defines a type that has explicitly opted into being dynamically scoped.
 * Types satisfying this concept typically inherit from their binding (e.g.,
 * `global_binding<T>`), and should use `binding self; self->...` instead of
 * `this->...` to access the currently bound instance.
 */
template <typename T>
concept dynamically_scoped = requires {
    requires binder_instance<typename T::binding>;
    requires std::is_base_of_v<typename T::binding, T>;
};

/**
 * Global Binding: Manages a single, non-thread-local active pointer for type T.
 *
 * Inheriting from this allows convenient access to the globally active
 * instance:
 * @code
 *   global_binding<T>::instance(); // get the pointer
 *   global_binding<T> self;
 *   self->do_something();          // operates on the globally bound instance
 * @endcode
 *
 * Note: Use the `ID` template parameter to maintain multiple distinct global
 * pointers for the same type.
 */
template <typename T, auto ID = 0>
struct [[nodiscard]] global_binding {
    using type = T;
    using pointer = T*;
    using const_pointer = T const*;
    using binding = global_binding;

    static constexpr pointer exchange(pointer inp_ptr) noexcept {
        pointer const old_ptr = instance(); // NOLINT(*-misplaced-const)
        instance() = inp_ptr;
        return old_ptr;
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept {
        assert(instance() != nullptr);
        return instance();
    }

    [[nodiscard]] pointer ptr() const noexcept { return operator->(); }

    [[nodiscard]] static pointer& instance() noexcept {
        static pointer inst = nullptr;
        return inst;
    }
};

// todo: we can have a `global_multi_binding` which would allow to have multiple
// instances for the same access point.

/**
 * Thread-Local Binding: Manages a per-thread active pointer for type T.
 *
 * This will be ensuring concurrent requests on different threads do not
 * overwrite each other's context.
 *
 * Note: Accessing `self->` before a scope is bound will trigger an assertion.
 */
template <typename T, auto ID = 0>
struct [[nodiscard]] thread_binding {
    using type = T;
    using pointer = T*;
    using const_pointer = T const*;
    using binding = thread_binding;

    static constexpr pointer exchange(pointer inp_ptr) noexcept {
        pointer const old_ptr = instance(); // NOLINT(*-misplaced-const)
        instance() = inp_ptr;
        return old_ptr;
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept {
        assert(instance() != nullptr &&
               "Global binding accessed without being set.");
        return instance();
    }

    [[nodiscard]] pointer ptr() const noexcept { return operator->(); }

    [[nodiscard]] static pointer& instance() noexcept {
        thread_local pointer inst = nullptr;
        return inst;
    }
};

/**
 * RAII guard to create a new Locally-Bound-Global scope.
 *
 * Overrides the current binding for T with the provided instance, and restores
 * the previous binding upon destruction. This forms a LIFO stack of contexts.
 *
 * @code
 *   {
 *      context_type ctx2;
 *      dynamic_scope scope{context, ctx2};
 *      // now `context` is pointing to `ctx2` until `scope` goes out of scope
 *   }
 * @endcode
 *
 * @warning Because restoration happens in the destructor, instances of
 * `dynamic_scope` MUST be strictly scoped on the stack. Interleaving lifetimes
 * or passing these across asynchronous boundaries without careful LIFO
 * guarantees will corrupt the binding state.
 */
template <dynamically_scoped T>
struct [[nodiscard]] dynamic_scope {
    using binding = T::binding;
    using pointer = binding::pointer;
    using type = binding::type;

    explicit constexpr dynamic_scope(pointer inp_ptr) noexcept
        : prev{binding::exchange(inp_ptr)} {}

    explicit constexpr dynamic_scope(type& ref) noexcept
        : dynamic_scope{std::addressof(ref)} {}

    explicit constexpr dynamic_scope(T const&, pointer inp_ptr) noexcept
        : dynamic_scope{inp_ptr} {}

    explicit constexpr dynamic_scope(T const&, type& ref) noexcept
        : dynamic_scope{ref} {}

    dynamic_scope(dynamic_scope const& obj) = delete;
    dynamic_scope(dynamic_scope&& obj) noexcept = default;
    dynamic_scope& operator=(dynamic_scope const& obj) = delete;
    dynamic_scope& operator=(dynamic_scope&& obj) noexcept = default;

    constexpr ~dynamic_scope() noexcept { binding::exchange(prev); }

  private:
    pointer prev = nullptr;
};

template <typename T, typename... Args>
dynamic_scope(T*, Args&&...) -> dynamic_scope<std::remove_const_t<T>>;

template <typename T, typename... Args>
dynamic_scope(T&, Args&&...) -> dynamic_scope<std::remove_const_t<T>>;

///////////////////////////////////////

struct basic_simple_context : global_binding<basic_simple_context> {
    static constexpr global_binding self{};

    int value = 20;

    void set(int val) const { self->value = val; }

    int get() const noexcept { return self->value; }
};

static constexpr basic_simple_context simple_context;

////////////////////////////////////////////

struct AnySimpleContext {
    virtual ~AnySimpleContext() = default;
    virtual void set(int val) = 0;
    virtual int get() const = 0;
};

// The type-erased wrapper that delegates to your unrelated types
template <typename T>
struct AnySimpleContextModel final : AnySimpleContext {
    T* instance;
    explicit AnySimpleContextModel(T* obj) : instance(obj) {}

    void set(int val) override { instance->set(val); }
    int get() const override { return instance->get(); }
};

struct generic_simple_context : global_binding<AnySimpleContext> {
    static constexpr global_binding self{};

    void set(int val) const { self->set(val); }
    int get() const noexcept { return self->get(); }
};

static constexpr generic_simple_context polymorphic_context;

template <typename T>
struct polymorphic_dynamic_scope {
    AnySimpleContextModel<T> model;
    dynamic_scope<generic_simple_context> scope;

    // Binds the unrelated type by wrapping it in the model
    explicit polymorphic_dynamic_scope(T& ref) noexcept
        : model(std::addressof(ref)), scope(polymorphic_context, model) {}
};

// int main() {
//     using namespace std;

//     basic_simple_context one;
//     one.value = 30;
//     dynamic_scope s1{simple_context, one};
//     println("{}", simple_context.get());
//     assert(simple_context.get() == 30);
//     {
//         basic_simple_context two;
//         dynamic_scope s2{simple_context, two};
//         simple_context.set(123);
//         println("{}", simple_context.get());
//         assert(two.value == 123);
//     }
// }

struct UnrelatedA {
    int a_val = 10;
    void set(int val) { a_val = val; }
    int get() const { return a_val; }
};

struct UnrelatedB {
    int b_val = 20;
    void set(int val) { b_val = val; }
    int get() const { return b_val; }
};

int main() {
    UnrelatedA a;
    {
        // Bind UnrelatedA
        polymorphic_dynamic_scope scope_a{a};
        polymorphic_context.set(100);
        assert(a.a_val == 100);

        UnrelatedB b;
        {
            // Push UnrelatedB over UnrelatedA
            polymorphic_dynamic_scope scope_b{b};
            polymorphic_context.set(200);
            assert(b.b_val == 200);
        } // UnrelatedB popped

        assert(polymorphic_context.get() == 100); // Back to UnrelatedA
    }
}
