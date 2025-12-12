#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include <string>
#include <vector>
#include "Expression.hpp"
#include "BNFTokenizer.hpp"
#include "Arena.hpp"

/**
 * @brief Represents a single grammar rule.
 * 
 * Contains a rule name (left-hand side) and the corresponding expression
 * tree (right-hand side) that defines the rule's structure.
 */
struct Rule {
	std::string name;       ///< Name of the rule (left-hand side)
	Expression* rootExpr;   ///< Root expression node (right-hand side)

	/**
	 * @brief Constructs an empty rule.
	 */
	Rule();

	/**
	 * @brief Destructor that cleans up the root expression.
	 */
	~Rule();
};

/**
 * @brief Container for BNF grammar rules and parser.
 * 
 * Manages a collection of grammar rules and provides functionality to
 * parse rule definitions and convert them into expression trees. Uses
 * recursive descent parsing to handle BNF syntax including alternatives,
 * sequences, repetitions, and optional elements.
 */
class Grammar {
public:
	/**
	 * @brief Constructs an empty grammar.
	 */
	Grammar();

	/**
	 * @brief Destructor that cleans up all stored rules.
	 */
	~Grammar();

	/**
	 * @brief Adds a new rule from textual BNF format.
	 * @param ruleText Rule in format "name ::= expression"
	 */
	void addRule(const std::string& ruleText);

	/**
	 * @brief Retrieves a rule by name.
	 * @param name The name of the rule to find
	 * @return Pointer to the rule, or nullptr if not found
	 */
	Rule* getRule(const std::string& name) const;

	/**
	 * @brief Attach an arena to allocate rules/expressions. Optional.
	 * When set, created nodes should be allocated from the arena.
	 */
	void setArena(Arena* a) { arena = a; }

private:
	/**
	 * @brief Parses alternatives separated by '|' operators.
	 * @param tz Tokenizer to read from
	 * @return Expression representing the alternatives
	 */
	Expression* parseExpression(BNFTokenizer& tz);

	/**
	 * @brief Parses a sequence of terms.
	 * @param tz Tokenizer to read from
	 * @return Expression representing the sequence
	 */
	Expression* parseSequence(BNFTokenizer& tz);

	/**
	 * @brief Parses terms with repetition {} or optional [] modifiers.
	 * @param tz Tokenizer to read from
	 * @return Expression representing the term
	 */
	Expression* parseTerm(BNFTokenizer& tz);

	/**
	 * @brief Parses basic factors (symbols and terminals).
	 * @param tz Tokenizer to read from
	 * @return Expression representing the factor
	 */
	Expression* parseFactor(BNFTokenizer& tz);

	/**
	 * @brief Parses a character class expression in parentheses.
	 * @param tz Tokenizer to read from
	 * @return Expression representing the character class
	 */
	Expression* parseCharClass(BNFTokenizer& tz);

	/**
	 * @brief Converts a token to a character value.
	 * @param t Token (terminal or hex)
	 * @return Character value
	 */
	unsigned char tokenToChar(const Token& t) const;

	std::vector<Rule*> rules;   ///< Collection of grammar rules
	Arena* arena;               ///< Optional arena for allocations (nullable)
};
#endif
