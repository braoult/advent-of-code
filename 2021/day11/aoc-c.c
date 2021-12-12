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
static int gsize;                                 /* grid size */

static int stack[MAX_GRID * MAX_GRID];            /* flashing stack */
static int nstack;                                /* current stack size */

#define loop_for_grid(l, c, val)                \
    for (l = 1; l <= gsize; ++l)                \
        for (c = 1; c <= gsize; ++c)            \
            val = grid[l][c];

#define VALID(x, y)    ((x) >= 0 && (y) >= 0 && (x) < gsize && (y) < gsize)

inline static int push(int l, int c)
{
    return ((stack[nstack++] = l * gsize + c));
}

inline static int pop()
{
    if (!nstack)
        return -1;
    return stack[--nstack];
}

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

inline static int inc_cell(int l, int c)
{
    if (VALID(l, c) && ++grid[l][c] == 10) {
        push(l, c);
        return 1;
    }
    return 0;
}

void flash_grid(int (*arr)[MAX_GRID], int step)
{
    log_f(3, "step = %d\n", step);
    for (int l = 1; l <= gsize; ++l) {
        for (int c = 1; c <= gsize; ++c) {
            log(3, "%2d ", arr[l][c]);
        }
        log(3, "\n");
    }
}

/* run 1 step */
static void step(int nsteps)
{
    int vpop;
    int flash = 0;
    //int flashed[MAX_GRID][MAX_GRID];

    //init_grid(flashed);
    /* create an empty grid */

    for (int step = 1; step <= nsteps; ++step) {
        /* 1) increase all energy levels */
        for (int l = 0; l < gsize; ++l) {
            for (int c = 0; c < gsize; ++c) {
                flash += inc_cell(l, c);
            }
        }

        /* 2) perform BFS until stack is empty */
        while ((vpop = pop()) !=  -1) {
            int pl = vpop / gsize, pc = vpop % gsize;
            for (int l = -1; l <= 1; ++l)
                for (int c = -1; c <= 1; ++c)
                    flash += inc_cell(pl + l, pc + c);
        }
        //print_grid(grid, 1);

        /* 3) cleanup flashed cells */
        for (int l = 0; l < gsize; ++l) {
            for (int c = 0; c < gsize; ++c) {
                if (grid[l][c] > 9)
                    grid[l][c] = 0;
            }
        }
        print_grid(grid, step);
        printf("step = %d flashed = %d\n", step, flash);
    }
}

static s64 doit(int part)
{
    int l = 0, c;
    ssize_t len;
    size_t alloc = 0;
    char *buf = NULL, *p;

    len = getline(&buf, &alloc, stdin);
    gsize = len - 1;
    for (l = 0; l < gsize; ++l) {
        buf[gsize] = 0;
        log_f(10, "len = %d str = [%s]\n", grid, buf);
        for (c = 0, p = buf; c < gsize; c++, p++)
            grid[l][c] = *p - '0';
        getline(&buf, &alloc, stdin);
    }
    free(buf);
    print_grid(grid, 0);
    step(100);
    print_grid(grid, 1);
    return part;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    u32 exercise = 1, debug = 1;
    s64 res;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug = atoi(optarg);
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
    debug_level_set(debug);
    if (optind < ac)
        return usage(*av);

    //init_borders();
    res = doit(exercise);
    printf("%s : res=%lu\n", *av, res);
    exit (0);
}
