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

typedef enum { DOWN = 'D', LEFT = 'L', RIGHT = 'R', UP = 'U' } dir_t;

struct point {
    int x, y;
};

/**
 * struct wire - one segment on wire.
 * @dist: total distance from start before the current segment
 * @dir: indicates which of ends[0] and ends[1] was the original line end point
 * @ends: segment ends, ordered (ends[0].x <= ends[1].x and ends[0].y <= ends[1].y)
 * @list: segments list
 */
struct wire {
    int dist;
    int dir;
    struct point ends[2];
    struct list_head list;
};

static pool_t *wire_pool;

static struct list_head *parse(struct list_head *wires)
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL, *token;

    for (int line = 0; line < 2; ++line) {
        struct point p0 = {0, 0}, p1 = {0, 0};
        int totdist = 0;

        if ((buflen = getline(&buf, &alloc, stdin)) <= 0) {
            fprintf(stderr, "error %d reading input.\n", errno);
            wires = NULL;
            goto end;
        }

        for (token = strtok(buf, ","); token; token = strtok(NULL, ",")) {
            dir_t dir = *token;
            int dist = atoi(token + 1);
            struct wire *new = pool_get(wire_pool);

            INIT_LIST_HEAD(&new->list);
            new->dist = totdist;
            totdist += abs(dist);
            new->dir = !!(dir == DOWN || dir == LEFT);
            if (dir == DOWN || dir == UP)         /* vertical segment */
                p1.y = p0.y + (1 - (new->dir * 2)) * dist;
            else                                  /* horizontal segment */
                p1.x = p0.x + (1 - (new->dir * 2)) * dist;
            new->ends[0].x = min(p0.x, p1.x);     /* order ends (p0.? <= p1.?) */
            new->ends[0].y = min(p0.y, p1.y);
            new->ends[1].x = max(p0.x, p1.x);
            new->ends[1].y = max(p0.y, p1.y);
            list_add_tail(&new->list, &wires[line]);
            p0 = p1;
        }
    }
end:
    free(buf);
    return wires;
}

static struct point *intersect(struct wire *w0, struct wire *w1, struct point *ret)
{
    if (w0->ends[0].x == w0->ends[1].x) {         /* w0 vertical */
        /* BUG: overlapping wires is not handled */
        if (w0->ends[0].x >= w1->ends[0].x && w0->ends[1].x <= w1->ends[1].x &&
            w0->ends[0].y <= w1->ends[0].y && w0->ends[1].y >= w1->ends[1].y) {
            ret->x = w0->ends[0].x;
            ret->y = w1->ends[0].y;
            return ret;
        }
        return NULL;
    } else {                                      /* w0 horizontal */
        if (w0->ends[0].x <= w1->ends[0].x && w0->ends[1].x >= w1->ends[1].x &&
            w0->ends[0].y <= w1->ends[1].y && w0->ends[1].y >= w1->ends[0].y) {
            ret->x = w1->ends[0].x;
            ret->y = w0->ends[0].y;
            return ret;
        }
        return NULL;
    }
}

/*  part 1 */
#define manhattan(point) (abs(point.x) + abs(point.y))
/* part 2 */
#define signal(point, w0, w1)                                                  \
    (w0->dist + w1->dist +                                                     \
     abs(cross.x - w0->ends[w0->dir].x) + abs(cross.y - w0->ends[w0->dir].y) + \
     abs(cross.x - w1->ends[w1->dir].x) + abs(cross.y - w1->ends[w1->dir].y))

static int doit(struct list_head *w, int part)
{
    struct wire *w0, *w1;
    struct point cross;
    s32 res = INT32_MAX;

    list_for_each_entry(w0, &w[0], list)
        list_for_each_entry(w1, &w[1], list)
            if (intersect(w0, w1, &cross))
                res = min_not_zero(res, part == 1 ?
                                   manhattan(cross) :
                                   signal(cross, w0, w1));
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
    struct list_head wires[2] = { LIST_HEAD_INIT(wires[0]), LIST_HEAD_INIT(wires[1]) };

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                part = atoi(optarg);
                if (part < 1 || part > 2)
            default:
                    return usage(*av);
        }
    }

    if (optind < ac)
        return usage(*av);
    wire_pool = pool_create("wire", 256, sizeof(struct wire));

    if (parse(wires)) {
        printf("%s : res=%d\n", *av, doit(wires, part));
        pool_destroy(wire_pool);
    }
    exit (0);
}
