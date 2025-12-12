#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>
#include <vector>
#include <bitset>

/**
 * @brief Represents a single character range.
 * 
 * Used within character class expressions to store ranges like 'a' ... 'z'.
 */
struct CharRange {
    unsigned char start;  ///< Start character of the range
    unsigned char end;    ///< End character of the range (inclusive)
    
    /**
     * @brief Constructs a character range.
     * @param s Start character
     * @param e End character
     */
    CharRange(unsigned char s, unsigned char e);
    
    /**
     * @brief Default constructor.
     */
    CharRange();
};

/**
 * @brief Represents a grammar expression node used by the interpreter.
 *
 * The Expression structure models different grammar constructs (sequence,
 * alternative, optional, repetition, symbol, terminal). Nodes may have
 * child expressions for composite constructs.
 */
struct Expression {
    /**
     * @brief Type of the expression node.
     *
     * - EXPR_SEQUENCE: a sequence of child expressions evaluated in order.
     * - EXPR_ALTERNATIVE: a choice between child expressions.
     * - EXPR_OPTIONAL: an optional child expression.
     * - EXPR_REPEAT: a repeating child expression (zero or more).
     * - EXPR_SYMBOL: a non-terminal symbol reference.
     * - EXPR_TERMINAL: a terminal token/value.
     * - EXPR_CHAR_RANGE: a character range (e.g., 'a' ... 'z').
     * - EXPR_CHAR_CLASS: a character class (e.g., ( 'a' ... 'z' '0' '9' )).
     */
    enum Type {
        EXPR_SEQUENCE,
        EXPR_ALTERNATIVE,
        EXPR_OPTIONAL,
        EXPR_REPEAT,
        EXPR_SYMBOL,
        EXPR_TERMINAL,
        EXPR_CHAR_RANGE,
        EXPR_CHAR_CLASS
    };

    // The node type.
    Type type;
    // Child expressions (used for composite types like sequence/alternative).
    std::vector<Expression*> children;
    // Optional textual value (e.g. symbol name or terminal text).
    std::string value;
    
    // ===== Character Range/Class specific fields =====
    // For EXPR_CHAR_RANGE: stores the start and end character
    CharRange charRange;

    // For EXPR_CHAR_CLASS: 256-bit bitmap representing membership
    // bitmap[c] == true means the class matches byte value c.
    std::bitset<256> charBitmap;

    // Convenience inline matcher for character classes
    inline bool classMatches(unsigned char c) const {
        return charBitmap.test(static_cast<size_t>(c));
    }

    /**
     * @brief Constructs an Expression of the given type.
     *
     * @param t Type of expression to create.
     */
    Expression(Type t);

    /**
     * @brief Destructor. Responsible for cleaning up owned children.
     */
    ~Expression();
};

#endif
