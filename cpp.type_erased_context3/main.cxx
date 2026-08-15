import standard;

#include <functional>
#include <memory>
#include <print>
#include <stdexcept>
#include <type_traits>
#include <utility>

struct basic_dynamic_context {
  private:
    using erased_callback = std::move_only_function<void(void*)>;
    using context_provider = std::move_only_function<void(erased_callback)>;

    context_provider provide_context_;
    void const* type_tag_ = nullptr;

    template <class T>
    static void const* type_token() noexcept {
        static const int token{};
        return std::addressof(token);
    }

  public:
    template <class CtxT>
    void set(CtxT& ctx) {
        using context_type = std::remove_cvref_t<CtxT>;

        type_tag_ = type_token<context_type>();

        provide_context_ = [&ctx](erased_callback callback) {
            callback(std::addressof(ctx));
        };
    }

    template <class FuncT>
    void call(FuncT& func) {
        // The module provides the type lost by the type erasure.
        using context_type = typename FuncT::context_type;

        if (!provide_context_) {
            throw std::logic_error("Context was not set");
        }

        if (type_tag_ != type_token<context_type>()) {
            throw std::bad_cast{};
        }

        provide_context_([&func](void* erased_context) {
            auto& ctx = *static_cast<context_type*>(erased_context);
            std::invoke(func, ctx);
        });
    }
};

struct fake_context {};

struct fake_mod {
    using context_type = fake_context;

    void operator()(fake_context&) const { std::println("mod got context"); }
};

int main() {
    basic_dynamic_context dynamic_context;

    fake_context ctx;
    fake_mod mod;

    dynamic_context.set(ctx);
    dynamic_context.call(mod);
}
