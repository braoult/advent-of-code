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
    s64 volume;
    int x[2], y[2], z[2];
    struct list_head list_step;
} step_t;

LIST_HEAD(list_step);

pool_t *pool_step;

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

static inline int cube_volume(step_t *c)
{
    return (c->x[1] - c->x[0] + 1) *
        (c->y[1] - c->y[0] + 1) *
        (c->z[1] - c->z[0] + 1);
}

static void print_cubes()
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

static step_t *read_instruction(step_t *cube)
{
    char onoff[5];

    if (scanf("%5s x=%d..%d,y=%d..%d,z=%d..%d\n", onoff,
              &cube->x[0], &cube->x[1],
              &cube->y[0], &cube->y[1],
              &cube->z[0], &cube->z[1]) == 7) {
        cube->onoff = onoff[1] == 'n';
        return cube;
    }
    return NULL;
}

/* intersect 2 cubes (x axis calculation):
 *
 *  x10       x11
 *  +---------+
 *  |     x0  |
 *  |     +---|---+
 *  |     |   |   |
 *  +---------+   |
 *        |   x1  |
 *        +-------+
 *        x20     x21
 *
 * x0 = MAX(x10, x20)
 * x1 = MIN(x11, x21)
 *
 * If x0 > x1, cubew do not intersect.
 */
static step_t *cube_intersect(step_t *c1, step_t *c2)
{
    step_t *cube = NULL;
    int x[2], y[2], z[2];

    x[0] = MAX(c1->x[0], c2->x[0]);
    x[1] = MIN(c1->x[1], c2->x[1]);
    if (x[0] > x[1])
        goto end;
    y[0] = MAX(c1->y[0], c2->y[0]);
    y[1] = MIN(c1->y[1], c2->y[1]);
    if (y[0] > y[1])
        goto end;
    z[0] = MAX(c1->z[0], c2->z[0]);
    z[1] = MIN(c1->z[1], c2->z[1]);
    if (z[0] > z[1])
        goto end;
    cube = pool_get(pool_step);
    for (int i = 0; i < 2; ++i) {
        cube->x[i] = x[i];
        cube->y[i] = y[i];
        cube->z[i] = z[i];
    }
    cube->volume = -cube_volume(cube);
    list_add_tail(&cube->list_step, &list_step);
end:
    return cube;
}

static int part1()
{
    step_t cur;
    static char cuboid[101][101][101];
    int res = 0;

    while (read_instruction(&cur)) {
        int x1, x2, y1, y2, z1, z2;

        x1 = MAX(cur.x[0], -50);
        x2 = MIN(cur.x[1], 50);

        y1 = MAX(cur.y[0], -50);
        y2 = MIN(cur.y[1], 50);

        z1 = MAX(cur.z[0], -50);
        z2 = MIN(cur.z[1], 50);

        for (int x = x1; x <= x2; ++x) {
            for (int y = y1; y <= y2; ++y) {
                for (int z = z1; z <= z2; ++z) {
                    log(1, "(%d,%d,%d)=%d\n", x, y, z, cur.onoff);
                    cuboid[x+50][y+50][z+50] = cur.onoff;
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

/* For part 2, we loop over all instructions (cuboid on/off):
 * For all previous cuboids, search for intersection, then add a "negative"
 * cuboid for it. If new cuboid is "on", add it also to the list.
 */
static int part2()
{
    step_t *cur, *tmp, *inter, *new;
    int res = 0;

    while ((new = read_instruction(tmp = pool_get(pool_step)))) {
        list_for_each_entry(cur, &list_step, list_step) {
            inter = cube_intersect(new, cur);
        }
        list_add_tail(&cur->list_step, &list_step);
        //list_add();
            //list_for_each_entry_safe(cur, tmp, &list_step, list_step) {
            //res++;
    }
    print_cubes();
    return res;

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

    pool_step = pool_create("steps", 512, sizeof(step_t));

    printf("%s : res=%d\n", *av, part == 1? part1(): part2());

    exit(0);
}
