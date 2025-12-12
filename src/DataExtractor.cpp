#include "../include/DataExtractor.hpp"
#include "../include/Debug.hpp"
#include <iostream>
#include <sstream>

// Constructor with default settings
DataExtractor::DataExtractor() 
    : extractTerminals(false), flattenReps(false) {
    DEBUG_MSG("DataExtractor: initialized with default settings");
}

// Extract data from AST root node
ExtractedData DataExtractor::extract(ASTNode* root) {
    DEBUG_MSG("DataExtractor::extract: starting extraction");
    ExtractedData out;
    if (root) {
        visit(root, out);
        DEBUG_MSG("DataExtractor::extract: extraction completed");
    } else {
        DEBUG_MSG("DataExtractor::extract: null root node");
    }
    return out;
}

// Set specific symbols to extract
void DataExtractor::setSymbols(const std::vector<std::string>& symbols) {
    targetSymbols.clear();
    for (size_t i = 0; i < symbols.size(); ++i) {
        targetSymbols.insert(symbols[i]);
    }
    std::ostringstream oss;
    oss << symbols.size();
    DEBUG_MSG("DataExtractor::setSymbols: configured " + oss.str() + " target symbols");
}

// Set whether to include terminals
void DataExtractor::includeTerminals(bool include) {
    extractTerminals = include;
    DEBUG_MSG("DataExtractor::includeTerminals: set to " + std::string(include ? "true" : "false"));
}

// Set whether to flatten repetitions
void DataExtractor::flattenRepetitions(bool flatten) {
    flattenReps = flatten;
    DEBUG_MSG("DataExtractor::flattenRepetitions: set to " + std::string(flatten ? "true" : "false"));
}

// Reset configuration to defaults
void DataExtractor::resetConfig() {
    targetSymbols.clear();
    extractTerminals = false;
    flattenReps = false;
    DEBUG_MSG("DataExtractor::resetConfig: reset to default settings");
}

// Check if a symbol should be extracted based on configuration
bool DataExtractor::shouldExtract(const std::string& symbol) const {
    // Check if we have specific symbols configured
    if (!targetSymbols.empty()) {
        return targetSymbols.find(symbol) != targetSymbols.end();
    }

    // Check terminal extraction setting
    if (isTerminal(symbol) && !extractTerminals) {
        return false;
    }

    // Extract non-terminals by default
    if (isNonTerminal(symbol)) {
        return true;
    }

    // Extract terminals only if configured
    if (isTerminal(symbol) && extractTerminals) {
        return true;
    }

    return false;
}

// Recursively visit AST nodes to extract data (C++98 compatible)
void DataExtractor::visit(ASTNode* node, ExtractedData& out) {
    if (!node)
        return;

    // Handle repetition flattening
    if (flattenReps && node->symbol == "<rep>") {
        DEBUG_MSG("DataExtractor::visit: flattening repetition node");
        // For repetitions, directly visit children without recording the <rep> node
        for (size_t i = 0; i < node->children.size(); ++i) {
            visit(node->children[i], out);
        }
        return;
    }

    // 1) Check if we should extract this symbol
    if (shouldExtract(node->symbol)) {
        DEBUG_MSG("DataExtractor::visit: extracting symbol '" + node->symbol + "' with value '" + node->matched + "'");
        out.values[node->symbol].push_back(node->matched);
    } else {
        DEBUG_MSG("DataExtractor::visit: skipping symbol '" + node->symbol + "' (filtered out)");
    }

    // 2) Continue recursively using C++98 compatible for loop
    for (size_t i = 0; i < node->children.size(); ++i) {
        visit(node->children[i], out);
    }
}
