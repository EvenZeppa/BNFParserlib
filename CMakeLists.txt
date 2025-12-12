cmake_minimum_required(VERSION 3.10)
project(BNFParserLib VERSION 1.0.0 LANGUAGES CXX)

# Set C++98 standard
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -g")

# Include directories
include_directories(include)

# Collect source files for the library (exclude main.cpp if it exists)
file(GLOB LIB_SOURCES "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Create static library
add_library(bnf STATIC ${LIB_SOURCES})

# Set library properties
set_target_properties(bnf PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/AST.hpp;include/BNFParser.hpp;include/BNFTokenizer.hpp;include/DataExtractor.hpp;include/Debug.hpp;include/Expression.hpp;include/ExtractedData.hpp;include/Grammar.hpp;include/TestFramework.hpp"
)

# Optional: Build examples if they exist (can be toggled)
option(BNFPARSER_BUILD_EXAMPLES "Build example executables" ON)
file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
if(BNFPARSER_BUILD_EXAMPLES AND EXAMPLE_SOURCES)
    foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        target_link_libraries(${EXAMPLE_NAME} bnf)
        set_target_properties(${EXAMPLE_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        )
        # Keep example builds consistent with the library's warning level
        target_compile_options(${EXAMPLE_NAME} PRIVATE -Wall -Wextra -Werror)
    endforeach()
endif()

# Enable testing
enable_testing()

# Collect test files
file(GLOB TEST_SOURCES "tests/*.cpp")

# Create test executables
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    set(TEST_EXECUTABLE "${TEST_NAME}_runner")
    
    add_executable(${TEST_EXECUTABLE} ${TEST_SOURCE})
    target_link_libraries(${TEST_EXECUTABLE} bnf)
    
    # Set test output directory
    set_target_properties(${TEST_EXECUTABLE} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )
    
    # Register test with CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_EXECUTABLE})
    
    # Set test properties for better output
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30
        WILL_FAIL FALSE
    )
endforeach()

# Installation rules
install(TARGETS bnf
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/bnfparser
)

# Install examples (optional)
if(EXAMPLE_SOURCES)
    install(DIRECTORY examples/
        DESTINATION share/bnfparser/examples
        FILES_MATCHING PATTERN "*.cpp"
    )
endif()

# Optional: Create a config file for find_package support
# Commented out for now as it requires additional setup
# include(CMakePackageConfigHelpers)
# configure_package_config_file(...)

# Print build information
message(STATUS "BNFParserLib Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: C++98")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Library Sources: ${LIB_SOURCES}")
if(TEST_SOURCES)
    message(STATUS "  Test Files: ${TEST_SOURCES}")
endif()
if(EXAMPLE_SOURCES)
    message(STATUS "  Example Files: ${EXAMPLE_SOURCES}")
endif()
