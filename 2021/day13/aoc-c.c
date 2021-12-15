/* aoc-c.c: Advent of Code 2021, day 13 parts 1 & 2
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

#include "debug.h"
#include "pool.h"
#include "bits.h"
#include "list.h"

#define MAX_POINTS 1024
#define MAX_FLIPS  1024

typedef struct square {
    int x;
    int y;
} square_t;

typedef struct flip {
    char xy;                                      /* 0: x, 1: y */
    int val;
} flip_t;

static square_t points[MAX_POINTS];               /* points list */
static int npoints;                               /* ... and size */

static flip_t flips[MAX_FLIPS];                   /* flips list */
static int nflips;                                /* ... and size */

static void print_dots()
{
    int prevy = 0, x = 0, y = 0;

    for (int i = 0; i < npoints; ++i) {
        y = points[i].y;
        if (y != prevy) {
            putchar('\n');
            prevy = y;
            x = 0;
        }
        /* print spaces before next point */
        while(x < points[i].x) {
            putchar(' ');
            x++;
        }
        x++;
        putchar('#');
    }
    putchar('\n');
}

/* sort array and pack it (remove duplicates). sort by y first, then x.
 */
static int ins_sort_points()
{
    for (int i = 1, j = 0; i < npoints; i++, j = i - 1) {
        square_t cur = points[i];

        while (j >= 0 && ((points[j].y > cur.y) ||
                          ((points[j].y == cur.y) && (points[j].x > cur.x)))) {
            points[j + 1] = points[j];
            j = j - 1;
        }
        points[j + 1] = cur;
    }
    int cur = 0;
    if (npoints > 1) {
        for (int i = 0; i < npoints - 1; ++i) {
            if (points[i].x != points[i + 1].x || points[i].y != points[i + 1].y) {
                points[cur++] = points[i];
            }
        }
        points[cur++] = points[npoints-1];
    }
    npoints = cur;

    return npoints;
}

static void flipx(int n)
{
    for (int i = 0; i < npoints; ++i) {
        square_t *p = points+i;
        if (points[i].x > n) {
            p->x = 2 * n - p->x;
        }
    }
}

static void flipy(int n)
{
    for (int i = 0; i < npoints; ++i) {
        square_t *p = points+i;
        if (points[i].y > n) {
            p->y = 2 * n - p->y;
        }
    }
}

/* read data and create graph.
 */
static int read_input()
{
    ssize_t len;
    size_t alloc = 0;
    char *buf;

    /* get points list */
    while ((len = getline(&buf, &alloc, stdin)) > 1) {
        sscanf(buf, "%d,%d", &points[npoints].x, &points[npoints].y);
        npoints++;
    }
    /* get flip list */
    while ((len = getline(&buf, &alloc, stdin)) > 1) {
        sscanf(buf, "fold along %c=%d", &flips[nflips].xy, &flips[nflips].val);
        nflips++;
    }
    free(buf);
    return 1;
}

static int part1()
{
    if (flips[0].xy == 'x')
        flipx(flips[0].val);
    else
        flipy(flips[0].val);
    ins_sort_points();
    return npoints;
}

static int part2()
{
    for (int i = 0; i < nflips; ++i) {
        if (flips[i].xy == 'x') {
            flipx(flips[i].val);
        } else {
            flipy(flips[i].val);
        }
    }
    ins_sort_points();
    print_dots();
    return npoints;
}

static int doit(int part)
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

    printf("%s : res=%d\n", *av, doit(part));
    exit (0);
}
