/*-
 * Copyright (c) 2025 Alfredo Mazzinghi
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This software was developed by SRI International, the University of
 * Cambridge Computer Laboratory (Department of Computer Science and
 * Technology), and Capabilities Limited under Defense Advanced Research
 * Projects Agency (DARPA) Contract No. FA8750-24-C-B047 ("DEC").
 */

#include <assert.h>
#include <cheriintrin.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/queue.h>
#include <cheri/cherireg.h>

#include "thunk.h"

struct block {
        TAILQ_ENTRY(block) blk_link;
        void *blk_root_cap;
};

pthread_mutex_t block_list_mutex = PTHREAD_MUTEX_INITIALIZER;
TAILQ_HEAD(block_head, block) block_list;

/*
 * For testing, thunk_xmalloc rounds up allocation up to a page
 * and maintains a global list of allocations.
 * This is not designed to be a good allocator, just as a fast way to
 * get some memory rolling.
 */

void *
thunk_xmalloc(size_t size)
{
        struct block *blk;

        blk = malloc(sizeof(*blk));
        if (blk == NULL) {
                return (NULL);
        }
        blk->blk_root_cap = mmap(NULL, size,
            PROT_READ | PROT_WRITE | PROT_EXEC | PROT_CAP,
            MAP_ANON | MAP_PRIVATE, -1, 0);
        if (blk->blk_root_cap == MAP_FAILED) {
                free(blk);
                return (NULL);
        }
        pthread_mutex_lock(&block_list_mutex);
        TAILQ_INSERT_HEAD(&block_list, blk, blk_link);
        pthread_mutex_unlock(&block_list_mutex);

        return (cheri_perms_clear(blk->blk_root_cap, CHERI_PERM_SW_VMEM));
}

void
thunk_xfree(void *ptr)
{
        struct block *blk;
        int rv;

        if (ptr == NULL)
                return;

        assert(cheri_is_valid(ptr) && "Attempt to free invalid capability");
        assert(cheri_is_unsealed(ptr) && "Attempt to free sealed capability");

        pthread_mutex_lock(&block_list_mutex);
        TAILQ_FOREACH(blk, &block_list, blk_link) {
                if (blk->blk_root_cap == ptr) {
                        assert(cheri_is_subset(blk->blk_root_cap, ptr) &&
                            "Freed capability is not a subset of the root");
                        break;
                }
        }
        assert(blk != NULL && "Invalid pointer to free");
        TAILQ_REMOVE(&block_list, blk, blk_link);
        pthread_mutex_unlock(&block_list_mutex);

        rv = munmap(blk->blk_root_cap, cheri_length_get(blk->blk_root_cap));
        assert(rv == 0 && "Failed to munmap memory");
        free(blk);
}
