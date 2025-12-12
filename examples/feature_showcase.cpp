#include <cassert>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "Grammar.hpp"
#include "BNFParser.hpp"
#include "Arena.hpp"
#include "ExpressionInterner.hpp"

// Simple assertion helpers used by all phases
static void expectMatch(const std::string& title,
                        BNFParser& parser,
                        const std::string& rule,
                        const std::string& input,
                        const std::string& expectedMatched) {
    size_t consumed = 0;
    ASTNode* ast = parser.parse(rule, input, consumed);
    assert(ast && "Parser returned null AST");
    assert(consumed == expectedMatched.size());
    assert(ast->matched == expectedMatched);
    std::cout << "  [ok] " << title << " => '" << ast->matched << "'" << std::endl;
    delete ast;
}

static void expectFail(const std::string& title,
                       BNFParser& parser,
                       const std::string& rule,
                       const std::string& input) {
    size_t consumed = 0;
    ASTNode* ast = parser.parse(rule, input, consumed);
    assert(ast == 0);
    std::cout << "  [fail as expected] " << title << " (consumed=" << consumed << ")" << std::endl;
}

// FIRST-set helper used for the pruning demonstration (mirrors library logic)
struct DemoFirstInfo {
    std::bitset<256> chars;
    bool nullable;
    DemoFirstInfo() : nullable(false) {}
};

static void mergeFirst(DemoFirstInfo& dst, const DemoFirstInfo& src) {
    dst.chars |= src.chars;
    dst.nullable = dst.nullable || src.nullable;
}

static const DemoFirstInfo& computeFirstDemo(Expression* expr,
                                             Grammar& grammar,
                                             std::map<Expression*, DemoFirstInfo>& cache) {
    std::map<Expression*, DemoFirstInfo>::iterator it = cache.find(expr);
    if (it != cache.end()) return it->second;

    DemoFirstInfo fi;
    switch (expr->type) {
        case Expression::EXPR_TERMINAL: {
            std::string lit = expr->value;
            if (lit.size() >= 2 && (lit[0] == '\'' || lit[0] == '"')) {
                lit = lit.substr(1, lit.size() - 2);
            }
            if (!lit.empty()) fi.chars.set(static_cast<unsigned char>(lit[0]));
            else fi.nullable = true;
            break;
        }
        case Expression::EXPR_SYMBOL: {
            Rule* r = grammar.getRule(expr->value);
            if (r && r->rootExpr) {
                fi = computeFirstDemo(r->rootExpr, grammar, cache);
            }
            break;
        }
        case Expression::EXPR_SEQUENCE: {
            fi.nullable = true;
            for (size_t i = 0; i < expr->children.size(); ++i) {
                const DemoFirstInfo& child = computeFirstDemo(expr->children[i], grammar, cache);
                mergeFirst(fi, child);
                if (!child.nullable) {
                    fi.nullable = false;
                    break;
                }
            }
            break;
        }
        case Expression::EXPR_ALTERNATIVE: {
            for (size_t i = 0; i < expr->children.size(); ++i) {
                const DemoFirstInfo& child = computeFirstDemo(expr->children[i], grammar, cache);
                mergeFirst(fi, child);
            }
            break;
        }
        case Expression::EXPR_OPTIONAL: {
            fi.nullable = true;
            if (!expr->children.empty()) {
                const DemoFirstInfo& child = computeFirstDemo(expr->children[0], grammar, cache);
                mergeFirst(fi, child);
            }
            break;
        }
        case Expression::EXPR_REPEAT: {
            fi.nullable = true;
            if (!expr->children.empty()) {
                const DemoFirstInfo& child = computeFirstDemo(expr->children[0], grammar, cache);
                mergeFirst(fi, child);
            }
            break;
        }
        case Expression::EXPR_CHAR_RANGE: {
            unsigned char start = expr->charRange.start;
            unsigned char end = expr->charRange.end;
            for (unsigned int c = start; c <= end; ++c) {
                fi.chars.set(static_cast<unsigned char>(c));
                if (c == 255) break;
            }
            fi.nullable = false;
            break;
        }
        case Expression::EXPR_CHAR_CLASS: {
            for (size_t i = 0; i < 256; ++i) {
                if (expr->classMatches(static_cast<unsigned char>(i))) fi.chars.set(i);
            }
            fi.nullable = false;
            break;
        }
        default:
            break;
    }

    return cache.insert(std::make_pair(expr, fi)).first->second;
}

static std::string renderChar(unsigned char c) {
    if (c >= 32 && c <= 126) {
        char buf[4];
        buf[0] = '\'';
        buf[1] = static_cast<char>(c);
        buf[2] = '\'';
        buf[3] = '\0';
        return std::string(buf);
    }
    std::ostringstream ss;
    ss << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
       << static_cast<int>(c);
    return ss.str();
}

static std::string renderFirstSet(const DemoFirstInfo& fi) {
    std::ostringstream ss;
    size_t printed = 0;
    for (size_t i = 0; i < 256; ++i) {
        if (!fi.chars.test(i)) continue;
        if (printed > 0) ss << ", ";
        ss << renderChar(static_cast<unsigned char>(i));
        printed++;
        if (printed >= 12 && fi.chars.count() > printed) {
            ss << ", ...";
            break;
        }
    }
    if (printed == 0) ss << "(empty)";
    if (fi.nullable) ss << " (nullable)";
    return ss.str();
}


// Phase 3: arena-backed allocation and expression interning
static void phaseArenaAndInterner() {
    std::cout << "\n=== Phase 3: Arena and Interner ===" << std::endl;

    Arena arena(2048);
    ExpressionInterner interner;
    Grammar g;
    g.setArena(&arena);
    g.setInterner(&interner);

    g.addRule("<digit> ::= '0' ... '9'");
    g.addRule("<hex-digit> ::= <digit> | 'a' ... 'f' | 'A' ... 'F'");
    g.addRule("<octet> ::= <hex-digit> <hex-digit>");
    g.addRule("<octet-copy> ::= <hex-digit> <hex-digit>");

    // Two color formats share the same sub-expressions; the interner deduplicates them
    g.addRule("<color-long> ::= '#' <octet> <octet> <octet>");
    g.addRule("<color-short> ::= '#' <hex-digit> <hex-digit> <hex-digit>");
    g.addRule("<color> ::= <color-long> | <color-short>");

    BNFParser parser(g);

    expectMatch("hex color (long)", parser, "<color>", "#1a2b3c", "#1a2b3c");
    expectMatch("hex color (short)", parser, "<color>", "#abc", "#abc");

    Expression* oct = g.getRule("<octet>")->rootExpr;
    Expression* octCopy = g.getRule("<octet-copy>")->rootExpr;
    assert(oct == octCopy && "Interner should reuse identical expression trees");
    std::cout << "  [info] interner reused <octet> tree (pointer equality verified)" << std::endl;

    std::cout << "Phase 3 complete and testable." << std::endl;
}

// Phase 4: FIRST-set memoization to prune alternatives and reduce backtracking
static void phaseFirstSetPruning() {
    std::cout << "\n=== Phase 4: FIRST-set Pruning ===" << std::endl;
    Grammar g;

    g.addRule("<space> ::= ' ' { ' ' }");
    g.addRule("<path-char> ::= ( 'a' ... 'z' 'A' ... 'Z' '0' ... '9' '/' '.' '_' '-' )");
    g.addRule("<path> ::= '/' <path-char> { <path-char> }");

    g.addRule("<command-get> ::= 'GET' <space> <path>");
    g.addRule("<command-post> ::= 'POST' <space> <path>");
    g.addRule("<command-ping> ::= 'PING'");
    g.addRule("<command-delete> ::= 'DELETE' <space> <path>");
    g.addRule("<request> ::= <command-get> | <command-post> | <command-ping> | <command-delete>");

    std::map<Expression*, DemoFirstInfo> cache;
    Rule* req = g.getRule("<request>");
    assert(req && req->rootExpr && "Request rule should exist");
    const DemoFirstInfo& firstRequest = computeFirstDemo(req->rootExpr, g, cache);

    std::cout << "  FIRST(<request>) = " << renderFirstSet(firstRequest) << std::endl;
    if (req->rootExpr->type == Expression::EXPR_ALTERNATIVE) {
        for (size_t i = 0; i < req->rootExpr->children.size(); ++i) {
            const DemoFirstInfo& fi = computeFirstDemo(req->rootExpr->children[i], g, cache);
            std::cout << "    alt[" << i << "] FIRST = " << renderFirstSet(fi) << std::endl;
        }
    }

    BNFParser parser(g);
    expectMatch("GET request", parser, "<request>", "GET /index.html", "GET /index.html");
    expectMatch("POST request", parser, "<request>", "POST /submit", "POST /submit");
    expectMatch("PING request", parser, "<request>", "PING", "PING");
    expectFail("FIRST pruning rejects TRACE", parser, "<request>", "TRACE /bad");

    std::cout << "Phase 4 complete and testable." << std::endl;
}

static void runIrcNickScenario() {
    Grammar g;
    g.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
    g.addRule("<digit> ::= '0' ... '9'");
    g.addRule("<nick-char> ::= <letter> | <digit> | '_' | '-' | '[' | ']' | '\\\\'");
    g.addRule("<nick> ::= <letter> { <nick-char> }");

    BNFParser parser(g);
    expectMatch("IRC nick valid", parser, "<nick>", "alice_42", "alice_42");
    expectFail("IRC nick cannot start with digit", parser, "<nick>", "9lives");
}

static void runHexLiteralScenario() {
    Grammar g;
    g.addRule("<hex-digit> ::= ( '0' ... '9' 'a' ... 'f' 'A' ... 'F' )");
    g.addRule("<hex-prefix> ::= '0' 'x' | '0' 'X'");
    g.addRule("<hex-number> ::= <hex-prefix> <hex-digit> { <hex-digit> }");

    BNFParser parser(g);
    expectMatch("hex literal uppercase", parser, "<hex-number>", "0xDEADBEEF", "0xDEADBEEF");
    expectMatch("hex literal lowercase", parser, "<hex-number>", "0Xc0ffee", "0Xc0ffee");
    expectFail("hex literal requires at least one digit", parser, "<hex-number>", "0x");
}

static void runPrintableWordScenario() {
    Grammar g;
    g.addRule("<printable> ::= ( 0x21 ... 0x7E )");
    g.addRule("<printable-word> ::= <printable> { <printable> }");

    BNFParser parser(g);
    expectMatch("printable ASCII word", parser, "<printable-word>", "Hello-World_123", "Hello-World_123");
    expectFail("rejects control characters", parser, "<printable-word>", std::string("hi\n"));
}

static void runMiniProtocolScenario() {
    Grammar g;
    g.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
    g.addRule("<digit> ::= '0' ... '9'");
    g.addRule("<nick-char> ::= <letter> | <digit> | '_' | '-' ");
    g.addRule("<nick> ::= <letter> { <nick-char> }");
    g.addRule("<space> ::= ' ' { ' ' }");
    g.addRule("<printable> ::= ( 0x20 ... 0x7E )");
    g.addRule("<text> ::= <printable> { <printable> }");
    g.addRule("<message> ::= 'MSG' <space> <nick> <space> ':' <text> '\\r\\n'");

    BNFParser parser(g);
    expectMatch("mini protocol message", parser, "<message>", "MSG alice :hello there\r\n", "MSG alice :hello there\r\n");
    expectFail("mini protocol invalid nick", parser, "<message>", "MSG 9bad :oops\r\n");
}

// Phase 5: complex real-world-like scenarios combining all features
static void phaseComplexScenarios() {
    std::cout << "\n=== Phase 5: Complex Scenarios ===" << std::endl;
    runIrcNickScenario();
    runHexLiteralScenario();
    runPrintableWordScenario();
    runMiniProtocolScenario();
    std::cout << "Phase 5 complete and testable." << std::endl;
}

// Phase 2: sequences, repetition, optional elements, and alternation
static void phaseSequencesAlternation() {
    std::cout << "\n=== Phase 2: Sequences, Repetition, Alternation ===" << std::endl;
    Grammar g;

    g.addRule("<lower> ::= 'a' ... 'z'");
    g.addRule("<upper> ::= 'A' ... 'Z'");
    g.addRule("<letter> ::= <lower> | <upper>");
    g.addRule("<digit> ::= '0' ... '9'");

    // Repetition and sequencing for identifiers
    g.addRule("<word> ::= <letter> { <letter> | <digit> }");

    // Optional sign before integers
    g.addRule("<maybe-sign> ::= [ '+' | '-' ]");
    g.addRule("<integer> ::= <maybe-sign> <digit> { <digit> }");

    // Alternation inside a character class for hexadecimal digits
    g.addRule("<hex-digit> ::= ( '0' ... '9' 'a' ... 'f' 'A' ... 'F' )");
    g.addRule("<hex-number> ::= '0' 'x' <hex-digit> { <hex-digit> }");

    // Alternatives that overlap to demonstrate selection
    g.addRule("<identifier-or-int> ::= <word> | <integer>");
    g.addRule("<maybe-hex> ::= <hex-number> | <integer>");

    BNFParser parser(g);

    expectMatch("word with trailing digits", parser, "<word>", "abc123", "abc123");
    expectMatch("positive integer", parser, "<integer>", "+42", "+42");
    expectMatch("negative integer", parser, "<integer>", "-7", "-7");
    expectMatch("hexadecimal number", parser, "<hex-number>", "0x1aF", "0x1aF");
    expectMatch("identifier chosen over int", parser, "<identifier-or-int>", "alpha1", "alpha1");
    expectMatch("integer chosen over identifier", parser, "<identifier-or-int>", "123", "123");
    expectMatch("maybe-hex picks hex", parser, "<maybe-hex>", "0xBEEF", "0xBEEF");
    expectFail("hex requires prefix", parser, "<hex-number>", "1234");

    std::cout << "Phase 2 complete and testable." << std::endl;
}

// Phase 1: character ranges and inclusive/exclusive character classes
static void phaseRangesAndClasses() {
    std::cout << "\n=== Phase 1: Ranges and Classes ===" << std::endl;
    Grammar g;

    // Character ranges using ellipsis and hex literals
    g.addRule("<lower> ::= 'a' ... 'z'");
    g.addRule("<digit> ::= '0' ... '9'");
    g.addRule("<ascii> ::= 0x00 ... 0x7F");

    // Inclusive and exclusive classes
    g.addRule("<vowel> ::= ( 'a' 'e' 'i' 'o' 'u' )");
    g.addRule("<consonant> ::= ( ^ 'a' 'e' 'i' 'o' 'u' )");
    g.addRule("<token> ::= <lower> <digit>");

    BNFParser parser(g);

    expectMatch("lowercase range", parser, "<lower>", "m", "m");
    expectMatch("digit range", parser, "<digit>", "5", "5");
    expectMatch("ascii full range", parser, "<ascii>", std::string(1, '\x7F'), std::string(1, '\x7F'));
    expectMatch("inclusive class (vowel)", parser, "<vowel>", "i", "i");
    expectMatch("exclusive class (consonant)", parser, "<consonant>", "b", "b");
    expectFail("exclusive class rejects vowel", parser, "<consonant>", "a");
    expectMatch("range sequencing", parser, "<token>", "a7", "a7");

    std::cout << "Phase 1 complete and testable." << std::endl;
}

int main() {
    std::cout << "BNFParserLib Feature Showcase" << std::endl;
    std::cout << "==============================" << std::endl;

    phaseRangesAndClasses();
    phaseSequencesAlternation();
    phaseArenaAndInterner();
    phaseFirstSetPruning();
    phaseComplexScenarios();

    return 0;
}
