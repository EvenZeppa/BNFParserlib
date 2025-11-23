#include "../include/TestFramework.hpp"
#include "../include/Grammar.hpp"
#include "../include/BNFTokenizer.hpp"
#include "../include/Expression.hpp"
#include <string>
#include <vector>
#include <iostream>

// Fonction utilitaire pour compter le nombre de noeuds dans l'AST
int countNodes(Expression* expr) {
	if (!expr) return 0;
	int cnt = 1;
	for (size_t i = 0; i < expr->children.size(); ++i)
		cnt += countNodes(expr->children[i]);
	return cnt;
}

/**
 * @brief Test simple letter rule parsing.
 */
void test_simple_letter_rule(TestRunner& runner) {
	Grammar g;
	g.addRule("<letter> ::= 'A' | 'B' | 'C'");
	Rule* r = g.getRule("<letter>");
	ASSERT_NOT_NULL(runner, r);
	ASSERT_NOT_NULL(runner, r->rootExpr);

	Expression* expr = r->rootExpr;
	ASSERT_EQ(runner, expr->type, Expression::EXPR_ALTERNATIVE);
	ASSERT_EQ(runner, expr->children.size(), 3); // A, B, C

	for (size_t i = 0; i < expr->children.size(); ++i)
		ASSERT_EQ(runner, expr->children[i]->type, Expression::EXPR_TERMINAL);
}

/**
 * @brief Test simple nick rule parsing with repetition.
 */
void test_simple_nick_rule(TestRunner& runner) {
	Grammar g;
	g.addRule("<letter> ::= 'A' | 'B' | 'C'");
	g.addRule("<number> ::= '0' | '1' | '2'");
	g.addRule("<nick> ::= <letter> { <letter> | <number> }");

	Rule* r = g.getRule("<nick>");
	ASSERT_NOT_NULL(runner, r);
	ASSERT_NOT_NULL(runner, r->rootExpr);

	Expression* expr = r->rootExpr;

	// L'AST devrait être une SEQUENCE
	ASSERT_EQ(runner, expr->type, Expression::EXPR_SEQUENCE);
	ASSERT_EQ(runner, expr->children.size(), 2);

	// Premier enfant : <letter>
	ASSERT_EQ(runner, expr->children[0]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(runner, expr->children[0]->value, "<letter>");

	// Deuxième enfant : EXPR_REPEAT contenant EXPR_ALTERNATIVE
	Expression* rep = expr->children[1];
	ASSERT_EQ(runner, rep->type, Expression::EXPR_REPEAT);
	ASSERT_EQ(runner, rep->children.size(), 1);

	Expression* alt = rep->children[0];
	ASSERT_EQ(runner, alt->type, Expression::EXPR_ALTERNATIVE);
	ASSERT_EQ(runner, alt->children.size(), 2);

	// Vérifier les enfants de l'alternative
	ASSERT_EQ(runner, alt->children[0]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(runner, alt->children[0]->value, "<letter>");
	ASSERT_EQ(runner, alt->children[1]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(runner, alt->children[1]->value, "<number>");
}

/**
 * @brief Test simple command rule parsing with complex alternatives.
 */
void test_simple_command_rule(TestRunner& runner) {
	Grammar g;
	g.addRule("<letter> ::= 'A' | 'B' | 'C'");
	g.addRule("<number> ::= '0' | '1' | '2'");
	g.addRule("<command>  ::= <letter> { <letter> } | <number> <number> <number>");

	Rule* r = g.getRule("<command>");
	ASSERT_NOT_NULL(runner, r);
	ASSERT_NOT_NULL(runner, r->rootExpr);

	Expression* expr = r->rootExpr;
	// L'AST devrait être une ALTERNATIVE
	ASSERT_EQ(runner, expr->type, Expression::EXPR_ALTERNATIVE);
	ASSERT_EQ(runner, expr->children.size(), 2);

	// Premier enfant : SEQUENCE <letter> { <letter> }
	Expression* seq1 = expr->children[0];
	ASSERT_EQ(runner, seq1->type, Expression::EXPR_SEQUENCE);
	ASSERT_EQ(runner, seq1->children.size(), 2);
	ASSERT_EQ(runner, seq1->children[0]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(runner, seq1->children[0]->value, "<letter>");
	ASSERT_EQ(runner, seq1->children[1]->type, Expression::EXPR_REPEAT);
	ASSERT_EQ(runner, seq1->children[1]->children.size(), 1);
	ASSERT_EQ(runner, seq1->children[1]->children[0]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(runner, seq1->children[1]->children[0]->value, "<letter>");

	// Deuxième enfant : SEQUENCE <number> <number> <number>
	Expression* seq2 = expr->children[1];
	ASSERT_EQ(runner, seq2->type, Expression::EXPR_SEQUENCE);
	ASSERT_EQ(runner, seq2->children.size(), 3);
	for (size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(runner, seq2->children[i]->type, Expression::EXPR_SYMBOL);
		ASSERT_EQ(runner, seq2->children[i]->value, "<number>");
	}
}

int main() {
	TestSuite suite("Grammar Test Suite");
	
	// Register all test functions
	suite.addTest("Simple Letter Rule", test_simple_letter_rule);
	suite.addTest("Simple Nick Rule", test_simple_nick_rule);
	suite.addTest("Simple Command Rule", test_simple_command_rule);
	
	// Run all tests
	TestRunner results = suite.run();
	results.printSummary();
	
	return results.allPassed() ? 0 : 1;
}
