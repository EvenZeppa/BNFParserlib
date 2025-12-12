/**
 * Example: Character Classes
 * 
 * Demonstrates inclusive and exclusive character classes:
 *   - Inclusive class: ( 'a' 'e' 'i' ) matches any listed character
 *   - Exclusive class: ( ^ 'a' 'e' 'i' ) matches any character NOT listed
 *   - Classes can include ranges: ( 'a' ... 'z' '0' ... '9' )
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"

int main() {
    std::cout << "=== Character Classes Example ===" << std::endl;

    Grammar grammar;

    // Inclusive character class: matches any vowel
    grammar.addRule("<vowel> ::= ( 'a' 'e' 'i' 'o' 'u' )");
    
    // Exclusive character class: matches any character except vowels
    // (Note: this is simplified - in practice would match any char in the parser's range)
    grammar.addRule("<consonant> ::= ( ^ 'a' 'e' 'i' 'o' 'u' ' ' )");
    
    // Mix inclusive class with ranges for hex digits
    grammar.addRule("<hex-digit> ::= ( '0' ... '9' 'a' ... 'f' 'A' ... 'F' )");
    
    // Printable ASCII using an inclusive range
    grammar.addRule("<printable> ::= ( 0x21 ... 0x7E )");

    BNFParser parser(grammar);

    // Test inclusive class (vowel)
    {
        std::string input = "e";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<vowel>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Matched vowel: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to match vowel" << std::endl;
        }
    }

    // Test exclusive class (consonant)
    {
        std::string input = "b";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<consonant>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Matched consonant: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to match consonant" << std::endl;
        }
    }

    // Test that exclusive class rejects vowels
    {
        std::string input = "a";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<consonant>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly matched vowel as consonant" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected vowel for <consonant> rule" << std::endl;
        }
    }

    // Test hex-digit class (includes multiple ranges)
    {
        std::string input = "F";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-digit>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Matched hex digit: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to match hex digit" << std::endl;
        }
    }

    // Test hex-digit with lowercase
    {
        std::string input = "c";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-digit>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Matched hex digit: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to match hex digit" << std::endl;
        }
    }

    // Test hex-digit rejects non-hex letters
    {
        std::string input = "g";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<hex-digit>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly matched non-hex letter" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected 'g' for <hex-digit>" << std::endl;
        }
    }

    // Test printable ASCII
    {
        std::string input = "@";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<printable>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Matched printable character: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to match printable" << std::endl;
        }
    }

    std::cout << "\nCharacter classes provide flexible pattern matching!" << std::endl;
    return 0;
}
