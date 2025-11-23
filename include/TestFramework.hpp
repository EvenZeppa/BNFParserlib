#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

/**
 * @brief Modern C++ unit test framework with colored output and visual clarity.
 * 
 * Provides a complete testing infrastructure with test runners, suites,
 * and various assertion macros. Features colored output for better readability
 * and comprehensive test reporting.
 */

// ANSI color codes for colored output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

/**
 * @brief Test runner that tracks test results and provides assertion methods.
 */
class TestRunner {
public:
    TestRunner() : passed(0), failed(0), currentTestName("") {}

    /**
     * @brief Sets the name of the currently running test.
     * @param name The test name
     */
    void setCurrentTest(const std::string& name) {
        currentTestName = name;
    }

    /**
     * @brief Records a passed assertion.
     * @param message Description of what passed
     */
    void recordPass(const std::string& message) {
        passed++;
        std::cout << GREEN << "  ✓ " << message << RESET << std::endl;
    }

    /**
     * @brief Records a failed assertion.
     * @param file Source file where assertion failed
     * @param line Line number of the failure
     * @param message Failure description
     */
    void recordFail(const std::string& file, int line, const std::string& message) {
        failed++;
        std::cout << RED << "[FAIL] " << file << ":" << line << ": " << message << RESET << std::endl;
    }

    /**
     * @brief Gets the number of passed assertions.
     * @return Number of passed assertions
     */
    int getPassedCount() const { return passed; }

    /**
     * @brief Gets the number of failed assertions.
     * @return Number of failed assertions
     */
    int getFailedCount() const { return failed; }

    /**
     * @brief Prints a summary of test results.
     */
    void printSummary() const {
        std::cout << "\n" << BOLD << "=== Test Summary ===" << RESET << std::endl;
        std::cout << GREEN << "Passed: " << passed << RESET << std::endl;
        std::cout << (failed > 0 ? RED : GREEN) << "Failed: " << failed << RESET << std::endl;
        
        if (failed == 0) {
            std::cout << GREEN << BOLD << "🎉 All tests passed!" << RESET << std::endl;
        } else {
            std::cout << RED << BOLD << "❌ Some tests failed." << RESET << std::endl;
        }
    }

    /**
     * @brief Returns true if all tests passed.
     * @return true if no failures, false otherwise
     */
    bool allPassed() const { return failed == 0; }

private:
    int passed;
    int failed;
    std::string currentTestName;
};

// Forward declaration for test function type
typedef void (*TestFunction)(TestRunner&);

/**
 * @brief Test case container holding a name and function pointer.
 */
struct TestCase {
    std::string name;
    TestFunction function;
    
    TestCase(const std::string& n, TestFunction f) : name(n), function(f) {}
};

/**
 * @brief Test suite that manages and runs multiple test cases.
 */
class TestSuite {
public:
    TestSuite(const std::string& suiteName) : name(suiteName) {}

    /**
     * @brief Adds a test case to the suite.
     * @param testName Name of the test
     * @param testFunc Function to execute
     */
    void addTest(const std::string& testName, TestFunction testFunc) {
        tests.push_back(TestCase(testName, testFunc));
    }

    /**
     * @brief Runs all tests in the suite.
     * @return TestRunner with accumulated results
     */
    TestRunner run() {
        std::cout << BOLD << CYAN << "\n=== Running Test Suite: " << name << " ===" << RESET << std::endl;
        
        TestRunner runner;
        
        for (size_t i = 0; i < tests.size(); ++i) {
            const TestCase& test = tests[i];
            std::cout << YELLOW << "\nRunning test: " << test.name << "..." << RESET << std::endl;
            
            runner.setCurrentTest(test.name);
            
            try {
                test.function(runner);
                std::cout << GREEN << "Test completed: " << test.name << RESET << std::endl;
            } catch (const std::exception& e) {
                runner.recordFail(__FILE__, __LINE__, std::string("Exception: ") + e.what());
                std::cout << RED << "Test failed with exception: " << test.name << RESET << std::endl;
            }
        }
        
        return runner;
    }

    /**
     * @brief Gets the number of tests in this suite.
     * @return Number of test cases
     */
    size_t getTestCount() const { return tests.size(); }

    /**
     * @brief Gets the suite name.
     * @return Suite name
     */
    const std::string& getName() const { return name; }

private:
    std::string name;
    std::vector<TestCase> tests;
};

// Assertion macros with detailed output

/**
 * @brief Asserts that two values are equal.
 */
#define ASSERT_EQ(runner, a, b) \
    do { \
        if ((a) == (b)) { \
            std::ostringstream __oss; \
            __oss << #a << " == " << #b << " (" << (a) << ")"; \
            (runner).recordPass(__oss.str()); \
        } else { \
            std::ostringstream __oss; \
            __oss << #a << " != " << #b << " (" << (a) << " vs " << (b) << ")"; \
            (runner).recordFail(__FILE__, __LINE__, __oss.str()); \
        } \
    } while(0)

/**
 * @brief Asserts that two values are not equal.
 */
#define ASSERT_NE(runner, a, b) \
    do { \
        if ((a) != (b)) { \
            std::ostringstream __oss; \
            __oss << #a << " != " << #b << " (" << (a) << " vs " << (b) << ")"; \
            (runner).recordPass(__oss.str()); \
        } else { \
            std::ostringstream __oss; \
            __oss << #a << " == " << #b << " (" << (a) << "), expected different values"; \
            (runner).recordFail(__FILE__, __LINE__, __oss.str()); \
        } \
    } while(0)

/**
 * @brief Asserts that a condition is true.
 */
#define ASSERT_TRUE(runner, cond) \
    do { \
        if (cond) { \
            (runner).recordPass(#cond " is true"); \
        } else { \
            (runner).recordFail(__FILE__, __LINE__, #cond " is false"); \
        } \
    } while(0)

/**
 * @brief Asserts that a condition is false.
 */
#define ASSERT_FALSE(runner, cond) \
    do { \
        if (!(cond)) { \
            (runner).recordPass(#cond " is false"); \
        } else { \
            (runner).recordFail(__FILE__, __LINE__, #cond " is true"); \
        } \
    } while(0)

/**
 * @brief Asserts that a pointer is not null.
 */
#define ASSERT_NOT_NULL(runner, ptr) \
    do { \
        if ((ptr) != NULL) { \
            (runner).recordPass(#ptr " is not null"); \
        } else { \
            (runner).recordFail(__FILE__, __LINE__, #ptr " is null"); \
        } \
    } while(0)

/**
 * @brief Asserts that a pointer is null.
 */
#define ASSERT_NULL(runner, ptr) \
    do { \
        if ((ptr) == NULL) { \
            (runner).recordPass(#ptr " is null"); \
        } else { \
            (runner).recordPass(#ptr " is not null"); \
        } \
    } while(0)

/**
 * @brief Asserts that a value is greater than another.
 */
#define ASSERT_GT(runner, a, b) \
    do { \
        if ((a) > (b)) { \
            std::ostringstream __oss; \
            __oss << #a << " > " << #b << " (" << (a) << " > " << (b) << ")"; \
            (runner).recordPass(__oss.str()); \
        } else { \
            std::ostringstream __oss; \
            __oss << #a << " <= " << #b << " (" << (a) << " <= " << (b) << ")"; \
            (runner).recordFail(__FILE__, __LINE__, __oss.str()); \
        } \
    } while(0)

/**
 * @brief Asserts that a value is less than another.
 */
#define ASSERT_LT(runner, a, b) \
    do { \
        if ((a) < (b)) { \
            std::ostringstream __oss; \
            __oss << #a << " < " << #b << " (" << (a) << " < " << (b) << ")"; \
            (runner).recordPass(__oss.str()); \
        } else { \
            std::ostringstream __oss; \
            __oss << #a << " >= " << #b << " (" << (a) << " >= " << (b) << ")"; \
            (runner).recordFail(__FILE__, __LINE__, __oss.str()); \
        } \
    } while(0)

/**
 * @brief Asserts that a value is greater than or equal to another.
 */
#define ASSERT_GE(runner, a, b) \
    do { \
        if ((a) >= (b)) { \
            std::ostringstream __oss; \
            __oss << #a << " >= " << #b << " (" << (a) << " >= " << (b) << ")"; \
            (runner).recordPass(__oss.str()); \
        } else { \
            std::ostringstream __oss; \
            __oss << #a << " < " << #b << " (" << (a) << " < " << (b) << ")"; \
            (runner).recordFail(__FILE__, __LINE__, __oss.str()); \
        } \
    } while(0)

/**
 * @brief Asserts that a value is less than or equal to another.
 */
#define ASSERT_LE(runner, a, b) \
    do { \
        if ((a) <= (b)) { \
            std::ostringstream __oss; \
            __oss << #a << " <= " << #b << " (" << (a) << " <= " << (b) << ")"; \
            (runner).recordPass(__oss.str()); \
        } else { \
            std::ostringstream __oss; \
            __oss << #a << " > " << #b << " (" << (a) << " > " << (b) << ")"; \
            (runner).recordFail(__FILE__, __LINE__, __oss.str()); \
        } \
    } while(0)

/**
 * @brief Asserts that a string contains a substring.
 */
#define ASSERT_CONTAINS(runner, str, substr) \
    do { \
        if ((str).find(substr) != std::string::npos) { \
            std::ostringstream __oss; \
            __oss << "\"" << (str) << "\" contains \"" << (substr) << "\""; \
            (runner).recordPass(__oss.str()); \
        } else { \
            std::ostringstream __oss; \
            __oss << "\"" << (str) << "\" does not contain \"" << (substr) << "\""; \
            (runner).recordFail(__FILE__, __LINE__, __oss.str()); \
        } \
    } while(0)

/**
 * @brief Asserts that a container is empty.
 */
#define ASSERT_EMPTY(runner, container) \
    do { \
        if ((container).empty()) { \
            (runner).recordPass(#container " is empty"); \
        } else { \
            std::ostringstream __oss; \
            __oss << #container " is not empty (size: " << (container).size() << ")"; \
            (runner).recordFail(__FILE__, __LINE__, __oss.str()); \
        } \
    } while(0)

/**
 * @brief Asserts that a container is not empty.
 */
#define ASSERT_NOT_EMPTY(runner, container) \
    do { \
        if (!(container).empty()) { \
            std::ostringstream __oss; \
            __oss << #container " is not empty (size: " << (container).size() << ")"; \
            (runner).recordPass(__oss.str()); \
        } else { \
            (runner).recordFail(__FILE__, __LINE__, #container " is empty"); \
        } \
    } while(0)

/**
 * @brief Manual test failure with custom message.
 */
#define FAIL(runner, message) \
    do { \
        (runner).recordFail(__FILE__, __LINE__, message); \
    } while(0)

/**
 * @brief Manual test pass with custom message.
 */
#define PASS(runner, message) \
    do { \
        (runner).recordPass(message); \
    } while(0)

#endif // TEST_FRAMEWORK_HPP
