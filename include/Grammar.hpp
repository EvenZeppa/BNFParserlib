#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include <string>
#include <vector>
#include "Expression.hpp"
#include "BNFTokenizer.hpp"

struct Rule {
	std::string name;
	Expression* rootExpr;

	Rule();
	~Rule();
};

class Grammar {
public:
	Grammar();
	~Grammar();

	void addRule(const std::string& ruleText);
	Rule* getRule(const std::string& name) const;

	// Flag debug
	bool debug;

private:
	Expression* parseExpression(BNFTokenizer& tz);
	Expression* parseSequence(BNFTokenizer& tz);
	Expression* parseTerm(BNFTokenizer& tz);
	Expression* parseFactor(BNFTokenizer& tz);

	std::vector<Rule*> rules;
};
#endif
