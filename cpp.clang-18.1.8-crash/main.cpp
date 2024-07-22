namespace details {

    template <typename SigPtr>
    union storage {
        void* obj_ptr = nullptr;

        template <typename T>
        storage(T& inp_ptr) : obj_ptr(&inp_ptr) {}
    };

    template <typename T, typename SigPtr>
    static constexpr auto get(storage<SigPtr> obj) noexcept {
        return static_cast<T*>(obj.obj_ptr);
    }

} // namespace details

template <typename Signature>
struct function_ref;

template <typename Return, typename... Args>
struct function_ref<Return(Args...)> {
    using signature_ptr = Return (*)(Args...);

  private:
    using storage_type = details::storage<signature_ptr>;
    using self_signature = Return (*)(storage_type, Args...);

    template <typename T>
    static constexpr Return
    invoker(storage_type obj,
            Args... args) noexcept(noexcept((*details::get<T>(obj))(args...))) {
        return static_cast<Return>((*details::get<T>(obj))(args...));
    }

    storage_type obj{this};
    self_signature erased_func = nullptr;

  public:
    template <typename T>
    function_ref(T& inp_func)
        : obj{inp_func}, erased_func{&function_ref::invoker<T>} {}

    Return operator()(Args... args) const {
        return (*erased_func)(obj, args...);
    }
};

int main() {
    auto ret20 = [] { return 20; };
    function_ref<int()> view{ret20};
    if (view() == 20) {
        return 1;
    }
    return 0;
}
