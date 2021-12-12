/* aoc-c.c: Advent of Code 2021, day 11 parts 1 & 2
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

#define MAX_GRID 10

static int grid[MAX_GRID][MAX_GRID];
static int gsize;                                 /* real grid size */

static int stack[MAX_GRID * MAX_GRID];            /* flashing stack */
static int nstack;                                /* current stack size */

#define VALID(x, y)    ((x) >= 0 && (y) >= 0 && (x) < gsize && (y) < gsize)

inline static int push(int l, int c)
{
    return ((stack[nstack++] = l * gsize + c));
}

inline static int pop()
{
    return nstack? stack[--nstack]: -1;
}

#ifdef DEBUG
static void print_grid(int (*arr)[MAX_GRID], int step)
{
    log_f(3, "step = %d\n", step);
    for (int l = 0; l < gsize; ++l) {
        for (int c = 0; c < gsize; ++c) {
            log(3, "%2d ", arr[l][c]);
        }
        log(3, "\n");
    }
}
#endif

inline static int flash_cell_maybe(int l, int c)
{
    if (VALID(l, c) && ++grid[l][c] == 10) {
        push(l, c);
        return 1;
    }
    return 0;
}

/* run 1 step */
static int do_step()
{
    int vpop, flash = 0;

    /* 1) increase all energy levels */
    for (int l = 0; l < gsize; ++l)
        for (int c = 0; c < gsize; ++c)
            flash += flash_cell_maybe(l, c);

    /* 2) perform BFS until stack is empty */
    while ((vpop = pop()) !=  -1)
        for (int l = -1, pl = vpop / gsize, pc = vpop % gsize; l <= 1; ++l)
            for (int c = -1; c <= 1; ++c)
                flash += flash_cell_maybe(pl + l, pc + c);

    /* 3) cleanup flashed cells */
    for (int l = 0; l < gsize; ++l)
        for (int c = 0; c < gsize; ++c)
            if (grid[l][c] > 9)
                grid[l][c] = 0;
    return flash;
}

static int part1()
{
    int flash = 0;

    for (int step = 0; step < 100; ++step)
        flash += do_step();
    return flash;
}

static int part2()
{
    int step = 1;

    while (do_step() != 100)
        step++;
    return step;
}

static int doit(int part)
{
    int l = 0, c;
    ssize_t len;
    size_t alloc = 0;
    char *buf = NULL, *p;

    len = getline(&buf, &alloc, stdin);
    gsize = len - 1;
    for (l = 0; l < gsize; ++l) {
        buf[gsize] = 0;
        for (c = 0, p = buf; c < gsize; c++, p++)
            grid[l][c] = *p - '0';
        getline(&buf, &alloc, stdin);
    }
    free(buf);
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

    printf("%s : res=%d\n", *av, doit(part));
    exit (0);
}
