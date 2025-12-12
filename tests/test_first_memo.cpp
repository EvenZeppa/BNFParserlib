#include "../include/TestFramework.hpp"
#include "../include/Grammar.hpp"
#include "../include/BNFParser.hpp"

void test_first_basic(TestRunner& runner) {
    Grammar g;
    g.addRule("<s> ::= 'a' 'x' | 'b' 'y'");

    BNFParser p(g);
    size_t consumed = 0;
    ASTNode* node = p.parse("<s>", "by", consumed);
    ASSERT_NOT_NULL(runner, node);
    ASSERT_EQ(runner, consumed, 2u);
    delete node;
}

void test_first_nullable_alt(TestRunner& runner) {
    Grammar g;
    g.addRule("<s> ::= [ 'a' ] | 'b'");

    BNFParser p(g);
    size_t consumed = 0;
    ASTNode* node = p.parse("<s>", "b", consumed);
    ASSERT_NOT_NULL(runner, node);
    ASSERT_EQ(runner, consumed, 1u);
    delete node;
}

void test_first_class_range(TestRunner& runner) {
    Grammar g;
    g.addRule("<s> ::= ( 'a' ... 'c' ) 'x' | 'z' 'y'");

    BNFParser p(g);
    size_t consumed = 0;
    ASTNode* node = p.parse("<s>", "ax", consumed);
    ASSERT_NOT_NULL(runner, node);
    ASSERT_EQ(runner, consumed, 2u);
    delete node;

    consumed = 0;
    ASTNode* node2 = p.parse("<s>", "zy", consumed);
    ASSERT_NOT_NULL(runner, node2);
    ASSERT_EQ(runner, consumed, 2u);
    delete node2;
}

int main() {
    TestSuite suite("FIRST Memoization Test Suite");
    suite.addTest("Basic", test_first_basic);
    suite.addTest("Nullable Alt", test_first_nullable_alt);
    suite.addTest("Class and Range", test_first_class_range);
    TestRunner results = suite.run();
    results.printSummary();
    return results.allPassed() ? 0 : 1;
}
