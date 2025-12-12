#ifndef ARENA_HPP
#define ARENA_HPP

#include <cstddef>
#include <vector>

/**
 * @brief Simple bump-pointer arena allocator with fixed-size blocks.
 *
 * Allocations are not individually freed; memory is released when the arena
 * is destroyed or reset(). Suitable for AST/Expression lifetimes.
 */
class Arena {
public:
    explicit Arena(std::size_t blockSize = 4096);
    ~Arena();

    void* allocate(std::size_t size, std::size_t alignment = sizeof(void*));

    void reset();

private:
    struct Block { char* data; std::size_t used; std::size_t size; };
    std::vector<Block> blocks;
    std::size_t defaultBlockSize;

    void addBlock(std::size_t minSize);
};

#endif // ARENA_HPP
