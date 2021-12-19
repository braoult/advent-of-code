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

int a[] = {
    ['a'] = 0,
    ['b'] = 1,
    ['c'] = 2
};

typedef struct square {
    uchar cost;
    uchar visited;
    u64   totcost;;
} square_t;

typedef struct priority_queue {
    int l;
    int c;
    struct list_head list;
} pqueue_t;

static LIST_HEAD(plist_head);
static struct square array[MAX_SIZE*5][MAX_SIZE*5];
static int asize;
static pool_t *pool;

#define VALID(x, y) ((x) >= 0 && (x) < size && (y) >= 0 && (y) < size )

static void print_array()
{
    log(3, "array (%dx%d):\n", asize, asize);
    for (int i = 0; i < asize; ++i) {
        for (int j = 0; j < asize; ++j)
            log(3, "%1d ", array[i][j].cost);
        //log(3, "%1d/%2d ", array[i][j].cost, array[i][j].totcost);
        log(3, "\n");
    }
}
static void print_array_total()
{
    log(3, "array (%dx%d):\n", asize, asize);
    for (int i = 0; i < asize; ++i) {
        for (int j = 0; j < asize; ++j)
            log(3, "%4d ", array[i][j].totcost);
        //log(3, "%1d/%2d ", array[i][j].cost, array[i][j].totcost);
        log(3, "\n");
    }
}

static void print_queue()
{
    log(3, "queue:\n", asize, asize);
    pqueue_t *tmp;
    int i = 1;

    list_for_each_entry(tmp, &plist_head, list) {
        int l = tmp->l, c = tmp->c;
        u64 cost = array[l][c].cost;
        u64 acc = array[l][c].totcost;
        log(3, "%d: (%d,%d): cost=%lu acc=%lu\n", i, l, c, cost, acc);
        i++;
    }
}

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

        //printf("%d: size=%d [%s]\n", l, asize, buf);
        for (c = 0; buf[c]; ++c) {
            array[l][c].cost = buf[c] - '0';
            array[l][c].totcost = UINT64_MAX;
            array[l][c].visited = 0;
        }
        l++;
    }
    free(buf);
    asize = l;
    //print_array();
    return asize;
}

/* insert l,c in queue, keeping cost sorted */
inline static pqueue_t *push(int l, int c, u32 parentcost)
{
    pqueue_t *queue;
    u64 newcost;

    if (l < 0 || c < 0 || l >= asize || c >= asize)
        return NULL;
    newcost = parentcost + array[l][c].cost;
    if (newcost > array[l][c].totcost)
        return NULL;

    queue = pool_get(pool);
    queue->l = l;
    queue->c = c;
    array[l][c].totcost = newcost;

    log_f(3, "(%d,%d) pcost=%u\n", l, c, newcost);
    list_add_tail(&queue->list, &plist_head);
    return queue;
}

inline static pqueue_t *pop()
{
    pqueue_t *min;
    pqueue_t *tmp, *cur;

    if (!(min = list_first_entry_or_null(&plist_head, pqueue_t, list)))
        return NULL;
    list_for_each_entry_safe(cur, tmp, &plist_head, list) {
        int l = cur->l, c = cur->c;
        if (array[l][c].totcost < array[min->l][min->c].totcost)
            min = cur;
    }
    list_del(&min->list);
    //if (array[l][c].visited) {
    //    pool_add(pool, cur);
    //    continue;
    //}
    log_f(3, "(%d,%d) cost=%u\n", min->l, min->c, array[min->l][min->c].totcost);
    return min;
//    if ((p = list_first_entry_or_null(&plist_head, pqueue_t, list)))
    //      list_del(&p->list);
    //return p;
}


static u64 part1()
{
    pqueue_t *pqueue;

    push(0, 0, 0);
    print_array();
    while ((pqueue = pop())) {
        int l = pqueue->l, c = pqueue->c;
        u64 acc = array[l][c].totcost;
        bool stop = true;

        if (array[l][c].visited)
            goto free_pqueue;
        if (push(l, c - 1, acc))
            stop = false;
        if (push(l, c + 1, acc))
            stop = false;
        if (push(l - 1, c, acc))
            stop = false;
        if (push(l + 1, c, acc))
            stop = false;
        array[l][c].visited = 1;
        //if (stop)
        //    break;
    free_pqueue:
        pool_add(pool, pqueue);                   /* recycle pqueue in memory pool */
    }

    /* as we accounted [0][0] cost, we must substract it */
    print_array();
    print_array_total();
    return array[asize - 1][asize - 1].totcost - array[0][0].cost;
}

static u64 part2()
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
                        //log(3, "new(%d, %d) = %u\n", newl, newc, array[newl][newc].cost);
                    }
                }
            }
        }
    }
    asize *= 5;
    return part1();
}

static u64 doit(int part)
{
    read_input();

    return part == 1? part1(): part2();
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

    printf("%s : res=%lu\n", *av, doit(part));
    exit (0);
}
