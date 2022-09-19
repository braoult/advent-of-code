/* aoc-c.c: Advent of Code 2019, day 3 parts 1 & 2
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
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "br.h"
#include "bits.h"
#include "debug.h"
#include "list.h"
#include "pool.h"
#include "debug.h"

typedef enum { DOWN = 'D', LEFT = 'L', RIGHT = 'R', UP = 'U' } dir_t;

struct point {
    int x, y;
};

struct wire {
    int dist;                                     /* total distance before this wire */
    struct point delta;                           /* current delta */
    struct point p0, p1;
    struct list_head list;                        /* wire list */
};

static struct wires {
    int nwires[2];
    struct list_head head[2];
} wires[2];

static pool_t *wire_pool;

static struct wires *parse()
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL, *token;

    /* initialize wires lists */
    INIT_LIST_HEAD(&wires->head[0]);
    INIT_LIST_HEAD(&wires->head[1]);

    for (int line = 0; line < 2; ++line) {
        int x0 = 0, y0 = 0;
        int x1 = 0, y1 = 0;
        int totdist = 0;

        if ((buflen = getline(&buf, &alloc, stdin)) <= 0) {
            fprintf(stderr, "error %d reading file.\n", errno);
            goto end;
        }

        for (token = strtok(buf, ","); token; token = strtok(NULL, ",")) {
            dir_t dir = *token;
            int dist = atoi(token + 1);
            struct wire *new = pool_get(wire_pool);
            INIT_LIST_HEAD(&new->list);
            new->dist = totdist;
            new->delta.x = 0;
            new->delta.y = 0;
            totdist += abs(dist);
            switch(dir) {
                case DOWN:
                    y1 = y0 - dist;
                    new->delta.y = -dist;
                    break;
                case UP:
                    y1 = y0 + dist;
                    new->delta.y = dist;
                    break;
                case LEFT:
                    x1 = x0 - dist;
                    new->delta.x = -dist;
                    break;
                case RIGHT:
                    x1 = x0 + dist;
                    new->delta.x = dist;
            }
            new->p0.x = min(x0, x1);
            new->p0.y = min(y0, y1);
            new->p1.x = max(x0, x1);
            new->p1.y = max(y0, y1);
            list_add_tail(&new->list, &wires->head[line]);
            x0 = x1;
            y0 = y1;
            wires->nwires[line]++;
        }
    }
end:
    free(buf);
    return wires;
}

#define manhattan(x, y) (abs(x) + abs(y))

static struct point *intersect(struct wire *w1, struct wire *w2, struct point *ret)
{
    log_f(3, "(%d,%d)-(%d,%d) (%d,%d)-(%d,%d): ",
          w1->p0.x, w1->p0.y, w1->p1.x, w1->p1.y,
          w2->p0.x, w2->p0.y, w2->p1.x, w2->p1.y);
    if (w1->p0.x == w1->p1.x) {                       /* w1 vertical */
        /* TODO: overlapping wires (multiple intersections) */
        if (w1->p0.x >= w2->p0.x && w1->p1.x <= w2->p1.x &&
            w1->p0.y <= w2->p0.y && w1->p1.y >= w2->p1.y) {
            log(3, "intersect 1 at (%d, %d)\n", w1->p0.x, w2->p0.y);
            ret->x = w1->p0.x;
            ret->y = w2->p0.y;
            return ret;
            //return manhatan(w1->p0.x, w2->p0.y);
        }
        log(3, "no intersection\n");
        return NULL;
    } else {                                      /* w1 horizontal */
        if (w1->p0.x <= w2->p0.x && w1->p1.x >= w2->p1.x &&
            w1->p0.y <= w2->p1.y && w1->p1.y >= w2->p0.y) {
            log(3, "intersect 2 at (%d, %d)\n", w2->p0.x, w1->p0.y);
            ret->x = w2->p0.x;
            ret->y = w1->p0.y;
            return ret;
            //return manhatan(w2->p0.x, w1->p0.y);
        }
        log(3, "no intersection\n");
        return NULL;
    }
}

static s32 part1(struct wires *w)
{
    struct wire *w0, *w1;
    struct point cross;
    s32 res = INT32_MAX;

    list_for_each_entry(w0, &w->head[0], list) {
        list_for_each_entry(w1, &w->head[1], list) {
            if (intersect(w0, w1, &cross)) {
                int tmp = manhattan(cross.x, cross.y);
                log(3, "new manhattan: %d\n", tmp);
                res = min(tmp, res);
            }
        }
    }
    return res;
}

static int part2(struct wires *w)
{
    struct wire *w0, *w1;
    struct point cross;
    s32 res = INT32_MAX;

    list_for_each_entry(w0, &w->head[0], list) {
        list_for_each_entry(w1, &w->head[1], list) {
            if (intersect(w0, w1, &cross)) {
                if (cross.x || cross.y) {
                    int tmp = w0->dist + w1->dist;

                    /* as w->p0 contains the lowest value of (x, y), we lost the
                     * starting point information. We adjust remaining steps with
                     * the delta sign, which indicates if we swapped p0 and p1.
                     */
                    tmp += w0->delta.x + w0->delta.y > 0 ?
                        abs(cross.x - w0->p0.x) + abs(cross.y - w0->p0.y) :
                        abs(cross.x - w0->p1.x) + abs(cross.y - w0->p1.y);
                    tmp += w1->delta.x + w1->delta.y > 0 ?
                        abs(cross.x - w1->p0.x) + abs(cross.y - w1->p0.y) :
                        abs(cross.x - w1->p1.x) + abs(cross.y - w1->p1.y);
                    /*  new best */
                    res = min(tmp, res);
                }
            }
        }
    }
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
    struct wires *wires;

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
    wire_pool = pool_create("wire", 256, sizeof(struct wire));

    wires = parse();
    printf("%s : res=%d\n", *av, part == 1? part1(wires): part2(wires));
    pool_destroy(wire_pool);
    exit (0);
}
