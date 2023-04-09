/* aoc-c.c: Advent of Code 2022, day 14
 *
 * Copyright (C) 2022-2023 Bruno Raoult ("br")
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
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

#include "br.h"
#include "list.h"
#include "pool.h"
#include "debug.h"

#include "aoc.h"

pool_t *pool_segment;

#define INF (-1)                                  /* found infinite fall position */

typedef enum {                                    /* map cells */
    EMPTY = '.',
    STONE = '#',
    SAND  = 'o',
} type_t;

typedef struct segment {
    int x1, y1, x2, y2;
    struct list_head list;
} segment_t;

LIST_HEAD(segments);

typedef struct map {
    int xmin, xmax, ymin, ymax;
    int size_x, size_y;
    int deltax, deltay;
    int dropcol;                                  /* drop sand here */
    int dropped;                                  /* number of sand units */
    char *m;
} map_t;

#define XY(m, x, y)   ((y) * m->size_x + (x))
#define P(m, x, y)    (m->m[XY(m, (x), (y))])

static int drop_sand(struct map *m, int x, int y)
{
    int ret = 0, tmp;

    if (y >= m->ymax)                             /* part 1: nothing left under */
        return INF;

    if (P(m, x, y+1) == EMPTY) {                  /* down */
        if ((tmp = drop_sand(m, x, y+1)) < 0)
            return INF;
        ret += tmp;
    }
    if (P(m, x-1, y+1) == EMPTY) {                /* left */
        if ((tmp = drop_sand(m, x-1, y+1)) < 0)
            return INF;
        ret += tmp;
    }
    if (P(m, x+1, y+1) == EMPTY) {                /* right */
        if ((tmp = drop_sand(m, x+1, y+1)) < 0)
            return INF;
        ret += tmp;
    }
    /* the 3 lower adjacent cells are filled */
    P(m, x, y) = SAND;
    m->dropped++;
    if (y == 0)                                   /* part 2 */
        return INF;
    return ret;
}

static struct map *gen_map(struct map *m, int part)
{
    segment_t *cur;

    if (part == 1) {
        m->size_x = (m->xmax - m->xmin) + 3;
        m->size_y = m->ymax + 1;
        m->deltax = m->xmin - 1;
    } else {
        m->ymax += 2;
        m->xmin = 500 - m->ymax - 1;
        m->xmax = 500 + m->ymax + 1;
        m->deltax = m->xmin;
        m->size_x = (m->xmax - m->xmin);
        m->size_y = m->ymax + 3;
    }
    m->dropcol = 500 - m->deltax;
    m->dropped = 0;

    m->m = malloc(m->size_x * m->size_y);
    memset(m->m, '.', m->size_x * m->size_y);
    list_for_each_entry(cur, &segments, list) {
        int x1 = cur->x1 - m->deltax, y1 = cur->y1 - m->deltay,
            x2 = cur->x2 - m->deltax, y2 = cur->y2 - m->deltay;
        int dx = 0, dy = 0;
        if (x1 == x2)
            dy = y2 - y1 > 0 ? 1 : -1;
        else
            dx = x2 - x1 > 0 ? 1 : -1;
        do {
            P(m, x1, y1) = '#';
            x1 += dx, y1 += dy;
        } while (x1 != x2 || y1 != y2);
        P(m, x2, y2) = '#';
    }
    if (part == 2)
        for (int i = 0; i < m->size_x; ++i)
            P(m, m->xmin - m->deltax + i, m->ymax) = STONE;
    return m;
}

static struct map *parse(map_t *m)
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL, *cur;
    int i, scanned;
    int x1, y1, x2, y2, x, y, n;
    segment_t *segment;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        i = 0;
        buf[--buflen] = 0;
        cur = buf;
        while (1) {
            scanned = sscanf(cur, "%d,%d ->%n", &x, &y, &n);
            if (scanned  != 2)
                break;
            m->xmin = min(x, m->xmin);
            m->xmax = max(x, m->xmax);
            m->ymin = min(y, m->ymin);
            m->ymax = max(y, m->ymax);
            if (i) {
                x1 = x2;
                y1 = y2;
            }
            x2 = x;
            y2 = y;
            if (!i)                               /* first point */
                goto next;
            segment = pool_get(pool_segment);
            segment->x1 = x1;
            segment->y1 = y1;
            segment->x2 = x2;
            segment->y2 = y2;
            list_add_tail(&segment->list, &segments);
        next:
            i++;
            cur += n;
            if (cur - buf >= buflen)
                break;
        }
    }
    free(buf);
    return m;
}

static int doit(map_t *m, int part)
{
    m = gen_map(parse(m), part);
    drop_sand(m, m->dropcol, 0);
    return m->dropped;
}


int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    static map_t m;
    m.xmin = INT_MAX; m.xmax = INT_MIN;
    m.ymin = INT_MAX, m.ymax = INT_MIN;

    pool_segment =  pool_create("segment", 512, sizeof(segment_t));

    printf("%s: res=%d\n", *av, doit(&m, part));
    free(m.m);
    pool_destroy(pool_segment);
    exit(0);
}
