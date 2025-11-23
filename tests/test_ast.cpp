#include "../include/TestFramework.hpp"
#include "../include/AST.hpp"
#include <sstream>
#include <iostream>

/**
 * @brief Test AST node creation functionality.
 */
void test_node_creation(TestRunner& runner) {
    ASTNode* node = new ASTNode("root");
    ASSERT_EQ(runner, node->symbol, "root");
    ASSERT_TRUE(runner, node->matched.empty());
    ASSERT_EQ(runner, node->children.size(), 0);
    delete node;
}

/**
 * @brief Test AST node with matched text.
 */
void test_node_with_match(TestRunner& runner) {
    ASTNode* node = new ASTNode("letter");
    node->matched = "A";
    ASSERT_EQ(runner, node->symbol, "letter");
    ASSERT_EQ(runner, node->matched, "A");
    delete node;
}

/**
 * @brief Test adding children to AST nodes.
 */
void test_add_children(TestRunner& runner) {
    ASTNode* root = new ASTNode("root");
    ASTNode* child1 = new ASTNode("child1");
    ASTNode* child2 = new ASTNode("child2");

    root->children.push_back(child1);
    root->children.push_back(child2);

    ASSERT_EQ(runner, root->children.size(), 2);
    ASSERT_EQ(runner, root->children[0]->symbol, "child1");
    ASSERT_EQ(runner, root->children[1]->symbol, "child2");

    delete root; // doit aussi delete les enfants
}

/**
 * @brief Test nested AST tree structures.
 */
void test_nested_tree(TestRunner& runner) {
    ASTNode* root = new ASTNode("root");
    ASTNode* branch = new ASTNode("branch");
    ASTNode* leaf = new ASTNode("leaf");

    branch->children.push_back(leaf);
    root->children.push_back(branch);

    ASSERT_EQ(runner, root->children.size(), 1);
    ASSERT_EQ(runner, root->children[0]->children.size(), 1);
    ASSERT_EQ(runner, root->children[0]->children[0]->symbol, "leaf");

    delete root; // doit delete tous les enfants
}

/**
 * @brief Test AST printing functionality (ensures printAST doesn't crash).
 */
void test_printAST(TestRunner& runner) {
    ASTNode* root = new ASTNode("root");
    ASTNode* child = new ASTNode("child");
    child->matched = "X";
    root->children.push_back(child);

    std::ostringstream oss;
    std::streambuf* oldCout = std::cout.rdbuf(oss.rdbuf());
    printAST(root, 0);
    std::cout.rdbuf(oldCout);

    std::string output = oss.str();
    ASSERT_TRUE(runner, output.find("root") != std::string::npos);
    ASSERT_TRUE(runner, output.find("child") != std::string::npos);
    ASSERT_TRUE(runner, output.find("X") != std::string::npos);

    delete root;
}

int main() {
    TestSuite suite("AST Test Suite");
    
    // Register all test functions
    suite.addTest("Node Creation", test_node_creation);
    suite.addTest("Node with Match", test_node_with_match);
    suite.addTest("Add Children", test_add_children);
    suite.addTest("Nested Tree", test_nested_tree);
    suite.addTest("Print AST", test_printAST);
    
    // Run all tests
    TestRunner results = suite.run();
    results.printSummary();
    
    return results.allPassed() ? 0 : 1;
}
