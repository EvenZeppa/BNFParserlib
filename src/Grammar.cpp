#include "../include/Grammar.hpp"
#include "../include/Debug.hpp"
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
Grammar::Grammar() {}
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
// Also handles character ranges and character classes.
Expression* Grammar::parseFactor(BNFTokenizer& tz) {
    Token t = tz.next();

    // Character class in parentheses
    if (t.type == Token::TOK_LPAREN) {
        return parseCharClass(tz);
    }

    // Check for character range (terminal/hex followed by ellipsis)
    if (t.type == Token::TOK_TERMINAL || t.type == Token::TOK_HEX) {
        Token peek = tz.peek();
        if (peek.type == Token::TOK_ELLIPSIS) {
            // This is a character range
            tz.next(); // consume ellipsis
            Token endToken = tz.next();
            
            if (endToken.type != Token::TOK_TERMINAL && endToken.type != Token::TOK_HEX) {
                std::cerr << "Expected terminal or hex after ellipsis in range" << std::endl;
                return NULL;
            }
            
            unsigned char start = tokenToChar(t);
            unsigned char end = tokenToChar(endToken);
            
            Expression* e = new Expression(Expression::EXPR_CHAR_RANGE);
            e->charRange = CharRange(start, end);
            
            std::stringstream ss;
            ss << "parseFactor: EXPR_CHAR_RANGE, start=" << (int)start << ", end=" << (int)end;
            DEBUG_MSG(ss.str());
            
            return e;
        }
        
        // Regular terminal (not a range)
        Expression* e = new Expression(Expression::EXPR_TERMINAL);
        e->value = t.value;

        std::stringstream ss;
        ss << "parseFactor: EXPR_TERMINAL, value=" << t.value;
        DEBUG_MSG(ss.str());

        return e;
    }

    if (t.type == Token::TOK_SYMBOL) {
        Expression* e = new Expression(Expression::EXPR_SYMBOL);
        e->value = t.value;

        std::stringstream ss;
        ss << "parseFactor: EXPR_SYMBOL, value=" << t.value;
        DEBUG_MSG(ss.str());

        return e;
    }

    if (t.type == Token::TOK_WORD) {
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

// parseCharClass: parse a character class expression in parentheses
// Format: ( [^] (terminal|hex|range)* )
Expression* Grammar::parseCharClass(BNFTokenizer& tz) {
    Expression* cls = new Expression(Expression::EXPR_CHAR_CLASS);
    // Build bitmap progressively; default all bits to false
    cls->charBitmap.reset();
    
    // Check for exclusion marker (^)
    Token t = tz.peek();
    bool isExclusion = false;
    if (t.type == Token::TOK_CARET) {
        tz.next(); // consume caret
        isExclusion = true;
    }
    
    // Parse character list and ranges until we hit ')'
    while (true) {
        t = tz.peek();
        
        if (t.type == Token::TOK_RPAREN) {
            tz.next(); // consume ')'
            break;
        }
        
        if (t.type == Token::TOK_END) {
            std::cerr << "Unexpected end of input in character class" << std::endl;
            delete cls;
            return NULL;
        }
        
        if (t.type == Token::TOK_TERMINAL || t.type == Token::TOK_HEX) {
            tz.next(); // consume terminal/hex
            
            // Check if this is a range
            Token peek = tz.peek();
            if (peek.type == Token::TOK_ELLIPSIS) {
                tz.next(); // consume ellipsis
                Token endToken = tz.next();
                
                if (endToken.type != Token::TOK_TERMINAL && endToken.type != Token::TOK_HEX) {
                    std::cerr << "Expected terminal or hex after ellipsis in character class range" << std::endl;
                    delete cls;
                    return NULL;
                }
                
                unsigned char start = tokenToChar(t);
                unsigned char end = tokenToChar(endToken);
                if (start <= end) {
                    for (unsigned int c = start; c <= end; ++c) {
                        cls->charBitmap.set(c, true);
                    }
                } else {
                    // if reversed, still support by swapping
                    for (unsigned int c = end; c <= start; ++c) {
                        cls->charBitmap.set(c, true);
                    }
                }
                DEBUG_MSG("parseCharClass: added range to bitmap " << (int)start << " ... " << (int)end);
            } else {
                // Single character
                unsigned char ch = tokenToChar(t);
                cls->charBitmap.set(ch, true);
                DEBUG_MSG("parseCharClass: added char to bitmap " << (int)ch);
            }
        } else {
            std::cerr << "Unexpected token in character class: " << t.value << std::endl;
            delete cls;
            return NULL;
        }
    }
    // Apply exclusion by inverting bitmap if needed
    if (isExclusion) {
        cls->charBitmap.flip();
        DEBUG_MSG("parseCharClass: applied exclusion (bitmap inverted)");
    }

    DEBUG_MSG("parseCharClass: bitmap built");
    return cls;
}

// tokenToChar: convert a terminal or hex token to a character value
unsigned char Grammar::tokenToChar(const Token& t) const {
    if (t.type == Token::TOK_TERMINAL) {
        // Terminal tokens are stored without quotes
        if (t.value.empty()) return 0;
        return static_cast<unsigned char>(t.value[0]);
    }
    
    if (t.type == Token::TOK_HEX) {
        // Parse hexadecimal value (format: 0xNN)
        std::string hexStr = t.value.substr(2); // skip "0x"
        unsigned int val = 0;
        std::stringstream ss;
        ss << std::hex << hexStr;
        ss >> val;
        return static_cast<unsigned char>(val);
    }
    
    return 0;
}

