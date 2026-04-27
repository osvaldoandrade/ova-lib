#include "../../include/memory_pool.h"

#include <stdlib.h>
#include <string.h>

/**
 * Each free block contains a pointer to the next free block.
 * This intrusive node is stored inside the block memory itself when the
 * block is not in use by the caller.
 */
typedef struct free_node {
    struct free_node *next;
} free_node;

/**
 * A singly-linked list of large allocations ("chunks").
 * Each chunk holds @c blocks_per_chunk contiguous blocks.
 */
typedef struct chunk {
    struct chunk *next;
    /* The raw allocation follows immediately after this header. */
} chunk;

/**
 * Internal memory pool representation.
 */
struct memory_pool {
    size_t      block_size;        /**< Usable size of each block.            */
    size_t      aligned_block;     /**< Actual stride (>= sizeof(free_node)). */
    int         blocks_per_chunk;  /**< Blocks allocated in each chunk.       */
    free_node  *free_list;         /**< Head of the intrusive free list.      */
    chunk      *chunks;            /**< Head of the chunk list for cleanup.   */
    size_t      total_blocks;      /**< Total blocks across all chunks.       */
    size_t      free_blocks;       /**< Blocks on the free list.              */
    void       *user_data;         /**< User-provided context pointer.        */
};

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */

/**
 * Initialise the free list for a newly allocated chunk.
 * The usable area starts right after the @c chunk header.
 */
static void init_chunk_free_list(memory_pool *pool, chunk *c) {
    unsigned char *base = (unsigned char *)(c + 1);

    for (int i = 0; i < pool->blocks_per_chunk; i++) {
        free_node *node = (free_node *)(base + (size_t)i * pool->aligned_block);
        node->next      = pool->free_list;
        pool->free_list  = node;
    }

    pool->total_blocks += (size_t)pool->blocks_per_chunk;
    pool->free_blocks  += (size_t)pool->blocks_per_chunk;
}

/**
 * Allocate a new chunk and prepend it to the chunk list.
 * Returns 0 on success, -1 on failure.
 */
static int add_chunk(memory_pool *pool) {
    size_t payload = pool->aligned_block * (size_t)pool->blocks_per_chunk;
    chunk *c = (chunk *)malloc(sizeof(chunk) + payload);
    if (!c) {
        return -1;
    }

    c->next       = pool->chunks;
    pool->chunks  = c;

    init_chunk_free_list(pool, c);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

memory_pool *create_memory_pool(size_t block_size, int initial_blocks) {
    if (block_size == 0 || initial_blocks <= 0) {
        return NULL;
    }

    memory_pool *pool = (memory_pool *)calloc(1, sizeof(memory_pool));
    if (!pool) {
        return NULL;
    }

    /* Each block must be large enough to hold a free_node pointer. */
    pool->block_size      = block_size;
    pool->aligned_block   = block_size < sizeof(free_node)
                            ? sizeof(free_node)
                            : block_size;
    pool->blocks_per_chunk = initial_blocks;
    pool->free_list        = NULL;
    pool->chunks           = NULL;
    pool->total_blocks     = 0;
    pool->free_blocks      = 0;

    if (add_chunk(pool) != 0) {
        free(pool);
        return NULL;
    }

    return pool;
}

void *memory_pool_alloc(memory_pool *pool) {
    if (!pool) {
        return NULL;
    }

    /* Grow when the free list is exhausted. */
    if (!pool->free_list) {
        if (add_chunk(pool) != 0) {
            return NULL;
        }
    }

    free_node *node = pool->free_list;
    pool->free_list = node->next;
    pool->free_blocks--;

    /* Zero-fill so the caller gets a clean block. */
    memset(node, 0, pool->block_size);
    return (void *)node;
}

void memory_pool_free(memory_pool *pool, void *ptr) {
    if (!pool || !ptr) {
        return;
    }

    free_node *node = (free_node *)ptr;
    node->next      = pool->free_list;
    pool->free_list = node;
    pool->free_blocks++;
}

size_t memory_pool_active_count(const memory_pool *pool) {
    if (!pool) {
        return 0;
    }
    return pool->total_blocks - pool->free_blocks;
}

void memory_pool_destroy(memory_pool *pool) {
    if (!pool) {
        return;
    }

    chunk *c = pool->chunks;
    while (c) {
        chunk *next = c->next;
        free(c);
        c = next;
    }

    free(pool);
}

void memory_pool_set_user_data(memory_pool *pool, void *user_data) {
    if (pool) {
        pool->user_data = user_data;
    }
}

void *memory_pool_get_user_data(const memory_pool *pool) {
    return pool ? pool->user_data : NULL;
}
