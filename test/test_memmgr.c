#include <memmgr.h>
#include <stdio.h>
#include <assert.h>

void test_memmgr_init() {
    printf("Running test_memmgr_init...\n");
    memmgr_init(nullptr, nullptr);
    memmgr_stats();
    printf("test_memmgr_init passed!\n\n");
}

void test_memmgr_alloc() {
    printf("Running test_memmgr_alloc...\n");
    constexpr size_t size = 1024;
    const mem_block_t block = memmgr_alloc(size);
    assert(block.ptr != nullptr);
    assert(block.size == size);
    printf("Allocated %zu bytes at %p\n", block.size, block.ptr);
    memmgr_free(block);
    printf("Freed memory block.\n");
    printf("test_memmgr_alloc passed!\n\n");
}

void test_memmgr_calloc() {
    printf("Running test_memmgr_calloc...\n");
    constexpr size_t num = 10;
    constexpr size_t size = 256;
    const mem_block_t block = memmgr_calloc(num, size);
    assert(block.ptr != nullptr);
    assert(block.size == num * size);
    printf("Callocated %zu bytes at %p\n", block.size, block.ptr);
    memmgr_free(block);
    printf("Freed memory block.\n");
    printf("test_memmgr_calloc passed!\n\n");
}

void test_memmgr_alloc_aligned() {
    printf("Running test_memmgr_alloc_aligned...\n");
    constexpr size_t size = 1024;
    constexpr size_t alignment = 64;
    const mem_block_t block = memmgr_alloc_aligned(size, alignment);
    assert(block.ptr != nullptr);
    assert(block.size == size);
    assert(((uintptr_t)block.ptr % alignment) == 0);
    printf("Allocated %zu bytes with alignment %zu at %p\n", block.size, alignment, block.ptr);
    memmgr_free_aligned(block);
    printf("Freed aligned memory block.\n");
    printf("test_memmgr_alloc_aligned passed!\n\n");
}

void test_memmgr_realloc() {
    printf("Running test_memmgr_realloc...\n");
    constexpr size_t initial_size = 512;
    constexpr size_t new_size = 2048;
    mem_block_t block = memmgr_alloc(initial_size);
    assert(block.ptr != nullptr);
    block = memmgr_realloc(block, new_size);
    assert(block.ptr != nullptr);
    assert(block.size == new_size);
    printf("Reallocated memory block to %zu bytes at %p\n", block.size, block.ptr);
    memmgr_free(block);
    printf("Freed reallocated memory block.\n");
    printf("test_memmgr_realloc passed!\n\n");
}

void test_memmgr_stats() {
    printf("Running test_memmgr_stats...\n");
    memmgr_stats();
    printf("test_memmgr_stats passed!\n\n");
}

void test_memmgr_debug() {
    printf("Running test_memmgr_debug...\n");
    constexpr size_t size = 1024;
    const mem_block_t block1 = memmgr_alloc(size);
    const mem_block_t block2 = memmgr_alloc(size);
    memmgr_debug();
    memmgr_free(block1);
    memmgr_free(block2);
    printf("Freed all blocks.\n");
    printf("test_memmgr_debug passed!\n\n");
}

void test_memmgr_cleanup() {
    printf("Running test_memmgr_cleanup...\n");
    memmgr_alloc(1024);
    memmgr_alloc(2048);
    memmgr_cleanup();
    printf("Cleaned up all allocated memory blocks.\n");
    memmgr_stats();
    printf("test_memmgr_cleanup passed!\n\n");
}

int main() {
    test_memmgr_init();
    test_memmgr_alloc();
    test_memmgr_calloc();
    test_memmgr_alloc_aligned();
    test_memmgr_realloc();
    test_memmgr_stats();
    test_memmgr_debug();
    test_memmgr_cleanup();
    printf("All tests passed!\n");
    return 0;
}
