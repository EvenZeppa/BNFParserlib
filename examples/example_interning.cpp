/**
 * Example: Expression Interning
 * 
 * Demonstrates expression interning - deduplicating identical expression trees.
 * When multiple rules use the same sub-expression, the interner ensures only
 * one copy exists in memory, sharing the same pointer.
 * 
 * Benefits:
 *   - Reduced memory usage
 *   - Faster equality checks (pointer comparison)
 *   - Improved cache locality
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"
#include "Arena.hpp"
#include "ExpressionInterner.hpp"

int main() {
    std::cout << "=== Expression Interning Example ===" << std::endl;

    // Create arena and interner
    Arena arena(2048);
    ExpressionInterner interner;
    
    // Create grammar and attach both arena and interner
    Grammar grammar;
    grammar.setArena(&arena);
    grammar.setInterner(&interner);
    
    std::cout << "Enabled expression interning on grammar" << std::endl;

    // Define base rules
    grammar.addRule("<digit> ::= '0' ... '9'");
    grammar.addRule("<hex-digit> ::= <digit> | 'a' ... 'f' | 'A' ... 'F'");
    
    // Define two rules with identical sub-expressions
    // Both use the same pattern: <hex-digit> <hex-digit>
    grammar.addRule("<octet> ::= <hex-digit> <hex-digit>");
    grammar.addRule("<octet-copy> ::= <hex-digit> <hex-digit>");
    
    std::cout << "Created two rules with identical sub-expressions:" << std::endl;
    std::cout << "  <octet>      ::= <hex-digit> <hex-digit>" << std::endl;
    std::cout << "  <octet-copy> ::= <hex-digit> <hex-digit>" << std::endl;

    // Retrieve the root expressions
    Expression* octet = grammar.getRule("<octet>")->rootExpr;
    Expression* octetCopy = grammar.getRule("<octet-copy>")->rootExpr;

    // Check if they point to the same memory location
    if (octet == octetCopy) {
        std::cout << "\n✓ SUCCESS: Both rules share the same expression object!" << std::endl;
        std::cout << "  <octet>      address: " << static_cast<void*>(octet) << std::endl;
        std::cout << "  <octet-copy> address: " << static_cast<void*>(octetCopy) << std::endl;
        std::cout << "  The interner deduplicated the identical expression trees." << std::endl;
    } else {
        std::cout << "\n✗ Rules have different expression objects (interning may not be working)" << std::endl;
    }

    // Define composite rules that reuse the octet pattern
    grammar.addRule("<color-rgb> ::= '#' <octet> <octet> <octet>");
    grammar.addRule("<color-rgba> ::= '#' <octet> <octet> <octet> <octet>");
    
    std::cout << "\nCreated composite rules reusing <octet>" << std::endl;

    // Create parser and test
    BNFParser parser(grammar);

    {
        std::string input = "#1a2b3c";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<color-rgb>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed RGB color: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse RGB color" << std::endl;
        }
    }

    {
        std::string input = "#1a2b3cff";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<color-rgba>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed RGBA color: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse RGBA color" << std::endl;
        }
    }

    std::cout << "\nExpression interning benefits:" << std::endl;
    std::cout << "  - Reduces memory by sharing identical sub-expressions" << std::endl;
    std::cout << "  - Enables O(1) equality checks via pointer comparison" << std::endl;
    std::cout << "  - Improves cache efficiency" << std::endl;

    return 0;
}
