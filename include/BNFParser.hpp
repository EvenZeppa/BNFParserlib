#ifndef BNF_PARSER_HPP
#define BNF_PARSER_HPP

#include "Grammar.hpp"
#include "AST.hpp"
#include <string>
#include <map>
#include <bitset>

/**
 * @brief Parser for BNF grammars that generates Abstract Syntax Trees.
 * 
 * Takes a grammar and input text, then attempts to parse the input according
 * to the grammar rules, producing an AST representing the parsed structure.
 * Uses recursive descent parsing with backtracking for alternatives.
 */
class BNFParser {
public:
    /**
     * @brief Constructs a parser for the given grammar.
     * @param g The grammar containing the parsing rules
     */
    BNFParser(const Grammar& g);

    /**
     * @brief Destructor for cleanup.
     */
    ~BNFParser();

    /**
     * @brief Parses input text according to the specified grammar rule.
     * @param ruleName Name of the grammar rule to use as starting point
     * @param input The text to parse
     * @param consumed Output parameter for the number of characters consumed
     * @return Pointer to the root AST node, or nullptr if parsing failed
     */
	ASTNode* parse(const std::string& ruleName,
				const std::string& input,
				size_t& consumed) const;

private:
    struct FirstInfo {
        std::bitset<256> chars;
        bool nullable;
        FirstInfo() : nullable(false) {}
    };

    const Grammar& grammar;  ///< Reference to the grammar rules
    mutable std::map<Expression*, FirstInfo> firstCache; ///< FIRST-set memo

    /**
     * @brief Removes surrounding quotes from a string.
     * @param s The string to process
     * @return String with quotes removed if present
     */
    std::string stripQuotes(const std::string& s) const;

    /**
     * @brief Recursively parses an expression and builds AST nodes.
     * @param expr The expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseExpression(Expression* expr,
                         const std::string& input,
                         size_t& pos,
                         ASTNode*& outNode) const;

    /**
     * @brief Parses terminal expressions (quoted strings).
     * @param expr The terminal expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseTerminal(Expression* expr,
                       const std::string& input,
                       size_t& pos,
                       ASTNode*& outNode) const;

    /**
     * @brief Parses symbol expressions (non-terminal references).
     * @param expr The symbol expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseSymbol(Expression* expr,
                     const std::string& input,
                     size_t& pos,
                     ASTNode*& outNode) const;

    /**
     * @brief Parses sequence expressions (ordered list of sub-expressions).
     * @param expr The sequence expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseSequence(Expression* expr,
                       const std::string& input,
                       size_t& pos,
                       ASTNode*& outNode) const;

    /**
     * @brief Parses alternative expressions (choice between sub-expressions).
     * @param expr The alternative expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseAlternative(Expression* expr,
                          const std::string& input,
                          size_t& pos,
                          ASTNode*& outNode) const;

    /**
     * @brief Parses optional expressions (zero or one occurrence).
     * @param expr The optional expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseOptional(Expression* expr,
                       const std::string& input,
                       size_t& pos,
                       ASTNode*& outNode) const;

    /**
     * @brief Parses repetition expressions (zero or more occurrences).
     * @param expr The repetition expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseRepeat(Expression* expr,
                     const std::string& input,
                     size_t& pos,
                     ASTNode*& outNode) const;

    /**
     * @brief Parses character range expressions.
     * @param expr The character range expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseCharRange(Expression* expr,
                        const std::string& input,
                        size_t& pos,
                        ASTNode*& outNode) const;

    /**
     * @brief Parses character class expressions.
     * @param expr The character class expression to parse
     * @param input The input text
     * @param pos Current position in input (updated during parsing)
     * @param outNode Output parameter for the generated AST node
     * @return true if parsing succeeded, false otherwise
     */
    bool parseCharClass(Expression* expr,
                        const std::string& input,
                        size_t& pos,
                        ASTNode*& outNode) const;

    // FIRST-set computation with memoization
    const FirstInfo& computeFirst(Expression* expr) const;
    void mergeFirst(FirstInfo& dst, const FirstInfo& src) const;
    void addChar(FirstInfo& fi, unsigned char c) const;
    std::string terminalFirstString(Expression* expr) const;
};

#endif
