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

/**
 * @brief Test character range expression.
 */
void test_char_range(TestRunner& runner) {
    Expression* range = new Expression(Expression::EXPR_CHAR_RANGE);
    range->charRange = CharRange('a', 'z');
    
    ASSERT_EQ(runner, range->type, Expression::EXPR_CHAR_RANGE);
    ASSERT_EQ(runner, static_cast<int>(range->charRange.start), static_cast<int>('a'));
    ASSERT_EQ(runner, static_cast<int>(range->charRange.end), static_cast<int>('z'));
    
    delete range;
}

/**
 * @brief Test inclusive character class expression.
 */
void test_inclusive_char_class(TestRunner& runner) {
    Expression* cls = new Expression(Expression::EXPR_CHAR_CLASS);
    // Build bitmap for [a-z0-9_]
    for (unsigned char c = 'a'; c <= 'z'; ++c) cls->charBitmap.set(c);
    for (unsigned char c = '0'; c <= '9'; ++c) cls->charBitmap.set(c);
    cls->charBitmap.set('_');
    
    ASSERT_EQ(runner, cls->type, Expression::EXPR_CHAR_CLASS);
    // Validate bitmap contains expected characters
    ASSERT_EQ(runner, cls->classMatches('a'), true);
    ASSERT_EQ(runner, cls->classMatches('z'), true);
    ASSERT_EQ(runner, cls->classMatches('0'), true);
    ASSERT_EQ(runner, cls->classMatches('9'), true);
    ASSERT_EQ(runner, cls->classMatches('_'), true);
    ASSERT_EQ(runner, cls->classMatches('!'), false);
    
    delete cls;
}

/**
 * @brief Test exclusive character class expression.
 */
void test_exclusive_char_class(TestRunner& runner) {
    Expression* cls = new Expression(Expression::EXPR_CHAR_CLASS);
    // Exclusive class of [^ ,\n]
    // Start with all bits set to 1, then unset excluded ones
    cls->charBitmap.set();
    cls->charBitmap.reset(' ');
    cls->charBitmap.reset(',');
    cls->charBitmap.reset(static_cast<unsigned char>(0x0A));
    
    ASSERT_EQ(runner, cls->type, Expression::EXPR_CHAR_CLASS);
    // Validate excluded characters are not matched, others are
    ASSERT_EQ(runner, cls->classMatches(' '), false);
    ASSERT_EQ(runner, cls->classMatches(','), false);
    ASSERT_EQ(runner, cls->classMatches(static_cast<unsigned char>(0x0A)), false);
    ASSERT_EQ(runner, cls->classMatches('A'), true);
    
    delete cls;
}

int main() {
    TestSuite suite("Expression Test Suite");
    
    // Register all test functions
    suite.addTest("Simple Sequence", test_simple_sequence);
    suite.addTest("Simple Alternative", test_simple_alternative);
    suite.addTest("Nested Expression", test_nested_expression);
    suite.addTest("Character Range", test_char_range);
    suite.addTest("Inclusive Character Class", test_inclusive_char_class);
    suite.addTest("Exclusive Character Class", test_exclusive_char_class);
    
    // Run all tests
    TestRunner results = suite.run();
    results.printSummary();
    
    return results.allPassed() ? 0 : 1;
}
