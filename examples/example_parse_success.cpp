/**
 * Example: Successful Parsing with ParseContext
 * 
 * Demonstrates the new unified ParseContext API for parsing.
 * Shows how to check success status, retrieve the AST, and
 * get information about how much input was consumed.
 * 
 * The ParseContext structure provides all parsing information in one place:
 *   - success: whether parsing succeeded
 *   - ast: the parsed Abstract Syntax Tree
 *   - consumed: number of characters matched
 *   - errorPos/expected: error details (for failures)
 */

#include <iostream>
#include <string>
#include "Grammar.hpp"
#include "BNFParser.hpp"
#include "ParseContext.hpp"

void printAST(ASTNode* node, int indent = 0) {
    if (!node) return;
    
    std::string indentStr(indent * 2, ' ');
    std::cout << indentStr << "Node: " << node->symbol;
    if (!node->matched.empty()) {
        std::cout << " => '" << node->matched << "'";
    }
    std::cout << std::endl;
    
    for (size_t i = 0; i < node->children.size(); ++i) {
        printAST(node->children[i], indent + 1);
    }
}

int main() {
    std::cout << "=== ParseContext Success Examples ===" << std::endl;
    std::cout << std::endl;

    // Create a simple grammar for arithmetic expressions
    Grammar grammar;
    grammar.addRule("<digit> ::= '0' ... '9'");
    grammar.addRule("<number> ::= <digit> { <digit> }");
    grammar.addRule("<sign> ::= '+' | '-'");
    grammar.addRule("<signed-number> ::= [ <sign> ] <number>");

    BNFParser parser(grammar);

    // Example 1: Parse a simple number
    {
        std::cout << "Example 1: Parse '42'" << std::endl;
        std::cout << "-------------------" << std::endl;
        
        ParseContext ctx;
        parser.parse("<number>", "42", ctx);
        
        if (ctx.success) {
            std::cout << "✓ Parsing SUCCEEDED" << std::endl;
            std::cout << "  Consumed: " << ctx.consumed << " characters" << std::endl;
            std::cout << "  Matched: '" << ctx.ast->matched << "'" << std::endl;
            std::cout << "\n  AST Structure:" << std::endl;
            printAST(ctx.ast, 2);
            delete ctx.ast;
        } else {
            std::cout << "✗ Parsing failed (unexpected!)" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 2: Parse a signed number
    {
        std::cout << "Example 2: Parse '+123'" << std::endl;
        std::cout << "--------------------" << std::endl;
        
        ParseContext ctx;
        parser.parse("<signed-number>", "+123", ctx);
        
        if (ctx.success) {
            std::cout << "✓ Parsing SUCCEEDED" << std::endl;
            std::cout << "  Consumed: " << ctx.consumed << " characters" << std::endl;
            std::cout << "  Matched: '" << ctx.ast->matched << "'" << std::endl;
            std::cout << "\n  AST Structure:" << std::endl;
            printAST(ctx.ast, 2);
            delete ctx.ast;
        } else {
            std::cout << "✗ Parsing failed (unexpected!)" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 3: Parse negative number
    {
        std::cout << "Example 3: Parse '-999'" << std::endl;
        std::cout << "--------------------" << std::endl;
        
        ParseContext ctx;
        parser.parse("<signed-number>", "-999", ctx);
        
        if (ctx.success) {
            std::cout << "✓ Parsing SUCCEEDED" << std::endl;
            std::cout << "  Consumed: " << ctx.consumed << " characters" << std::endl;
            std::cout << "  Matched: '" << ctx.ast->matched << "'" << std::endl;
            delete ctx.ast;
        } else {
            std::cout << "✗ Parsing failed (unexpected!)" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 4: Character class example
    {
        std::cout << "Example 4: Character classes and sequences" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        Grammar g2;
        g2.addRule("<letter> ::= ( 'a' ... 'z' 'A' ... 'Z' )");
        g2.addRule("<word> ::= <letter> { <letter> }");
        
        BNFParser parser2(g2);
        ParseContext ctx;
        parser2.parse("<word>", "Hello", ctx);
        
        if (ctx.success) {
            std::cout << "✓ Parsing SUCCEEDED" << std::endl;
            std::cout << "  Input: 'Hello'" << std::endl;
            std::cout << "  Consumed: " << ctx.consumed << " characters" << std::endl;
            std::cout << "  Matched: '" << ctx.ast->matched << "'" << std::endl;
            delete ctx.ast;
        } else {
            std::cout << "✗ Parsing failed (unexpected!)" << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 5: Partial match (parse what's possible)
    {
        std::cout << "Example 5: Partial matching" << std::endl;
        std::cout << "-------------------------" << std::endl;
        std::cout << "Note: Parser matches as much as possible from the start" << std::endl;
        
        ParseContext ctx;
        parser.parse("<number>", "123abc", ctx);
        
        if (ctx.success) {
            std::cout << "✓ Parsing SUCCEEDED (partial match)" << std::endl;
            std::cout << "  Input: '123abc'" << std::endl;
            std::cout << "  Consumed: " << ctx.consumed << " characters" << std::endl;
            std::cout << "  Matched: '" << ctx.ast->matched << "'" << std::endl;
            std::cout << "  Remaining: '" << std::string("123abc").substr(ctx.consumed) << "'" << std::endl;
            delete ctx.ast;
        } else {
            std::cout << "✗ Parsing failed (unexpected!)" << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "===========================================\n";
    std::cout << "ParseContext provides:\n";
    std::cout << "  - ctx.success: boolean success indicator\n";
    std::cout << "  - ctx.ast: the parsed AST (if successful)\n";
    std::cout << "  - ctx.consumed: characters consumed\n";
    std::cout << "  - Clean, unified interface!\n";

    return 0;
}
