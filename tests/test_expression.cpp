#include "../include/Expression.hpp"
#include "../include/TestFramework.hpp"

/**
 * @brief Test simple sequence expression functionality.
 */
void test_simple_sequence(TestRunner& runner) {
    Expression* seq = new Expression(Expression::EXPR_SEQUENCE);
    Expression* t1 = new Expression(Expression::EXPR_TERMINAL);
    t1->value = "A";
    Expression* t2 = new Expression(Expression::EXPR_TERMINAL);
    t2->value = "B";
    seq->children.push_back(t1);
    seq->children.push_back(t2);

    ASSERT_EQ(runner, seq->type, Expression::EXPR_SEQUENCE);
    ASSERT_EQ(runner, seq->children.size(), 2u);
    ASSERT_EQ(runner, seq->children[0]->value, std::string("A"));
    ASSERT_EQ(runner, seq->children[1]->value, std::string("B"));

    delete seq;
}

/**
 * @brief Test simple alternative expression functionality.
 */
void test_simple_alternative(TestRunner& runner) {
    Expression* alt = new Expression(Expression::EXPR_ALTERNATIVE);
    Expression* t1 = new Expression(Expression::EXPR_TERMINAL);
    t1->value = "X";
    Expression* t2 = new Expression(Expression::EXPR_TERMINAL);
    t2->value = "Y";
    alt->children.push_back(t1);
    alt->children.push_back(t2);

    ASSERT_EQ(runner, alt->type, Expression::EXPR_ALTERNATIVE);
    ASSERT_EQ(runner, alt->children.size(), 2u);
    ASSERT_EQ(runner, alt->children[0]->value, std::string("X"));
    ASSERT_EQ(runner, alt->children[1]->value, std::string("Y"));

    delete alt;
}

/**
 * @brief Test nested expression functionality.
 */
void test_nested_expression(TestRunner& runner) {
    Expression* rep = new Expression(Expression::EXPR_REPEAT);
    Expression* term = new Expression(Expression::EXPR_TERMINAL);
    term->value = "Z";
    rep->children.push_back(term);

    ASSERT_EQ(runner, rep->type, Expression::EXPR_REPEAT);
    ASSERT_EQ(runner, rep->children.size(), 1u);
    ASSERT_EQ(runner, rep->children[0]->value, std::string("Z"));

    delete rep;
}

int main() {
    TestSuite suite("Expression Test Suite");
    
    // Register all test functions
    suite.addTest("Simple Sequence", test_simple_sequence);
    suite.addTest("Simple Alternative", test_simple_alternative);
    suite.addTest("Nested Expression", test_nested_expression);
    
    // Run all tests
    TestRunner results = suite.run();
    results.printSummary();
    
    return results.allPassed() ? 0 : 1;
}
