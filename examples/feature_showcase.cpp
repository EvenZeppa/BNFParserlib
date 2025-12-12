#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "Grammar.hpp"
#include "BNFParser.hpp"

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

    return 0;
}
