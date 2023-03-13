#include <functional>
#include <memory>
#include <type_traits>

// source: https://github.com/foonathan/type_safe/blob/main/include/type_safe/reference.hpp#L489-L655

namespace detail
{
    template <typename Returned, typename Required>
    struct compatible_return_type
    : std::integral_constant<bool, std::is_void<Required>::value
                                       || std::is_convertible<Returned, Required>::value>
    {};

    struct matching_function_pointer_tag
    {};
    struct matching_functor_tag
    {};
    struct invalid_functor_tag
    {};

    template <typename Func, typename Return, typename... Args>
    struct get_callable_tag
    {
        // use unary + to convert to function pointer
        template <typename T,
                  typename Result = decltype((+std::declval<T&>())(std::declval<Args>()...))>
        static matching_function_pointer_tag test(
            int, T& obj,
            typename std::enable_if<compatible_return_type<Result, Return>::value, int>::type = 0);

        template <typename T,
                  typename Result = decltype(std::declval<T&>()(std::declval<Args>()...))>
        static matching_functor_tag test(
            short, T& obj,
            typename std::enable_if<compatible_return_type<Result, Return>::value, int>::type = 0);

        static invalid_functor_tag test(...);

        using type = decltype(test(0, std::declval<Func&>()));
    };

    template <typename Result, typename Func, typename Return, typename... Args>
    using enable_function_tag = typename std::enable_if<
        std::is_same<typename get_callable_tag<Func, Return, Args...>::type, Result>::value,
        int>::type;
} // namespace detail


template <typename Signature>
class function_ref;

/// A reference to a function.
///
/// This is a lightweight reference to a function.
/// It can refer to any function that is compatible with given signature.
///
/// A function is compatible if it is callable with regular function call syntax
/// from the given argument types, and its return type is either implicitly
/// convertible to the specified return type or the specified return type is
/// `void`.
///
/// In general it will store a pointer to the functor,
/// requiring an lvalue.
/// But if it is created with a function pointer or something convertible to a
/// function pointer, it will store the function pointer itself. This allows
/// creating it from stateless lambdas. \notes Due to implementation reasons, it
/// does not support member function pointers, as it requires regular function
/// call syntax. Create a reference to the object returned by [std::mem_fn](),
/// if that is required.
template <typename Return, typename... Args>
class function_ref<Return(Args...)> {
  public:
    using signature = Return(Args...);

    /// \effects Creates a reference to the function specified by the function
    /// pointer. \requires `fptr` must not be `nullptr`. \notes (2) only
    /// participates in overload resolution if the type of the function is
    /// compatible with the specified signature. \group function_ptr_ctor
    function_ref(Return (*fptr)(Args...))
        : function_ref(detail::matching_function_pointer_tag{}, fptr) {}

    /// \group function_ptr_ctor
    /// \param 1
    /// \exclude
    template <typename Return2, typename... Args2>
    function_ref(Return2 (*fptr)(Args2...),
                 typename std::enable_if<
                     detail::compatible_return_type<Return2, Return>::value,
                     int>::type = 0)
        : function_ref(detail::matching_function_pointer_tag{}, fptr) {}

    /// \effects Creates a reference to the function created by the stateless
    /// lambda. \notes This constructor is intended for stateless lambdas, which
    /// are implicitly convertible to function pointers. It does not participate
    /// in overload resolution, unless the type is implicitly convertible to a
    /// function pointer that is compatible with the specified signature. \notes
    /// Due to to implementation reasons, it does not work for polymorphic
    /// lambdas, it needs an explicit cast to the desired function pointer type.
    /// A polymorphic lambda convertible to a direct match function pointer,
    /// works however.
    /// \param 1
    /// \exclude
    template <typename Functor, typename = detail::enable_function_tag<
                                    detail::matching_function_pointer_tag,
                                    Functor, Return, Args...>>
    function_ref(const Functor& f)
        : function_ref(detail::matching_function_pointer_tag{}, +f) {}

    /// \effects Creates a reference to the specified functor.
    /// It will store a pointer to the function object,
    /// so it must live as long as the reference.
    /// \notes This constructor does not participate in overload resolution,
    /// unless the functor is compatible with the specified signature.
    /// \param 1
    /// \exclude
    template <typename Functor,
              typename = detail::enable_function_tag<
                  detail::matching_functor_tag, Functor, Return, Args...>>
    explicit function_ref(Functor& f) : cb_(&invoke_functor<Functor>) {
        ::new (get_memory()) void*(&f);
    }

    /// Converting copy constructor.
    /// \effects Creates a reference to the same function referred by `other`.
    /// \notes This constructor does not participate in overload resolution,
    /// unless the signature of `other` is compatible with the specified
    /// signature. \notes This constructor may create a bigger conversion chain.
    /// For example, if `other` has signature `void(const char*)` it can refer
    /// to a function taking `std::string`. If this signature than accepts a
    /// type `T` implicitly convertible to `const char*`, calling this will call
    /// the function taking `std::string`, converting `T -> std::string`, even
    /// though such a conversion would be ill-formed otherwise. \param 1
    /// \exclude
    template <typename Return2, typename... Args2>
    explicit function_ref(
        const function_ref<Return2(Args2...)>& other,
        typename std::enable_if<
            detail::compatible_return_type<Return2, Return>::value, int>::type =
            0)
        : storage_(other.storage_), cb_(other.cb_) {}

    /// \effects Rebinds the reference to the specified functor.
    /// \notes This assignment operator only participates in overload
    /// resolution, if the argument can also be a valid constructor argument.
    /// \param 1
    /// \exclude
    template <typename Functor,
              typename = typename std::enable_if<
                  !std::is_same<typename std::decay<Functor>::type,
                                function_ref>::value,
                  decltype(function_ref(std::declval<Functor&&>()))>::type>
    void assign(Functor&& f) noexcept {
        auto ref = function_ref(std::forward<Functor>(f));
        storage_ = ref.storage_;
        cb_ = ref.cb_;
    }

    /// \effects Invokes the stored function with the specified arguments and
    /// returns the result.
    Return operator()(Args... args) const {
        return cb_(get_memory(), static_cast<Args>(args)...);
    }

  private:
    template <typename Functor>
    static Return invoke_functor(const void* memory, Args... args) {
        using ptr_t = void*;
        ptr_t ptr = *static_cast<const ptr_t*>(memory);
        Functor& func = *static_cast<Functor*>(ptr);
        return static_cast<Return>(func(static_cast<Args>(args)...));
    }

    template <typename PointerT, typename StoredT>
    static Return invoke_function_pointer(const void* memory, Args... args) {
        auto ptr = *static_cast<const StoredT*>(memory);
        auto func = reinterpret_cast<PointerT>(ptr);
        return static_cast<Return>(func(static_cast<Args>(args)...));
    }

    template <typename Return2, typename... Args2>
    function_ref(detail::matching_function_pointer_tag,
                 Return2 (*fptr)(Args2...)) {
        using pointer_type = Return2 (*)(Args2...);
        using stored_pointer_type = void (*)();

        //DEBUG_ASSERT(fptr, detail::precondition_error_handler{},
         //            "function pointer must not be null");
        ::new (get_memory())
            stored_pointer_type(reinterpret_cast<stored_pointer_type>(fptr));

        cb_ = &invoke_function_pointer<pointer_type, stored_pointer_type>;
    }

    void* get_memory() noexcept { return &storage_; }

    const void* get_memory() const noexcept { return &storage_; }

    // using storage  = aligned_union_t<void*, Return (*)(Args...)>;
    //
    template <typename... Ts>
    struct storage_type {
        // std::aligned_union_t<0, Ts...> t_buff;
        alignas(Ts...) std::byte data[std::max({sizeof(Ts)...})];
    };
    using storage = storage_type<void*, Return (*)(Args...)>;
    using callback = Return (*)(const void*, Args...);

    storage storage_;
    callback cb_;
};



