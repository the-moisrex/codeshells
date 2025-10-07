import standard;

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <type_traits>
#include <utility>
#include <exception>
#include <stdexcept>
#include <source_location>
#include <cstring>
#include <cmath>
#include <map>
#include <set>
#include <functional>  // Added this header for std::function

namespace testing {

// Color codes for output
namespace colors {
    const char* RED = "\033[31m";
    const char* GREEN = "\033[32m";
    const char* YELLOW = "\033[33m";
    const char* BLUE = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN = "\033[36m";
    const char* WHITE = "\033[37m";
    const char* RESET = "\033[0m";
}

// Test result enumeration
enum class TestResult {
    PASSED,
    FAILED,
    SKIPPED
};

// Test information structure
struct TestInfo {
    std::string test_case_name;
    std::string test_name;
    std::function<void()> test_func;
    TestResult result;
    std::string failure_message;
    std::chrono::nanoseconds duration;
    
    TestInfo(std::string case_name, std::string name, std::function<void()> func)
        : test_case_name(std::move(case_name)), test_name(std::move(name)), 
          test_func(std::move(func)), result(TestResult::PASSED), duration(0) {}
};

// Test registry
class TestRegistry {
public:
    static TestRegistry& getInstance() {
        static TestRegistry instance;
        return instance;
    }
    
    void addTest(TestInfo test_info) {
        tests_.push_back(std::move(test_info));
    }
    
    void runAllTests() {
        std::cout << colors::CYAN << "[==========] " << colors::RESET 
                  << "Running " << tests_.size() << " tests from " 
                  << getTestCaseCount() << " test cases." << std::endl;
        
        int total_passed = 0;
        int total_failed = 0;
        std::chrono::nanoseconds total_duration(0);
        
        // Group tests by test case
        std::map<std::string, std::vector<TestInfo*>> test_cases;
        for (auto& test : tests_) {
            test_cases[test.test_case_name].push_back(&test);
        }
        
        // Run tests by test case
        for (const auto& [test_case_name, case_tests] : test_cases) {
            std::cout << colors::CYAN << "[----------] " << colors::RESET 
                      << case_tests.size() << " tests from " << test_case_name << std::endl;
            
            for (auto* test : case_tests) {
                auto start = std::chrono::high_resolution_clock::now();
                
                try {
                    test->test_func();
                    test->result = TestResult::PASSED;
                    total_passed++;
                    
                    std::cout << colors::GREEN << "[       OK ] " << colors::RESET 
                              << test_case_name << "." << test->test_name 
                              << " (" << formatDuration(test->duration) << ")" << std::endl;
                } catch (const std::exception& e) {
                    test->result = TestResult::FAILED;
                    test->failure_message = e.what();
                    total_failed++;
                    
                    std::cout << colors::RED << "[  FAILED  ] " << colors::RESET 
                              << test_case_name << "." << test->test_name 
                              << " (" << formatDuration(test->duration) << ")" << std::endl;
                    std::cout << colors::RED << test->failure_message << colors::RESET << std::endl;
                }
                
                auto end = std::chrono::high_resolution_clock::now();
                test->duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                total_duration += test->duration;
            }
            
            std::cout << colors::CYAN << "[----------] " << colors::RESET 
                      << case_tests.size() << " tests from " << test_case_name 
                      << " (" << formatDuration(getTestCaseDuration(test_case_name)) << " total)" 
                      << std::endl << std::endl;
        }
        
        // Final recap
        std::cout << colors::CYAN << "[==========] " << colors::RESET 
                  << tests_.size() << " tests from " << getTestCaseCount() 
                  << " test cases ran. (" << formatDuration(total_duration) << " total)" << std::endl;
        
        std::cout << "[  PASSED  ] " << total_passed << " tests." << std::endl;
        
        if (total_failed > 0) {
            std::cout << colors::RED << "[  FAILED  ] " << colors::RESET 
                      << total_failed << " tests, listed below:" << std::endl;
            
            for (const auto& test : tests_) {
                if (test.result == TestResult::FAILED) {
                    std::cout << colors::RED << "[  FAILED  ] " << colors::RESET 
                              << test.test_case_name << "." << test.test_name << std::endl;
                }
            }
            
            std::cout << std::endl << total_failed << " FAILED TESTS" << std::endl;
        }
    }
    
private:
    std::vector<TestInfo> tests_;
    
    size_t getTestCaseCount() const {
        std::set<std::string> test_cases;
        for (const auto& test : tests_) {
            test_cases.insert(test.test_case_name);
        }
        return test_cases.size();
    }
    
    std::chrono::nanoseconds getTestCaseDuration(const std::string& test_case_name) const {
        std::chrono::nanoseconds duration(0);
        for (const auto& test : tests_) {
            if (test.test_case_name == test_case_name) {
                duration += test.duration;
            }
        }
        return duration;
    }
    
    std::string formatDuration(std::chrono::nanoseconds duration) const {
        auto ns = duration.count();
        if (ns < 1000) {
            return std::to_string(ns) + " ns";
        } else if (ns < 1000000) {
            return std::to_string(static_cast<double>(ns) / 1000.0) + " μs";
        } else {
            return std::to_string(static_cast<double>(ns) / 1000000.0) + " ms";
        }
    }
};

// Assertion helper class
class AssertionHelper {
public:
    AssertionHelper(bool condition, const char* condition_str, 
                   const char* file, int line, const char* function)
        : condition_(condition), condition_str_(condition_str), 
          file_(file), line_(line), function_(function) {}
    
    ~AssertionHelper() noexcept(false) {
        if (!condition_) {
            std::stringstream ss;
            ss << colors::RED << "Assertion failed at " << file_ << ":" << line_ 
               << " in " << function_ << colors::RESET << std::endl;
            ss << "  Expected: " << condition_str_ << std::endl;
            
            if (!message_.empty()) {
                ss << "  Message: " << message_ << std::endl;
            }
            
            throw std::runtime_error(ss.str());
        }
    }
    
    template<typename T>
    AssertionHelper& operator<<(const T& value) {
        std::stringstream ss;
        ss << value;
        message_ += ss.str();
        return *this;
    }
    
private:
    bool condition_;
    std::string condition_str_;
    std::string file_;
    int line_;
    std::string function_;
    std::string message_;
};

// Comparison assertion helper
template<typename T1, typename T2>
class ComparisonAssertionHelper {
public:
    ComparisonAssertionHelper(const T1& val1, const T2& val2, const char* val1_str, 
                             const char* val2_str, const char* op_str,
                             const char* file, int line, const char* function)
        : val1_(val1), val2_(val2), val1_str_(val1_str), val2_str_(val2_str), 
          op_str_(op_str), file_(file), line_(line), function_(function) {}
    
    ~ComparisonAssertionHelper() noexcept(false) {
        bool result = false;
        
        if (op_str_ == "==") {
            result = (val1_ == val2_);
        } else if (op_str_ == "!=") {
            result = (val1_ != val2_);
        } else if (op_str_ == "<") {
            result = (val1_ < val2_);
        } else if (op_str_ == "<=") {
            result = (val1_ <= val2_);
        } else if (op_str_ == ">") {
            result = (val1_ > val2_);
        } else if (op_str_ == ">=") {
            result = (val1_ >= val2_);
        }
        
        if (!result) {
            std::stringstream ss;
            ss << colors::RED << "Assertion failed at " << file_ << ":" << line_ 
               << " in " << function_ << colors::RESET << std::endl;
            ss << "  Expected: " << val1_str_ << " " << op_str_ << " " << val2_str_ << std::endl;
            ss << "  Actual: " << val1_ << " vs " << val2_ << std::endl;
            
            if (!message_.empty()) {
                ss << "  Message: " << message_ << std::endl;
            }
            
            throw std::runtime_error(ss.str());
        }
    }
    
    template<typename T>
    ComparisonAssertionHelper& operator<<(const T& value) {
        std::stringstream ss;
        ss << value;
        message_ += ss.str();
        return *this;
    }
    
private:
    T1 val1_;
    T2 val2_;
    std::string val1_str_;
    std::string val2_str_;
    std::string op_str_;
    std::string file_;
    int line_;
    std::string function_;
    std::string message_;
};

// String comparison assertion helper
class StringComparisonAssertionHelper {
public:
    StringComparisonAssertionHelper(const char* val1, const char* val2, const char* val1_str, 
                                   const char* val2_str, const char* op_str,
                                   const char* file, int line, const char* function)
        : val1_(val1), val2_(val2), val1_str_(val1_str), val2_str_(val2_str), 
          op_str_(op_str), file_(file), line_(line), function_(function) {}
    
    ~StringComparisonAssertionHelper() noexcept(false) {
        bool result = false;
        
        if (op_str_ == "==") {
            result = (std::strcmp(val1_, val2_) == 0);
        } else if (op_str_ == "!=") {
            result = (std::strcmp(val1_, val2_) != 0);
        }
        
        if (!result) {
            std::stringstream ss;
            ss << colors::RED << "Assertion failed at " << file_ << ":" << line_ 
               << " in " << function_ << colors::RESET << std::endl;
            ss << "  Expected: " << val1_str_ << " " << op_str_ << " " << val2_str_ << std::endl;
            ss << "  Actual: \"" << val1_ << "\" vs \"" << val2_ << "\"" << std::endl;
            
            if (!message_.empty()) {
                ss << "  Message: " << message_ << std::endl;
            }
            
            throw std::runtime_error(ss.str());
        }
    }
    
    template<typename T>
    StringComparisonAssertionHelper& operator<<(const T& value) {
        std::stringstream ss;
        ss << value;
        message_ += ss.str();
        return *this;
    }
    
private:
    const char* val1_;
    const char* val2_;
    std::string val1_str_;
    std::string val2_str_;
    std::string op_str_;
    std::string file_;
    int line_;
    std::string function_;
    std::string message_;
};

// Float comparison assertion helper
template<typename T1, typename T2>
class FloatComparisonAssertionHelper {
public:
    FloatComparisonAssertionHelper(const T1& val1, const T2& val2, const char* val1_str, 
                                  const char* val2_str, const char* op_str,
                                  const char* file, int line, const char* function)
        : val1_(val1), val2_(val2), val1_str_(val1_str), val2_str_(val2_str), 
          op_str_(op_str), file_(file), line_(line), function_(function) {}
    
    ~FloatComparisonAssertionHelper() noexcept(false) {
        bool result = false;
        
        if (op_str_ == "==") {
            if constexpr (std::is_same_v<T1, float> && std::is_same_v<T2, float>) {
                result = (std::abs(val1_ - val2_) < 1e-6f);
            } else {
                result = (std::abs(val1_ - val2_) < 1e-9);
            }
        }
        
        if (!result) {
            std::stringstream ss;
            ss << colors::RED << "Assertion failed at " << file_ << ":" << line_ 
               << " in " << function_ << colors::RESET << std::endl;
            ss << "  Expected: " << val1_str_ << " " << op_str_ << " " << val2_str_ << std::endl;
            ss << "  Actual: " << val1_ << " vs " << val2_ << std::endl;
            
            if (!message_.empty()) {
                ss << "  Message: " << message_ << std::endl;
            }
            
            throw std::runtime_error(ss.str());
        }
    }
    
    template<typename T>
    FloatComparisonAssertionHelper& operator<<(const T& value) {
        std::stringstream ss;
        ss << value;
        message_ += ss.str();
        return *this;
    }
    
private:
    T1 val1_;
    T2 val2_;
    std::string val1_str_;
    std::string val2_str_;
    std::string op_str_;
    std::string file_;
    int line_;
    std::string function_;
    std::string message_;
};

// Near comparison assertion helper
template<typename T1, typename T2, typename T3>
class NearComparisonAssertionHelper {
public:
    NearComparisonAssertionHelper(const T1& val1, const T2& val2, const T3& abs_error, 
                                 const char* val1_str, const char* val2_str, const char* abs_error_str,
                                 const char* file, int line, const char* function)
        : val1_(val1), val2_(val2), abs_error_(abs_error), val1_str_(val1_str), 
          val2_str_(val2_str), abs_error_str_(abs_error_str), file_(file), 
          line_(line), function_(function) {}
    
    ~NearComparisonAssertionHelper() noexcept(false) {
        bool result = (std::abs(val1_ - val2_) < abs_error_);
        
        if (!result) {
            std::stringstream ss;
            ss << colors::RED << "Assertion failed at " << file_ << ":" << line_ 
               << " in " << function_ << colors::RESET << std::endl;
            ss << "  Expected: The difference between " << val1_str_ << " and " << val2_str_ 
               << " is less than " << abs_error_str_ << std::endl;
            ss << "  Actual: The difference is " << std::abs(val1_ - val2_) 
               << " which is greater than " << abs_error_ << std::endl;
            
            if (!message_.empty()) {
                ss << "  Message: " << message_ << std::endl;
            }
            
            throw std::runtime_error(ss.str());
        }
    }
    
    template<typename T>
    NearComparisonAssertionHelper& operator<<(const T& value) {
        std::stringstream ss;
        ss << value;
        message_ += ss.str();
        return *this;
    }
    
private:
    T1 val1_;
    T2 val2_;
    T3 abs_error_;
    std::string val1_str_;
    std::string val2_str_;
    std::string abs_error_str_;
    std::string file_;
    int line_;
    std::string function_;
    std::string message_;
};

// Exception classes for exception assertions
class ExpectThrowException : public std::runtime_error {
public:
    ExpectThrowException(const char* statement, const char* exception_type, 
                        const char* file, int line, const char* function, const std::string& message)
        : std::runtime_error(message), statement_(statement), exception_type_(exception_type), 
          file_(file), line_(line), function_(function) {}
    
    const char* statement() const { return statement_; }
    const char* exception_type() const { return exception_type_; }
    const char* file() const { return file_; }
    int line() const { return line_; }
    const char* function() const { return function_; }
    
private:
    const char* statement_;
    const char* exception_type_;
    const char* file_;
    int line_;
    const char* function_;
};

class ExpectNoThrowException : public std::runtime_error {
public:
    ExpectNoThrowException(const char* statement, const char* file, int line, 
                          const char* function, const std::string& message)
        : std::runtime_error(message), statement_(statement), file_(file), 
          line_(line), function_(function) {}
    
    const char* statement() const { return statement_; }
    const char* file() const { return file_; }
    int line() const { return line_; }
    const char* function() const { return function_; }
    
private:
    const char* statement_;
    const char* file_;
    int line_;
    const char* function_;
};

// Assertion macros
#define EXPECT_TRUE(condition) \
    testing::AssertionHelper(condition, #condition, __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_FALSE(condition) \
    testing::AssertionHelper(!(condition), #condition, __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_TRUE(condition) \
    testing::AssertionHelper(condition, #condition, __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_FALSE(condition) \
    testing::AssertionHelper(!(condition), #condition, __FILE__, __LINE__, __FUNCTION__)

// Comparison assertion macros
#define EXPECT_EQ(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, "==", __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_NE(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, "!=", __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_LT(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, "<", __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_LE(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, "<=", __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_GT(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, ">", __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_GE(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, ">=", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_EQ(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, "==", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_NE(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, "!=", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_LT(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, "<", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_LE(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, "<=", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_GT(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, ">", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_GE(val1, val2) \
    testing::ComparisonAssertionHelper(val1, val2, #val1, #val2, ">=", __FILE__, __LINE__, __FUNCTION__)

// String comparison assertions
#define EXPECT_STREQ(val1, val2) \
    testing::StringComparisonAssertionHelper(val1, val2, #val1, #val2, "==", __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_STRNE(val1, val2) \
    testing::StringComparisonAssertionHelper(val1, val2, #val1, #val2, "!=", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_STREQ(val1, val2) \
    testing::StringComparisonAssertionHelper(val1, val2, #val1, #val2, "==", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_STRNE(val1, val2) \
    testing::StringComparisonAssertionHelper(val1, val2, #val1, #val2, "!=", __FILE__, __LINE__, __FUNCTION__)

// Floating point comparison assertions
#define EXPECT_FLOAT_EQ(val1, val2) \
    testing::FloatComparisonAssertionHelper(val1, val2, #val1, #val2, "==", __FILE__, __LINE__, __FUNCTION__)

#define EXPECT_DOUBLE_EQ(val1, val2) \
    testing::FloatComparisonAssertionHelper(val1, val2, #val1, #val2, "==", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_FLOAT_EQ(val1, val2) \
    testing::FloatComparisonAssertionHelper(val1, val2, #val1, #val2, "==", __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_DOUBLE_EQ(val1, val2) \
    testing::FloatComparisonAssertionHelper(val1, val2, #val1, #val2, "==", __FILE__, __LINE__, __FUNCTION__)

// Near comparison assertion
#define EXPECT_NEAR(val1, val2, abs_error) \
    testing::NearComparisonAssertionHelper(val1, val2, abs_error, #val1, #val2, #abs_error, __FILE__, __LINE__, __FUNCTION__)

#define ASSERT_NEAR(val1, val2, abs_error) \
    testing::NearComparisonAssertionHelper(val1, val2, abs_error, #val1, #val2, #abs_error, __FILE__, __LINE__, __FUNCTION__)

// Exception assertions
#define EXPECT_THROW(statement, exception_type) \
    try { \
        statement; \
        throw testing::ExpectThrowException(#statement, #exception_type, __FILE__, __LINE__, __FUNCTION__, "Expected to throw " #exception_type " but didn't throw"); \
    } catch (const exception_type&) { \
        /* Expected exception */ \
    } catch (const std::exception& e) { \
        throw testing::ExpectThrowException(#statement, #exception_type, __FILE__, __LINE__, __FUNCTION__, "Expected to throw " #exception_type " but threw a different exception: " + std::string(e.what())); \
    } catch (...) { \
        throw testing::ExpectThrowException(#statement, #exception_type, __FILE__, __LINE__, __FUNCTION__, "Expected to throw " #exception_type " but threw a non-standard exception"); \
    }

#define EXPECT_ANY_THROW(statement) \
    try { \
        statement; \
        throw testing::ExpectThrowException(#statement, "any exception", __FILE__, __LINE__, __FUNCTION__, "Expected to throw an exception but didn't throw"); \
    } catch (...) { \
        /* Expected exception */ \
    }

#define EXPECT_NO_THROW(statement) \
    try { \
        statement; \
    } catch (const std::exception& e) { \
        throw testing::ExpectNoThrowException(#statement, __FILE__, __LINE__, __FUNCTION__, "Expected not to throw but threw: " + std::string(e.what())); \
    } catch (...) { \
        throw testing::ExpectNoThrowException(#statement, __FILE__, __LINE__, __FUNCTION__, "Expected not to throw but threw a non-standard exception"); \
    }

#define ASSERT_THROW(statement, exception_type) EXPECT_THROW(statement, exception_type)
#define ASSERT_ANY_THROW(statement) EXPECT_ANY_THROW(statement)
#define ASSERT_NO_THROW(statement) EXPECT_NO_THROW(statement)

// Test fixture base class
class Test {
public:
    virtual ~Test() = default;
    virtual void SetUp() {}
    virtual void TearDown() {}
};

// Test macro
#define TEST(test_case_name, test_name) \
    void test_case_name##_##test_name(); \
    namespace { \
        struct test_case_name##_##test_name##_Registrar { \
            test_case_name##_##test_name##_Registrar() { \
                testing::TestRegistry::getInstance().addTest( \
                    testing::TestInfo(#test_case_name, #test_name, []() { \
                        test_case_name##_##test_name(); \
                    })); \
            } \
        }; \
        static test_case_name##_##test_name##_Registrar test_case_name##_##test_name##_registrar; \
    } \
    void test_case_name##_##test_name()

// Test fixture macro
#define TEST_F(test_fixture, test_name) \
    class test_fixture##_##test_name##_Test : public test_fixture { \
    public: \
        void TestBody(); \
    }; \
    namespace { \
        struct test_fixture##_##test_name##_Registrar { \
            test_fixture##_##test_name##_Registrar() { \
                testing::TestRegistry::getInstance().addTest( \
                    testing::TestInfo(#test_fixture, #test_name, []() { \
                        test_fixture##_##test_name##_Test test; \
                        test.SetUp(); \
                        test.TestBody(); \
                        test.TearDown(); \
                    })); \
            } \
        }; \
        static test_fixture##_##test_name##_Registrar test_fixture##_##test_name##_registrar; \
    } \
    void test_fixture##_##test_name##_Test::TestBody()

// Types for typed tests
template<typename... Ts>
struct Types {
    using type = Types<Ts...>;
};

// Typed test suite macro
#define TYPED_TEST_SUITE(test_suite_name, types) \
    template<typename T> \
    class test_suite_name : public testing::Test { \
    public: \
        using TypeParam = T; \
    }; \
    namespace { \
        struct test_suite_name##_Registrar { \
            test_suite_name##_Registrar() { \
                registerTypedTests<types>(#test_suite_name); \
            } \
            template<typename TypeList> \
            void registerTypedTests(const char* suite_name); \
        }; \
        static test_suite_name##_Registrar test_suite_name##_registrar; \
    } \
    template<typename TypeList> \
    void test_suite_name##_Registrar::registerTypedTests(const char* suite_name)

// Typed test macro
#define TYPED_TEST(test_suite_name, test_name) \
    template<typename T> \
    class test_suite_name##_##test_name##_Test : public test_suite_name<T> { \
    public: \
        void TestBody(); \
    }; \
    namespace { \
        template<typename T> \
        struct test_suite_name##_##test_name##_RegistrarHelper { \
            void operator()() { \
                testing::TestRegistry::getInstance().addTest( \
                    testing::TestInfo(#test_suite_name, #test_name, []() { \
                        test_suite_name##_##test_name##_Test<T> test; \
                        test.SetUp(); \
                        test.TestBody(); \
                        test.TearDown(); \
                    })); \
            } \
        }; \
    } \
    template<typename TypeList> \
    void test_suite_name##_Registrar::registerTypedTests(const char* suite_name) { \
        instantiateTypedTests<TypeList>(suite_name); \
    } \
    template<typename... Ts> \
    void test_suite_name##_Registrar::instantiateTypedTests(const char* suite_name) { \
        (test_suite_name##_##test_name##_RegistrarHelper<Ts>()(), ...); \
    } \
    template<typename T> \
    void test_suite_name##_##test_name##_Test<T>::TestBody()

// Main function to run all tests
int RUN_ALL_TESTS() {
    TestRegistry::getInstance().runAllTests();
    return 0;
}

}  // namespace testing

// Basic test
TEST(BasicTest, Simple) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_TRUE(true) << "This should pass";
}

// Test fixture
class MyTestFixture : public testing::Test {
    public:
    void SetUp() override {
        value = 42;
    }
    
    int value;
};

TEST_F(MyTestFixture, CheckValue) {
    EXPECT_EQ(value, 42);
}

// Typed test
using types = testing::Types<int, double, long>;
TYPED_TEST_SUITE(MyTypedTest, types);

TYPED_TEST(MyTypedTest, TypeSpecific) {
    TypeParam value = static_cast<TypeParam>(10);
    EXPECT_EQ(value * 2, static_cast<TypeParam>(20));
}

int main() {
    return testing::RUN_ALL_TESTS();
}
