/* aoc-c.c: Advent of Code 2022, day 14
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
    CURR  = 'v'
} type_t;

typedef struct segment {
    int x1, y1, x2, y2;
    struct list_head list;
} segment_t;

LIST_HEAD(segments);


static struct map {
    int xmin, xmax, ymin, ymax;
    int size_x, size_y;
    int deltax, deltay;
    int dropcol;                                  /* drop sand here */
    int dropped;                                  /* number of sand units */
    char *m;
} map = {
    .xmin = INT_MAX, .xmax = INT_MIN, .ymin = INT_MAX, .ymax = INT_MIN,
};

static void print_segments()
{
    segment_t *cur;
    log(2, "segments:\n");
    list_for_each_entry(cur, &segments, list) {
        log(2, "segment: (%d,%d) -> (%d,%d)\n", cur->x1, cur->y1, cur->x2, cur->y2);
    }
}

#define XY(m, x, y)   ((y) * m->size_x + (x))
#define P(m, x, y)    (m->m[XY(m, (x), (y))])

static void print_map(struct map *m)
{
    log_f(3, "xmin=%d xmax=%d ymin=%d ymax=%d deltax=%d deltay=%d sizex=%d sizey=%d\n",
          m->xmin, m->xmax, m->ymin, m->ymax, m->deltax, m->deltay, m->size_x, m->size_y);
    for (int skip=0; skip < m->dropcol + 3; ++skip)
        log(3, " ");
    log(3, "+\n");
    for (int y = 0; y < m->size_y; ++y) {
        log(3, "%02d ", y);
        for (int x = 0; x < m->size_x; ++x) {
            log(3, "%c", P(m, x, y));
        }
        log(3, "\n");
    }
}

static int drop_sand(struct map *m, int x, int y)
{
    int ret = 0, tmp;

    log_f(3, "x=%d y=%d\n", x, y);
    if (y >= m->ymax)                              /* nothing left under */
        return INF;

    if (P(m, x, y) != EMPTY) {
        log(3, "ASSERT EMPTY(%d, %d) = %c\n", x, y, P(m, x, y));
        exit(0);
    }
    //P(m, x, y) = CURR;
    //print_map(m);
    //P(m, x, y) = EMPTY;
    //print_map(m);

    log(4, "DOWN = (%d,%d)=%c %c\n", x, y+1, P(m, x, y+1), P(m, 7, 9));

    if (P(m, x, y+1) == EMPTY) {                  /* down */
        //log(4, "zob1 %c!=%c\n", P(m, x, y+1), EMPTY);
        if ((tmp = drop_sand(m, x, y+1)) < 0)
            return INF;
        ret += tmp;
    }
    if (//P(m, x-1, y) == EMPTY &&
        P(m, x-1, y+1) == EMPTY) {                /* left */
        //puts("zob2");
        if ((tmp = drop_sand(m, x-1, y+1)) < 0)
            return INF;
        ret += tmp;
    }
    if (//P(m, x+1, y) == EMPTY &&
        P(m, x+1, y+1) == EMPTY) {                /* right */
        //puts("zob3");
        if ((tmp = drop_sand(m, x+1, y+1)) < 0)
            return INF;
        ret += tmp;
    }
    /* the 3 lower adjacent cells are filled */
    P(m, x, y) = SAND;
    m->dropped++;
    if (y == 0) {
        print_map(m);
        return INF;
    }
    log(3, "DROPPED=%d\n", m->dropped);
    if (!(m->dropped %100))
        print_map(m);
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
        m->xmin = 500 - m->ymax;
        m->xmax = 500 + m->ymax;
        m->deltax = m->xmin - 1;
        m->size_x = (m->xmax - m->xmin) + 3;
        //m->size_x = m->xmax;
        m->size_y = m->ymax + 2;
    }
    size_t size = m->size_x * m->size_y;
    //m->deltax = - 1;

//m->x1 -= m->deltax;
    //m->y1 -= m->deltay;
    //m->x2 -= m->deltax;
    //m->y2 -= m->deltay,
    m->dropcol = 500 - m->deltax;
    m->dropped = 0;

    m->m = malloc(size);
    memset(m->m, '.', size);
    list_for_each_entry(cur, &segments, list) {
        int x1 = cur->x1 - m->deltax, y1 = cur->y1 - m->deltay,
            x2 = cur->x2 - m->deltax, y2 = cur->y2 - m->deltay;
        int dx = 0, dy = 0;
        if (x1 == x2)
            dy = y2 - y1 > 0 ? 1 : -1;
        else
            dx = x2 - x1 > 0 ? 1 : -1;
        log(3, "From (%d,%d) -> (%d,%d), dx=%d dy=%d <=> (%d,%d) -> (%d,%d)\n",
            cur->x1, cur->y1, cur->x2, cur->y2, dx, dy, x1, y1, x2, y2 );
        do {
            log(3, "Setting (%d,%d)\n", x1, y1);
            P(m, x1, y1) = '#';
            x1 += dx, y1 += dy;
        } while (x1 != x2 || y1 != y2);
        log(3, "Setting (%d,%d)\n", x2, y2);
        P(m, x2, y2) = '#';
        log(2, "segment: (%d,%d) -> (%d,%d)\n", cur->x1, cur->y1, cur->x2, cur->y2);
    }
    if (part == 2)
        for (int i = 0; i < m->xmax; ++i)
            P(m, i, m->ymax) = STONE;

    log(3, "deltax=%d dx=%d dy=%d size=%lu drop=%d\n\n",
        m->deltax, m->size_x, m->size_y, size, m->dropcol);
    print_map(m);
    return m;
}

static struct map *parse()
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
        log(3, "INPUT: len=%lu <%s>\n", buflen, buf);
        while (1) {
            scanned = sscanf(cur, "%d,%d ->%n", &x, &y, &n);
            log(5, "scanned=%d n=%d x=%d y=%d\n", scanned, n, x, y);
            if (scanned  != 2)
                break;
            map.xmin = min(x, map.xmin);
            map.xmax = max(x, map.xmax);
            map.ymin = min(y, map.ymin);
            map.ymax = max(y, map.ymax);
            if (i) {
                x1 = x2;
                y1 = y2;
            }
            x2 = x;
            y2 = y;
            if (!i) {                             /* first point */
                goto next;
            }
            segment = pool_get(pool_segment);
            segment->x1 = x1;
            segment->y1 = y1;
            segment->x2 = x2;
            segment->y2 = y2;
            log(3, "segment: (%d,%d) -> (%d,%d)\n", x1, y1, x2, y2);
            if (x1 != x2 && y1 != y2) {
                log(3, "Ooops!\n");
                exit(1);
            }
            list_add_tail(&segment->list, &segments);
        next:
            i++;
            cur += n;
            if (cur - buf >= buflen)
                break;
        }
    }
    log(3, "xmin=%d xmax=%d ymin=%d ymax=%d\n", map.xmin, map.xmax, map.ymin, map.ymax);
    print_segments();
    return &map;
}

static int part1()
{
    struct map *m = gen_map(parse(), 1);
    drop_sand(m, m->dropcol, 0);
    return m->dropped;
}

static int part2()
{
    struct map *m = gen_map(parse(), 2);
    drop_sand(m, m->dropcol, 0);
    return m->dropped;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_segment =  pool_create("segment", 512, sizeof(segment_t));

    printf("%s: res=%d\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_segment);
    exit(0);
}
