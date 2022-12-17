/* aoc-c.c: Advent of Code 2022, day 9
 *
 * Copyright (C) 2022 Bruno Raoult ("br")
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
#include <string.h>

#include "aoc.h"

#include "br.h"
#include "debug.h"
#include "pool.h"
#include "list.h"
#include "hashtable.h"
#include "pjwhash-inline.h"

#define HBITS 12                                   /* 12 bits: 4096 buckets */

typedef struct pos {
    int x;
    int y;
} pos_t;

typedef struct visited {
    uint hash;
    pos_t coord;
    short nvisits;
    struct hlist_node hlist;
} visited_t;

enum dir {
    D = 'D',
    L = 'L',
    R = 'R',
    U = 'U'
};

typedef struct move {
    enum dir dir;
    int nmoves;
    struct list_head list;
} move_t;

DEFINE_HASHTABLE(h_visited, HBITS);
LIST_HEAD(moves);

static pool_t *pool_visited;
static pool_t *pool_move;

/**
 * find_visited - find entry in an hashtable bucket
 */
static visited_t *find_visited(struct hlist_head *bucket, uint hash, pos_t *pos)
{
    visited_t *cur;

    hlist_for_each_entry(cur, bucket, hlist)
        if (cur->hash == hash && cur->coord.x == pos->x && cur->coord.y == pos->y)
            return cur;
    return NULL;
}


static visited_t *add_visited(pos_t *pos, uint hash, uint bucket)
{
    visited_t *new = pool_get(pool_visited);

    new->hash = hash;
    new->coord.x = pos->x;
    new->coord.y = pos->y;
    hlist_add_head(&new->hlist, &h_visited[bucket]);
    return new;
}

static int add_visited_maybe(pos_t *pos)
{
    uint hash = pjwhash(pos, sizeof (*pos));
    uint bucket = hash_32(hash, HBITS);
    if (! find_visited(h_visited + bucket, hash, pos)) {
        add_visited(pos, hash, bucket);
        return 1;
    }
    return 0;
}

static int move_tails(pos_t *pos, int ntails)
{
    int i;
    pos_t *cur, *next;
    int didmove = 1;

    for (i = 0; i < ntails && didmove; ++i) {
        didmove = 0;
        pos_t diff;
        cur = pos + i;
        next = cur + 1;
        diff.x = cur->x - next->x;
        diff.y = cur->y - next->y;
        if (abs(diff.x) > 1 || abs(diff.y) > 1) { /* do move */
            if (diff.x)
                next->x += diff.x / abs(diff.x);
            if (diff.y)
                next->y += diff.y / abs(diff.y);
            didmove = 1;
        }
    }
    return didmove? add_visited_maybe(pos+ntails): 0;
}

static void move_head(pos_t *pos, move_t *move)
{
    log_f(3, "(%d,%d,%c) -> ", pos->x, pos->y, move->dir);
    switch (move->dir) {
        case U:
            pos->y++;
            break;
        case R:
            pos->x++;
            break;
        case D:
            pos->y--;
            break;
        case L:
            pos->x--;
            break;
        default:
            log(3, "ERR=%x ", move->dir);
    }
    log_f(3, "(%d,%d)\n", pos->x, pos->y);
}

static pos_t *part1(int *ntails)
{
    static pos_t nodes[] = { [0 ... 1] = {.x = 0, .y = 0}};
    *ntails = ARRAY_SIZE(nodes) - 1;
    return nodes;
}

static pos_t *part2(int *ntails)
{
    static pos_t nodes[] = { [0 ... 9] = {.x = 0, .y = 0}};
    *ntails = ARRAY_SIZE(nodes) - 1;
    return nodes;
}

static int solve(int part)
{
    int ntails;
    pos_t *nodes = part == 1? part1(&ntails): part2(&ntails);
    move_t *move;
    int res = 1;                                  /* for (0,0) */

    add_visited_maybe( &(pos_t) {0, 0});
    list_for_each_entry(move, &moves, list) {
        for (int i = 0; i < move->nmoves; ++i) {
            move_head(nodes, move);
            res += move_tails(nodes, ntails);
        }
    }
    return res;
}

static int parse()
{
    char dir;
    int nmoves;
    int count = 0;
    move_t *move;

    while (scanf(" %c %d", &dir, &nmoves) == 2) {
        move = pool_get(pool_move);
        move->dir = dir;
        move->nmoves = nmoves;
        list_add_tail(&move->list, &moves);
        count++;
    }
    return count;
}

/*
 * static int solve(int part)
 * {
 *     ulong bucket;
 *     dir_t *cur;
 *     int res = 0, needed;
 *
 *     if (part == 1) {
 *         hash_for_each(hasht_dir, bucket, cur, hlist)
 *             if (cur->size <= 100000)
 *                 res += cur->size;
 *     } else {
 *         res = find_dirname("/", 1)->size;
 *         needed = res - (70000000-30000000);
 *         hash_for_each(hasht_dir, bucket, cur, hlist)
 *             if (cur->size >= needed && cur->size < res)
 *                 res = cur->size;
 *     }
 *     return res;
 * }
 */

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_visited = pool_create("visited", 128, sizeof(visited_t));
    pool_move = pool_create("dirs", 128, sizeof(move_t));

    parse();
    printf("%s: res=%d\n", *av, solve(part));
    pool_destroy(pool_visited);
    pool_destroy(pool_move);
    exit(0);
}
