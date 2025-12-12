/**
 * Example: Hexadecimal Literal Parsing
 * 
 * Real-world example: parsing hexadecimal numbers as found in programming
 * languages like C, C++, JavaScript, etc.
 * 
 * Hex literal format:
 *   - Starts with "0x" or "0X" prefix
 *   - Followed by one or more hex digits (0-9, a-f, A-F)
 *   - Examples: 0xFF, 0xdeadbeef, 0X1A2B
 * 
 * Demonstrates character classes combining multiple ranges and case-insensitivity.
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"

int main() {
    std::cout << "=== Hexadecimal Literal Parsing Example ===" << std::endl;

    Grammar grammar;

    // Hex digit: 0-9, a-f, A-F (case insensitive)
    grammar.addRule("<hex-digit> ::= ( '0' ... '9' 'a' ... 'f' 'A' ... 'F' )");
    
    // Hex prefix: either 0x or 0X
    grammar.addRule("<hex-prefix> ::= '0' 'x' | '0' 'X'");
    
    // Hex literal: prefix followed by one or more hex digits
    grammar.addRule("<hex-literal> ::= <hex-prefix> <hex-digit> { <hex-digit> }");

    std::cout << "\nHexadecimal Literal Format:" << std::endl;
    std::cout << "  - Prefix: 0x or 0X" << std::endl;
    std::cout << "  - Digits: 0-9, a-f, A-F (case insensitive)" << std::endl;
    std::cout << "  - At least one digit required after prefix" << std::endl;
    std::cout << std::endl;

    BNFParser parser(grammar);

    // Valid hex literals
    {
        std::string input = "0xFF";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid hex literal: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid hex literal" << std::endl;
        }
    }

    {
        std::string input = "0xDEADBEEF";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid hex literal: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid hex literal" << std::endl;
        }
    }

    {
        std::string input = "0Xc0ffee";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid hex literal (uppercase X): '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid hex literal" << std::endl;
        }
    }

    {
        std::string input = "0x0";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid hex literal (single digit): '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse single-digit hex" << std::endl;
        }
    }

    {
        std::string input = "0x1A2B3C4D";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid hex literal (mixed case): '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse mixed-case hex" << std::endl;
        }
    }

    // Invalid: no digits after prefix
    {
        std::string input = "0x";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly accepted hex literal without digits" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected: no digits after prefix ('0x')" << std::endl;
        }
    }

    // Invalid: contains non-hex digit
    {
        std::string input = "0xG00D";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast && consumed == 7) {
            std::cout << "✗ Incorrectly accepted invalid hex digit 'G'" << std::endl;
            delete ast;
        } else if (ast) {
            std::cout << "✓ Parsed valid prefix, stopped at 'G': '" << ast->matched << "'" << std::endl;
            std::cout << "  (consumed " << consumed << " chars)" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected invalid hex digit" << std::endl;
        }
    }

    // Invalid: missing prefix
    {
        std::string input = "ABCD";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly accepted hex without prefix" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected: missing 0x prefix ('ABCD')" << std::endl;
        }
    }

    // Invalid: wrong prefix
    {
        std::string input = "0b1010";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-literal>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly accepted binary prefix as hex" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected: wrong prefix ('0b')" << std::endl;
        }
    }

    std::cout << "\nThis example demonstrates parsing numeric literals with prefix requirements!" << std::endl;

    return 0;
}
