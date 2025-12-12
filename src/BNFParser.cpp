#include "../include/BNFParser.hpp"
#include "../include/Expression.hpp"
#include "../include/Debug.hpp"
#include <iostream>
#include <cstring>

// BNFParser implementation
BNFParser::BNFParser(const Grammar& g)
    : grammar(g)
{
}

BNFParser::~BNFParser() {}

void BNFParser::mergeFirst(FirstInfo& dst, const FirstInfo& src) const {
    dst.chars |= src.chars;
    dst.nullable = dst.nullable || src.nullable;
}

void BNFParser::addChar(FirstInfo& fi, unsigned char c) const {
    fi.chars.set(static_cast<size_t>(c));
}

std::string BNFParser::terminalFirstString(Expression* expr) const {
    std::string literal = stripQuotes(expr->value);
    return literal;
}

const BNFParser::FirstInfo& BNFParser::computeFirst(Expression* expr) const {
    std::map<Expression*, FirstInfo>::iterator it = firstCache.find(expr);
    if (it != firstCache.end()) return it->second;

    FirstInfo fi;
    switch (expr->type) {
        case Expression::EXPR_TERMINAL: {
            std::string lit = terminalFirstString(expr);
            if (!lit.empty()) {
                addChar(fi, static_cast<unsigned char>(lit[0]));
            } else {
                fi.nullable = true;
            }
            break;
        }
        case Expression::EXPR_SYMBOL: {
            Rule* rr = grammar.getRule(expr->value);
            if (rr && rr->rootExpr) {
                fi = computeFirst(rr->rootExpr);
            }
            break;
        }
        case Expression::EXPR_SEQUENCE: {
            fi.nullable = true;
            for (size_t i = 0; i < expr->children.size(); ++i) {
                const FirstInfo& childFirst = computeFirst(expr->children[i]);
                mergeFirst(fi, childFirst);
                if (!childFirst.nullable) {
                    fi.nullable = false;
                    break;
                }
            }
            break;
        }
        case Expression::EXPR_ALTERNATIVE: {
            for (size_t i = 0; i < expr->children.size(); ++i) {
                const FirstInfo& childFirst = computeFirst(expr->children[i]);
                mergeFirst(fi, childFirst);
            }
            break;
        }
        case Expression::EXPR_OPTIONAL: {
            fi.nullable = true;
            if (!expr->children.empty()) {
                const FirstInfo& childFirst = computeFirst(expr->children[0]);
                mergeFirst(fi, childFirst);
            }
            break;
        }
        case Expression::EXPR_REPEAT: {
            fi.nullable = true;
            if (!expr->children.empty()) {
                const FirstInfo& childFirst = computeFirst(expr->children[0]);
                mergeFirst(fi, childFirst);
            }
            break;
        }
        case Expression::EXPR_CHAR_RANGE: {
            unsigned char start = expr->charRange.start;
            unsigned char end = expr->charRange.end;
            for (unsigned int c = start; c <= end; ++c) {
                addChar(fi, static_cast<unsigned char>(c));
                if (c == 255) break; // avoid overflow
            }
            fi.nullable = false;
            break;
        }
        case Expression::EXPR_CHAR_CLASS: {
            fi.nullable = false;
            for (size_t i = 0; i < 256; ++i) {
                if (expr->classMatches(static_cast<unsigned char>(i))) fi.chars.set(i);
            }
            break;
        }
        default:
            break;
    }

    std::pair<std::map<Expression*, FirstInfo>::iterator, bool> inserted = firstCache.insert(std::make_pair(expr, fi));
    return inserted.first->second;
}

// Remove surrounding quotes from terminal strings
std::string BNFParser::stripQuotes(const std::string& s) const{
    if (s.size() >= 2 && ((s[0] == '\'' && s[s.size()-1] == '\'') ||
                          (s[0] == '"'  && s[s.size()-1] == '"')))
    {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

// Main parsing entry point - parses input according to the specified rule
ASTNode* BNFParser::parse(const std::string& ruleName,
                          const std::string& input,
                          size_t& consumed) const
{
    DEBUG_MSG("Starting parse for rule: " + ruleName + " with input: '" + input + "'");
    consumed = 0;

    // Find the requested grammar rule
    Rule* r = grammar.getRule(ruleName);
    if (!r) {
        DEBUG_MSG("Rule not found: " + ruleName);
        std::cerr << "BNFParser::parse: rule not found: " << ruleName << std::endl;
        return 0;
    }

    // Attempt to parse the input using the rule's expression
    size_t pos = 0;
    ASTNode* root = 0;
    bool ok = parseExpression(r->rootExpr, input, pos, root);

    if (!ok) {
        DEBUG_MSG("Parse failed for rule: " + ruleName);
        if (root) delete root;
        return 0;
    }

    consumed = pos;   // Export how much input was consumed by the parser
    DEBUG_MSG("Parse successful, consumed " << consumed << " characters");

    return root;
}


// Recursive expression parser dispatcher - delegates to specific parsing functions
bool BNFParser::parseExpression(Expression* expr,
                                const std::string& input,
                                size_t& pos,
                                ASTNode*& outNode) const
{
    if (!expr) {
        DEBUG_MSG("parseExpression: null expression");
        return false;
    }

    DEBUG_MSG("parseExpression: type=" << expr->type << " at pos=" << pos);

    switch (expr->type) {
        case Expression::EXPR_TERMINAL:
            return parseTerminal(expr, input, pos, outNode);
        case Expression::EXPR_SYMBOL:
            return parseSymbol(expr, input, pos, outNode);
        case Expression::EXPR_SEQUENCE:
            return parseSequence(expr, input, pos, outNode);
        case Expression::EXPR_ALTERNATIVE:
            return parseAlternative(expr, input, pos, outNode);
        case Expression::EXPR_OPTIONAL:
            return parseOptional(expr, input, pos, outNode);
        case Expression::EXPR_REPEAT:
            return parseRepeat(expr, input, pos, outNode);
        case Expression::EXPR_CHAR_RANGE:
            return parseCharRange(expr, input, pos, outNode);
        case Expression::EXPR_CHAR_CLASS:
            return parseCharClass(expr, input, pos, outNode);
        default:
            DEBUG_MSG("parseExpression: unsupported expr type " << expr->type);
            std::cerr << "BNFParser::parseExpression: unsupported expr type\n";
            return false;
    }
}

// Parse terminal expressions (quoted strings)
bool BNFParser::parseTerminal(Expression* expr,
                              const std::string& input,
                              size_t& pos,
                              ASTNode*& outNode) const
{
    std::string literal = stripQuotes(expr->value);
    DEBUG_MSG("parseTerminal: trying to match '" << literal << "' at pos=" << pos);

    size_t len = literal.size();
    if (len == 0) {
        DEBUG_MSG("parseTerminal: empty literal");
        return false;
    }

    if (pos + len <= input.size() && input.compare(pos, len, literal) == 0) {
        DEBUG_MSG("parseTerminal: matched '" << literal << "'");
        ASTNode* node = new ASTNode(literal);
        node->matched = literal;
        pos += len;
        outNode = node;
        return true;
    }
    
    DEBUG_MSG("parseTerminal: failed to match '" + literal + "'");
    return false;
}

// Parse symbol expressions (non-terminal references)
bool BNFParser::parseSymbol(Expression* expr,
                            const std::string& input,
                            size_t& pos,
                            ASTNode*& outNode) const
{
    DEBUG_MSG("parseSymbol: resolving symbol '" << expr->value << "' at pos=" << pos);
    
    Rule* rr = grammar.getRule(expr->value);
    if (!rr) {
        DEBUG_MSG("parseSymbol: unknown symbol " << expr->value);
        std::cerr << "BNFParser::parseSymbol: unknown symbol " << expr->value << std::endl;
        return false;
    }
    
    size_t savedPos = pos;
    ASTNode* child = 0;
    bool ok = parseExpression(rr->rootExpr, input, pos, child);
    if (!ok) {
        DEBUG_MSG("parseSymbol: failed to parse symbol " << expr->value);
        pos = savedPos;
        return false;
    }

    DEBUG_MSG("parseSymbol: successfully parsed symbol " << expr->value);
    ASTNode* node = new ASTNode(expr->value);
    if (child) {
        node->children.push_back(child);
        node->matched = child->matched;
    }
    outNode = node;
    return true;
}

// Parse sequence expressions (ordered list of sub-expressions)
bool BNFParser::parseSequence(Expression* expr,
                              const std::string& input,
                              size_t& pos,
                              ASTNode*& outNode) const
{
    DEBUG_MSG("parseSequence: parsing " << expr->children.size() << " elements at pos=" << pos);

    size_t savedPos = pos;
    std::vector<ASTNode*> tmpChildren;
    std::string matchedAccum;

    for (size_t i = 0; i < expr->children.size(); ++i) {
        ASTNode* childNode = 0;
        bool ok = parseExpression(expr->children[i], input, pos, childNode);
        if (!ok) {
            DEBUG_MSG("parseSequence: failed at element " << i);
            for (size_t j = 0; j < tmpChildren.size(); ++j)
                delete tmpChildren[j];
            pos = savedPos;
            return false;
        }
        tmpChildren.push_back(childNode);
        if (childNode) matchedAccum += childNode->matched;
    }

    DEBUG_MSG("parseSequence: successfully parsed all elements, matched='" << matchedAccum << "'");
    ASTNode* parent = new ASTNode("<seq>");
    parent->matched = matchedAccum;
    parent->children.reserve(tmpChildren.size());
    for (size_t k = 0; k < tmpChildren.size(); ++k)
        parent->children.push_back(tmpChildren[k]);

    outNode = parent;
    return true;
}

// Parse alternative expressions (choice between sub-expressions)
bool BNFParser::parseAlternative(Expression* expr,
                                 const std::string& input,
                                 size_t& pos,
                                 ASTNode*& outNode) const
{
    DEBUG_MSG("parseAlternative: trying " << expr->children.size() << " alternatives at pos=" << pos);

    ASTNode* bestNode = 0;
    size_t bestPos = pos;
    bool anyMatch = false;

    bool hasChar = pos < input.size();
    unsigned char look = hasChar ? static_cast<unsigned char>(input[pos]) : 0;

    for (size_t i = 0; i < expr->children.size(); ++i) {
        if (hasChar) {
            const FirstInfo& fi = computeFirst(expr->children[i]);
            if (!fi.nullable && !fi.chars.test(look)) {
                DEBUG_MSG("parseAlternative: skipping alt " << i << " due to FIRST mismatch");
                continue;
            }
            if (!fi.nullable && fi.chars.none()) {
                DEBUG_MSG("parseAlternative: skipping alt " << i << " (empty FIRST and not nullable)");
                continue;
            }
        } else {
            const FirstInfo& fi = computeFirst(expr->children[i]);
            if (!fi.nullable) {
                DEBUG_MSG("parseAlternative: skipping alt " << i << " at EOF due to non-nullable FIRST");
                continue;
            }
        }
        size_t savedPos = pos;
        ASTNode* branchNode = 0;
        bool ok = parseExpression(expr->children[i], input, pos, branchNode);

        if (ok) {
            DEBUG_MSG("parseAlternative: alternative " << i << " matched, advanced to pos=" << pos);
            anyMatch = true;
            if (pos > bestPos) {
                if (bestNode) delete bestNode;
                bestNode = new ASTNode("<alt>");
                bestNode->children.push_back(branchNode);
                bestNode->matched = branchNode->matched;
                bestPos = pos;
            } else {
                delete branchNode;
            }
        } else {
            DEBUG_MSG("parseAlternative: alternative " << i << " failed");
        }
        pos = savedPos;
    }

    if (!anyMatch) {
        DEBUG_MSG("parseAlternative: no alternatives matched");
        return false;
    }

    DEBUG_MSG("parseAlternative: best match advanced to pos=" << bestPos);
    pos = bestPos;
    outNode = bestNode;
    return true;
}

// Parse optional expressions (zero or one occurrence)
bool BNFParser::parseOptional(Expression* expr,
                              const std::string& input,
                              size_t& pos,
                              ASTNode*& outNode) const
{
    DEBUG_MSG("parseOptional: attempting optional at pos=" << pos);

    size_t savedPos = pos;
    ASTNode* inside = 0;
    bool ok = parseExpression(expr->children[0], input, pos, inside);
    if (!ok) {
        DEBUG_MSG("parseOptional: optional content not found, creating empty node");
        pos = savedPos;
        ASTNode* node = new ASTNode("<opt>");
        node->matched = "";
        outNode = node;
        return true;
    }
    
    DEBUG_MSG("parseOptional: optional content matched");
    ASTNode* node = new ASTNode("<opt>");
    if (inside) {
        node->children.push_back(inside);
        node->matched = inside->matched;
    }
    outNode = node;
    return true;
}

// Parse repetition expressions (zero or more occurrences)
bool BNFParser::parseRepeat(Expression* expr,
                           const std::string& input,
                           size_t& pos,
                           ASTNode*& outNode) const
{
    DEBUG_MSG("parseRepeat: starting repetition at pos=" << pos);

    std::vector<ASTNode*> items;
    std::string matchedAccum;
    int iterations = 0;
    
    while (true) {
        size_t iterSaved = pos;
        ASTNode* it = 0;
        bool ok = parseExpression(expr->children[0], input, pos, it);
        if (!ok) {
            pos = iterSaved;
            break;
        }
        if (it && it->matched.empty()) {
            delete it;
            pos = iterSaved;
            break;
        }
        if (it) {
            matchedAccum += it->matched;
            items.push_back(it);
            iterations++;
            DEBUG_MSG("parseRepeat: iteration " << iterations << " matched");
        } else {
            break;
        }
        if (pos >= input.size()) break;
    }

    DEBUG_MSG("parseRepeat: completed with " << iterations << " iterations");
    ASTNode* parent = new ASTNode("<rep>");
    parent->matched = matchedAccum;
    for (size_t i = 0; i < items.size(); ++i)
        parent->children.push_back(items[i]);
    outNode = parent;
    return true;
}

// Parse character range expressions - match one character within the range
bool BNFParser::parseCharRange(Expression* expr,
                               const std::string& input,
                               size_t& pos,
                               ASTNode*& outNode) const
{
    if (pos >= input.size()) {
        DEBUG_MSG("parseCharRange: reached end of input");
        return false;
    }
    
    unsigned char ch = static_cast<unsigned char>(input[pos]);
    unsigned char start = expr->charRange.start;
    unsigned char end = expr->charRange.end;
    
    DEBUG_MSG("parseCharRange: checking if " << (int)ch << " is in range [" 
              << (int)start << ", " << (int)end << "]");
    
    if (ch >= start && ch <= end) {
        DEBUG_MSG("parseCharRange: matched character " << (int)ch);
        ASTNode* node = new ASTNode("<char-range>");
        node->matched = std::string(1, ch);
        pos++;
        outNode = node;
        return true;
    }
    
    DEBUG_MSG("parseCharRange: character " << (int)ch << " not in range");
    return false;
}

// Parse character class expressions - match one character against the class
bool BNFParser::parseCharClass(Expression* expr,
                               const std::string& input,
                               size_t& pos,
                               ASTNode*& outNode) const
{
    if (pos >= input.size()) {
        DEBUG_MSG("parseCharClass: reached end of input");
        return false;
    }
    
    unsigned char ch = static_cast<unsigned char>(input[pos]);
    bool match = expr->classMatches(ch);
    
    if (match) {
        DEBUG_MSG("parseCharClass: matched character " << (int)ch);
        ASTNode* node = new ASTNode("<char-class>");
        node->matched = std::string(1, ch);
        pos++;
        outNode = node;
        return true;
    }
    
    DEBUG_MSG("parseCharClass: character " << (int)ch << " did not match class");
    return false;
}
