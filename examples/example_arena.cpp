/**
 * Example: Arena-Backed Allocation
 * 
 * Demonstrates using an Arena for fast, bulk memory allocation.
 * The Arena preallocates a memory pool and serves allocations from it,
 * reducing malloc overhead and improving performance for many small objects.
 * 
 * All expressions created through the grammar will use arena allocation
 * when setArena() is called on the grammar.
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"
#include "Arena.hpp"

int main() {
    std::cout << "=== Arena-Backed Allocation Example ===" << std::endl;

    // Create an arena with 2KB initial capacity
    Arena arena(2048);
    
    std::cout << "Created arena with 2048 bytes block size" << std::endl;

    // Create grammar and attach the arena
    Grammar grammar;
    grammar.setArena(&arena);
    
    std::cout << "Attached arena to grammar - expressions will use arena allocation" << std::endl;

    // Define rules - expressions allocated from arena
    grammar.addRule("<digit> ::= '0' ... '9'");
    grammar.addRule("<hex-digit> ::= <digit> | 'a' ... 'f' | 'A' ... 'F'");
    grammar.addRule("<octet> ::= <hex-digit> <hex-digit>");
    grammar.addRule("<color> ::= '#' <octet> <octet> <octet>");

    std::cout << "Added grammar rules (allocated from arena)" << std::endl;

    // Create parser
    BNFParser parser(grammar);

    // Parse a hex color
    {
        std::string input = "#1a2b3c";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<color>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed hex color: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse color" << std::endl;
        }
    }

    // Parse another color
    {
        std::string input = "#FF00AB";
        size_t consumed = 0;
        ASTNode* ast = parser.parse("<color>", input, consumed);
        
        if (ast) {
            std::cout << "✓ Parsed hex color: '" << ast->matched << "'" << std::endl;
            delete ast;
        } else {
            std::cout << "✗ Failed to parse color" << std::endl;
        }
    }

    std::cout << "\nArena allocation provides:" << std::endl;
    std::cout << "  - Fast bulk allocation from preallocated pool" << std::endl;
    std::cout << "  - Reduced malloc/free overhead" << std::endl;
    std::cout << "  - Automatic cleanup when arena is destroyed" << std::endl;

    // Arena automatically frees all allocated memory when destroyed
    return 0;
}
