# BNFParserLib

A robust C++98 compatible library for parsing Backus-Naur Form (BNF) grammars and generating Abstract Syntax Trees (AST). This library provides a complete solution for grammar definition, parsing, and AST manipulation with utilities for extracting structured data from parse results.

## Features

- **BNF Grammar Parsing**: Define and parse BNF grammars with support for:
  - Terminal symbols (literals)
  - Non-terminal symbols
  - Alternatives (`|`)
  - Sequences
  - Optional elements (`[ ]`)
  - Repetitions (`{ }`)
  - Character ranges (`'a' ... 'z'`)
  - Character classes (inclusive/exclusive)
  
- **AST Generation**: Generate structured Abstract Syntax Trees from parsed input

- **Partial Parsing**: Extract valid data from partially invalid inputs
  - Collect successfully parsed sub-elements even when overall parse fails
  - Detailed error tracking with position and expected symbols
  - Perfect for user input validation and error recovery

- **Data Extraction**: Powerful utilities to extract and query specific symbols from ASTs with configurable options:
  - Symbol filtering
  - Terminal inclusion/exclusion
  - Repetition flattening
  
- **Modern Test Framework**: Comprehensive test suite with colored output and detailed reporting

- **C++98 Compatibility**: Full compatibility with older C++ standards for maximum portability

## Project Structure

```
BNFInterpreter/
├── include/           # Header files
│   ├── AST.hpp        # Abstract Syntax Tree definitions
│   ├── BNFParser.hpp  # Main BNF parser class
│   ├── Grammar.hpp    # Grammar rule definitions
│   ├── Expression.hpp # Expression tree structures
│   ├── DataExtractor.hpp # AST data extraction utilities
│   └── ...
├── src/               # Implementation files
├── tests/             # Unit tests with modern test framework
├── examples/          # Usage examples
└── CMakeLists.txt     # CMake build configuration
```

## Installation

### Prerequisites

- CMake 3.10 or later
- C++98 compatible compiler (GCC, Clang, MSVC)

### Building from Source

1. **Clone the repository:**
   ```bash
   git clone https://github.com/EvenZeppa/BNFParserLib.git
   cd BNFParserLib
   ```

2. **Create build directory and configure:**
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. **Build the library and tests:**
   ```bash
   make
   ```

4. **Run the test suite:**
   ```bash
   ctest --verbose
   ```

### Installation Options

**Install system-wide:**
```bash
sudo make install
```

**Install to custom directory:**
```bash
cmake -DCMAKE_INSTALL_PREFIX=/your/custom/path ..
make install
```

## Usage

### Basic Example

Here's a simple example demonstrating how to use the BNF parser library:

```cpp
#include "Grammar.hpp"
#include "BNFParser.hpp"
#include "DataExtractor.hpp"
#include <iostream>

int main() {
    // 1. Create and configure grammar
    Grammar grammar;
    grammar.addRule("<letter> ::= 'a' | 'b' | 'c'");
    grammar.addRule("<word> ::= <letter> { <letter> }");
    grammar.addRule("<message> ::= <word> ' ' <word>");
    
    // 2. Create parser
    BNFParser parser(grammar);
    
    // 3. Parse input text
    std::string input = "hello world";
    size_t consumed = 0;
    ASTNode* ast = parser.parse("<message>", input, consumed);
    
    if (ast && consumed == input.length()) {
        std::cout << "Parsing successful!" << std::endl;
        std::cout << "Matched: " << ast->matched << std::endl;
        
        // 4. Extract structured data from AST
        DataExtractor extractor;
        ExtractedData data = extractor.extract(ast);
        
        // 5. Query extracted data
        if (data.has("<word>")) {
            std::vector<std::string> words = data.all("<word>");
            std::cout << "Found " << words.size() << " words:" << std::endl;
            for (size_t i = 0; i < words.size(); ++i) {
                std::cout << "  - " << words[i] << std::endl;
            }
        }
        
        delete ast;
    } else {
        std::cout << "Parsing failed" << std::endl;
    }
    
    return 0;
}
```

### Advanced Data Extraction

The `DataExtractor` class provides powerful configuration options:

```cpp
DataExtractor extractor;

// Configure symbol filtering
std::vector<std::string> symbols;
symbols.push_back("<word>");
symbols.push_back("<letter>");
extractor.setSymbols(symbols);

// Include terminal symbols in output
extractor.includeTerminals(true);

// Enable repetition flattening
extractor.flattenRepetitions(true);

// Extract data with configuration
ExtractedData data = extractor.extract(ast);

// Query utilities
bool hasWords = data.has("<word>");
size_t wordCount = data.count("<word>");
std::string firstWord = data.first("<word>");
std::vector<std::string> allWords = data.all("<word>");
```

### Partial Parsing (New in v1.1)

The `ParseContext` API enables partial parsing - extracting valid data from partially invalid inputs and getting detailed error information:

```cpp
#include "Grammar.hpp"
#include "BNFParser.hpp"
#include "ParseContext.hpp"

int main() {
    // Define grammar for IRC channel list
    Grammar g;
    g.addRule("<letter> ::= 'a' ... 'z' | 'A' ... 'Z'");
    g.addRule("<digit> ::= '0' ... '9'");
    g.addRule("<chanchar> ::= <letter> | <digit> | '_'");
    g.addRule("<channel> ::= '#' <letter> { <chanchar> }");
    g.addRule("<comma> ::= ','");
    g.addRule("<chanlist> ::= <channel> { <comma> <channel> }");
    
    BNFParser parser(g);
    ParseContext ctx;
    
    // Parse input with some invalid channels
    parser.parse("<chanlist>", "#chan1,invalid,#chan3", ctx);
    
    // Check overall success
    if (ctx.success) {
        std::cout << "Matched: " << ctx.ast->matched << std::endl;
    }
    
    // Access successfully parsed sub-elements (even if overall parse failed)
    std::cout << "Valid channels found: " << ctx.partialNodes.size() << std::endl;
    for (size_t i = 0; i < ctx.partialNodes.size(); ++i) {
        std::cout << "  - " << ctx.partialNodes[i]->matched << std::endl;
    }
    
    // Access detailed error information
    std::cout << "Failures found: " << ctx.failures.size() << std::endl;
    for (size_t i = 0; i < ctx.failures.size(); ++i) {
        const FailedNode& fail = ctx.failures[i];
        std::cout << "  At position " << fail.position << ": "
                  << fail.text << std::endl;
        std::cout << "  Expected: " << fail.expected << std::endl;
    }
    
    // Clean up
    if (ctx.ast) delete ctx.ast;
    for (size_t i = 0; i < ctx.partialNodes.size(); ++i) {
        delete ctx.partialNodes[i];
    }
    
    return 0;
}
```

**ParseContext Fields:**
- `ast` - The complete AST (if successful), or nullptr
- `success` - True if parse succeeded completely
- `consumed` - Number of characters consumed
- `errorPos` - Position of furthest failure
- `expected` - Description of what was expected at errorPos
- `partialNodes` - Successfully parsed sub-elements (populated on partial failures)
- `failures` - Detailed information about each failed sub-element

**Use Cases:**
- User input validation with detailed feedback
- Parsing lists where some entries may be invalid
- Configuration files with optional/malformed entries
- Extracting valid data from partially corrupted input
- Progressive parsing with error recovery

See `examples/example_partial_parse.cpp` for comprehensive demonstrations.

### Grammar Definition Syntax

The library supports standard BNF notation:

```
<rule-name> ::= <expression>
```

**Supported constructs:**
- **Terminals**: `'literal'` - Matches exact text
- **Non-terminals**: `<symbol>` - References other rules
- **Alternatives**: `A | B` - Matches A or B
- **Sequences**: `A B` - Matches A followed by B  
- **Optional**: `[ A ]` - Matches A or nothing
- **Repetition**: `{ A }` - Matches zero or more A

**Example IRC-like grammar:**
```cpp
grammar.addRule("<letter> ::= 'a' | 'b' | 'c' | ... | 'z' | 'A' | ... | 'Z'");
grammar.addRule("<digit> ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'");
grammar.addRule("<nick-char> ::= <letter> | <digit> | '_' | '-'");
grammar.addRule("<nick> ::= <letter> { <nick-char> }");
grammar.addRule("<command> ::= 'JOIN' | 'PART' | 'PRIVMSG'");
grammar.addRule("<channel> ::= '#' <nick>");
grammar.addRule("<message> ::= ':' <nick> ' ' <command> ' ' <channel>");
```

## API Reference

### Core Classes

#### `Grammar`
- `addRule(const std::string& rule)` - Add a BNF rule
- `getRule(const std::string& name)` - Get rule by name
- `hasRule(const std::string& name)` - Check if rule exists

#### `BNFParser`  
- `BNFParser(const Grammar& g)` - Constructor
- `parse(const std::string& ruleName, const std::string& input, size_t& consumed)` - Legacy parse (backward compatible)
- `parse(const std::string& ruleName, const std::string& input, ParseContext& ctx)` - Unified parse with full context

#### `ParseContext`
- `ASTNode* ast` - Root AST node (nullptr on failure)
- `bool success` - True if parsing succeeded
- `size_t consumed` - Characters consumed from input
- `size_t errorPos` - Position of furthest parse failure
- `std::string expected` - Description of expected input at errorPos
- `std::vector<ASTNode*> partialNodes` - Successfully parsed sub-elements (on partial failures)
- `std::vector<FailedNode> failures` - Detailed failure information
- `void reset()` - Reset context to initial state
- `void updateError(size_t pos, const std::string& expected)` - Update error tracking

#### `FailedNode`
- `size_t position` - Position where parsing failed
- `std::string text` - Text fragment that failed to parse
- `std::string expected` - What was expected at this position
- `std::string ruleName` - Rule being parsed when failure occurred

#### `ASTNode`
- `std::string symbol` - Node symbol name
- `std::string matched` - Matched text content
- `std::vector<ASTNode*> children` - Child nodes

#### `DataExtractor`
- `setSymbols(const std::vector<std::string>& symbols)` - Filter symbols
- `includeTerminals(bool include)` - Include/exclude terminals
- `flattenRepetitions(bool flatten)` - Flatten repetition nodes
- `resetConfig()` - Reset to default configuration
- `extract(ASTNode* ast)` - Extract data from AST

#### `ExtractedData`
- `has(const std::string& symbol)` - Check if symbol exists
- `count(const std::string& symbol)` - Get occurrence count
- `first(const std::string& symbol)` - Get first occurrence
- `all(const std::string& symbol)` - Get all occurrences

## Testing

The library includes a comprehensive test suite with modern colored output:

### Running Tests

```bash
# Run all tests
ctest

# Run tests with verbose output
ctest --verbose

# Run specific test
ctest -R test_parser

# Run tests in parallel
ctest -j4
```

### Test Coverage

- **172 total tests** across 11 test suites:
  - AST functionality (14 tests)
  - Expression parsing (11 tests) 
  - Grammar rules (41 tests)
  - BNF parsing (35 tests)
  - Tokenization (34 tests)
  - Data extraction (28 tests)
  - Partial parsing (9 tests)

### Test Framework Features

- Colored output (✅ green for pass, ❌ red for fail)
- Detailed assertion reporting with expected vs actual values
- Modern test organization with clear suite structure
- C++98 compatible implementation

## Examples

The `examples/` directory contains comprehensive usage examples:

### Core Functionality
- `example_ranges.cpp` - Character range demonstrations ('a' ... 'z')
- `example_classes.cpp` - Character classes (inclusive/exclusive)
- `example_sequences.cpp` - Sequences, repetitions, optional, alternation
- `example_arena.cpp` - Arena-backed memory allocation
- `example_interning.cpp` - Expression interning and deduplication
- `example_first_set.cpp` - FIRST-set lookahead pruning optimization

### Practical Applications
- `example_irc_nickname.cpp` - IRC nickname validation
- `example_hex_literals.cpp` - Hexadecimal literal parsing
- `example_mini_protocol.cpp` - Mini protocol message parsing
- `example_partial_parse.cpp` - **Partial parsing with error recovery**

### Error Reporting
- `example_parse_success.cpp` - Successful parsing demonstrations
- `example_parse_errors.cpp` - Comprehensive error reporting examples

Build and run examples:
```bash
cd build
make
./examples/example_partial_parse
./examples/example_parse_errors
```

## Integration

### Using with CMake

After installation, integrate into your project:

```cmake
find_package(BNFParserLib REQUIRED)
target_link_libraries(your_target bnf)
```

### Manual Integration

Include headers and link against the static library:

```cpp
#include <bnfparser/Grammar.hpp>
#include <bnfparser/BNFParser.hpp>
// ... other headers
```

Compile with:
```bash
g++ -std=c++98 -Wall -Wextra -Werror your_code.cpp -lbnf
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass: `ctest --verbose`
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Compatibility

- **C++ Standard**: C++98 and later
- **Compilers**: GCC 4.x+, Clang 3.x+, MSVC 2010+
- **Platforms**: Linux, macOS, Windows
- **CMake**: 3.10+

## Performance

- **Memory Management**: Explicit RAII with proper cleanup
- **Parsing Algorithm**: Recursive descent with backtracking
- **AST Storage**: Tree structure with efficient traversal
- **Thread Safety**: Not thread-safe (design for single-threaded use)
