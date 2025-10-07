import standard;

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

#if defined(__GNUG__)
# include <cxxabi.h>
#endif

namespace testing {

// -------------------- Utilities --------------------
inline std::string demangle(const char* name) {
#if defined(__GNUG__)
    int status = 0;
    char* dem = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    if (dem) {
        std::string s(dem);
        free(dem);
        return s;
    }
    return std::string(name);
#else
    return std::string(name);
#endif
}

template<typename T>
inline std::string TypeName() {
    return demangle(typeid(T).name());
}

// ANSI colors
namespace color {
constexpr const char* RESET = "\033[0m";
constexpr const char* RED   = "\033[31m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* YELLOW= "\033[33m";
constexpr const char* BLUE  = "\033[34m";
constexpr const char* CYAN  = "\033[36m";
} // namespace color

// -------------------- Test Registry --------------------
struct TestInfo {
    std::string suite;
    std::string name;
    std::string full_name() const { return suite + "." + name; }
    std::function<void()> func;
    std::int64_t duration_ns = 0;
    bool failed = false;
};

class Registry {
public:
    static Registry& Instance() {
        static Registry inst;
        return inst;
    }

    void RegisterTest(const std::string& suite, const std::string& name, std::function<void()> fn) {
        std::lock_guard<std::mutex> lk(mu_);
        tests_.push_back(TestInfo{suite, name, fn, 0, false});
    }

    std::vector<TestInfo>& tests() { return tests_; }

    void AddFailure(const std::string& suite, const std::string& test) {
        std::lock_guard<std::mutex> lk(mu_);
        for (auto &t : tests_) if (t.suite==suite && t.name==test) { t.failed = true; break; }
    }

private:
    std::mutex mu_;
    std::vector<TestInfo> tests_;
};

// convenience
inline void RegisterTest(const char* suite, const char* name, std::function<void()> fn) {
    Registry::Instance().RegisterTest(suite, name, std::move(fn));
}

// -------------------- Assertion infra --------------------
struct AssertionContext {
    const char* file = "";
    int line = 0;
    std::string expr1;
    std::string expr2;
    std::string macro_name; // EXPECT/ASSERT
    std::string extra;
    bool success = true;
    std::string message() const {
        std::ostringstream ss;
        if (!success) {
            ss << color::RED << macro_name << " failed: " << color::RESET;
            ss << expr1;
            if (!expr2.empty()) ss << " vs " << expr2;
            if (!extra.empty()) ss << "\n  " << extra;
            ss << "\n  (" << file << ":" << line << ")";
        } else {
            ss << color::GREEN << macro_name << " OK: " << color::RESET << expr1;
            if (!expr2.empty()) ss << " vs " << expr2;
        }
        return ss.str();
    }
};

// An object returned by assertion helper. Supports operator<< chaining.
class AssertionResult {
public:
    AssertionResult(bool success, const char* file, int line, std::string expr1,
                    std::string expr2, std::string macro_name, const char* suite, const char* test)
        : ctx_(), suite_(suite), test_(test) {
        ctx_.success = success;
        ctx_.file = file;
        ctx_.line = line;
        ctx_.expr1 = std::move(expr1);
        ctx_.expr2 = std::move(expr2);
        ctx_.macro_name = std::move(macro_name);
    }

    // streaming operator
    template<typename T>
    AssertionResult& operator<<(T&& v) {
        std::ostringstream tmp;
        tmp << std::forward<T>(v);
        if (!ctx_.extra.empty()) ctx_.extra += " ";
        ctx_.extra += tmp.str();
        return *this;
    }

    // destructor performs reporting
    ~AssertionResult() {
        if (!ctx_.success) {
            std::cerr << ctx_.message() << "\n";
            // mark test failed in registry
            Registry::Instance().AddFailure(suite_, test_);
            ++testing::internal::global_failures;
            ++testing::internal::global_assertions;
        } else {
            ++testing::internal::global_assertions;
        }
    }

    // allow test bodies to query success (rarely used)
    bool ok() const { return ctx_.success; }

private:
    AssertionContext ctx_;
    const char* suite_;
    const char* test_;
};

namespace internal {
    inline std::atomic<int> global_assertions{0};
    inline std::atomic<int> global_failures{0};
}

// helpers to create assertion results
template<typename A, typename B>
AssertionResult make_eq(const A& a, const B& b, const char* file, int line,
                        const char* exprA, const char* exprB,
                        const char* suite, const char* test, bool is_assert) {
    bool ok = (a == b);
    std::ostringstream e1, e2;
    e1 << a;
    e2 << b;
    return AssertionResult(ok, file, line, std::string(exprA) + " (" + e1.str() + ")", std::string(exprB) + " (" + e2.str() + ")", is_assert ? "ASSERT_EQ" : "EXPECT_EQ", suite, test);
}

inline AssertionResult make_true(bool value, const char* file, int line, const char* expr,
                                 const char* suite, const char* test, bool is_assert) {
    bool ok = value;
    return AssertionResult(ok, file, line, std::string(expr) + " (" + (value ? "true" : "false") + ")", std::string(), is_assert ? "ASSERT_TRUE" : "EXPECT_TRUE", suite, test);
}

// generic functions for macros to call (returns rvalue object to allow operator<<)
#define DEFINE_EXPECT_IMPL(NAME, PREDICATE) \
template<typename A, typename B> \
inline AssertionResult NAME(const A& a, const B& b, const char* file, int line, const char* exprA, const char* exprB, const char* suite, const char* test, bool is_assert) { \
    bool ok = (PREDICATE); \
    std::ostringstream sa, sb; sa << a; sb << b; \
    return AssertionResult(ok, file, line, std::string(exprA) + " (" + sa.str() + ")", std::string(exprB) + " (" + sb.str() + ")", (is_assert ? "ASSERT_" NAME : "EXPECT_" NAME), suite, test); \
}

DEFINE_EXPECT_IMPL(EQ, a == b)
#undef DEFINE_EXPECT_IMPL

// EXPECT/ASSERT TRUE/FALSE helpers
inline AssertionResult expect_true(bool v, const char* file, int line, const char* expr, const char* suite, const char* test, bool is_assert) {
    return make_true(v, file, line, expr, suite, test, is_assert);
}

// -------------------- TEST macro machinery --------------------

class Test {
public:
    virtual ~Test() = default;
    virtual void SetUp() {}
    virtual void TearDown() {}
};

struct CurrentTestInfo {
    const char* suite = "";
    const char* test = "";
};
inline CurrentTestInfo& current_test() {
    static thread_local CurrentTestInfo info;
    return info;
}

// Runner
inline int RunAllTests() {
    auto& reg = Registry::Instance();
    auto& tests = reg.tests();
    std::cout << color::CYAN << "[==========] Running " << tests.size() << " tests.\n" << color::RESET;

    using clock = std::chrono::high_resolution_clock;
    std::int64_t total_ns = 0;
    int run = 0;
    for (auto& t : tests) {
        ++run;
        std::cout << color::BLUE << "[ RUN      ] " << color::RESET << t.suite << "." << t.name << "\n";
        current_test().suite = t.suite.c_str();
        current_test().test = t.name.c_str();
        auto start = clock::now();
        try {
            t.func();
        } catch (const std::exception& ex) {
            std::cerr << color::RED << "[  EXC     ] Exception in test: " << ex.what() << color::RESET << "\n";
            Registry::Instance().AddFailure(t.suite, t.name);
        } catch (...) {
            std::cerr << color::RED << "[  EXC     ] Unknown exception in test" << color::RESET << "\n";
            Registry::Instance().AddFailure(t.suite, t.name);
        }
        auto end = clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        t.duration_ns = ns;
        total_ns += ns;
        if (t.failed) {
            std::cout << color::RED << "[  FAILED  ] " << color::RESET << t.suite << "." << t.name
                      << " (" << ns << " ns, " << (ns/1000) << " us)\n";
        } else {
            std::cout << color::GREEN << "[       OK ] " << color::RESET << t.suite << "." << t.name
                      << " (" << ns << " ns, " << (ns/1000) << " us)\n";
        }
    }

    std::cout << color::CYAN << "[==========] " << tests.size() << " tests ran. Total time: "
              << total_ns << " ns (" << (total_ns/1000) << " us)." << color::RESET << "\n";

    int failures = (int)internal::global_failures.load();
    int assertions = (int)internal::global_assertions.load();
    std::cout << (failures ? color::RED : color::GREEN)
              << "[  SUMMARY ] " << tests.size() << " tests, " << assertions << " assertions, "
              << failures << " failures." << color::RESET << "\n";

    // Per-test recap
    std::cout << color::YELLOW << "Per-test timing (ns / us):\n" << color::RESET;
    for (auto &t : tests) {
        std::cout << "  " << t.suite << "." << t.name << ": " << t.duration_ns << " ns, " << (t.duration_ns/1000) << " us"
                  << (t.failed ? " [FAILED]" : "") << "\n";
    }

    return failures ? 1 : 0;
}

// -------------------- typed-test support --------------------

// testing::Types
template<typename... Ts>
struct Types { using type_list = std::tuple<Ts...>; };

// ForEachType helper: apply a templated callable to each type in Types<...>
template<typename TypeList>
struct ForEachType;

template<typename... Ts>
struct ForEachType<Types<Ts...>> {
    template<typename F>
    static void apply(F&& f) {
        // fold expression to call templated lambda for each type
        (f.template operator()<Ts>(), ...);
    }
};

// -------------------- Macros --------------------

// Helper to make unique names
#define GTEST_CLONE_CONCAT_INTERNAL_(a, b) a##b
#define GTEST_CLONE_CONCAT(a, b) GTEST_CLONE_CONCAT_INTERNAL_(a, b)
#define GTEST_CLONE_UNIQUE_NAME(base) GTEST_CLONE_CONCAT(base, __COUNTER__)

// TEST macro (creates a function and registers it)
#define TEST(test_suite_name, test_name) \
    struct GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_Test) : public ::testing::Test { \
        void TestBody(); \
        static void RunIt() { GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_Test) t; t.SetUp(); t.TestBody(); t.TearDown(); } \
    }; \
    static int GTEST_CLONE_UNIQUE_NAME(_reg_) = ( ::testing::RegisterTest(#test_suite_name, #test_name, &GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_Test)::RunIt), 0 ); \
    void GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_Test)::TestBody()

// TEST_F maps to TEST (this clone keeps things simple)
#define TEST_F(test_fixture, test_name) TEST(test_fixture, test_name)

// -------------------- Assertion macros (support <<) --------------------
// They return a temporary AssertionResult object (rvalue), so `EXPECT_EQ(a,b) << "..."` works.

#define EXPECT_EQ(a, b) (::testing::make_eq((a), (b), __FILE__, __LINE__, #a, #b, ::testing::current_test().suite, ::testing::current_test().test, false))
#define ASSERT_EQ(a, b) (::testing::make_eq((a), (b), __FILE__, __LINE__, #a, #b, ::testing::current_test().suite, ::testing::current_test().test, true))

#define EXPECT_TRUE(x) (::testing::expect_true((x), __FILE__, __LINE__, #x, ::testing::current_test().suite, ::testing::current_test().test, false))
#define ASSERT_TRUE(x) (::testing::expect_true((x), __FILE__, __LINE__, #x, ::testing::current_test().suite, ::testing::current_test().test, true))

// Some convenience other assertions
#define EXPECT_NE(a,b) ((a) != (b) ? ::testing::AssertionResult(true, __FILE__, __LINE__, #a, #b, "EXPECT_NE", ::testing::current_test().suite, ::testing::current_test().test) : ::testing::AssertionResult(false, __FILE__, __LINE__, #a, #b, "EXPECT_NE", ::testing::current_test().suite, ::testing::current_test().test))

// -------------------- Typed TEST macros --------------------

// TYPED_TEST_SUITE must accept template type lists with commas. Make it variadic to absorb extra tokens.
#define TYPED_TEST_SUITE(test_suite_name, types, ...) \
    using GTEST_CLONE_CONCAT(test_suite_name, _Types) = types;

// TYPED_TEST: create a function template (TypeParam available) and register one test per type listed in suite's Types alias
#define TYPED_TEST(test_suite_name, test_name) \
    template<typename TypeParam> void GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_TypedTest)(); \
    namespace { \
        struct GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_Registrar) { \
            GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_Registrar)() { \
                using TL = GTEST_CLONE_CONCAT(test_suite_name, _Types); \
                ::testing::ForEachType<TL>::apply([&]<typename T>() { \
                    std::string composed = std::string(#test_name) + "<" + ::testing::TypeName<T>() + ">"; \
                    ::testing::RegisterTest(#test_suite_name, composed.c_str(), [](){ GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_TypedTest)<T>(); }); \
                }); \
            } \
        }; \
        static GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_Registrar) GTEST_CLONE_UNIQUE_NAME(_typed_reg_); \
    } \
    template<typename TypeParam> void GTEST_CLONE_CONCAT(test_suite_name, _##test_name##_TypedTest)()

} // namespace testing

// ------------ main helper (optional) ---------------
// Provide a default main if user doesn't define one.
#ifndef GTEST_CLONE_NO_DEFAULT_MAIN
int main(int argc, char** argv) {
    (void)argc; (void)argv;
    return ::testing::RunAllTests();
}
#endif


#include <string>

TEST(Simple, Passes) {
    EXPECT_EQ(1 + 1, 2) << "1+1 should be 2";
    EXPECT_TRUE(true);
}

TEST(Simple, FailsButContinues) {
    EXPECT_EQ(2 * 2, 5) << "wrong multiply";
    ASSERT_EQ(3, 4) << "ASSERT won't abort here (intentional for this clone)";
    EXPECT_TRUE(false) << "still continues";
}

// Typed tests
template <typename T>
class MyTypedTest : public testing::Test {
public:
    using TypeParam = T;
};

TYPED_TEST_SUITE(MyTypedTest, testing::Types<int, double, long>);

TYPED_TEST(MyTypedTest, WorksWithTypeParam) {
    // TypeParam is available:
    using T = TypeParam;
    if constexpr (std::is_integral_v<T>) {
        EXPECT_TRUE(sizeof(T) >= 4) << "integral type too small";
    } else {
        EXPECT_TRUE(sizeof(T) >= 8) << "floating point size check";
    }
}

