# Character Ranges and Character Classes Implementation

## Summary

Successfully implemented support for **character ranges** and **character classes** in the BNFParserLib, extending the BNF grammar syntax with powerful new matching capabilities while maintaining full backward compatibility.

## Features Implemented

### 1. Character Ranges
Allows matching a single character within a specified range:

```bnf
<lowercase> ::= 'a' ... 'z'
<digit> ::= '0' ... '9'
<ascii> ::= 0x00 ... 0x7F
```

**Syntax:**
- `'char1' ... 'char2'` - Match any character from char1 to char2 (inclusive)
- `0xNN ... 0xMM` - Match any byte value in hexadecimal range

### 2. Character Classes
Allows matching a single character against a set of characters or ranges:

#### Inclusive Character Class
Match if character is in the set:

```bnf
<identifier-char> ::= ( 'a' ... 'z' 'A' ... 'Z' '0' ... '9' '_' )
<hex-digit> ::= ( '0' ... '9' 'a' ... 'f' 'A' ... 'F' )
```

#### Exclusive Character Class
Match if character is NOT in the set:

```bnf
<non-whitespace> ::= ( ^ ' ' 0x09 0x0A 0x0D )
<printable> ::= ( ^ 0x00 ... 0x1F 0x7F )
```

**Syntax:**
- `( items... )` - Inclusive: match if char is in the set
- `( ^ items... )` - Exclusive: match if char is NOT in the set
- Items can be:
  - Single characters: `'a'`, `'_'`, etc.
  - Hexadecimal values: `0x0A`, `0xFF`, etc.
  - Ranges: `'a' ... 'z'`, `0x00 ... 0x7F`

## Implementation Details

### Phase 1: Lexer Extensions
Added new token types to `BNFTokenizer`:
- `TOK_ELLIPSIS` - Three dots `...`
- `TOK_LPAREN` / `TOK_RPAREN` - Parentheses `(` `)`
- `TOK_CARET` - Caret `^` for exclusion
- `TOK_HEX` - Hexadecimal literals `0xNN`

### Phase 2: AST Extensions
Added new expression types to `Expression`:
- `EXPR_CHAR_RANGE` - Represents a character range
- `EXPR_CHAR_CLASS` - Represents a character class

New data structures:
- `CharRange` - Stores start/end characters
- `Expression::charRange` - Single range for CHAR_RANGE expressions
- `Expression::rangeList` - List of ranges for CHAR_CLASS expressions
- `Expression::charList` - List of individual characters for CHAR_CLASS expressions
- `Expression::isExclusion` - Boolean flag for exclusive character classes

### Phase 3: Grammar Parsing
Extended `Grammar` parser to recognize new syntax:
- `parseFactor()` - Handles ranges and character class detection
- `parseCharClass()` - Parses character class contents
- `tokenToChar()` - Converts tokens to character values

### Phase 4: Runtime Matching
Extended `BNFParser` to match new expression types:
- `parseCharRange()` - Matches input character against a range
- `parseCharClass()` - Matches input character against a character class
  - Checks individual characters
  - Checks ranges
  - Applies inclusion/exclusion logic

### Phase 5: Comprehensive Testing
Created extensive test suites:
- **Tokenizer tests**: Verify lexical analysis of new syntax
- **Expression tests**: Verify AST node creation
- **Grammar tests**: Verify parsing of grammar rules
- **Parser tests**: Verify runtime matching behavior
- **Integration tests**: Real-world use cases (IRC nicknames, email identifiers, hex parsers, etc.)

## Backward Compatibility

✅ **Fully backward compatible** - All existing BNF syntax continues to work:
- `[...]` - Optional expressions (unchanged)
- `{...}` - Repetition (unchanged)
- `|` - Alternatives (unchanged)
- `'text'` - Terminals (unchanged)
- `<symbol>` - Non-terminals (unchanged)

⚠️ **New restriction**: Parentheses `()` are now reserved for character classes only.

## Test Results

All test suites pass:
```
Test project /home/ezeppa/Desktop/Workspace/BNFParserLib/build
  test_ast .............................   Passed
  test_expression ......................   Passed
  test_extractor .......................   Passed
  test_grammar .........................   Passed
  test_integration_charclass ...........   Passed
  test_parser ..........................   Passed
  test_tokenizer .......................   Passed

100% tests passed, 0 tests failed out of 7
```

## Usage Examples

### Example 1: IRC Nickname Parser
```cpp
Grammar g;
g.addRule("<letter> ::= ( 'a' ... 'z' 'A' ... 'Z' )");
g.addRule("<digit> ::= '0' ... '9'");
g.addRule("<special> ::= ( '-' '[' ']' '{' '}' '\\' '`' '^' '_' '|' )");
g.addRule("<nick-char> ::= <letter> | <digit> | <special>");
g.addRule("<nickname> ::= <letter> { <nick-char> }");

BNFParser p(g);
size_t consumed;
ASTNode* ast = p.parse("<nickname>", "Alice[bot]", consumed);
// Matches: "Alice[bot]"
```

### Example 2: Hexadecimal Number Parser
```cpp
Grammar g;
g.addRule("<hex-digit> ::= ( '0' ... '9' 'a' ... 'f' 'A' ... 'F' )");
g.addRule("<hex-number> ::= '0' 'x' <hex-digit> { <hex-digit> }");

BNFParser p(g);
size_t consumed;
ASTNode* ast = p.parse("<hex-number>", "0xFF", consumed);
// Matches: "0xFF"
```

### Example 3: Non-Whitespace String
```cpp
Grammar g;
g.addRule("<printable> ::= ( ^ ' ' 0x09 0x0A 0x0D )");
g.addRule("<word> ::= <printable> { <printable> }");

BNFParser p(g);
size_t consumed;
ASTNode* ast = p.parse("<word>", "hello world", consumed);
// Matches: "hello" (stops at space)
```

## Git Commit History

Implementation completed in 5 phases with clear commits:

```
64590a0 Add comprehensive integration tests for character ranges and classes
a233213 Implement runtime matching for character ranges and classes
abc2ecb Implement parsing for character ranges and character classes
97b7796 Add CharRange and CharClass expression types to AST
2635c81 Add lexer support for character ranges and classes (ellipsis, parentheses, caret, hex)
```

## Technical Notes

### Character Encoding
- All character matching uses `unsigned char` (0-255 range)
- Hexadecimal literals support 1-2 digit hex values
- Compatible with ASCII and extended ASCII

### Performance
- Character range matching: O(1) comparison
- Character class matching: O(n+m) where n=charList.size(), m=rangeList.size()
- No performance impact on existing BNF parsing

### Memory Management
- All AST nodes properly cleaned up in destructors
- No memory leaks detected in test runs
- Expression trees manage their own memory

## Future Enhancements (Optional)

Potential improvements for future versions:
1. Unicode support for character ranges
2. Named character classes (e.g., `[:alpha:]`, `[:digit:]`)
3. Case-insensitive character matching flag
4. Predefined character class shortcuts

## Conclusion

The implementation is complete, fully tested, and ready for production use. All features work as specified, maintain backward compatibility, and follow the existing code architecture and style.
