/* aoc-c: Advent2021 game, day 5 part 1
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

typedef struct square {
    s32 x;
    s32 y;
} square_t;

typedef struct move {
    square_t from;
    square_t to;
    struct list_head list;
} move_t;

u32 *board;

LIST_HEAD(move_head);

static s32 nmoves = 0, max_x = 0, max_y = 0;

#ifdef DEBUG
static void print_moves()
{
    move_t *move;
    int i = 0;

    printf("nmoves: %u\n", nmoves);
    list_for_each_entry(move, &move_head, list) {
        printf("%03d: %d,%d to %d,%d\n",
               i,
               move->from.x, move->from.y,
               move->to.x, move->to.y);
        i++;
    }
}

static int print_board()
{
    int x, y;
    for (y = 0; y < max_y; ++y) {
        for (x = 0; x < max_x; ++x)
            printf("%c", board[SQUARE(x, y)]? '0' + board[SQUARE(x, y)]: '.');
        printf("\n");
    }
    return 0;
}
#endif

#define SQUARE(x, y)   ((y) * max_y + x)

static int init_board()
{
    if (!(board = calloc((max_x + 1) * (max_y + 1), sizeof (*board))))
        return -1;
    return 0;
}

static int count_squares()
{
    int x, y, res=0;

    for (y = 0; y < max_y; ++y) {
        for (x = 0; x < max_x; ++x) {
            if (board[SQUARE(x, y)] > 1)
                res++;
        }
    }
    return res;
}

static struct list_head *read_moves()
{
    char *buf = NULL;
    size_t alloc = 0;
    ssize_t len;
    static pool_t *pool;
    move_t *move;

    if (!(pool = pool_init("moves", 1024, sizeof (struct move))))
        return NULL;

    while ((len = getline(&buf, &alloc, stdin)) >= 0) {
        if (!(move = pool_get(pool)))
            return NULL;
        list_add_tail(&move->list, &move_head);
        sscanf(buf, "%d,%d -> %d,%d",
               &move->from.x, &move->from.y,
               &move->to.x, &move->to.y);
        if (move->from.x > max_x)
            max_x = move->from.x;
        if (move->to.x > max_x)
            max_x = move->to.x;
        if (move->from.y > max_y)
            max_y = move->from.y;
        if (move->to.y > max_y)
            max_y = move->to.y;
        nmoves++;
    }
    max_x++;                                      /* board starts at 0,0 */
    max_y++;
    free(buf);
    return &move_head;
}

inline static void do_moves(move_t *move, int dir_x, int dir_y)
{
    int x = move->from.x, y = move->from.y;
    int x2 = move->to.x, y2 = move->to.y;

    while (1) {
        board[SQUARE(x, y)]++;
        if ((dir_x && x == x2) ||  (dir_y && y == y2))
            break;
        x += dir_x;
        y += dir_y;
    }
}

static int move_rook(move_t *move)
{
    int dir_x = 0, dir_y = 0;

    /* different row or column */
    if (move->from.x != move->to.x && move->from.y != move->to.y)
        return 0;
    if (move->from.x == move->to.x)
        dir_y = move->from.y < move->to.y? 1: -1;
    else
        dir_x = move->from.x < move->to.x? 1: -1;
    do_moves(move, dir_x, dir_y);
    return 1;
}

static int move_bishop(move_t *move)
{
    int dir_x = 1, dir_y = 1;

    /* different diagonal */
    if (abs(move->to.x - move->from.x) != abs(move->to.y - move->from.y))
        return 0;
    if (move->from.x > move->to.x)                /* x goes backward */
        dir_x = -1;
    if (move->from.y > move->to.y)                /* y goes backward */
        dir_y = -1;
    do_moves(move, dir_x, dir_y);
    return 1;
}

static int doit(int part)
{
    move_t *move;

    list_for_each_entry(move, &move_head, list) {
        move_rook(move);
        if (part == 2)
            move_bishop(move);
    }
    return count_squares();
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    u32 exercise = 1, res;

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

    read_moves();
    init_board();
    res = doit(exercise);
    printf ("%s : res=%d\n", *av, res);
    /* TODO: free board/mem pool */
    exit (0);
}
