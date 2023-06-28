#include <coroutine>
#include <iostream>
#include <utility>

using namespace std;

static int next_id = 1;

template <class T>
struct task {
    std::string spaces = "";
    int id;
    struct promise_type {
        std::string spaces = "";
        promise_type() {
            spaces += " ";
            cout << spaces << "promise(" << id << ")::ctor" << endl;
        }
        int id = next_id++;
        auto get_return_object() {
            cout << spaces << "promise(" << id << ")::get_return_object"
                 << endl;
            return task(
                id, spaces,
                std::coroutine_handle<promise_type>::from_promise(*this));
        }
        std::suspend_always initial_suspend() {
            cout << spaces << "promise(" << id << ")::init_suspend" << endl;
            return {};
        }
        struct final_awaiter {
            string spaces = "";
            promise_type* p;

            final_awaiter(string sp, promise_type* pt) : spaces{sp}, p{pt} {
                spaces += " ";
                cout << spaces << "promise(" << p->id
                     << ")::final_awaiter::ctor" << endl;
            }

            ~final_awaiter() {
                cout << spaces << "promise(" << p->id
                     << ")::final_awaiter::dtor" << endl;
            }

            bool await_ready() noexcept {
                cout << spaces << "promise(" << p->id
                     << ")::final_awaiter::await_ready" << endl;
                return false;
            }
            void await_resume() noexcept {
                cout << spaces << "promise(" << p->id
                     << ")::final_awaiter::await_resume" << endl;
            }
            std::coroutine_handle<>
            await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                cout << spaces << "promise(" << p->id
                     << ")::final_awaiter::await_suspend" << endl;
                // final_awaiter::await_suspend is called when the execution of
                // the current coroutine (referred to by 'h') is about to
                // finish. If the current coroutine was resumed by another
                // coroutine via co_await get_task(), a handle to that coroutine
                // has been stored as h.promise().previous. In that case, return
                // the handle to resume the previous coroutine. Otherwise,
                // return noop_coroutine(), whose resumption does nothing.

                if (auto previous = h.promise().previous; previous)
                    return previous;
                else
                    return std::noop_coroutine();
            }
        };
        final_awaiter final_suspend() noexcept {
            cout << spaces << "promise(" << id << ")::final_suspend" << endl;
            return {spaces, this};
        }
        void unhandled_exception() {
            cout << spaces << "promise(" << id << ")::exception" << endl;
            throw;
        }
        void return_value(T value) {
            cout << spaces << "promise(" << id << ")::return_value(" << value
                 << ")" << endl;
            result = std::move(value);
        }

        ~promise_type() {
            cout << spaces << "promise(" << id << ")::dtor" << endl;
        }

        T result;
        std::coroutine_handle<> previous;
    };

    task(int inp_id, string sp, std::coroutine_handle<promise_type> h)
        : id{inp_id}, spaces{sp}, coro(h) {
        spaces += " ";
        cout << spaces << "task(" << id << ")::init: " << coro.promise().id
            << " result: "  << coro.promise().result << endl;
    }
    task(task&& t) = delete;
    ~task() {
        cout << spaces << "task(" << id << ")::dtor: " << coro.promise().id
             << endl;
        coro.destroy();
    }

    struct awaiter {
        int id;
        std::string spaces = "";
        awaiter(int inp_id, string sp, coroutine_handle<promise_type> inp_coro)
            : id{inp_id}, spaces{sp}, coro{move(inp_coro)} {
            spaces += " ";
            cout << spaces << "task(" << id << ")::awaiter::ctor" << endl;
        }
        ~awaiter() { cout << spaces << "task::awaiter::dtor" << endl; }
        bool await_ready() {
            cout << spaces << "task(" << id << ")::awaiter::ready" << endl;
            return false;
        }
        T await_resume() {
            cout << spaces << "task(" << id
                 << ")::awaiter::resume: " << coro.promise().id
                 << " result: " << coro.promise().result << endl;
            return std::move(coro.promise().result);
        }
        auto await_suspend(std::coroutine_handle<> h) {
            cout << spaces << "task(" << id << ")::awaiter::suspend: old("
                 << coro.promise().id << " result: " << coro.promise().result
                 << ") new("
                 << ")" << endl;
            coro.promise().previous = h;
            return coro;
        }
        std::coroutine_handle<promise_type> coro;
    };
    awaiter operator co_await() {
        cout << spaces << "task(" << id << ")::co_await: " << coro.promise().id
             << endl;
        return awaiter{id, spaces, coro};
    }
    T operator()() {
        cout << spaces << "task(" << id
             << ")::operator T: " << coro.promise().id << endl;
        coro.resume();
        return std::move(coro.promise().result);
    }

  private:
    std::coroutine_handle<promise_type> coro;
};

static int random_val = 10;
task<int> get_random() {
    std::cout << "--- in get_random() ---" << endl;
    co_return random_val++;
}

task<int> test() {
    std::cout << "--- in test() ---" << endl;
    task<int> v = get_random();
    task<int> u = get_random();
    std::cout << "--- in test() - about to co_await ---" << endl;
    int x = (co_await v + co_await u);
    std::cout << "--- in test() - after co_awaits ---" << endl;
    co_return x;
}

int main() {
    task<int> t = test();
    int result = t();
    std::cout << "--- Result" << result << " ---" << endl;
    std::cout << "--- Next Task id: " << next_id << " ---" << endl;
    std::cout << "--- Next random value: " << random_val << " ---" << endl;
}
