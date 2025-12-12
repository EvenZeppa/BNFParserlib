#include "../include/AST.hpp"
#include "../include/Debug.hpp"

// ASTNode implementation
ASTNode::ASTNode(const std::string& s) : symbol(s) {
    DEBUG_MSG("ASTNode created: '" << s << "'");
}

// Destructor recursively deletes all child nodes to prevent memory leaks
ASTNode::~ASTNode() {
    DEBUG_MSG("ASTNode destroyed: '" << symbol << "' with " << children.size() << " children");
    for (size_t i = 0; i < children.size(); ++i)
        delete children[i];
}

// Helper function to print indentation for hierarchical display
static void printIndent(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  "; // two spaces per level
}

// Print AST structure in a readable hierarchical format
void printAST(const ASTNode* node, int indent) {
    if (!node) {
        printIndent(indent);
        std::cout << "(null)\n";
        return;
    }

    printIndent(indent);

    // Display the node symbol/name
    std::cout << node->symbol;

    // Show matched text if available (useful for understanding repetitions and alternatives)
    if (!node->matched.empty())
        std::cout << "  [matched=\"" << node->matched << "\"]";

    std::cout << "\n";

    // Recursively display all children with increased indentation
    for (size_t i = 0; i < node->children.size(); ++i)
        printAST(node->children[i], indent + 1);
}
