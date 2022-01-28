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
    int sign;                                     /* 0: negative */
    int x[2], y[2], z[2];                         /* look for 'Arghh' below ;-) */
    struct list_head list_step;
} step_t;

LIST_HEAD(list_step);

pool_t *pool_step;
int ncubes;

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

static inline s64 cube_volume(step_t *c)
{
    /* Arghh... The missing cast below implied int overflow, and did cost
     * me 2 days :-(
     */
    s64 volume = (s64)(c->x[1] - c->x[0] + 1) *
        (c->y[1] - c->y[0] + 1) *
        (c->z[1] - c->z[0] + 1);
    return c->sign? volume: -volume;
}

static step_t *read_instruction(step_t *cube)
{
    char onoff[5];

    if (scanf("%5s x=%d..%d,y=%d..%d,z=%d..%d\n", onoff,
              &cube->x[0], &cube->x[1],
              &cube->y[0], &cube->y[1],
              &cube->z[0], &cube->z[1]) == 7) {
        cube->sign = onoff[1] == 'n';
        return cube;
    }
    return NULL;
}

/* intersect 2 cubes (x axis example):
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
 * If x0 > x1, cubes do not intersect.
 */
static inline step_t *cube_intersect(step_t *old, step_t *new)
{
    step_t *cube = NULL;
    int x[2], y[2], z[2];

    x[0] = MAX(old->x[0], new->x[0]);
    x[1] = MIN(old->x[1], new->x[1]);
    if (x[0] > x[1])
        return NULL;
    y[0] = MAX(old->y[0], new->y[0]);
    y[1] = MIN(old->y[1], new->y[1]);
    if (y[0] > y[1])
        return NULL;
    z[0] = MAX(old->z[0], new->z[0]);
    z[1] = MIN(old->z[1], new->z[1]);
    if (z[0] > z[1])
        return NULL;
    /* intersection exists, we create a new cuboid and fill it
     */
    cube = pool_get(pool_step);
    for (int i = 0; i < 2; ++i) {
        cube->x[i] = x[i];
        cube->y[i] = y[i];
        cube->z[i] = z[i];
    }
    return cube;
}

/* brute force approach, I did not test with part 2 algorithm,
 * my guess is that it could be worse...
 */
static s64 part1()
{
    step_t cur;
    static char cuboid[101][101][101];
    s64 res = 0;

    while (read_instruction(&cur)) {
        int x1, x2, y1, y2, z1, z2;

        x1 = MAX(cur.x[0], -50);
        x2 = MIN(cur.x[1], 50);

        y1 = MAX(cur.y[0], -50);
        y2 = MIN(cur.y[1], 50);

        z1 = MAX(cur.z[0], -50);
        z2 = MIN(cur.z[1], 50);

        for (int x = x1; x <= x2; ++x)
            for (int y = y1; y <= y2; ++y)
                for (int z = z1; z <= z2; ++z)
                    cuboid[x+50][y+50][z+50] = cur.sign;
    }

    for (int x = 0; x < 101; ++x)
        for (int y = 0; y < 101; ++y)
            for (int z = 0; z < 101; ++z)
                res += cuboid[x][y][z];
    return res;
}

/* For part 2, we loop over all instructions (input on/off cuboids):
 * For all previous cuboids, search for intersection, then add a
 * cuboid to negate it (this resets this intersection to zero).
 * If new cuboid is "on", add it also to the list, to make this intersection
 * "becoming 1" again.
 */
static s64 part2()
{
    step_t *cur, *tmp, *new, *inter;
    s64 res = 0;

    pool_step = pool_create("steps", 2048, sizeof(step_t));

    while ((new = read_instruction(tmp = pool_get(pool_step)))) {
        LIST_HEAD(list_tmp);                      /* temp intersections list */
        list_for_each_entry(cur, &list_step, list_step) {
            /* intersection found: we insert it to negate cur
             * ones.
             */
            if ((inter = cube_intersect(cur, new))) {
                inter->sign = !cur->sign;         /* negates intersection */
                list_add_tail(&inter->list_step, &list_tmp);
                res += cube_volume(inter);
            }
        }
        /* add temp intersections list to global's tail
         */
        list_splice_tail(&list_tmp, &list_step);
        /* add the new "on" cuboid to the list
         */
        if (new->sign) {
            list_add_tail(&new->list_step, &list_step);
            res += cube_volume(new);
        } else {
            pool_add(pool_step, tmp);             /* release memory */
        }
    }
    pool_destroy(pool_step);
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

    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());

    exit(0);
}
