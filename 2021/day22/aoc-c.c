/* aoc-c.c: Advent of Code 2021, day 21 parts 1 & 2
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
#include <unistd.h>
#include <errno.h>

#include "pool.h"
#include "debug.h"
#include "bits.h"
#include "list.h"

typedef struct step {
    int onoff;
    int x[2], y[2], z[2];
    struct list_head list_step;
} step_t;

LIST_HEAD(list_step);

pool_t *pool_step;

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

static void print_steps()
{
    step_t *cur;
    int nlines = 1;

    list_for_each_entry(cur, &list_step, list_step) {
        log(1, "%d: %s x=(%d,%d) y=(%d,%d) z=(%d,%d)\n",
            nlines++,
            cur->onoff? "on": "off",
            cur->x[0], cur->x[1],
            cur->y[0], cur->y[1],
            cur->z[0], cur->z[1]);
    }
}

static int read_input()
{
    char onoff[5];
    int nlines = 0;
    step_t input, *cur;

    pool_step = pool_create("steps", 512, sizeof(step_t));
    while (scanf("%5s x=%d..%d,y=%d..%d,z=%d..%d\n", onoff,
                 &input.x[0], &input.x[1],
                 &input.y[0], &input.y[1],
                 &input.z[0], &input.z[1]) == 7) {
        cur = pool_get(pool_step);
        *cur = input;
        cur->onoff = onoff[1] == 'n';
        list_add_tail(&cur->list_step, &list_step);
        nlines++;
    }
    return nlines;
}

static int part1()
{
    step_t *cur;
    static char cuboid[101][101][101];
    u64 res = 0;

/*    log(1, "%s x=(%d,%d) y=(%d,%d) z=(%d,%d)\n",
        cur->onoff? "on": "off",
        cur->x[0], cur->x[1],
        cur->y[0], cur->y[1],
        cur->z[0], cur->z[1]);
*/

    list_for_each_entry(cur, &list_step, list_step) {
        int x1, x2, y1, y2, z1, z2;

        x1 = MAX(cur->x[0], -50);
        x2 = MIN(cur->x[1], 50);

        y1 = MAX(cur->y[0], -50);
        y2 = MIN(cur->y[1], 50);

        z1 = MAX(cur->z[0], -50);
        z2 = MIN(cur->z[1], 50);

        for (int x = x1; x <= x2; ++x) {
            for (int y = y1; y <= y2; ++y) {
                for (int z = z1; z <= z2; ++z) {
                    log(1, "(%d,%d,%d)=%d\n", x, y, z, cur->onoff);
                    cuboid[x+50][y+50][z+50] = cur->onoff;
                }
            }
        }
    }

    for (int x = 0; x < 101; ++x) {
        for (int y = 0; y < 101; ++y) {
            for (int z = 0; z < 101; ++z) {
                res += cuboid[x][y][z];
            }
        }
    }
    return res;
}

static int part2()
{
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
    print_steps();
    printf("%s : res=%d\n", *av, part == 1? part1(): part2());

    exit(0);
}
