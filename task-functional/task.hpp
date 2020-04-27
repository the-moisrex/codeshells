#include <utility>
#include <type_traits>
#include <functional>
#include <memory>

/**************************************************************************************************/

#define STLAB_FEATURE_PRIVATE_OBJC_ARC() 0
#define STLAB_FEATURE_PRIVATE_COROUTINES() 0

#define STLAB_FEATURE(X) (STLAB_FEATURE_PRIVATE_##X())

/**************************************************************************************************/

#define STLAB_TASK_SYSTEM_PRIVATE_PORTABLE() 0
#define STLAB_TASK_SYSTEM_PRIVATE_LIBDISPATCH() 0
#define STLAB_TASK_SYSTEM_PRIVATE_EMSCRIPTEN() 0
#define STLAB_TASK_SYSTEM_PRIVATE_PNACL() 0
#define STLAB_TASK_SYSTEM_PRIVATE_WINDOWS() 0

#define STLAB_TASK_SYSTEM(X) (STLAB_TASK_SYSTEM_PRIVATE_##X())

#define STLAB_CPP_VERSION(X) (STLAB_CPP_VERSION_PRIVATE() == (X))
#define STLAB_CPP_VERSION_LESS_THAN(X) (STLAB_CPP_VERSION_PRIVATE() < (X))
#define STLAB_CPP_VERSION_AT_LEAST(X) (STLAB_CPP_VERSION_PRIVATE() >= (X))

#if __APPLE__

    #undef STLAB_TASK_SYSTEM_PRIVATE_LIBDISPATCH
    #define STLAB_TASK_SYSTEM_PRIVATE_LIBDISPATCH() 1

    #if defined(__has_feature)
        #if __has_feature(objc_arc)
            #undef STLAB_FEATURE_PRIVATE_OBJC_ARC
            #define STLAB_FEATURE_PRIVATE_OBJC_ARC() 1
        #endif
    #endif

#elif __EMSCRIPTEN__

    #undef STLAB_TASK_SYSTEM_PRIVATE_EMSCRIPTEN
    #define STLAB_TASK_SYSTEM_PRIVATE_EMSCRIPTEN() 1

#elif __pnacl__

    #undef STLAB_TASK_SYSTEM_PRIVATE_PNACL
    #define STLAB_TASK_SYSTEM_PRIVATE_PNACL() 1

#elif _MSC_VER

    #undef STLAB_TASK_SYSTEM_PRIVATE_WINDOWS
    #define STLAB_TASK_SYSTEM_PRIVATE_WINDOWS() 1

    #if _MSVC_LANG == 201103L
        #define STLAB_CPP_VERSION_PRIVATE() 11
    #elif _MSVC_LANG == 201402L
        #define STLAB_CPP_VERSION_PRIVATE() 14
    #elif _MSC_FULL_VER >= 191225830 && _MSVC_LANG == 201703L
        #define STLAB_CPP_VERSION_PRIVATE() 17
    #else
        #warning Unknown version of C+; assuming C++20.
        #define STLAB_CPP_VERSION_PRIVATE() 20
    #endif

#else

    #undef STLAB_TASK_SYSTEM_PRIVATE_PORTABLE
    #define STLAB_TASK_SYSTEM_PRIVATE_PORTABLE() 1

#endif

#if !defined(STLAB_CPP_VERSION_PRIVATE)
    #if __cplusplus == 201103L
        #define STLAB_CPP_VERSION_PRIVATE() 11
    #elif __cplusplus == 201402L
        #define STLAB_CPP_VERSION_PRIVATE() 14
    #elif __cplusplus == 201703L
        #define STLAB_CPP_VERSION_PRIVATE() 17
    #else
        #warning Unknown version of C+; assuming C++20.
        #define STLAB_CPP_VERSION_PRIVATE() 20
    #endif
#endif

inline namespace v1 {
/**************************************************************************************************/

/*
    tasks are functions with a mutable call operator to support moving items through for single
    invocations.
*/
template <class>
class task;

template <class R, class... Args>
class task<R(Args...)> {
    // REVISIT (sean.parent) : Use `if constexpr` here when we move to C++17
    template <class F>
    using possibly_empty_t =
        std::integral_constant<bool,
                               std::is_pointer<std::decay_t<F>>::value ||
                                   std::is_member_pointer<std::decay_t<F>>::value ||
                                   std::is_same<std::function<R(Args...)>, std::decay_t<F>>::value>;

    template <class F>
    static auto is_empty(const F& f) -> std::enable_if_t<possibly_empty_t<F>::value, bool> {
        return !f;
    }

    template <class F>
    static auto is_empty(const F&) -> std::enable_if_t<!possibly_empty_t<F>::value, bool> {
        return false;
    }

    struct concept_t {
        void (*dtor)(void*);
        void (*move_ctor)(void*, void*) noexcept;
        R (*invoke)(void*, Args&&...);
        const std::type_info& (*target_type)() noexcept;
        void* (*pointer)(void*)noexcept;
        const void* (*const_pointer)(const void*)noexcept;
    };

    template <class F, bool Small>
    struct model;

    template <class F>
    struct model<F, true> {
        template <class G> // for forwarding
        model(G&& f) : _f(std::forward<G>(f)) {}
        model(model&&) noexcept = delete;

        static void dtor(void* self) { static_cast<model*>(self)->~model(); }
        static void move_ctor(void* self, void* p) noexcept {
            new (p) model(std::move(static_cast<model*>(self)->_f));
        }
        static auto invoke(void* self, Args&&... args) -> R {
            return std::move(static_cast<model*>(self)->_f)(std::forward<Args>(args)...);
        }
        static auto target_type() noexcept -> const std::type_info& { return typeid(F); }
        static auto pointer(void* self) noexcept -> void* { return &static_cast<model*>(self)->_f; }
        static auto const_pointer(const void* self) noexcept -> const void* {
            return &static_cast<const model*>(self)->_f;
        }
#if defined(__GNUC__) && __GNUC__ < 7 && !defined(__clang__)
        static const concept_t _vtable;
#else
        static constexpr concept_t _vtable = { dtor, move_ctor, invoke,
                                             target_type, pointer, const_pointer };
#endif
        F _f;
    };

    template <class F>
    struct model<F, false> {
        template <class G> // for forwarding
        model(G&& f) : _p(std::make_unique<F>(std::forward<G>(f))) {}
        model(model&&) noexcept = default;

        static void dtor(void* self) { static_cast<model*>(self)->~model(); }
        static void move_ctor(void* self, void* p) noexcept {
            new (p) model(std::move(*static_cast<model*>(self)));
        }
        static auto invoke(void* self, Args&&... args) -> R {
            return std::move(*static_cast<model*>(self)->_p)(std::forward<Args>(args)...);
        }
        static auto target_type() noexcept -> const std::type_info& { return typeid(F); }
        static auto pointer(void* self) noexcept -> void* {
            return static_cast<model*>(self)->_p.get();
        }
        static auto const_pointer(const void* self) noexcept -> const void* {
            return static_cast<const model*>(self)->_p.get();
        }

#if defined(__GNUC__) && __GNUC__ < 7 && !defined(__clang__)
        static const concept_t _vtable;
#else
        static constexpr concept_t _vtable = { dtor, move_ctor, invoke,
                                             target_type, pointer, const_pointer };
#endif

        std::unique_ptr<F> _p;
    };

    // empty (default) vtable
    static void dtor(void*) {}
    static void move_ctor(void*, void*) noexcept {}
    static auto invoke(void*, Args&&...) -> R { throw std::bad_function_call(); }
    static auto target_type_() noexcept -> const std::type_info& { return typeid(void); }
    static auto pointer(void*) noexcept -> void* { return nullptr; }
    static auto const_pointer(const void*) noexcept -> const void* { return nullptr; }

#if defined(__GNUC__) && __GNUC__ < 7 && !defined(__clang__)
    static const concept_t _vtable;
#else
    static constexpr concept_t _vtable = { dtor, move_ctor, invoke,
                                         target_type_, pointer, const_pointer };
#endif

    const concept_t* _vtable_ptr = &_vtable;

    /*
        REVISIT (sean.parent) : The size of 256 was an arbitrary choice with no data to back it up.
        Desire is to have something large enough to hold the vast majority of lamda expressions.
        A single small heap allocation/deallocation takes about as much time as copying 10K of POD
        data on my MacBook Pro. tasks are move only object so the exepectation is that moving the
        funciton object is proportional to the sizeof the object. We try to construct tasks emplace
        and so they are rarely moved (need to review the code to make sure that is true). The
        concept_t will consume one word so this gives us 31 words (on a 64 bit machine) for the model.
        Probably excessive but still allows 16 tasks on a cache line
        I welcome empirical data from an actual system on a better size.
        sizeof(std::function<R(Args...)>)
    */
    static constexpr std::size_t small_object_size = 256 - sizeof(void*);
    std::aligned_storage_t<small_object_size> _model;

public:
    using result_type = R;

    constexpr task() noexcept = default;
    constexpr task(std::nullptr_t) noexcept : task() {}
    task(const task&) = delete;
    task(task&& x) noexcept : _vtable_ptr(x._vtable_ptr) {
        _vtable_ptr->move_ctor(&x._model, &_model);
    }

    template <class F>
    task(F&& f) {
        using f_t = std::decay_t<F>;
        using model_t = model<f_t, sizeof(model<f_t, true>) <= small_object_size>;

        if (is_empty(f)) return;

        new (&_model) model_t(std::forward<F>(f));
        _vtable_ptr = &model_t::_vtable;
    }

    ~task() { _vtable_ptr->dtor(&_model); };

    task& operator=(const task&) = delete;

    task& operator=(task&& x) noexcept {
        _vtable_ptr->dtor(&_model);
        _vtable_ptr = x._vtable_ptr;
        _vtable_ptr->move_ctor(&x._model, &_model);
        return *this;
    }

    task& operator=(std::nullptr_t) noexcept { return *this = task(); }

    template <class F>
    task& operator=(F&& f) {
        return *this = task(std::forward<F>(f));
    }

    void swap(task& x) noexcept { std::swap(*this, x); }

    explicit operator bool() const { return _vtable_ptr->const_pointer(&_model) != nullptr; }

    const std::type_info& target_type() const noexcept { return _vtable_ptr->target_type(); }

    template <class T>
    T* target() {
        return (target_type() == typeid(T)) ? static_cast<T*>(_vtable_ptr->pointer(&_model)) :
                                              nullptr;
    }

    template <class T>
    const T* target() const {
        return (target_type() == typeid(T)) ?
                   static_cast<const T*>(_vtable_ptr->const_pointer(&_model)) :
                   nullptr;
    }

    R operator()(Args... args) { return _vtable_ptr->invoke(&_model, std::forward<Args>(args)...); }

    friend inline void swap(task& x, task& y) { return x.swap(y); }
    friend inline bool operator==(const task& x, std::nullptr_t) { return !static_cast<bool>(x); }
    friend inline bool operator==(std::nullptr_t, const task& x) { return !static_cast<bool>(x); }
    friend inline bool operator!=(const task& x, std::nullptr_t) { return static_cast<bool>(x); }
    friend inline bool operator!=(std::nullptr_t, const task& x) { return static_cast<bool>(x); }
};

#if STLAB_CPP_VERSION_LESS_THAN(17)

// In C++17 constexpr implies inline and these definitions are deprecated

#if defined(__GNUC__) && __GNUC__ < 7 && !defined(__clang__)
    template <class R, class... Args>
    const typename task<R(Args...)>::concept_t task<R(Args...)>::_vtable = { dtor, move_ctor, invoke,
                                                                           target_type_, pointer, const_pointer };
#else
    template <class R, class... Args>
    const typename task<R(Args...)>::concept_t task<R(Args...)>::_vtable;
#endif

#ifdef _MSC_VER

template <class R, class... Args>
template <class F>
const typename task<R(Args...)>::concept_t task<R(Args...)>::model<F, false>::_vtable;

template <class R, class... Args>
template <class F>
const typename task<R(Args...)>::concept_t task<R(Args...)>::model<F, true>::_vtable;

#else

#if defined(__GNUC__) && __GNUC__ < 7 && !defined(__clang__)

template <class R, class... Args>
template <class F>
const typename task<R(Args...)>::concept_t task<R(Args...)>::template model<F, false>::_vtable = { dtor, move_ctor, invoke,
                                                                                                 target_type, pointer, const_pointer };

template <class R, class... Args>
template <class F>
const typename task<R(Args...)>::concept_t task<R(Args...)>::template model<F, true>::_vtable = { dtor, move_ctor, invoke,
                                                                                                target_type, pointer, const_pointer };
#else

template <class R, class... Args>
template <class F>
const typename task<R(Args...)>::concept_t task<R(Args...)>::model<F, false>::_vtable;

template <class R, class... Args>
template <class F>
const typename task<R(Args...)>::concept_t task<R(Args...)>::model<F, true>::_vtable;

#endif

#endif

#endif

/**************************************************************************************************/

} // namespace v1



