/**
 * Example: FIRST-Set Lookahead Pruning
 * 
 * Demonstrates how FIRST-set analysis enables efficient parsing by
 * pruning alternatives that cannot possibly match based on the first
 * character of input.
 * 
 * When parsing alternatives A | B | C, the parser can quickly determine
 * which alternative to try by checking which one's FIRST-set contains
 * the current input character.
 * 
 * This example shows a mini HTTP-like protocol with different commands.
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"

int main() {
    std::cout << "=== FIRST-Set Lookahead Pruning Example ===" << std::endl;

    Grammar grammar;

    // Define common building blocks
    grammar.addRule("<space> ::= ' ' { ' ' }");
    grammar.addRule("<path-char> ::= ( 'a' ... 'z' 'A' ... 'Z' '0' ... '9' '/' '.' '_' '-' )");
    grammar.addRule("<path> ::= '/' <path-char> { <path-char> }");

    // Define different HTTP-like commands
    grammar.addRule("<command-get> ::= 'GET' <space> <path>");
    grammar.addRule("<command-post> ::= 'POST' <space> <path>");
    grammar.addRule("<command-put> ::= 'PUT' <space> <path>");
    grammar.addRule("<command-delete> ::= 'DELETE' <space> <path>");
    grammar.addRule("<command-ping> ::= 'PING'");

    // Alternative: any of the above commands
    // Parser uses FIRST-set to quickly determine which branch to try:
    //   - FIRST(<command-get>)    = { 'G' }
    //   - FIRST(<command-post>)   = { 'P' }
    //   - FIRST(<command-put>)    = { 'P' }
    //   - FIRST(<command-delete>) = { 'D' }
    //   - FIRST(<command-ping>)   = { 'P' }
    grammar.addRule("<request> ::= <command-get> | <command-post> | <command-put> | <command-delete> | <command-ping>");

    std::cout << "\nCreated mini protocol with commands:" << std::endl;
    std::cout << "  GET, POST, PUT, DELETE, PING" << std::endl;
    std::cout << "\nFIRST-set for <request> alternatives:" << std::endl;
    std::cout << "  GET:    { 'G' }" << std::endl;
    std::cout << "  POST:   { 'P' }" << std::endl;
    std::cout << "  PUT:    { 'P' }" << std::endl;
    std::cout << "  DELETE: { 'D' }" << std::endl;
    std::cout << "  PING:   { 'P' }" << std::endl;

    BNFParser parser(grammar);

    // Test GET command (FIRST = 'G')
    {
        std::string input = "GET /index.html";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<request>", input, consumed);
        
        if (ast) {
            std::cout << "\n✓ Parsed GET request: '" << ast->matched << "'" << std::endl;
            std::cout << "  Parser used FIRST-set to immediately try <command-get>" << std::endl;
            delete ast;
        } else {
            std::cout << "\n✗ Failed to parse GET request" << std::endl;
        }
    }

    // Test POST command (FIRST = 'P')
    {
        std::string input = "POST /api/data";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<request>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed POST request: '" << ast->matched << "'" << std::endl;
            std::cout << "  Parser checked FIRST-set and tried POST alternatives" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse POST request" << std::endl;
        }
    }

    // Test PING command (FIRST = 'P', but no path)
    {
        std::string input = "PING";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<request>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed PING request: '" << ast->matched << "'" << std::endl;
            std::cout << "  Parser tried alternatives with FIRST = 'P'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse PING request" << std::endl;
        }
    }

    // Test DELETE command (FIRST = 'D')
    {
        std::string input = "DELETE /resource";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<request>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed DELETE request: '" << ast->matched << "'" << std::endl;
            std::cout << "  Parser used FIRST-set to quickly select <command-delete>" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse DELETE request" << std::endl;
        }
    }

    // Test rejection: TRACE not in grammar
    // Parser checks FIRST-sets and finds no alternative starting with 'T'
    {
        std::string input = "TRACE /debug";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<request>", input, consumed);
        
        if (ast) {
            std::cout << "✗ Incorrectly parsed invalid command" << std::endl;
            delete ast;
        } else {
            std::cout << "✓ Correctly rejected TRACE command" << std::endl;
            std::cout << "  FIRST-set pruning: no alternative matches 'T'" << std::endl;
        }
    }

    std::cout << "\nFIRST-set pruning benefits:" << std::endl;
    std::cout << "  - Eliminates impossible alternatives immediately" << std::endl;
    std::cout << "  - Reduces backtracking and improves parse speed" << std::endl;
    std::cout << "  - Particularly effective with many alternatives" << std::endl;

    return 0;
}
