/* aoc-c.c: Advent of Code 2021, day 17 parts 1 & 2
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

struct target {
    s64 x1, y1;
    s64 x2, y2;
} target;

struct bounds {
    s64 dx_min, dx_max;
    s64 dy_min, dy_max;
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

/* nth_x will stop after initial velocity steps
 */
static s64 nth_x(s64 x0, s64 n)
{
    return n >= x0? nth(x0, x0): nth(x0, n);
}

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
    s64 count = 0;
    for (s64 dx = bounds.dx_min; dx <= bounds.dx_max ; ++dx) {
        for (s64 dy = bounds.dy_min; dy <= bounds.dy_max; ++dy) {
            for (s64 step = 1; ; step++) {
                s64 newx = nth_x(dx, step);
                s64 newy = nth(dy, step);
                if (newx > target.x2 || newy < target.y1)
                    goto nexty;
                if (hit(newx, newy)) {
                    count++;
                    goto nexty;
                }
            }
        nexty:
        }
    }
    return count;
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

    /* determine min possible initial dx :
     * ( dx )(dx + 1) <= minx * 2
     * <=> dx² + dx - minx * 2 <= 0
     * solution is :
     *   x = (-b ± sqrt(b² - 4 * a * c)) / 2 * b, with a = 1, b = 1, c = -2 * minx
     * => (-1 ± sqrt(1 + 8 * minx)) / 2 <= 0
     */
    bounds.dx_min = (s64) ((-1) + sqrt((double)(1 + 8 * target.x1))) / 2;
    bounds.dx_max = target.x2;
    bounds.dy_min = target.y1;
    bounds.dy_max = -target.y1;

    printf("%s : res=%ld\n", *av, part == 1? part1(): part2(&target));
    exit (0);
}
