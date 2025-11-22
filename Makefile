# Compiler
CXX = g++
# CONSTANTS = -DDEBUG_ACTIVE=0 -DDEBUG_GRAMMAR
CONSTANTS =
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude $(CONSTANTS)

# Directories
SRC_DIR = src
OBJ_DIR = obj
TESTS_DIR = tests

# Source files (exclude main.cpp for tests)
SRC = $(wildcard $(SRC_DIR)/*.cpp)
SRC_APP = $(SRC_DIR)/main.cpp
SRC_LIB = $(filter-out $(SRC_APP),$(SRC))

OBJ_LIB = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_LIB))
OBJ_APP = $(OBJ_LIB) $(OBJ_DIR)/main.o

# Test sources
TESTS = $(wildcard $(TESTS_DIR)/*.cpp)
TEST_EXEC = $(patsubst $(TESTS_DIR)/%.cpp,%_runner,$(TESTS))

# Default target
all: bnf_parser

# Build main application
bnf_parser: $(OBJ_APP)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test objects
$(OBJ_DIR)/%.o: $(TESTS_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generic test runners
%_runner: $(OBJ_LIB) $(OBJ_DIR)/%.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ > /dev/null 2>&1
	./$@

# Run all tests
tests: $(TEST_EXEC)
	@for t in $(TEST_EXEC); do ./$$t; done

# Clean
clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f bnf_parser $(TEST_EXEC)

re: fclean all

.PHONY: all clean fclean re tests
