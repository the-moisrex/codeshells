import standard;

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <chrono>
#include <iomanip>
#include <typeinfo>
#include <cmath>
#include <exception>

// ============================================================
// Colors (GTest-like)
// ============================================================
namespace Color {
    constexpr auto RESET  = "\033[0m";
    constexpr auto GREEN  = "\033[32m";
    constexpr auto RED    = "\033[31m";
    constexpr auto YELLOW = "\033[33m";
    constexpr auto CYAN   = "\033[36m";
    constexpr auto GRAY   = "\033[90m";
}

// ============================================================
// Core structures
// ============================================================
struct Failure {
    std::string file;
    int line;
    std::string msg;
};

struct TestAbort {}; // For ASSERTs that abort the test

struct TestContext {
    std::vector<Failure> fails;
    void fail(const std::string& file, int line, const std::string& msg) {
        fails.push_back({file, line, msg});
    }
    bool failed() const { return !fails.empty(); }
};

// ============================================================
// Test registry
// ============================================================
struct TestInfo {
    std::string suite, name, typeParam;
    std::function<void(TestContext&)> func;
};

struct TestRegistry {
    std::vector<TestInfo> tests;
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }
    void add(TestInfo t) { tests.push_back(std::move(t)); }
};

// ============================================================
// Utility
// ============================================================
template<typename T>
std::string to_str(const T& v) {
    std::ostringstream os;
    os << v;
    return os.str();
}
inline std::string to_str(const char* s) { return s ? s : "<null>"; }
inline std::string to_str(const std::string& s) { return s; }

// ============================================================
// Assertions
// ============================================================
#define MINI_FAIL(ctx, msg) ctx.fail(__FILE__, __LINE__, msg)

// ---- Core macros ----
#define EXPECT_TRUE(x) \
    do { if(!(x)) MINI_FAIL(ctx, "Expected true but was false: " #x); } while(0)

#define ASSERT_TRUE(x) \
    do { if(!(x)) { MINI_FAIL(ctx, "Expected true but was false: " #x); throw TestAbort(); } } while(0)

#define EXPECT_FALSE(x) \
    do { if((x)) MINI_FAIL(ctx, "Expected false but was true: " #x); } while(0)

#define ASSERT_FALSE(x) \
    do { if((x)) { MINI_FAIL(ctx, "Expected false but was true: " #x); throw TestAbort(); } } while(0)

#define EXPECT_EQ(a, b) \
    do { auto _a=(a), _b=(b); if(!(_a==_b)) { \
        std::ostringstream os; os<<"Expected equality of these values:\n  " #a " = "<<to_str(_a)<<"\n  " #b " = "<<to_str(_b); \
        MINI_FAIL(ctx, os.str()); } } while(0)

#define ASSERT_EQ(a, b) \
    do { auto _a=(a), _b=(b); if(!(_a==_b)) { \
        std::ostringstream os; os<<"Expected equality of these values:\n  " #a " = "<<to_str(_a)<<"\n  " #b " = "<<to_str(_b); \
        MINI_FAIL(ctx, os.str()); throw TestAbort(); } } while(0)

#define EXPECT_NE(a, b) \
    do { auto _a=(a), _b=(b); if((_a==_b)) { \
        std::ostringstream os; os<<"Expected: (" #a ") != (" #b "), both are "<<to_str(_a); MINI_FAIL(ctx, os.str()); } } while(0)

#define ASSERT_NE(a, b) \
    do { auto _a=(a), _b=(b); if((_a==_b)) { \
        std::ostringstream os; os<<"Expected: (" #a ") != (" #b "), both are "<<to_str(_a); MINI_FAIL(ctx, os.str()); throw TestAbort(); } } while(0)

#define EXPECT_LT(a, b) do { auto _a=(a), _b=(b); if(!(_a<_b)) MINI_FAIL(ctx, "Expected " #a " < " #b); } while(0)
#define EXPECT_LE(a, b) do { auto _a=(a), _b=(b); if(!(_a<=_b)) MINI_FAIL(ctx, "Expected " #a " <= " #b); } while(0)
#define EXPECT_GT(a, b) do { auto _a=(a), _b=(b); if(!(_a>_b)) MINI_FAIL(ctx, "Expected " #a " > " #b); } while(0)
#define EXPECT_GE(a, b) do { auto _a=(a), _b=(b); if(!(_a>=_b)) MINI_FAIL(ctx, "Expected " #a " >= " #b); } while(0)

#define EXPECT_NEAR(a, b, tol) \
    do { auto _a=(a), _b=(b), _t=(tol); if(std::fabs(_a-_b)>_t) { \
        std::ostringstream os; os<<"Expected |" #a "-" #b "| <= "<<_t<<", actual diff="<<std::fabs(_a-_b); MINI_FAIL(ctx, os.str()); } } while(0)

#define EXPECT_THROW(stmt, exType) \
    do { bool thrown=false; try { stmt; } catch(const exType&) { thrown=true; } catch(...) {} \
         if(!thrown) MINI_FAIL(ctx, "Expected exception of type " #exType " not thrown: " #stmt); } while(0)

#define EXPECT_NO_THROW(stmt) \
    do { try { stmt; } catch(...) { MINI_FAIL(ctx, "Expected no exception, but got one: " #stmt); } } while(0)

// ============================================================
// Test registration
// ============================================================
struct RegisterTest {
    RegisterTest(const char* suite, const char* name, std::function<void(TestContext&)> func, const char* type = nullptr) {
        TestRegistry::instance().add({suite, name, type ? type : "", std::move(func)});
    }
};

#define UNIQUE(base, line) base##line
#define UNIQUE_NAME(base, line) UNIQUE(base, line)

// ============================================================
// TEST and TEST_F
// ============================================================
#define TEST(suite, name) \
    static void UNIQUE_NAME(_mini_fn_, __LINE__)(TestContext&); \
    static RegisterTest UNIQUE_NAME(_mini_reg_, __LINE__)(#suite, #name, UNIQUE_NAME(_mini_fn_, __LINE__)); \
    static void UNIQUE_NAME(_mini_fn_, __LINE__)(TestContext& ctx)

struct TestFixture {
    virtual void SetUp() {}
    virtual void TearDown() {}
    virtual ~TestFixture() = default;
};

#define TEST_F(fixture, name) \
    struct UNIQUE_NAME(_mini_fx_, __LINE__) : public fixture { void TestBody(TestContext& ctx); }; \
    static void UNIQUE_NAME(_mini_fx_run_, __LINE__)(TestContext& ctx) { \
        UNIQUE_NAME(_mini_fx_, __LINE__) fx; fx.SetUp(); \
        try { fx.TestBody(ctx); } catch(TestAbort&) {} \
        fx.TearDown(); } \
    static RegisterTest UNIQUE_NAME(_mini_fx_reg_, __LINE__)(#fixture, #name, UNIQUE_NAME(_mini_fx_run_, __LINE__)); \
    void UNIQUE_NAME(_mini_fx_, __LINE__)::TestBody(TestContext& ctx)

// ============================================================
// TYPED TEST
// ============================================================
template <typename... Ts>
struct TypeList {};

template <template<typename> typename Fixture, typename... Ts>
struct TypedTestRegistrar {
    TypedTestRegistrar(const std::string& suite, const std::string& name,
                       std::function<void(TestContext&, const std::type_info&)> func) {
        (TestRegistry::instance().add({
            suite,
            name,
            typeid(Ts).name(),
            [=](TestContext& ctx){
                Fixture<Ts> fx;
                fx.SetUp();
                try { func(ctx, typeid(Ts)); } catch(TestAbort&) {}
                fx.TearDown();
            }
        }), ...);
    }
};

#define TYPED_TEST_SUITE(fixture, types) using fixture##_Types = types;

#define TYPED_TEST(fixture, name) \
    template<typename TypeParam> struct UNIQUE_NAME(_mini_typed_, __LINE__) : public fixture<TypeParam> { void TestBody(TestContext& ctx); }; \
    template<typename TypeParam> void UNIQUE_NAME(_mini_typed_, __LINE__)<TypeParam>::TestBody(TestContext& ctx)

#define REGISTER_TYPED_TEST(fixture, name) \
    static TypedTestRegistrar<fixture, typename fixture##_Types::template types...> \
    UNIQUE_NAME(_mini_typed_reg_, __LINE__)(#fixture, #name, [](TestContext& ctx, const std::type_info& tinfo){ \
        if (tinfo == typeid(int)) { fixture<int> fx; fx.SetUp(); fx.TestBody(ctx); fx.TearDown(); } \
        else if (tinfo == typeid(double)) { fixture<double> fx; fx.SetUp(); fx.TestBody(ctx); fx.TearDown(); } \
    });

// ============================================================
// Runner
// ============================================================
int RunAllTests() {
    using namespace std::chrono;
    int total = 0, failed = 0;
    auto start_all = high_resolution_clock::now();

    for (auto& t : TestRegistry::instance().tests) {
        ++total;
        std::string fullname = t.suite + "." + t.name;
        if (!t.typeParam.empty()) fullname += "<" + t.typeParam + ">";
        std::cout << Color::CYAN << "[ RUN      ] " << Color::RESET << fullname << "\n";

        TestContext ctx;
        auto start = high_resolution_clock::now();
        try { t.func(ctx); } catch(TestAbort&) {}
        auto end = high_resolution_clock::now();
        auto dur = duration_cast<milliseconds>(end - start).count();

        if (ctx.failed()) {
            ++failed;
            std::cout << Color::RED << "[  FAILED  ] " << Color::RESET << fullname 
                      << Color::GRAY << " (" << dur << " ms)\n";
            for (auto& f : ctx.fails)
                std::cout << "  " << f.file << ":" << f.line << ": " << f.msg << "\n";
        } else {
            std::cout << Color::GREEN << "[       OK ] " << Color::RESET << fullname 
                      << Color::GRAY << " (" << dur << " ms)" << Color::RESET << "\n";
        }
    }

    auto total_dur = duration_cast<milliseconds>(high_resolution_clock::now() - start_all).count();
    std::cout << "\n" << Color::GRAY << "[==========] " << Color::RESET
              << "Ran " << total << " test(s) in " << total_dur << " ms.\n";
    std::cout << Color::GRAY << "[  PASSED  ] " << Color::RESET << (total - failed) << " test(s).\n";
    if (failed)
        std::cout << Color::RED << "[  FAILED  ] " << Color::RESET << failed << " test(s), see above.\n";
    else
        std::cout << Color::GREEN << "[  PASSED  ] All tests passed!\n" << Color::RESET;
    return failed ? 1 : 0;
}

// ============================================================
// Example Tests
// ============================================================
TEST(Math, BasicOps) {
    EXPECT_EQ(2+2, 4);
    EXPECT_NE(5, 6);
    EXPECT_LT(1, 2);
    EXPECT_GE(3, 3);
    EXPECT_NEAR(3.14, 3.141, 0.01);
}

TEST(Exception, Throws) {
    EXPECT_THROW(throw std::runtime_error("fail"), std::runtime_error);
    EXPECT_NO_THROW(int x = 42);
}

struct MyFixture : TestFixture {
    int v{};
    void SetUp() override { v = 10; }
};
TEST_F(MyFixture, FixtureCheck) {
    ASSERT_EQ(v, 10);
    v++;
    EXPECT_GT(v, 10);
}

template<typename T>
struct TypedFix : TestFixture {
    T val{};
    void SetUp() override { val = T(5); }
};
using types = TypeList<int, double>;
TYPED_TEST_SUITE(TypedFix, types);
TYPED_TEST(TypedFix, Works) {
    this->val += 1;
    EXPECT_GT(this->val, 5);
}

int main() { return RunAllTests(); }

