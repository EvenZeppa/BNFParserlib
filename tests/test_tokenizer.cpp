#include "../include/TestFramework.hpp"
#include "../include/BNFTokenizer.hpp"
#include <string>

/**
 * @brief Test single terminal token parsing.
 */
void test_single_terminal(TestRunner& runner) {
    BNFTokenizer tz("'A'");
    Token t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_TERMINAL);
    ASSERT_EQ(runner, t.value, "A");
    ASSERT_EQ(runner, tz.next().type, Token::TOK_END);
}

/**
 * @brief Test single symbol token parsing.
 */
void test_single_symbol(TestRunner& runner) {
    BNFTokenizer tz("<letter>");
    Token t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_SYMBOL);
    ASSERT_EQ(runner, t.value, "<letter>");
    ASSERT_EQ(runner, tz.next().type, Token::TOK_END);
}

/**
 * @brief Test word token parsing.
 */
void test_word_token(TestRunner& runner) {
    BNFTokenizer tz("WORD");
    Token t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_WORD);
    ASSERT_EQ(runner, t.value, "WORD");
    ASSERT_EQ(runner, tz.next().type, Token::TOK_END);
}

/**
 * @brief Test pipe token parsing.
 */
void test_pipe_token(TestRunner& runner) {
    BNFTokenizer tz("|");
    Token t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_PIPE);
    ASSERT_EQ(runner, t.value, "|");
}

/**
 * @brief Test braces and brackets token parsing.
 */
void test_braces_and_brackets(TestRunner& runner) {
    BNFTokenizer tz("{ } [ ]");
    Token t1 = tz.next();
    ASSERT_EQ(runner, t1.type, Token::TOK_LBRACE);
    Token t2 = tz.next();
    ASSERT_EQ(runner, t2.type, Token::TOK_RBRACE);
    Token t3 = tz.next();
    ASSERT_EQ(runner, t3.type, Token::TOK_LBRACKET);
    Token t4 = tz.next();
    ASSERT_EQ(runner, t4.type, Token::TOK_RBRACKET);
    ASSERT_EQ(runner, tz.next().type, Token::TOK_END);
}

/**
 * @brief Test complex expression token parsing.
 */
void test_complex_expression(TestRunner& runner) {
    BNFTokenizer tz("<letter> { <letter> | '0' } [ 'X' ]");
    Token t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_SYMBOL);
    t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_LBRACE);
    t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_SYMBOL);
    t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_PIPE);
    t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_TERMINAL);
    t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_RBRACE);
    t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_LBRACKET);
    t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_TERMINAL);
    t = tz.next();
    ASSERT_EQ(runner, t.type, Token::TOK_RBRACKET);
    ASSERT_EQ(runner, tz.next().type, Token::TOK_END);
}

/**
 * @brief Test peek vs next token functionality.
 */
void test_peek_vs_next(TestRunner& runner) {
    BNFTokenizer tz("'A' | 'B'");
    Token t1 = tz.peek();
    ASSERT_EQ(runner, t1.type, Token::TOK_TERMINAL);
    ASSERT_EQ(runner, t1.value, "A");

    Token t2 = tz.next();
    ASSERT_EQ(runner, t2.type, Token::TOK_TERMINAL);
    ASSERT_EQ(runner, t2.value, "A");

    Token t3 = tz.peek();
    ASSERT_EQ(runner, t3.type, Token::TOK_PIPE);

    Token t4 = tz.next();
    ASSERT_EQ(runner, t4.type, Token::TOK_PIPE);

    Token t5 = tz.next();
    ASSERT_EQ(runner, t5.type, Token::TOK_TERMINAL);
    ASSERT_EQ(runner, t5.value, "B");
}

int main() {
    TestSuite suite("Tokenizer Test Suite");
    
    // Register all test functions
    suite.addTest("Single Terminal", test_single_terminal);
    suite.addTest("Single Symbol", test_single_symbol);
    suite.addTest("Word Token", test_word_token);
    suite.addTest("Pipe Token", test_pipe_token);
    suite.addTest("Braces and Brackets", test_braces_and_brackets);
    suite.addTest("Complex Expression", test_complex_expression);
    suite.addTest("Peek vs Next", test_peek_vs_next);
    
    // Run all tests
    TestRunner results = suite.run();
    results.printSummary();
    
    return results.allPassed() ? 0 : 1;
}
