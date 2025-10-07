import standard;

#ifndef GTEST_H_
#define GTEST_H_

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <type_traits>
#include <map>
#include <memory>
#include <algorithm>
#include <cstring>

// Color codes for terminal output
namespace colors {
    const char* RESET = "\033[0m";
    const char* RED = "\033[31m";
    const char* GREEN = "\033[32m";
    const char* YELLOW = "\033[33m";
    const char* BLUE = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN = "\033[36m";
    const char* WHITE = "\033[37m";
}

namespace testing {

    // Forward declarations
    class Test;
    class TestSuite;
    class UnitTest;
    
    // Information about a test
    class TestInfo {
    public:
        TestInfo(const std::string& test_case_name, const std::string& test_name,
                 Test* (*factory)(), const std::string& file, int line);
        
        const std::string& test_case_name() const { return test_case_name_; }
        const std::string& test_name() const { return test_name_; }
        const std::string& file() const { return file_; }
        int line() const { return line_; }
        bool should_run() const { return should_run_; }
        bool has_fatal_failure() const { return has_fatal_failure_; }
        bool has_nonfatal_failure() const { return has_nonfatal_failure_; }
        bool has_failure() const { return has_fatal_failure_ || has_nonfatal_failure_; }
        bool passed() const { return !has_failure(); }
        long elapsed_time() const { return elapsed_time_; }
        
        void set_should_run(bool should_run) { should_run_ = should_run; }
        void set_has_fatal_failure(bool has_fatal_failure) { has_fatal_failure_ = has_fatal_failure; }
        void set_has_nonfatal_failure(bool has_nonfatal_failure) { has_nonfatal_failure_ = has_nonfatal_failure; }
        void set_elapsed_time(long elapsed_time) { elapsed_time_ = elapsed_time; }
        
        void Run();
        
    private:
        std::string test_case_name_;
        std::string test_name_;
        Test* (*factory_)();
        std::string file_;
        int line_;
        bool should_run_;
        bool has_fatal_failure_;
        bool has_nonfatal_failure_;
        long elapsed_time_;
    };
    
    // Base class for all tests
    class Test {
    public:
        virtual ~Test() {}
        
        virtual void SetUp() {}
        virtual void TearDown() {}
        
        static bool HasFatalFailure() { return current_test_info_ && current_test_info_->has_fatal_failure(); }
        static bool HasNonfatalFailure() { return current_test_info_ && current_test_info_->has_nonfatal_failure(); }
        static bool HasFailure() { return HasFatalFailure() || HasNonfatalFailure(); }
        
        // Make current_test_info_ accessible to AssertionHelper
        static TestInfo* current_test_info() { return current_test_info_; }
        
    protected:
        static TestInfo* current_test_info_;
        
    private:
        virtual void TestBody() = 0;
        
        friend class TestInfo;
        void Run();
    };
    
    // Assertion result class
    class AssertionResult {
    public:
        AssertionResult(bool success) : success_(success) {}
        
        operator bool() const { return success_; }
        
        bool success() const { return success_; }
        const std::string& message() const { return message_; }
        
        AssertionResult& operator<<(const std::string& msg) {
            message_ += msg;
            return *this;
        }
        
        template<typename T>
        AssertionResult& operator<<(const T& value) {
            std::ostringstream oss;
            oss << value;
            message_ += oss.str();
            return *this;
        }
        
    private:
        bool success_;
        std::string message_;
    };
    
    // A collection of tests
    class TestSuite {
    public:
        TestSuite(const std::string& name) : name_(name) {}
        
        const std::string& name() const { return name_; }
        const std::vector<TestInfo*>& test_info_list() const { return test_infos_; }
        
        void AddTestInfo(TestInfo* test_info) {
            test_infos_.push_back(test_info);
        }
        
        long successful_test_count() const {
            return std::count_if(test_infos_.begin(), test_infos_.end(),
                                [](const TestInfo* test_info) { return test_info->passed(); });
        }
        
        long failed_test_count() const {
            return std::count_if(test_infos_.begin(), test_infos_.end(),
                                [](const TestInfo* test_info) { return !test_info->passed(); });
        }
        
        long total_test_count() const {
            return test_infos_.size();
        }
        
        long test_to_run_count() const {
            return std::count_if(test_infos_.begin(), test_infos_.end(),
                                [](const TestInfo* test_info) { return test_info->should_run(); });
        }
        
        bool Passed() const {
            return failed_test_count() == 0;
        }
        
        void Run();
        
    private:
        long GetTotalTime() const {
            long total = 0;
            for (const TestInfo* test_info : test_infos_) {
                total += test_info->elapsed_time();
            }
            return total;
        }
        
        std::string name_;
        std::vector<TestInfo*> test_infos_;
    };
    
    // Singleton class that manages all test suites
    class UnitTest {
    public:
        static UnitTest* GetInstance() {
            static UnitTest instance;
            return &instance;
        }
        
        TestSuite* GetTestSuite(const std::string& name) {
            if (test_suites_.find(name) == test_suites_.end()) {
                test_suites_[name] = std::make_unique<TestSuite>(name);
            }
            return test_suites_[name].get();
        }
        
        void AddTestInfo(const std::string& test_case_name, const std::string& test_name,
                        Test* (*factory)(), const std::string& file, int line) {
            TestSuite* test_suite = GetTestSuite(test_case_name);
            TestInfo* test_info = new TestInfo(test_case_name, test_name, factory, file, line);
            test_suite->AddTestInfo(test_info);
        }
        
        void AddTypedTestInfo(const std::string& test_case_name, const std::string& test_name,
                             Test* (*factory)(), const std::string& file, int line, 
                             const std::string& type_param) {
            std::string full_test_name = test_case_name + "/" + type_param + "." + test_name;
            TestSuite* test_suite = GetTestSuite(test_case_name + "/" + type_param);
            TestInfo* test_info = new TestInfo(test_case_name, test_name, factory, file, line);
            test_suite->AddTestInfo(test_info);
        }
        
        long successful_test_count() const {
            long total = 0;
            for (const auto& pair : test_suites_) {
                total += pair.second->successful_test_count();
            }
            return total;
        }
        
        long failed_test_count() const {
            long total = 0;
            for (const auto& pair : test_suites_) {
                total += pair.second->failed_test_count();
            }
            return total;
        }
        
        long total_test_count() const {
            long total = 0;
            for (const auto& pair : test_suites_) {
                total += pair.second->total_test_count();
            }
            return total;
        }
        
        bool Passed() const {
            return failed_test_count() == 0;
        }
        
        int Run();
        
    private:
        std::map<std::string, std::unique_ptr<TestSuite>> test_suites_;
    };
    
    // Initialize static member
    TestInfo* Test::current_test_info_ = nullptr;
    
    // TestInfo implementation
    TestInfo::TestInfo(const std::string& test_case_name, const std::string& test_name,
                       Test* (*factory)(), const std::string& file, int line)
        : test_case_name_(test_case_name), test_name_(test_name),
          factory_(factory), file_(file), line_(line),
          should_run_(true), has_fatal_failure_(false), 
          has_nonfatal_failure_(false), elapsed_time_(0) {}
    
    void TestInfo::Run() {
        Test::current_test_info_ = this;
        
        std::unique_ptr<Test> test(factory_());
        if (test) {
            auto start = std::chrono::high_resolution_clock::now();
            
            test->SetUp();
            test->TestBody();
            test->TearDown();
            
            auto end = std::chrono::high_resolution_clock::now();
            elapsed_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }
        
        Test::current_test_info_ = nullptr;
    }
    
    // Test implementation
    void Test::Run() {
        SetUp();
        TestBody();
        TearDown();
    }
    
    // TestSuite implementation
    void TestSuite::Run() {
        std::cout << colors::CYAN << "[----------] " << colors::RESET 
                  << total_test_count() << " test(s) from " << name_ << std::endl;
        
        for (TestInfo* test_info : test_infos_) {
            if (test_info->should_run()) {
                std::cout << colors::CYAN << "[ RUN      ] " << colors::RESET 
                          << name_ << "." << test_info->test_name() << std::endl;
                
                test_info->Run();
                
                if (test_info->passed()) {
                    std::cout << colors::GREEN << "[       OK ] " << colors::RESET 
                              << name_ << "." << test_info->test_name() 
                              << " (" << test_info->elapsed_time() << " ms)" << std::endl;
                } else {
                    std::cout << colors::RED << "[  FAILED  ] " << colors::RESET 
                              << name_ << "." << test_info->test_name() 
                              << " (" << test_info->elapsed_time() << " ms)" << std::endl;
                }
            }
        }
        
        std::cout << colors::CYAN << "[----------] " << colors::RESET 
                  << total_test_count() << " test(s) from " << name_ 
                  << " (" << GetTotalTime() << " ms total)" << std::endl;
    }
    
    // UnitTest implementation
    int UnitTest::Run() {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::cout << colors::CYAN << "[==========] " << colors::RESET 
                  << "Running " << total_test_count() << " test(s) from " 
                  << test_suites_.size() << " test suite(s)." << std::endl;
        
        for (auto& pair : test_suites_) {
            pair.second->Run();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        
        std::cout << colors::CYAN << "[==========] " << colors::RESET 
                  << total_test_count() << " test(s) from " << test_suites_.size() 
                  << " test suite(s) ran. (" << total_time << " ms total)" << std::endl;
        
        std::cout << colors::CYAN << "[  PASSED  ] " << colors::RESET 
                  << successful_test_count() << " test(s)." << std::endl;
        
        if (failed_test_count() > 0) {
            std::cout << colors::RED << "[  FAILED  ] " << colors::RESET 
                      << failed_test_count() << " test(s), listed below:" << std::endl;
            
            for (const auto& pair : test_suites_) {
                for (const TestInfo* test_info : pair.second->test_info_list()) {
                    if (!test_info->passed()) {
                        std::cout << colors::RED << "[  FAILED  ] " << colors::RESET 
                                  << pair.first << "." << test_info->test_name() << std::endl;
                    }
                }
            }
            
            std::cout << std::endl 
                      << failed_test_count() << " FAILED TEST(S)" << std::endl;
        }
        
        return Passed() ? 0 : 1;
    }
    
    // Helper functions for assertions
    namespace internal {
        
        template<typename T1, typename T2>
        AssertionResult EqHelper(const char* expr1, const char* expr2, const T1& val1, const T2& val2) {
            if (val1 == val2) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected equality of these values:" << std::endl
                << "  " << expr1 << std::endl
                << "    Which is: " << val1 << std::endl
                << "  " << expr2 << std::endl
                << "    Which is: " << val2;
            result << oss.str();
            return result;
        }
        
        template<typename T1, typename T2>
        AssertionResult NeHelper(const char* expr1, const char* expr2, const T1& val1, const T2& val2) {
            if (val1 != val2) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected inequality of these values:" << std::endl
                << "  " << expr1 << std::endl
                << "    Which is: " << val1 << std::endl
                << "  " << expr2 << std::endl
                << "    Which is: " << val2;
            result << oss.str();
            return result;
        }
        
        template<typename T1, typename T2>
        AssertionResult LtHelper(const char* expr1, const char* expr2, const T1& val1, const T2& val2) {
            if (val1 < val2) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected: (" << expr1 << ") < (" << expr2 << "), actual: " 
                << val1 << " vs " << val2;
            result << oss.str();
            return result;
        }
        
        template<typename T1, typename T2>
        AssertionResult LeHelper(const char* expr1, const char* expr2, const T1& val1, const T2& val2) {
            if (val1 <= val2) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected: (" << expr1 << ") <= (" << expr2 << "), actual: " 
                << val1 << " vs " << val2;
            result << oss.str();
            return result;
        }
        
        template<typename T1, typename T2>
        AssertionResult GtHelper(const char* expr1, const char* expr2, const T1& val1, const T2& val2) {
            if (val1 > val2) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected: (" << expr1 << ") > (" << expr2 << "), actual: " 
                << val1 << " vs " << val2;
            result << oss.str();
            return result;
        }
        
        template<typename T1, typename T2>
        AssertionResult GeHelper(const char* expr1, const char* expr2, const T1& val1, const T2& val2) {
            if (val1 >= val2) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected: (" << expr1 << ") >= (" << expr2 << "), actual: " 
                << val1 << " vs " << val2;
            result << oss.str();
            return result;
        }
        
        AssertionResult TrueHelper(const char* expr, bool value) {
            if (value) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected: " << expr << " evaluates to true, actual: false";
            result << oss.str();
            return result;
        }
        
        AssertionResult FalseHelper(const char* expr, bool value) {
            if (!value) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected: " << expr << " evaluates to false, actual: true";
            result << oss.str();
            return result;
        }
        
        AssertionResult StrEqHelper(const char* expr1, const char* expr2, const char* val1, const char* val2) {
            if ((val1 == nullptr && val2 == nullptr) || 
                (val1 != nullptr && val2 != nullptr && strcmp(val1, val2) == 0)) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected equality of these values:" << std::endl
                << "  " << expr1 << std::endl
                << "    Which is: " << (val1 ? val1 : "null") << std::endl
                << "  " << expr2 << std::endl
                << "    Which is: " << (val2 ? val2 : "null");
            result << oss.str();
            return result;
        }
        
        AssertionResult StrNeHelper(const char* expr1, const char* expr2, const char* val1, const char* val2) {
            if ((val1 == nullptr && val2 != nullptr) || 
                (val1 != nullptr && val2 == nullptr) ||
                (val1 != nullptr && val2 != nullptr && strcmp(val1, val2) != 0)) {
                return AssertionResult(true);
            }
            
            AssertionResult result(false);
            std::ostringstream oss;
            oss << "Expected inequality of these values:" << std::endl
                << "  " << expr1 << std::endl
                << "    Which is: " << (val1 ? val1 : "null") << std::endl
                << "  " << expr2 << std::endl
                << "    Which is: " << (val2 ? val2 : "null");
            result << oss.str();
            return result;
        }
    }
    
    // Assertion helper class for streaming
    class AssertionHelper {
    public:
        AssertionHelper(bool is_fatal, const char* file, int line)
            : is_fatal_(is_fatal), file_(file), line_(line) {}
        
        template<typename T>
        AssertionHelper& operator<<(const T& value) {
            std::ostringstream oss;
            oss << value;
            message_ += oss.str();
            return *this;
        }
        
        AssertionHelper& operator<<(const std::string& value) {
            message_ += value;
            return *this;
        }
        
        ~AssertionHelper() {
            std::cout << colors::RED << file_ << ":" << line_ << std::endl
                      << colors::RED << "Failure" << colors::RESET << std::endl
                      << message_ << std::endl;
            
            TestInfo* test_info = Test::current_test_info();
            if (test_info) {
                if (is_fatal_) {
                    test_info->set_has_fatal_failure(true);
                } else {
                    test_info->set_has_nonfatal_failure(true);
                }
            }
        }
        
    private:
        bool is_fatal_;
        const char* file_;
        int line_;
        std::string message_;
    };
    
    // Assertion macros
    #define EXPECT_TRUE(condition) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::TrueHelper(#condition, (condition)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_TRUE(condition) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::TrueHelper(#condition, (condition)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_FALSE(condition) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::FalseHelper(#condition, (condition)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_FALSE(condition) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::FalseHelper(#condition, (condition)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_EQ(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::EqHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_EQ(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::EqHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_NE(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::NeHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_NE(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::NeHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_LT(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::LtHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_LT(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::LtHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_LE(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::LeHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_LE(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::LeHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_GT(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::GtHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_GT(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::GtHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_GE(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::GeHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_GE(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::GeHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_STREQ(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::StrEqHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_STREQ(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::StrEqHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define EXPECT_STRNE(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::StrNeHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(false, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    #define ASSERT_STRNE(val1, val2) \
      do { \
        ::testing::AssertionResult result = ::testing::internal::StrNeHelper(#val1, #val2, (val1), (val2)); \
        if (!result) { \
          ::testing::AssertionHelper(true, __FILE__, __LINE__) << result.message(); \
        } \
      } while (false)
    
    // Test macros
    #define GTEST_TEST_(test_suite_name, test_name, parent_class, parent_id) \
      class GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) : public parent_class { \
      public: \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) {} \
      private: \
        virtual void TestBody(); \
        static ::testing::TestInfo* const test_info_; \
        GTEST_DISALLOW_COPY_AND_ASSIGN_(GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)); \
      }; \
      \
      ::testing::TestInfo* const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::test_info_ = \
        ::testing::UnitTest::GetInstance()->AddTestInfo( \
            #test_suite_name, #test_name, \
            []() -> ::testing::Test* { return new GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)(); }, \
            __FILE__, __LINE__); \
      \
      void GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::TestBody()
    
    #define TEST(test_suite_name, test_name) \
      GTEST_TEST_(test_suite_name, test_name, ::testing::Test, \
                  ::testing::internal::GetTypeId<::testing::Test>())
    
    #define TEST_F(test_fixture, test_name) \
      GTEST_TEST_(test_fixture, test_name, test_fixture, \
                  ::testing::internal::GetTypeId<test_fixture>())
    
    // TYPED_TEST support
    template <typename T>
    class WithParamInterface {
    public:
        typedef T ParamType;
        virtual ~WithParamInterface() {}
        
        static const ParamType& GetParam() {
            return *parameter_;
        }
        
    private:
        static const ParamType* parameter_;
        
        template <class TestClass>
        friend class TypedTestFactory;
    };
    
    template <typename T>
    const T* WithParamInterface<T>::parameter_ = nullptr;
    
    template <typename T>
    class TestWithParam : public Test, public WithParamInterface<T> {};
    
    template <class TestClass>
    class TypedTestFactory {
    public:
        TypedTestFactory(const typename TestClass::ParamType& parameter) 
            : parameter_(parameter) {}
        
        Test* CreateTest() {
            WithParamInterface<typename TestClass::ParamType>::parameter_ = &parameter_;
            return new TestClass();
        }
        
    private:
        typename TestClass::ParamType parameter_;
    };
    
    #define TYPED_TEST_SUITE(CaseName, Types) \
      template <typename TypeParam> \
      class CaseName : public ::testing::TestWithParam<TypeParam> { \
      };
    
    #define TYPED_TEST(CaseName, TestName) \
      template <typename TypeParam> \
      class GTEST_TYPED_TEST_CLASS_NAME_(CaseName, TestName) \
          : public CaseName<TypeParam> { \
      private: \
        virtual void TestBody(); \
        static ::testing::TestInfo* const test_info_; \
        GTEST_DISALLOW_COPY_AND_ASSIGN_(GTEST_TYPED_TEST_CLASS_NAME_(CaseName, TestName)); \
      }; \
      \
      template <typename TypeParam> \
      ::testing::TestInfo* const GTEST_TYPED_TEST_CLASS_NAME_(CaseName, TestName)::test_info_ = \
        ::testing::UnitTest::GetInstance()->AddTypedTestInfo( \
            #CaseName, #TestName, \
            []() -> ::testing::Test* { return new GTEST_TYPED_TEST_CLASS_NAME_(CaseName, TestName)(); }, \
            __FILE__, __LINE__, typeid(TypeParam).name()); \
      \
      template <typename TypeParam> \
      void GTEST_TYPED_TEST_CLASS_NAME_(CaseName, TestName)::TestBody()
    
    // Helper macros
    #define GTEST_DISALLOW_COPY_AND_ASSIGN_(Type) \
      Type(const Type&) = delete; \
      Type& operator=(const Type&) = delete
    
    namespace internal {
        template <typename T>
        struct TypeId {
            static const void* Get() {
                static char id;
                return &id;
            }
        };
        
        template <typename T>
        const void* GetTypeId() {
            return TypeId<T>::Get();
        }
    }
    
    // RUN_ALL_TESTS function
    int RUN_ALL_TESTS() {
        return UnitTest::GetInstance()->Run();
    }
    
} // namespace testing

#endif // GTEST_H_


using namespace std;
int main() {
    cout << "done" << endl;
    return 0;
}
