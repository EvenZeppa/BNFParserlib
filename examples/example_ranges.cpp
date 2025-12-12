/**
 * Example: Character Ranges
 * 
 * Demonstrates how to use character range syntax in BNF rules:
 *   - 'a' ... 'z' for lowercase letters
 *   - '0' ... '9' for digits
 *   - Hex notation like 0x00 ... 0x7F for broader ASCII ranges
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"

int main() {
    std::cout << "=== Character Ranges Example ===" << std::endl;

    // Create a grammar with character range rules
    Grammar grammar;
    
    // Define ranges using the ellipsis notation '...'
    grammar.addRule("<lowercase> ::= 'a' ... 'z'");
    grammar.addRule("<uppercase> ::= 'A' ... 'Z'");
    grammar.addRule("<digit> ::= '0' ... '9'");
    
    // Hex notation for broader ranges (full ASCII)
    grammar.addRule("<ascii-char> ::= 0x00 ... 0x7F");
    
    // Combine ranges: a simple alphanumeric token
    grammar.addRule("<alphanumeric> ::= <lowercase> | <uppercase> | <digit>");

    // Create parser from the grammar
    BNFParser parser(grammar);

    // Test lowercase range
    {
        std::string input = "m";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<lowercase>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed lowercase letter: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse lowercase" << std::endl;
        }
    }

    // Test digit range
    {
        std::string input = "7";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<digit>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed digit: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse digit" << std::endl;
        }
    }

    // Test alphanumeric (tries multiple alternatives)
    {
        std::string input = "Z";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<alphanumeric>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed alphanumeric: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse alphanumeric" << std::endl;
        }
    }

    // Test ASCII range with a control character
    {
        std::string input;
        input.push_back('\x1F'); // Unit separator (control char)
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<ascii-char>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed ASCII control character (0x1F)" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse ASCII character" << std::endl;
        }
    }

    // Test rejection: lowercase should not match uppercase
    {
        std::string input = "M";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<lowercase>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly matched uppercase as lowercase" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected uppercase for <lowercase> rule" << std::endl;
        }
    }

    std::cout << "\nCharacter ranges allow concise specification of char sets!" << std::endl;
    return 0;
}
