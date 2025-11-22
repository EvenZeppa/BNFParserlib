#include "UnitTest.hpp"
#include "Grammar.hpp"
#include "BNFTokenizer.hpp"
#include "BNFParserInternal.hpp"
#include "Expression.hpp"
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

void test_simple_letter_rule() {
	std::cout << "Running test_simple_letter_rule...\n";
	Grammar g;
	g.addRule("<letter> ::= 'A' | 'B' | 'C'");
	Rule* r = g.getRule("<letter>");
	ASSERT_TRUE(r != NULL);
	ASSERT_TRUE(r->rootExpr != NULL);

	Expression* expr = r->rootExpr;
	ASSERT_EQ(expr->type, Expression::EXPR_ALTERNATIVE);
	ASSERT_EQ(expr->children.size(), 3); // A, B, C

	for (size_t i = 0; i < expr->children.size(); ++i)
		ASSERT_EQ(expr->children[i]->type, Expression::EXPR_TERMINAL);
}

void test_simple_nick_rule() {
	std::cout << "Running test_simple_nick_rule...\n";
	Grammar g;
	g.addRule("<letter> ::= 'A' | 'B' | 'C'");
	g.addRule("<number> ::= '0' | '1' | '2'");
	g.addRule("<nick> ::= <letter> { <letter> | <number> }");

	Rule* r = g.getRule("<nick>");
	ASSERT_TRUE(r != NULL);
	ASSERT_TRUE(r->rootExpr != NULL);

	Expression* expr = r->rootExpr;

	// L'AST devrait être une SEQUENCE
	ASSERT_EQ(expr->type, Expression::EXPR_SEQUENCE);
	ASSERT_EQ(expr->children.size(), 2);

	// Premier enfant : <letter>
	ASSERT_EQ(expr->children[0]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(expr->children[0]->value, "<letter>");

	// Deuxième enfant : EXPR_REPEAT contenant EXPR_ALTERNATIVE
	Expression* rep = expr->children[1];
	ASSERT_EQ(rep->type, Expression::EXPR_REPEAT);
	ASSERT_EQ(rep->children.size(), 1);

	Expression* alt = rep->children[0];
	ASSERT_EQ(alt->type, Expression::EXPR_ALTERNATIVE);
	ASSERT_EQ(alt->children.size(), 2);

	// Vérifier les enfants de l'alternative
	ASSERT_EQ(alt->children[0]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(alt->children[0]->value, "<letter>");
	ASSERT_EQ(alt->children[1]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(alt->children[1]->value, "<number>");
}

void test_simple_command_rule() {
	std::cout << "Running test_simple_command_rule...\n";
	Grammar g;
	g.addRule("<letter> ::= 'A' | 'B' | 'C'");
	g.addRule("<number> ::= '0' | '1' | '2'");
	g.addRule("<command>  ::= <letter> { <letter> } | <number> <number> <number>");

	Rule* r = g.getRule("<command>");
	ASSERT_TRUE(r != NULL);
	ASSERT_TRUE(r->rootExpr != NULL);

	Expression* expr = r->rootExpr;
	// L'AST devrait être une ALTERNATIVE
	ASSERT_EQ(expr->type, Expression::EXPR_ALTERNATIVE);
	ASSERT_EQ(expr->children.size(), 2);

	// Premier enfant : SEQUENCE <letter> { <letter> }
	Expression* seq1 = expr->children[0];
	ASSERT_EQ(seq1->type, Expression::EXPR_SEQUENCE);
	ASSERT_EQ(seq1->children.size(), 2);
	ASSERT_EQ(seq1->children[0]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(seq1->children[0]->value, "<letter>");
	ASSERT_EQ(seq1->children[1]->type, Expression::EXPR_REPEAT);
	ASSERT_EQ(seq1->children[1]->children.size(), 1);
	ASSERT_EQ(seq1->children[1]->children[0]->type, Expression::EXPR_SYMBOL);
	ASSERT_EQ(seq1->children[1]->children[0]->value, "<letter>");

	// Deuxième enfant : SEQUENCE <number> <number> <number>
	Expression* seq2 = expr->children[1];
	ASSERT_EQ(seq2->type, Expression::EXPR_SEQUENCE);
	ASSERT_EQ(seq2->children.size(), 3);
	for (size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(seq2->children[i]->type, Expression::EXPR_SYMBOL);
		ASSERT_EQ(seq2->children[i]->value, "<number>");
	}
}

int main() {
	test_simple_letter_rule();
	test_simple_nick_rule();
	test_simple_command_rule();
	printTestSummary();
	return (failed == 0) ? 0 : 1;
}
