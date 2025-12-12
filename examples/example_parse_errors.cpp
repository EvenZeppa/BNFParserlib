/**
 * Example: Error Reporting with ParseContext
 * 
 * Demonstrates the comprehensive error reporting capabilities
 * of the ParseContext API.
 * 
 * When parsing fails, ParseContext provides:
 *   - errorPos: the furthest position reached before failure
 *   - expected: description of what was expected at that position
 * 
 * This "furthest failure" tracking helps pinpoint exactly where
 * and why the input doesn't match the grammar.
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"
#include "ParseContext.hpp"

void showError(const std::string& input, const ParseContext& ctx) {
    std::cout << "  Input: '" << input << "'" << std::endl;
    std::cout << "  Error at position " << ctx.errorPos;
    
    if (ctx.errorPos < input.size()) {
        std::cout << " (character: '" << input[ctx.errorPos] << "')";
    } else {
        std::cout << " (end of input)";
    }
    std::cout << std::endl;
    
    std::cout << "  Expected: " << ctx.expected << std::endl;
    
    // Visual pointer to error position
    std::cout << "  " << input << std::endl;
    std::cout << "  " << std::string(ctx.errorPos, ' ') << "^" << std::endl;
}

int main() {
    std::cout << "=== ParseContext Error Reporting Examples ===" << std::endl;
    std::cout << std::endl;

    // Example 1: Terminal mismatch
    {
        std::cout << "Example 1: Terminal Mismatch" << std::endl;
        std::cout << "----------------------------" << std::endl;
        
        Grammar g;
        g.addRule("<greeting> ::= 'hello' ' ' 'world'");
        
        BNFParser parser(g);
        ParseContext ctx;
        
        parser.parse("<greeting>", "hello universe", ctx);
        
        if (!ctx.success) {
            std::cout << "✗ Parsing FAILED (as expected)" << std::endl;
            showError("hello universe", ctx);
        }
        std::cout << std::endl;
    }

    // Example 2: Character range violation
    {
        std::cout << "Example 2: Character Range Violation" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        
        Grammar g;
        g.addRule("<digit> ::= '0' ... '9'");
        g.addRule("<number> ::= <digit> { <digit> }");
        
        BNFParser parser(g);
        ParseContext ctx;
        
        parser.parse("<number>", "abc", ctx);
        
        if (!ctx.success) {
            std::cout << "✗ Parsing FAILED (as expected)" << std::endl;
            showError("abc", ctx);
            std::cout << "  Note: Expected digit, got letter" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 3: Character class mismatch
    {
        std::cout << "Example 3: Character Class Mismatch" << std::endl;
        std::cout << "---------------------------------" << std::endl;
        
        Grammar g;
        g.addRule("<letter> ::= ( 'a' ... 'z' 'A' ... 'Z' )");
        g.addRule("<word> ::= <letter> { <letter> }");
        
        BNFParser parser(g);
        ParseContext ctx;
        
        parser.parse("<word>", "123Hello", ctx);
        
        if (!ctx.success) {
            std::cout << "✗ Parsing FAILED (as expected)" << std::endl;
            showError("123Hello", ctx);
            std::cout << "  Note: Word must start with a letter" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 4: Sequence incomplete
    {
        std::cout << "Example 4: Incomplete Sequence" << std::endl;
        std::cout << "-----------------------------" << std::endl;
        
        Grammar g;
        g.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
        g.addRule("<digit> ::= '0' ... '9'");
        g.addRule("<id> ::= <letter> <letter> <digit>");
        
        BNFParser parser(g);
        ParseContext ctx;
        
        parser.parse("<id>", "ab", ctx);
        
        if (!ctx.success) {
            std::cout << "✗ Parsing FAILED (as expected)" << std::endl;
            showError("ab", ctx);
            std::cout << "  Note: Sequence requires 2 letters + 1 digit" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 5: Alternative exhaustion
    {
        std::cout << "Example 5: No Alternative Matched" << std::endl;
        std::cout << "--------------------------------" << std::endl;
        
        Grammar g;
        g.addRule("<vowel> ::= 'a' | 'e' | 'i' | 'o' | 'u'");
        
        BNFParser parser(g);
        ParseContext ctx;
        
        parser.parse("<vowel>", "x", ctx);
        
        if (!ctx.success) {
            std::cout << "✗ Parsing FAILED (as expected)" << std::endl;
            showError("x", ctx);
            std::cout << "  Note: Parser tried all alternatives" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 6: Empty input
    {
        std::cout << "Example 6: Unexpected End of Input" << std::endl;
        std::cout << "--------------------------------" << std::endl;
        
        Grammar g;
        g.addRule("<digit> ::= '0' ... '9'");
        g.addRule("<number> ::= <digit> <digit> <digit>");
        
        BNFParser parser(g);
        ParseContext ctx;
        
        parser.parse("<number>", "12", ctx);
        
        if (!ctx.success) {
            std::cout << "✗ Parsing FAILED (as expected)" << std::endl;
            showError("12", ctx);
            std::cout << "  Note: Sequence needs 3 digits, got only 2" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 7: Complex expression failure
    {
        std::cout << "Example 7: Complex Expression Failure" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        
        Grammar g;
        g.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
        g.addRule("<digit> ::= '0' ... '9'");
        g.addRule("<special> ::= '_' | '-'");
        g.addRule("<id-char> ::= <letter> | <digit> | <special>");
        g.addRule("<identifier> ::= <letter> { <id-char> }");
        
        BNFParser parser(g);
        ParseContext ctx;
        
        parser.parse("<identifier>", "@invalid", ctx);
        
        if (!ctx.success) {
            std::cout << "✗ Parsing FAILED (as expected)" << std::endl;
            showError("@invalid", ctx);
            std::cout << "  Note: Identifier must start with a letter" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 8: Must start with specific character
    {
        std::cout << "Example 8: Must Start With Letter" << std::endl;
        std::cout << "--------------------------------" << std::endl;
        
        Grammar g;
        g.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
        g.addRule("<digit> ::= '0' ... '9'");
        g.addRule("<identifier> ::= <letter> { <letter> | <digit> }");
        
        BNFParser parser(g);
        ParseContext ctx;
        
        parser.parse("<identifier>", "123abc", ctx);
        
        if (!ctx.success) {
            std::cout << "✗ Parsing FAILED (as expected)" << std::endl;
            showError("123abc", ctx);
            std::cout << "  Note: Identifiers must start with a letter" << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "===========================================\n";
    std::cout << "ParseContext error reporting provides:\n";
    std::cout << "  - Precise error location (furthest failure)\n";
    std::cout << "  - Clear description of expected input\n";
    std::cout << "  - Easy debugging of parse failures\n";
    std::cout << "  - No need for separate parseWithErrors!\n";

    return 0;
}
