/* aoc-c: Advent of Code 2021, day 9 parts 1 & 2
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

#include "debug.h"
#include "bits.h"
#include "list.h"
#include "pool.h"

#define NLINES 1024                               /* alloc size */

struct data {
    int nlines;                                   /* current read lines */
    int alloc;                                    /* allocated lines */
    int length;                                   /* line length */
    char **lines;
};

#ifdef DEBUG
static void print_data(struct data *data)
{
    for (int i = 0; i < data->nlines; ++i)
        printf("[%03d]%s\n", i, data->lines[i]);
}
#endif

static struct data *read_file()
{
    size_t alloc = 0;
    static struct data data;
    ssize_t len;

    data.alloc = NLINES;
    if (!(data.lines = malloc(data.alloc * sizeof (*data.lines))))
        return NULL;

    while ((len = getline(&data.lines[data.nlines], &alloc, stdin)) > 0) {
        data.length = len - 1;
        data.lines[data.nlines][len - 1] = 0;
        if (data.nlines == data.alloc) {
            data.alloc += NLINES;
            if (!(data.lines = realloc(data.lines, data.alloc * sizeof (*data.lines))))
                return NULL;
        }
        data.nlines++;
    }
    return &data;
}

#define UP(l, c)    (data->lines[(l) - 1][c])
#define DOWN(l, c)  (data->lines[(l) + 1][c])
#define LEFT(l, c)  (data->lines[l][(c) - 1])
#define RIGHT(l, c) (data->lines[l][(c) + 1])

static u64 part1()
{
    int res = 0;
    struct data *data;
    int l, c;

    data = read_file();

    for (l = 0; l < data->nlines; ++l) {
        for (c = 0; c < data->length; ++c) {
            if (l > 0) {
                if (UP(l, c) <= data->lines[l][c])
                    continue;
            }
            if (c > 0) {
                if (LEFT(l, c) <= data->lines[l][c])
                    continue;
            }
            if (l < data->nlines - 1) {
                if (DOWN(l, c) <= data->lines[l][c])
                    continue;
            }
            if (c < data->length - 1) {
                if (RIGHT(l, c) <= data->lines[l][c])
                    continue;
            }
            res += data->lines[l][c] - '0' +  1;
        }
    }
    return res;
}

struct stack {
    int l;
    int c;
    struct list_head list;
};

LIST_HEAD(stack_head);
static pool_t *pool;

static struct stack *push(int l, int c)
{
    struct stack *stack;

    stack = pool_get(pool);
    stack->l = l;
    stack->c = c;
    list_add_tail(&stack->list, &stack_head);
    return stack;
}

static struct stack *pop()
{
    struct stack *stack;

    if (list_empty(&stack_head))
        return NULL;
    stack = list_first_entry(&stack_head, struct stack, list);
    list_del(&stack->list);
    return stack;
}

static int bfs(struct data *data)
{
    struct stack *stack;
    int res = 0, l, c;

    while ((stack = pop())) {
        l = stack->l;
        c = stack->c;
        res++;
        pool_add(pool, stack);
        if (l > 0) {
            if (UP(l, c) < '9') {
                data->lines[l - 1][c] = '9';
                push(l - 1, c);
            }
        }
        if (c > 0) {
            if (LEFT(l, c) < '9') {
                data->lines[l][c - 1] = '9';
                push(l, c - 1);
            }
        }
        if (l < data->nlines - 1) {
            if (DOWN(l, c) < '9') {
                data->lines[l + 1][c] = '9';
                push(l + 1, c);
            }
        }
        if (c < data->length - 1) {
            if (RIGHT(l, c) < '9') {
                data->lines[l][c + 1] = '9';
                push(l, c + 1);
            }
        }
    }
    return res;
}

static u64 part2()
{
    int res = 0, l, c, best[3] = { 0 }, lowbest;
    struct data *data;

    data = read_file();

    if (!(pool = pool_init("stack", 128, sizeof (struct stack))))
        return -1;
    for (l = 0; l < data->nlines; ++l) {
        for (c = 0; c < data->length; ++c) {
            if (data->lines[l][c] < '9') {
                data->lines[l][c] = '9';
                push(l, c);
                res = bfs(data);
                lowbest = 0;
                if (best[1] < best[lowbest])
                    lowbest = 1;
                if (best[2] < best[lowbest])
                    lowbest = 2;
                if (res > best[lowbest])
                    best[lowbest] = res;
            }

        }
    }
    return best[0] * best[1] * best[2];
}

static u64 doit(int part)
{
    return part == 1? part1(): part2();
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    u32 exercise = 1;
    u64 res;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                exercise = atoi(optarg);
                if (exercise < 1 || exercise > 2)
                    return usage(*av);
                break;
            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    res = doit(exercise);
    printf("%s : res=%lu\n", *av, res);
    exit (0);
}
