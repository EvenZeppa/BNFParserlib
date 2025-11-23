#include "../include/TestFramework.hpp"
#include "../include/Grammar.hpp"
#include "../include/BNFParser.hpp"
#include "../include/Expression.hpp"
#include <iostream>
#include <string>

//
//  Utilitaire simple: compte les noeuds dans l'AST
//
int countAST(ASTNode* n) {
    if (!n) return 0;
    int c = 1;
    for (size_t i = 0; i < n->children.size(); ++i)
        c += countAST(n->children[i]);
    return c;
}

/**
 * @brief Test parsing terminal simple.
 */
void test_parse_terminal(TestRunner& runner) {

    Grammar g;
    g.addRule("<A> ::= 'HELLO'");
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<A>", "HELLO", consumed);

    ASSERT_TRUE(runner, ast != 0);
    ASSERT_EQ(runner, ast->matched, "HELLO");
    ASSERT_EQ(runner, consumed, 5);

    // Un AST terminal = 1 noeud
    ASSERT_EQ(runner, countAST(ast), 1);

    delete ast;
}

//
//  TEST 2 : parsing terminal FAIL
//
void test_parse_terminal_fail(TestRunner& runner) {

    Grammar g;
    g.addRule("<A> ::= 'HELLO'");
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<A>", "HALLO", consumed);

    ASSERT_TRUE(runner, ast == 0);
    ASSERT_EQ(runner, consumed, 0);
}

//
//  TEST 3 : séquence simple
//
void test_parse_sequence(TestRunner& runner) {

    Grammar g;
    g.addRule("<seq> ::= 'A' 'B' 'C'");
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<seq>", "ABC", consumed);

    ASSERT_TRUE(runner, ast != 0);
    ASSERT_EQ(runner, ast->matched, "ABC");
    ASSERT_EQ(runner, consumed, 3);

    ASSERT_EQ(runner, ast->children.size(), 3);

    delete ast;
}

//
//  TEST 4 : alternative (+ longest match)
//
void test_parse_alternative(TestRunner& runner) {

    Grammar g;
    g.addRule("<alt> ::= 'A' | 'AB' | 'ABC'");
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<alt>", "ABC", consumed);

    ASSERT_TRUE(runner, ast != 0);
    ASSERT_EQ(runner, ast->matched, "ABC");
    ASSERT_EQ(runner, consumed, 3);

    delete ast;
}

//
//  TEST 5 : alternative FAIL
//
void test_parse_alternative_fail(TestRunner& runner) {

    Grammar g;
    g.addRule("<alt> ::= 'A' | 'B'");
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<alt>", "C", consumed);

    ASSERT_TRUE(runner, ast == 0);
    ASSERT_EQ(runner, consumed, 0);
}

//
//  TEST 6 : optional
//
void test_parse_optional(TestRunner& runner) {

    Grammar g;
    g.addRule("<opt> ::= 'A' [ 'B' ] 'C'");
    BNFParser p(g);

    size_t consumed = 0;

    // Cas 1 : optionnel présent
    ASTNode* ast1 = p.parse("<opt>", "ABC", consumed);
    ASSERT_TRUE(runner, ast1 != 0);
    ASSERT_EQ(runner, ast1->matched, "ABC");
    ASSERT_EQ(runner, consumed, 3);
    delete ast1;

    // Cas 2 : optionnel absent
    consumed = 0;
    ASTNode* ast2 = p.parse("<opt>", "AC", consumed);
    ASSERT_TRUE(runner, ast2 != 0);
    ASSERT_EQ(runner, ast2->matched, "AC");
    ASSERT_EQ(runner, consumed, 2);
    delete ast2;

    // Cas 3 : optionnel échoue -> parse doit échouer
    consumed = 0;
    ASTNode* ast3 = p.parse("<opt>", "AXC", consumed);
    ASSERT_TRUE(runner, ast3 == 0);
    ASSERT_EQ(runner, consumed, 0);
}

//
//  TEST 7 : répétition
//
void test_parse_repetition(TestRunner& runner) {

    Grammar g;
    g.addRule("<rep> ::= 'A' { 'B' }");
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<rep>", "ABBB", consumed);

    ASSERT_TRUE(runner, ast != 0);
    ASSERT_EQ(runner, ast->matched, "ABBB");
    ASSERT_EQ(runner, consumed, 4);

    ASSERT_EQ(runner, countAST(ast), 1 + 1 + 1 + 3); // seq + A + rep + 3*B

    delete ast;
}

//
//  TEST 8 : symbol references
//
void test_parse_symbol(TestRunner& runner) {

    Grammar g;
    g.addRule("<digit> ::= '0' | '1'");
    g.addRule("<bin> ::= <digit> <digit> <digit>");
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<bin>", "101", consumed);

    ASSERT_TRUE(runner, ast != 0);
    ASSERT_EQ(runner, ast->matched, "101");
    ASSERT_EQ(runner, consumed, 3);

    delete ast;
}

//
//  TEST 9 : consommation totale NON obligatoire
//
void test_parse_must_consume_all(TestRunner& runner) {

    Grammar g;
    g.addRule("<A> ::= 'HI'");
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<A>", "HI!", consumed);

    ASSERT_TRUE(runner, ast != 0);
    ASSERT_EQ(runner, ast->matched, "HI");
    ASSERT_EQ(runner, consumed, 2); // NE DOIT PAS échouer maintenant

    delete ast;
}

//
//  TEST 10 : règle inconnue
//
void test_unknown_rule(TestRunner& runner) {

    Grammar g; // aucune règle définie
    BNFParser p(g);

    size_t consumed = 0;
    ASTNode* ast = p.parse("<unknown>", "hello", consumed);

    ASSERT_TRUE(runner, ast == 0);
    ASSERT_EQ(runner, consumed, 0);
}

int main() {
    TestSuite suite("Parser Test Suite");
    
    // Register all test functions
    suite.addTest("Parse Terminal", test_parse_terminal);
    suite.addTest("Parse Terminal Fail", test_parse_terminal_fail);
    suite.addTest("Parse Sequence", test_parse_sequence);
    suite.addTest("Parse Alternative", test_parse_alternative);
    suite.addTest("Parse Alternative Fail", test_parse_alternative_fail);
    suite.addTest("Parse Optional", test_parse_optional);
    suite.addTest("Parse Repetition", test_parse_repetition);
    suite.addTest("Parse Symbol", test_parse_symbol);
    suite.addTest("Parse Must Consume All", test_parse_must_consume_all);
    suite.addTest("Unknown Rule", test_unknown_rule);
    
    // Run all tests
    TestRunner results = suite.run();
    results.printSummary();
    
    return results.allPassed() ? 0 : 1;
}
