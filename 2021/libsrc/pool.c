/* pool.c - A simple pool manager.
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "list.h"
#include "pool.h"
#include "debug.h"
#include "bits.h"

void pool_stats(pool_t *pool)
{
    if (pool) {
#       ifdef DEBUG_POOL
        block_t *block;

        log_f(1, "[%s] pool [%p]: blocks:%u avail:%u alloc:%u grow:%u eltsize:%lu\n",
              pool->name, (void *)pool, pool->nblocks, pool->available, pool->allocated,
              pool->growsize, pool->eltsize);
        log(5, "\tblocks: ");
        list_for_each_entry(block, &pool->list_blocks, list_blocks) {
            log(5, "%p ", block);
        }
        log(5, "\n");
#       endif
    }
}

pool_t *pool_create(const char *name, u32 growsize, size_t eltsize)
{
    pool_t *pool;

#   ifdef DEBUG_POOL
    log_f(1, "name=[%s] growsize=%u eltsize=%lu\n",
           name, growsize, eltsize);
#   endif
    /* we need at least this space in struct */
    if (eltsize < sizeof (struct list_head))
        return NULL;
    if ((pool = malloc(sizeof (*pool)))) {
        pool->name = strdup(name);
        pool->growsize = growsize;
        pool->eltsize = eltsize;
        pool->available = 0;
        pool->allocated = 0;
        pool->nblocks = 0;
        INIT_LIST_HEAD(&pool->list_available);
        INIT_LIST_HEAD(&pool->list_blocks);
    }
    return pool;
}

static u32 _pool_add(pool_t *pool, struct list_head *elt)
{
#   ifdef DEBUG_POOL
    log_f(6, "pool=%p &head=%p elt=%p off1=%lu off2=%lu\n",
           (void *)pool,
           (void *)&pool->list_available,
           (void *)elt,
           (void *)&pool->list_available-(void *)pool,
           offsetof(pool_t, list_available));
#   endif

    list_add(elt, &pool->list_available);
    return ++pool->available;
}

u32 pool_add(pool_t *pool, void *elt)
{
    return _pool_add(pool, elt);
}

static struct list_head *_pool_get(pool_t *pool)
{
    struct list_head *res = pool->list_available.next;
    pool->available--;
    list_del(res);
    return res;
}

void *pool_get(pool_t *pool)
{
    if (!pool)
        return NULL;
    if (!pool->available) {
        block_t *block = malloc(sizeof(block_t) + pool->eltsize * pool->growsize);
        void *cur;
        u32 i;

        if (!block) {
#           ifdef DEBUG_POOL
            log_f(1, "[%s]: failed block allocation\n");
#           endif
            return NULL;
        }

        /* maintain list of allocated blocks
         */
        list_add(&block->list_blocks, &pool->list_blocks);
        pool->nblocks++;

#       ifdef DEBUG_POOL
        log_f(1, "[%s]: growing pool from %u to %u elements. block=%p nblocks=%u\n",
              pool->name,
              pool->allocated,
              pool->allocated + pool->growsize,
              block,
              pool->nblocks);
#       endif

        pool->allocated += pool->growsize;
        for (i = 0; i < pool->growsize; ++i) {
            cur = block->data + i * pool->eltsize;
#           ifdef DEBUG_POOL
            log_f(7, "alloc=%p cur=%p\n", block, cur);
#           endif
            _pool_add(pool, (struct list_head *)cur);
        }
        //pool_stats(pool);
    }
    /* this is the effective address if the object (and also the
     * pool list_head address)
     */
    return _pool_get(pool);
}

void pool_destroy(pool_t *pool)
{
    block_t *block, *tmp;
    if (!pool)
        return;
    /* release memory blocks */
#   ifdef DEBUG_POOL
    log_f(1, "[%s]: releasing %d blocks and main structure\n", pool->name, pool->nblocks);
    log(5, "blocks:");
#   endif
    list_for_each_entry_safe(block, tmp, &pool->list_blocks, list_blocks) {
        list_del(&block->list_blocks);
        free(block);
#       ifdef DEBUG_POOL
        log(5, " %p", block);
#       endif
    }
#   ifdef DEBUG_POOL
    log(5, "\n");
#   endif
    free(pool->name);
    free(pool);
}

#ifdef BIN_pool
struct d {
    u16 data1;
    char c;
    struct list_head list;
};

static LIST_HEAD (head);

int main(int ac, char**av)
{
    pool_t *pool;
    int total;
    int action=0;
    u16 icur=0;
    char ccur='z';
    struct d *elt;

    debug_init(3);
    log_f(1, "%s: sizeof(d)=%lu sizeof(*d)=%lu off=%lu\n", *av, sizeof(elt),
           sizeof(*elt), offsetof(struct d, list));

    if ((pool = pool_create("dummy", 3, sizeof(*elt)))) {
        pool_stats(pool);
        for (int cur=1; cur<ac; ++cur) {
            total = atoi(av[cur]);
            if (action == 0) {                    /* add elt to list */
                log_f(2, "adding %d elements\n", total);
                for (int i = 0; i < total; ++i) {
                    elt = pool_get(pool);
                    elt->data1 = icur++;
                    elt->c = ccur--;
                    list_add(&elt->list, &head);
                }
                pool_stats(pool);
                action = 1;
            } else {                              /* remove one elt from list */
                log_f(2, "deleting %d elements\n", total);
                for (int i = 0; i < total; ++i) {
                    if (!list_empty(&head)) {
                        elt = list_last_entry(&head, struct d, list);
                        printf("elt=[%d, %c]\n", elt->data1, elt->c);
                        list_del(&elt->list);
                        pool_add(pool, elt);
                    }
                }
                pool_stats(pool);
                action = 0;
            }
        }
    }
    pool_stats(pool);
}
#endif
