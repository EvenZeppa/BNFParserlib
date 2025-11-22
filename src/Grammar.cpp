#include "Grammar.hpp"
#include "Debug.hpp"
#include <iostream>
#include <sstream>

// ---------------- Rule ----------------
// Constructor and destructor for Rule.
// Rule owns the root expression node for the grammar rule.
// The destructor frees the root expression to avoid leaks.
Rule::Rule() : rootExpr(0) {}
Rule::~Rule() { delete rootExpr; }

// ---------------- Grammar ----------------
// Grammar lifecycle: initialize debug flag and clean up allocated rules.
Grammar::Grammar() : debug(false) {}
Grammar::~Grammar() {
	for (size_t i = 0; i < rules.size(); ++i) {
		DEBUG_MSG("Deleting rule: " + rules[i]->name);
		delete rules[i];
	}
}

// addRule: parse a textual rule of the form "LHS ::= RHS".
// Trims the LHS, tokenizes the RHS and constructs the expression tree
// which becomes the rule's root expression.
void Grammar::addRule(const std::string& ruleText) {
    DEBUG_MSG("Adding rule: " + ruleText);

    size_t pos = ruleText.find("::=");
    if (pos == std::string::npos) {
        std::cerr << "Invalid rule: " << ruleText << std::endl;
        return;
    }

    std::string lhs = ruleText.substr(0, pos);
    std::string rhs = ruleText.substr(pos + 3);

    // trim spaces
    while (!lhs.empty() && lhs[0] == ' ') lhs.erase(0,1);
    while (!lhs.empty() && lhs[lhs.size()-1] == ' ') lhs.erase(lhs.size()-1,1);

    Rule* r = new Rule();
    r->name = lhs;

    BNFTokenizer tz(rhs);
    r->rootExpr = parseExpression(tz);

    DEBUG_MSG("Parsed rootExpr for rule: " + lhs);
    rules.push_back(r);
}


// getRule: perform a linear search through stored rules and return
// a pointer to the rule matching the provided name, or nullptr if not found.
Rule* Grammar::getRule(const std::string& name) const {
    DEBUG_MSG("Searching for rule: " + name);
    for (size_t i = 0; i < rules.size(); ++i) {
        DEBUG_MSG("Checking rule: " + rules[i]->name);
        if (rules[i]->name == name) {
            DEBUG_MSG("Rule found: " + name);
            return rules[i];
        }
    }
    return 0;
}


// ---------------- Parsing functions ----------------

// parseExpression: parse alternatives separated by '|' and build an
// EXPR_ALTERNATIVE node when multiple alternatives are present.
Expression* Grammar::parseExpression(BNFTokenizer& tz) {
    Expression* left = parseSequence(tz);
    Token t = tz.peek();
    if (t.type != Token::TOK_PIPE)
        return left;

    Expression* alt = new Expression(Expression::EXPR_ALTERNATIVE);
    alt->children.push_back(left);

    while (tz.peek().type == Token::TOK_PIPE) {
        tz.next(); // skip |
        Expression* right = parseSequence(tz);
        alt->children.push_back(right);
    }

    if (alt->children.size() == 1) {
        Expression* single = alt->children[0];
        delete alt;
        return single;
    }

    std::stringstream ss;
    ss << "parseExpression: type=EXPR_ALTERNATIVE, children=" << alt->children.size();
    DEBUG_MSG(ss.str());

    return alt;
}

// parseSequence: parse a series of terms into a sequence node. Stops when
// reaching end, a pipe '|', or a closing bracket/brace.
Expression* Grammar::parseSequence(BNFTokenizer& tz) {
    std::vector<Expression*> children;

    while (true) {
        Token t = tz.peek();
        if (t.type == Token::TOK_END ||
            t.type == Token::TOK_PIPE ||
            t.type == Token::TOK_RBRACE ||
            t.type == Token::TOK_RBRACKET)
            break;

        Expression* term = parseTerm(tz);
        if (term)
            children.push_back(term);
    }

    if (children.empty())
        return NULL;
    if (children.size() == 1)
        return children[0];

    Expression* seq = new Expression(Expression::EXPR_SEQUENCE);
    seq->children = children;

    std::stringstream ss;
    ss << "parseSequence: type=EXPR_SEQUENCE, children=" << seq->children.size();
    DEBUG_MSG(ss.str());

    return seq;
}

// parseTerm: handle repetition '{ ... }' and optional '[ ... ]' constructs.
// For other tokens, delegate to parseFactor.
Expression* Grammar::parseTerm(BNFTokenizer& tz) {
    Token t = tz.peek();

    if (t.type == Token::TOK_LBRACE) {
        tz.next();
        Expression* inside = parseExpression(tz);
        if (tz.next().type != Token::TOK_RBRACE)
            std::cerr << "Missing '}'" << std::endl;

        Expression* rep = new Expression(Expression::EXPR_REPEAT);
        rep->children.push_back(inside);

        std::stringstream ss;
        ss << "parseTerm: EXPR_REPEAT, children=" << rep->children.size();
        DEBUG_MSG(ss.str());

        return rep;
    }

    if (t.type == Token::TOK_LBRACKET) {
        tz.next();
        Expression* inside = parseExpression(tz);
        if (tz.next().type != Token::TOK_RBRACKET)
            std::cerr << "Missing ']'" << std::endl;

        Expression* opt = new Expression(Expression::EXPR_OPTIONAL);
        opt->children.push_back(inside);

        std::stringstream ss;
        ss << "parseTerm: EXPR_OPTIONAL, children=" << opt->children.size();
        DEBUG_MSG(ss.str());

        return opt;
    }

    return parseFactor(tz);
}

// parseFactor: create leaf expression nodes from tokens: symbols or terminals.
// Emits an error for unexpected tokens.
Expression* Grammar::parseFactor(BNFTokenizer& tz) {
    Token t = tz.next();

    if (t.type == Token::TOK_SYMBOL) {
        Expression* e = new Expression(Expression::EXPR_SYMBOL);
        e->value = t.value;

        std::stringstream ss;
        ss << "parseFactor: EXPR_SYMBOL, value=" << t.value;
        DEBUG_MSG(ss.str());

        return e;
    }

    if (t.type == Token::TOK_TERMINAL || t.type == Token::TOK_WORD) {
        Expression* e = new Expression(Expression::EXPR_TERMINAL);
        e->value = t.value;

        std::stringstream ss;
        ss << "parseFactor: EXPR_TERMINAL, value=" << t.value;
        DEBUG_MSG(ss.str());

        return e;
    }

    std::cerr << "Unexpected token: " << t.value << std::endl;
    return NULL;
}


