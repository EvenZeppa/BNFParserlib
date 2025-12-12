#include "../include/Arena.hpp"
#include <cstdlib>
#include <new>

Arena::Arena(std::size_t blockSize) : defaultBlockSize(blockSize) {
    blocks.reserve(4);
}

Arena::~Arena() {
    for (std::size_t i = 0; i < blocks.size(); ++i) {
        std::free(blocks[i].data);
    }
}

void Arena::addBlock(std::size_t minSize) {
    std::size_t size = minSize > defaultBlockSize ? minSize : defaultBlockSize;
    char* mem = static_cast<char*>(std::malloc(size));
    Block b; b.data = mem; b.used = 0; b.size = size;
    blocks.push_back(b);
}

void* Arena::allocate(std::size_t size, std::size_t alignment) {
    if (alignment < sizeof(void*)) alignment = sizeof(void*);
    // Try current block
    if (blocks.empty()) addBlock(size);
    Block& blk = blocks.back();
    // Align current pointer
    std::size_t base = reinterpret_cast<std::size_t>(blk.data);
    std::size_t curr = base + blk.used;
    std::size_t aligned = (curr + (alignment - 1)) & ~(alignment - 1);
    std::size_t offset = aligned - base;
    if (offset + size > blk.size) {
        // Need new block
        addBlock(size + alignment);
        Block& nb = blocks.back();
        base = reinterpret_cast<std::size_t>(nb.data);
        aligned = (base + (alignment - 1)) & ~(alignment - 1);
        offset = aligned - base;
        if (offset + size > nb.size) return 0; // allocation failed
        nb.used = offset + size;
        return nb.data + offset;
    }
    blk.used = offset + size;
    return blk.data + offset;
}

void Arena::reset() {
    for (std::size_t i = 0; i < blocks.size(); ++i) {
        blocks[i].used = 0;
    }
}
