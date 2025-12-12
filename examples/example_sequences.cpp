/**
 * Example: Sequences, Repetition, Optional, and Alternation
 * 
 * Demonstrates the core BNF composition operators:
 *   - Sequences: <a> <b> matches a followed by b
 *   - Repetition: { <a> } matches zero or more occurrences
 *   - Optional: [ <a> ] matches zero or one occurrence
 *   - Alternation: <a> | <b> matches either a or b
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"

int main() {
    std::cout << "=== Sequences, Repetition, Optional, Alternation Example ===" << std::endl;

    Grammar grammar;

    // Basic building blocks
    grammar.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
    grammar.addRule("<digit> ::= '0' ... '9'");

    // Sequence + Repetition: word starts with letter, followed by zero or more letters/digits
    grammar.addRule("<identifier> ::= <letter> { <letter> | <digit> }");

    // Optional: integer can have an optional sign
    grammar.addRule("<sign> ::= '+' | '-'");
    grammar.addRule("<integer> ::= [ <sign> ] <digit> { <digit> }");

    // Alternation: choose between different formats
    grammar.addRule("<hex-digit> ::= ( '0' ... '9' 'a' ... 'f' 'A' ... 'F' )");
    grammar.addRule("<hex-number> ::= '0' 'x' <hex-digit> { <hex-digit> }");
    grammar.addRule("<number> ::= <hex-number> | <integer>");

    BNFParser parser(grammar);

    // Test identifier (letter followed by letters/digits)
    {
        std::string input = "variable123";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<identifier>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed identifier: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse identifier" << std::endl;
        }
    }

    // Test identifier with just one letter
    {
        std::string input = "x";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<identifier>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed single-letter identifier: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse single-letter identifier" << std::endl;
        }
    }

    // Test integer with positive sign (optional)
    {
        std::string input = "+42";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<integer>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed signed integer: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse signed integer" << std::endl;
        }
    }

    // Test integer without sign (optional omitted)
    {
        std::string input = "99";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<integer>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed unsigned integer: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse unsigned integer" << std::endl;
        }
    }

    // Test integer with negative sign
    {
        std::string input = "-7";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<integer>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed negative integer: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse negative integer" << std::endl;
        }
    }

    // Test alternation: hex-number chosen over integer
    {
        std::string input = "0xDEAD";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<number>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed hex number (alternation): '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse hex number" << std::endl;
        }
    }

    // Test alternation: integer chosen when no hex prefix
    {
        std::string input = "1234";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<number>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed integer (alternation fallback): '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse integer" << std::endl;
        }
    }

    // Test repetition with zero occurrences (just the letter)
    {
        std::string input = "a";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<identifier>", input, consumed);
        
        if (ast && consumed == 1) {
            std::cout << "✓ Repetition allows zero occurrences: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed with zero repetitions" << std::endl;
        }
    }

    std::cout << "\nSequences, repetition, optional, and alternation enable complex patterns!" << std::endl;
    return 0;
}
