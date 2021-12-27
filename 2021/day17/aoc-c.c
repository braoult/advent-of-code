/* aoc-c.c: Advent of Code 2021, day 16 parts 1 & 2
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
#include <math.h>

#include "debug.h"
#include "pool.h"
#include "bits.h"
#include "list.h"

typedef struct vector {
    s64 x, y;
} vector_t;

#define TMIN 0
#define TMAX 1

struct target {
    s64 x1, y1;
    s64 x2, y2;
} target;

struct bounds {
    s64 dx_min, dx_max;
    s64 dy_min, dy_max;
    s64 steps_min;
} bounds;

inline static int hit(s64 x, s64 y)
{
    return x >= target.x1 && y >= target.y1 &&
        x <= target.x2 && y <= target.y2;
}

/* if we take hypothesis target y is always negative, we can exclude steps
 * which are still over 0. We reach back zero after 2 x d1 +1 steps for
 * initial d1 positive (0 otherwise).
 */
static s64 nth(s64 x, s64 n)
{
    return ((2 * x) + -1 * (n - 1)) * n / 2;
}

static s64 nth_x(s64 x0, s64 n)
{
    return n >= x0? nth(x0, x0): nth(x0, n);
    //return ((2 * x0) * -(n - 1)) / 2;
}

/* if we take hypothesis target y is always negative, we can exclude steps
 * which are still over 0. We reach back zero after 2 x d1 +1 steps for
 * initial d1 positive (0 otherwise).
 */
static s64 nth_yzero(s64 y0)
{
    return y0 >= 0? y0 * 2 + 1 : 0;
}

/* determine max possible initial dx :
 * ( dx )(dx + 1) <= minx * 2
 * <=> dx² + dx - minx * 2 <= 0
 * solution is x = (-b ± sqrt(b² - 4 * a * c)) / 2 * b, with a = 1, b = 1, c = -2 * minx
 * => (-1 ± sqrt(1 + 8 * minx)) / 2 <= 0
 */
static s64 dx_min(s64 xmin)
{
    s64 res = (s64) ((-1) + sqrt((double)(1 + 8 * xmin))) / 2;

    bounds.steps_min = 1;
    bounds.dx_min = res;

    /*nmin = (s64) (-(1+2*res) +
                  sqrt((float)(1 + 2 * res)
                       * (float)(1 + 2 * res)
                       - 4 * (2 * res - xmin))) / 2;
    */
    //nmin = ((float) 1 + (float)sqrt(1 + 4.0 * xmin)) / (float)2;
    //printf("xmin=%ld res=%ld nmin=%ld\n", xmin, res, nmin);
    //nmin = ((float) 1 - (float)sqrt((float)1 + 4.0 * xmin)) / (float)2;
    //printf("xmin=%ld res=%ld nmin=%ld\n", xmin, res, nmin);
    return res;
}

/*
static s64 max_y_steps(s64 xmax))
{
    return (xmax - 1) / 2;
}
*/

/* The highest solution is the solution of:
 * y1 * (y1 +1) / 2
 * (we can ignore x velocity, as we can reach any target x1 with x velocity
 * becoming zero at x1).
 */
static s64 part1()
{
    return target.y1 * (target.y1 + 1) / 2;
}

static s64 part2()
{
    return target.y1 * (target.y1 + 1) / 2;
}

/* read input
 */
static int read_input()
{
    if (scanf("target area: x=%ld..%ld, y=%ld..%ld",
              &target.x1, &target.x2,
              &target.y1, &target.y2))
        return 0;
    return 1;
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

    read_input();
    printf("%s : xmin=%ld ymin=%ld xmax=%ld ymax=%ld part1=%ld\n", *av,
           target.x1, target.y1, target.x2, target.y2,
           part1());
    //s64 dxmin = dx_min(target.x1);

    //for (s64 i = 0; i < 10; ++i) {
    dx_min(target.x1);
    bounds.dx_max = target.x2;
    bounds.dy_min = target.y1;
    /* y will comme back at zero with same dy as initial one. next step will be d0+1.
     * If d0+1 is > min target y, we will never reach target.
     */
    bounds.dy_max = -target.y1;

    printf("yzero(%ld) = %ld xmin(%ld) = %ld\n",
           target.x2, nth_yzero(target.x2),
           target.x1, dx_min(target.x1));

    /* loop on x initial acceletation */
    s64 count = 0, besty=0;
    for (s64 dx = bounds.dx_min; dx <= bounds.dx_max ; ++dx) {
        /* maybe here make steps for x only, and find all valid steps.
         * need to know if dx becomes zero and too low
         */
        for (s64 dy = bounds.dy_min; dy <= bounds.dy_max; ++dy) {
            s64 maxy = 0;
            for (s64 step = 1; ; step++) {
                s64 newx = nth_x(dx, step);
                s64 newy = nth(dy, step);
                printf("dx=%ld dy=%ld step=%ld x=%ld y=%ld\n", dx, dy, step, newx, newy);
                if (newx > target.x2) {
                    printf("\tnext y1\n");
                    goto nexty;
                }
                if (newy < target.y1) {
                    printf("\t\tnext y2\n");
                    goto nexty;
                }
                if (newy > maxy) {
                    maxy = newy;
                    printf("new maxy = %ld\n", maxy);
                }

                /* need to check if x can join minx */
                /* need to find max y bound */

                if (hit(newx, newy)) {
                    printf("\tHIT: %ld,%ld\n", newx, newy);
                    count++;
                    if (maxy > besty) {
                        besty = maxy;
                        printf("new besty = %ld\n", besty);
                    }
                    goto nexty;
                }
            }
        nexty:
        }
    nextx:
    }
    printf("count=%ld besty=%ld\n", count, besty);
    exit (0);

    /*
    for (s64 dx = 0; dx <=10; ++dx) {
        printf("x0=%2ld: ", dx);
        for (int i = 0; i < 15; ++i)
            printf(" %d=%ld/%ld", i, nth_x(dx, i), nth(dx, i));
        printf("\n");
    }
    */

    printf("%s : res=%ld\n", *av, part == 1? part1(): part2(&target));
    exit (0);
}
