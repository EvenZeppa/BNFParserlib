/**
 * Example: Mini Protocol Message Parsing
 * 
 * Real-world example: parsing a simple text-based protocol message format.
 * Similar to IRC, SMTP, or custom chat protocols.
 * 
 * Message format:
 *   MSG <nickname> :<message text>\r\n
 * 
 * Where:
 *   - "MSG" is a literal command keyword
 *   - <nickname> must start with a letter
 *   - Message text is printable ASCII after the colon
 *   - Terminated with CRLF (\r\n)
 * 
 * This demonstrates a complete mini protocol combining all BNF features.
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"

int main() {
    std::cout << "=== Mini Protocol Message Parsing Example ===" << std::endl;

    Grammar grammar;

    // Character classes
    grammar.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
    grammar.addRule("<digit> ::= '0' ... '9'");
    grammar.addRule("<nick-char> ::= <letter> | <digit> | '_' | '-'");
    
    // Nickname: starts with letter, followed by nick-chars
    grammar.addRule("<nickname> ::= <letter> { <nick-char> }");
    
    // Whitespace: one or more spaces
    grammar.addRule("<space> ::= ' ' { ' ' }");
    
    // Printable ASCII for message text (non-space printable chars plus optional spaces)
    grammar.addRule("<text-char> ::= ( 0x21 ... 0x7E )");
    grammar.addRule("<text> ::= <text-char> { <text-char> | ' ' }");
    
    grammar.addRule("<crlf> ::= '\r' '\n'");

    // Complete message format: MSG <space> <nickname> <space> : <text>
    // Note: simplified without CRLF for demonstration purposes
    grammar.addRule("<message> ::= 'MSG' <space> <nickname> <space> ':' <text> <crlf>");

    std::cout << "\nMini Protocol Message Format:" << std::endl;
    std::cout << "  MSG <nickname> :<text>" << std::endl;
    std::cout << "\nRules:" << std::endl;
    std::cout << "  - Command: MSG (literal)" << std::endl;
    std::cout << "  - Nickname: starts with letter, alphanumeric + _ -" << std::endl;
    std::cout << "  - Text: printable ASCII characters and spaces" << std::endl;
    std::cout << "  - Format: spaces separate command/nickname, colon before text" << std::endl;
    std::cout << std::endl;

    BNFParser parser(grammar);

    // Valid messages
    {
        std::string input = "MSG alice :Hello there!\r\n";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<message>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid message: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid message" << std::endl;
        }
    }

    {
        std::string input = "MSG bob_123 :status update\r\n";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<message>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid message: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid message" << std::endl;
        }
    }

    {
        std::string input = "MSG user-away :back in 5 minutes\r\n";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<message>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid message: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid message" << std::endl;
        }
    }

    {
        std::string input = "MSG X :ok\r\n";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<message>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Valid message (single-letter nick): '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse valid message" << std::endl;
        }
    }

    // Invalid: nickname starts with digit
    {
        std::string input = "MSG 9lives :meow\r\n";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<message>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly accepted invalid nickname" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected: nickname cannot start with digit" << std::endl;
        }
    }

    // Invalid: missing colon
    {
        std::string input = "MSG alice hello\r\n";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<message>", input, consumed);

        if (ast) {
            std::cout << "✗ Incorrectly accepted message without colon" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected: missing ':' before text" << std::endl;
        }
    }

    // Invalid: wrong command
    {
        std::string input = "SEND alice :hello\r\n";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<message>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly accepted wrong command" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected: wrong command (expected MSG)" << std::endl;
        }
    }

    std::cout << "\nThis example demonstrates complete protocol message validation!" << std::endl;
    std::cout << "You can extend this pattern for more complex protocols like IRC, SMTP, etc." << std::endl;

    return 0;
}
