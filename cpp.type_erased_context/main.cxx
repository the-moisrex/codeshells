import standard;

#include <any>
#include <functional>
#include <print>
#include <stdexcept>
#include <utility>

using namespace std;

struct [[nodiscard]] basic_dynamic_context final {
  private:
    std::any context_;

  public:
    template <class CtxT>
    void set(CtxT& ctx) {
        // Store a reference wrapper, not a copy of ctx.
        context_ = std::ref(ctx);
    }

    template <class CtxT, class FuncT>
    void call(FuncT& func) {
        auto* ref = std::any_cast<std::reference_wrapper<CtxT>>(&context_);

        if (ref == nullptr) {
            throw std::bad_any_cast{};
        }

        std::invoke(func, ref->get());
    }
};

struct fake_context {};

struct fake_mod {
    void operator()(fake_context&) { std::println("mod got context"); }
};

int main() {
    basic_dynamic_context dynamic_context;

    fake_context ctx;
    fake_mod mod;

    dynamic_context.set(ctx);
    dynamic_context.call<fake_context>(mod);
}
