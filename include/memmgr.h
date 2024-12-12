#ifndef MEMMGR_H
#define MEMMGR_H

#include <stddef.h>

/* C++11 or later? */
#if (defined(__cplusplus) && __cplusplus >= 201103L)
  #include <cstddef>
/* C2x/C23 or later? */
#elif (defined(__STDC__) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202000L))
#include <stddef.h> /* nullptr_t */
/* pre C23, pre C++11 or non-standard */
#else
  #define nullptr (void*)0
  typedef void* nullptr_t;
#endif

typedef void (*mem_error_handler_t)(const char *message);

typedef struct {
    void *ptr;
    size_t size;
} mem_block_t;

void memmgr_init(mem_error_handler_t error_handler, const char *log_path);

mem_block_t memmgr_alloc(size_t size);

mem_block_t memmgr_alloc_aligned(size_t size, size_t alignment);

mem_block_t memmgr_calloc(size_t num, size_t size);

mem_block_t memmgr_realloc(mem_block_t block, size_t size);

void memmgr_free(mem_block_t block);

void memmgr_free_aligned(mem_block_t block);

void memmgr_cleanup(void);

void memmgr_debug(void);

void memmgr_stats(void);

#endif
