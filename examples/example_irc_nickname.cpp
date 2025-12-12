/**
 * Example: IRC Nickname Validation
 * 
 * Real-world example: parsing IRC nicknames according to RFC rules.
 * 
 * IRC nickname rules:
 *   - Must start with a letter (a-z, A-Z)
 *   - Can contain letters, digits, and special characters: _ - [ ] \
 *   - Typically 1-9 characters (simplified here)
 * 
 * This demonstrates a practical application combining character ranges,
 * classes, sequences, and repetition.
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"

int main() {
    std::cout << "=== IRC Nickname Validation Example ===" << std::endl;

    Grammar grammar;

    // Define character sets
    grammar.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
    grammar.addRule("<digit> ::= '0' ... '9'");
    
    // Special characters allowed in nicknames (after first char)
    grammar.addRule("<special> ::= '_' | '-' | '[' | ']' | '\\\\'");
    
    // Nick characters: letters, digits, or special chars
    grammar.addRule("<nick-char> ::= <letter> | <digit> | <special>");
    
    // Nickname: must start with letter, followed by zero or more nick-chars
    grammar.addRule("<nickname> ::= <letter> { <nick-char> }");

    std::cout << "\nIRC Nickname Rules:" << std::endl;
    std::cout << "  - Must start with a letter" << std::endl;
    std::cout << "  - Can contain: letters, digits, _ - [ ] \\" << std::endl;
    std::cout << std::endl;

    BNFParser parser(grammar);

    // Valid nicknames
    {
        std::string input = "alice";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<nickname>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid nickname: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid nickname" << std::endl;
        }
    }

    {
        std::string input = "Bob_42";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<nickname>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid nickname: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid nickname" << std::endl;
        }
    }

    {
        std::string input = "user[away]";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<nickname>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid nickname: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid nickname" << std::endl;
        }
    }

    {
        std::string input = "CoolUser-123";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<nickname>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid nickname: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid nickname" << std::endl;
        }
    }

    // Single letter is valid
    {
        std::string input = "X";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<nickname>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid single-letter nickname: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse single-letter nickname" << std::endl;
        }
    }

    // Invalid: starts with digit
    {
        std::string input = "9lives";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<nickname>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly accepted nickname starting with digit" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected: cannot start with digit ('9lives')" << std::endl;
        }
    }

    // Invalid: starts with special character
    {
        std::string input = "_bob";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<nickname>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly accepted nickname starting with underscore" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected: cannot start with special char ('_bob')" << std::endl;
        }
    }

    // Invalid: contains disallowed character (space)
    {
        std::string input = "bad nick";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<nickname>", input, consumed);
        
        if (ast && consumed == 8) {
            std::cout << "✗ Incorrectly accepted nickname with space" << std::endl;
            delete ast;
        } else if (ast) {
            std::cout << "✓ Correctly parsed only valid prefix: '" << ast->matched << "'" << std::endl;
            std::cout << "  (stopped at space, consumed " << consumed << " chars)" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected nickname with space" << std::endl;
        }
    }

    std::cout << "\nThis example shows practical validation of protocol-specific identifiers!" << std::endl;

    return 0;
}
