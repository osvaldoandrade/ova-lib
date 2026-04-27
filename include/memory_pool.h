#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include "types.h"

/**
 * @file memory_pool.h
 * @brief Fixed-size memory pool allocator.
 *
 * Pre-allocates contiguous chunks of memory divided into equal-sized blocks,
 * providing O(1) allocation and deallocation via a free list.  When the pool
 * is exhausted, a new chunk is allocated transparently.
 */

/**
 * @brief Opaque memory pool type.
 */
typedef struct memory_pool memory_pool;

/**
 * @brief Create a new memory pool.
 *
 * @param block_size Size in bytes of each block that will be handed out.
 *                   Must be greater than zero.
 * @param initial_blocks Number of blocks to pre-allocate.
 *                       Must be greater than zero.
 * @return New memory pool instance, or NULL on invalid parameters or
 *         allocation failure.
 */
memory_pool *create_memory_pool(size_t block_size, int initial_blocks);

/**
 * @brief Allocate a block from the pool.
 *
 * If the free list is empty, a new chunk of the same size as the initial
 * allocation is added automatically.
 *
 * @param pool Memory pool instance.
 * @return Pointer to a block of at least @p block_size bytes,
 *         or NULL on failure.
 */
void *memory_pool_alloc(memory_pool *pool);

/**
 * @brief Return a previously allocated block to the pool.
 *
 * The caller must only pass pointers obtained from memory_pool_alloc()
 * on the same pool.  Passing NULL is a safe no-op.
 *
 * @param pool Memory pool instance.
 * @param ptr  Block pointer to recycle.
 */
void memory_pool_free(memory_pool *pool, void *ptr);

/**
 * @brief Return the number of blocks currently in use.
 *
 * @param pool Memory pool instance.
 * @return Number of allocated (in-use) blocks, or 0 if pool is NULL.
 */
size_t memory_pool_active_count(const memory_pool *pool);

/**
 * @brief Destroy the pool and release all backing memory.
 *
 * Every chunk allocated by the pool is freed.  After this call the pool
 * pointer is invalid and must not be used.
 *
 * @param pool Memory pool instance.  NULL is a safe no-op.
 */
void memory_pool_destroy(memory_pool *pool);

#endif /* MEMORY_POOL_H */
