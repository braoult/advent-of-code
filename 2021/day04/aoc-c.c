/* aoc-c: Advent of Code 2021, day 4 parts 1 & 2
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

struct board {
    u16 win;
    u16 rcount[5];
    u16 ccount[5];
    int board[5][5];
    struct list_head list;
};

struct draw {
    u16 count;
    u16 *draw;
} draw;

LIST_HEAD(list_head);
static int nboards = 0;

static void read_draw()
{
    char *buf = NULL, *token;
    size_t alloc = 0;
    ssize_t len;

    len = getline(&buf, &alloc, stdin);
    draw.draw = calloc(len/2, sizeof(*draw.draw));

    token = strtok(buf, ",\n");
    while (token) {
        draw.draw[draw.count++] = atoi(token);
        token = strtok(NULL, ",\n ");
    };
    free(buf);
    return;
}

#ifdef DEBUG
static void print_boards()
{
    int loop = 0, row, col;
    struct board *cur_board;

    list_for_each_entry(cur_board, &list_head, list) {
        printf("board %d\n", loop);
        for (row = 0; row < 5; ++row) {
            for (col = 0; col < 5; ++col) {
                printf("%d\t", cur_board->board[row][col]);
            }
            printf("\n");
        }
        ++loop;
    }
}
#endif

static struct list_head *read_boards()
{
    char *buf = NULL;
    size_t alloc = 0;
    ssize_t len;
    static pool_t *pool;
    struct board dummy;                           /* for maybe-uninitialized warning */
    struct board *cur_board = &dummy;
    int row = 0;

    if (!(pool = pool_init("boards", 128, sizeof (struct board))))
        return NULL;

    while ((len = getline(&buf, &alloc, stdin)) >= 0) {
        /* skip blank lines */
        if (len == 1) {
            row = 0;
            continue;
        }
        if (row == 0) {
            nboards++;
            if (!(cur_board = pool_get(pool)))
                return NULL;
            for (int i = 0; i < 5; ++i) {
                cur_board->rcount[i] = 0;
                cur_board->ccount[i] = 0;
                cur_board->win = 0;
            }
            list_add_tail(&cur_board->list, &list_head);
        }
        sscanf(buf, "%d%d%d%d%d",
               &cur_board->board[row][0],
               &cur_board->board[row][1],
               &cur_board->board[row][2],
               &cur_board->board[row][3],
               &cur_board->board[row][4]);
        row++;
    }
    free(buf);
    return &list_head;
}

inline static int calc_board(struct board *board, int num)
{
    int res = 0, row, col;

    for (row = 0; row < 5; ++row) {
        for (col = 0; col < 5; ++col) {
            if (board->board[row][col] > 0)
                res += board->board[row][col];
        }
    }
    return num * res;
}

inline static int check_win(int num, int part)
{
    struct board *board;
    u32 row, col;
    static int nwins = 0;

    list_for_each_entry(board, &list_head, list) {
        if (board->win)
            continue;
        for (row = 0; row < 5; ++row) {
            for (col = 0; col < 5; ++col) {
                if (board->board[row][col] == num) {
                    board->board[row][col] *= -1;
                    board->rcount[row]++;
                    board->ccount[col]++;

                    if (board->rcount[row] == 5 || /* found */
                        board->ccount[col] == 5) {
                        board->win = 1;
                        nwins++;
                        if (part == 1 || nwins == nboards)
                            return calc_board(board, num);
                    }
                }
            }
        }
    }
    return -1;
}

static int doit(int part)
{
    u32 num;
    int i, res;

    for (i = 0; i < draw.count; ++i) {
        num = draw.draw[i];
        if ((res = check_win(num, part)) > 0)
            return res;
    }
    return -1;                                    /* impossible */
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

    read_draw();
    read_boards();
    res = doit(exercise);
    printf ("%s : res=%d\n", *av, res);

    exit (0);
}
