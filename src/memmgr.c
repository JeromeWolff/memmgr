#include "memmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct {
    mem_block_t block;
    const char *file;
    int line;
} mem_tracker_t;

#define MAXIMAL_BLOCKS 1024

static mem_tracker_t allocated_blocks[MAXIMAL_BLOCKS];
static size_t block_count = 0;
static size_t allocated_bytes = 0;
static size_t total_freed_bytes = 0;

static mem_error_handler_t global_error_handler = nullptr;
static FILE *log_file = nullptr;
#ifdef _WIN32
static CRITICAL_SECTION mem_mutex;
#else
static pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static void default_global_error_handler(const char *message) {
    fprintf(stderr, "Memory Error: %s\n", message);
    exit(EXIT_FAILURE);
}

static void log_message(const char *message) {
    if (log_file) {
        fprintf(log_file, "%s\n", message);
        fflush(log_file);
    } else {
        printf("%s\n", message);
    }
}

void memmgr_init(const mem_error_handler_t error_handler, const char *log_path) {
    global_error_handler = error_handler ? error_handler : default_global_error_handler;
    block_count = 0;
    allocated_bytes = 0;
    total_freed_bytes = 0;
    memset(allocated_blocks, 0, sizeof(allocated_blocks));
    if (log_path) {
        log_file = fopen(log_path, "a");
        if (!log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", log_path);
        }
    }
#ifdef _WIN32
    InitializeCriticalSection(&mem_mutex);
#endif
    atexit(memmgr_cleanup);
}

static void lock_mutex() {
#ifdef _WIN32
    EnterCriticalSection(&mem_mutex);
#else
    pthread_mutex_lock(&mem_mutex);
#endif
}

static void unlock_mutex() {
#ifdef _WIN32
    LeaveCriticalSection(&mem_mutex);
#else
    pthread_mutex_unlock(&mem_mutex);
#endif
}

mem_block_t memmgr_alloc(const size_t size) {
    lock_mutex();
    if (block_count >= MAXIMAL_BLOCKS) {
        global_error_handler("Exceeded maximal allocation limit");
        unlock_mutex();
        return (mem_block_t){nullptr, 0};
    }
    void *ptr = malloc(size);
    if (!ptr) {
        global_error_handler("Failed to allocate memory");
        unlock_mutex();
        return (mem_block_t){nullptr, 0};
    }
    const mem_block_t block = {ptr, size};
    allocated_blocks[block_count++] = (mem_tracker_t){block, __FILE__, __LINE__};
    allocated_bytes += size;
    log_message("Allocated memory block");
    unlock_mutex();
    return block;
}

void *aligned_alloc_wrapper(size_t size, size_t alignment) {
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    void* ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
#endif
}

mem_block_t memmgr_alloc_aligned(const size_t size, const size_t alignment) {
    lock_mutex();
    if (block_count >= MAXIMAL_BLOCKS) {
        global_error_handler("Exceeded maximal allocation limit");
        unlock_mutex();
        return (mem_block_t){nullptr, 0};
    }
    void *ptr = aligned_alloc_wrapper(size, alignment);
    if (!ptr) {
        global_error_handler("Failed to allocate aligned memory");
        unlock_mutex();
        return (mem_block_t){nullptr, 0};
    }
    const mem_block_t block = {ptr, size};
    allocated_blocks[block_count++] = (mem_tracker_t){block, __FILE__, __LINE__};
    allocated_bytes += size;
    log_message("Allocated aligned memory block");
    unlock_mutex();
    return block;
}

mem_block_t memmgr_calloc(const size_t num, const size_t size) {
    lock_mutex();
    if (block_count >= MAXIMAL_BLOCKS) {
        global_error_handler("Exceeded maximal allocation limit");
        unlock_mutex();
        return (mem_block_t){nullptr, 0};
    }
    void *ptr = calloc(num, size);
    if (!ptr) {
        global_error_handler("Failed to allocate memory");
        unlock_mutex();
        return (mem_block_t){nullptr, 0};
    }
    const mem_block_t block = {ptr, num * size};
    allocated_blocks[block_count++] = (mem_tracker_t){block, __FILE__, __LINE__};
    allocated_bytes += num * size;
    log_message("Allocated memory block");
    unlock_mutex();
    return block;
}

mem_block_t memmgr_realloc(const mem_block_t block, const size_t size) {
    lock_mutex();
    for (size_t index = 0; index < block_count; ++index) {
        if (allocated_blocks[index].block.ptr == block.ptr) {
            void *new_ptr = realloc(block.ptr, size);
            if (!new_ptr) {
                global_error_handler("Failed to reallocate memory");
                unlock_mutex();
                return (mem_block_t){nullptr, 0};
            }
            allocated_bytes += size - block.size;
            allocated_blocks[index].block.ptr = new_ptr;
            allocated_blocks[index].block.size = size;
            log_message("Reallocated memory block");
            unlock_mutex();
            return (mem_block_t){new_ptr, size};
        }
    }
    global_error_handler("Failed to reallocate memory");
    unlock_mutex();
    return (mem_block_t){nullptr, 0};
}

void memmgr_free(const mem_block_t block) {
    lock_mutex();
    for (size_t index = 0; index < block_count; ++index) {
        if (allocated_blocks[index].block.ptr == block.ptr) {
            free(block.ptr);
            total_freed_bytes += allocated_blocks[index].block.size;
            allocated_blocks[index] = allocated_blocks[--block_count];
            log_message("Freed memory block");
            unlock_mutex();
            return;
        }
    }
    global_error_handler("Failed to free memory");
    unlock_mutex();
}

void aligned_free_wrapper(void *ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

void memmgr_free_aligned(mem_block_t block) {
    lock_mutex();
    for (size_t index = 0; index < block_count; ++index) {
        if (block.ptr == allocated_blocks[index].block.ptr) {
            aligned_free_wrapper(allocated_blocks[index].block.ptr);
            total_freed_bytes += allocated_blocks[index].block.size;
            allocated_blocks[index] = allocated_blocks[--block_count];
            log_message("Freed aligned memory block");
            unlock_mutex();
            return;
        }
    }
    global_error_handler("Failed to free aligned memory");
    unlock_mutex();
}

void memmgr_cleanup(void) {
    lock_mutex();
    for (size_t index = 0; index < block_count; ++index) {
        free(allocated_blocks[index].block.ptr);
        total_freed_bytes += allocated_blocks[index].block.size;
        log_message("Freed remaining memory blocks during cleanup");
    }
    block_count = 0;
    if (log_file) {
        fclose(log_file);
        log_file = nullptr;
    }
    unlock_mutex();
}

void memmgr_debug(void) {
    lock_mutex();
    printf("Active memory blocks:\n");
    for (size_t index = 0; index < block_count; ++index) {
        printf("Block %zu: %p (%zu bytes)\n",
               index + 1,
               allocated_blocks[index].block.ptr,
               allocated_blocks[index].block.size
        );
    }
}

void memmgr_stats(void) {
    lock_mutex();
    printf("Allocated memory: %llu bytes\n", allocated_bytes);
    printf("Freed memory: %llu bytes\n", total_freed_bytes);
    printf("Total memory: %llu bytes\n", allocated_bytes + total_freed_bytes);
    printf("Memory usage: %.2f%%\n", (double) allocated_bytes / (double) (allocated_bytes + total_freed_bytes) * 100);
    unlock_mutex();
}
