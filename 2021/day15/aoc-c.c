/* aoc-c.c: Advent of Code 2021, day 15 parts 1 & 2
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdint.h>

#include "debug.h"
#include "pool.h"
#include "bits.h"
#include "list.h"

#define MAX_SIZE 100

typedef struct square {
    uchar cost;
    uchar visited;
    u32   totcost;;
} square_t;

typedef struct priority_queue {
    u16 l;
    u16 c;
    struct list_head list;
} pqueue_t;

static LIST_HEAD(plist_head);
static struct square array[MAX_SIZE*5][MAX_SIZE*5];
static int asize;
static pool_t *pool;

/* read data and create graph.
 */
static int read_input()
{
    size_t alloc = 0;
    char *buf;
    ssize_t len;
    int l = 0, c;

    /* get points list */
    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        buf[--len] = 0;

        for (c = 0; buf[c]; ++c) {
            array[l][c].cost = buf[c] - '0';
            array[l][c].totcost = UINT32_MAX;
            array[l][c].visited = 0;
        }
        l++;
    }
    free(buf);
    asize = l;
    return asize;
}

/* insert l,c in queue */
inline static pqueue_t *push(int l, int c, u32 parentcost)
{
    pqueue_t *queue = NULL;
    u32 newcost;

    if (array[l][c].visited)
        return NULL;
    newcost = parentcost + array[l][c].cost;
    if (newcost > array[l][c].totcost)
        return NULL;
    array[l][c].totcost = newcost;
    queue = pool_get(pool);
    queue->l = l;
    queue->c = c;
    list_add_tail(&queue->list, &plist_head);
    return queue;
}

inline static pqueue_t *pop()
{
    pqueue_t *pmin, *tmp, *cur;
    u32 min = UINT32_MAX;

    if (!(pmin = list_first_entry_or_null(&plist_head, pqueue_t, list)))
        return NULL;
    list_for_each_entry_safe(cur, tmp, &plist_head, list) {
        if (array[cur->l][cur->c].totcost < min) {
            pmin = cur;
            min = array[cur->l][cur->c].totcost;
        }
    }
    list_del(&pmin->list);
    return pmin;
}


static u32 doit()
{
    pqueue_t *pqueue;

    push(0, 0, 0);
    while ((pqueue = pop())) {
        int l = pqueue->l, c = pqueue->c;
        u32 acc = array[l][c].totcost;

        if (array[l][c].visited)
            goto free_pqueue;
        if (c > 0)
            push(l, c - 1, acc);
        if (c < asize - 1)
            push(l, c + 1, acc);
        if (l > 0)
            push(l - 1, c, acc);
        if (l < asize - 1)
            push(l + 1, c, acc);
        array[l][c].visited = 1;
    free_pqueue:
        pool_add(pool, pqueue);                   /* recycle pqueue in memory pool */
    }

    /* as we accounted [0][0] cost, we must substract it */
    return array[asize - 1][asize - 1].totcost - array[0][0].cost;
}

static u32 part1()
{
    return doit();
}

static u32 part2()
{
    int l, c;

    /* initialize the rest of array */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (i || j) {
                for (l = 0; l < asize; ++l) {
                    for (c = 0; c < asize; ++c) {
                        int newl = l + i * asize;
                        int newc = c + j * asize;
                        array[newl][newc] = array[l][c];
                        array[newl][newc].cost += i + j;
                        if (array[newl][newc].cost > 9) {
                            array[newl][newc].cost -= 9;
                        }
                        /* invalid, but it works with 75% gain
                         * if (j < i)
                         *     array[newl][newc].visited = 1;
                         */
                    }
                }
            }
        }
    }
    asize *= 5;
    return doit();
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                part = atoi(optarg);
                if (part < 1 || part > 2)
                    return usage(*av);
                break;
            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    if (!(pool = pool_init("stack", 1024, sizeof (pqueue_t))))
        return -1;

    read_input();
    printf("%s : res=%u\n", *av, part == 1? part1(): part2());
    /* pool_stats(pool); */
    pool_destroy(pool);
    exit (0);
}
