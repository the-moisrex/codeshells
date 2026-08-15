import standard;

#include <functional>
#include <memory>
#include <print>
#include <stdexcept>
#include <type_traits>
#include <utility>

struct basic_dynamic_context {
  private:
    void* ptr_ = nullptr;
    void const* type_ = nullptr;

    template <typename T>
    static void const* type_token() noexcept {
        static const int token{};
        return std::addressof(token);
    }

  public:
    template <typename T>
    void set(T& value) noexcept {
        using U = std::remove_cvref_t<T>;

        ptr_ = std::addressof(value);
        type_ = type_token<U>();
    }

    template <typename T, typename F>
    decltype(auto) call(F&& function) const {
        using U = std::remove_cvref_t<T>;

        if (type_ != type_token<U>()) {
            throw std::bad_cast{};
        }

        return std::invoke(std::forward<F>(function), *static_cast<U*>(ptr_));
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
