/* aoc-c.c: Advent of Code 2022, day 15
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

static pool_t *pool_segment, *pool_pair;

#define HBITS 20                                  /* 20 bits: 1,048,576 buckets */
struct coord {
    int x, y;
};

#define TOP    0
#define RIGHT  1
#define BOTTOM 2
#define LEFT   3

/**
 * struct pair - input file pair list
 * @sensor, @beacon: struct coord sensor and beacon coordinates.
 * @manhattan: manhattan distance between sensor and beacon.
 * @parity: beacon coordinates parity (as bishop color in chess).
 * @corners: coordinates of rhombus immediately out of sensor range (clockwise).
 * @list: list of pairs.
 */
struct pair {
    struct coord sensor, beacon;
    int manhattan;
    int parity;
    struct coord corners[4];
    struct list_head list;
};
LIST_HEAD(pairs_head);

/**
 * struct row - row description
 * @row: row number.
 * @segments: segments list.
 * @hlist: htable bucket list.
 */
struct row {
    int row;
    int beacons[64];
    int nbeacons;
    struct list_head segments;
};

/**
 * struct map - full map
 * @min, @max: map's min and max coordinates.
 * @hash: rows hash table
 */
static struct map {
    struct coord min, max;
    struct row row;                               /* for part 1 */
    //hlist_head *hash;
} map = {
    .min = { INT_MIN, INT_MIN }, .max = {INT_MAX, INT_MAX },
    .row = { 0, {0}, 0, LIST_HEAD_INIT(map.row.segments) }
};

/**
 * struct segment - The main segment structure
 * @row: the row number
 * @start, @end: segment start and end
 * @list: sorted row's segments list
 *
 * If a row contains 2 segments 1-3 and 7-8, it would be represented as:
 *   +----------+        +----------+
 *   | start: 1 |<------>| start: 7 |
 *   | end: 3   |        | end: 8   |
 *   +----------+        +----------+
 *
 * This implies adding a segment must manage merging. For example, adding
 * segment 2-4 above would change the first segment to 1-4, or adding 0-9
 * should change the first segment to 0-9 and remove the second one.
 */
struct segment {
    int row;
    int start, end;
    struct list_head list;
};

static struct segment *get_segment(int row, int start, int end)
{
    struct segment *new = pool_get(pool_segment);

    log_f(5, "alloc segment (%d,%d) on row (%d)\n", start, end, row);
    new->row=row;
    new->start=start;
    new->end=end;
    INIT_LIST_HEAD(&new->list);
    return new;
}

static void merge_segment(int start, int end)
{
    struct segment *seg, *new;
    struct list_head *cur, *tmp;
    static int l = 9;

    new = get_segment(map.row.row, start, end);
    if (list_empty(&map.row.segments)) {
        list_add(&new->list, &map.row.segments);
        goto end;
    }
    list_for_each_safe(cur, tmp, &map.row.segments) {
        seg = list_entry(cur, struct segment, list);

        /* 1) check for disjoint segments */
        if (start > seg->end + 1) {
            continue;
        }
        if (end < seg->start - 1) {
            list_add_tail(&new->list, &seg->list);
            goto end;
        }

        /* 2) new is inside cur: do nothing */
        if (start >= seg->start && end <= seg->end) {
            log_f(l, "  overlap IN, do nothing\n");
            pool_add(pool_segment, new);
            goto end;
        }
        /* 3) cur inside new: remove cur */
        if (start <= seg->start && end >= seg->end) {
            log_f(l, "  overlap OUT, remove current\n");
            // TODO: avoid this
            list_del(cur);
            pool_add(pool_segment, seg);
            continue;
        }

        /* 4) new segment start is within current one */
        if (start >= seg->start && start <= seg->end + 1) {
            new->start = seg->start;
            list_del(cur);
            pool_add(pool_segment, seg);
            continue;
        }

        /* 5) new segment is left-adjacent to current */
        if (end == seg->start - 1) {
            seg->start = start;
            pool_add(pool_segment, new);
            goto end;
        }

        /* from here, we know there is an overlap */

        /* 6) adjust new start to current start */
        if (start >= seg->start)
            new->start = seg->start;

        /* 7) remove current if covered by new */
        if (end >= seg->end){
            list_del(cur);
            pool_add(pool_segment, seg);
            continue;
        }
        /* 8) replace current with new - finished */
        new->end = seg->end;
        list_add_tail(&new->list, cur);
        list_del(cur);
        pool_add(pool_segment, seg);
        goto end;
    }
    list_add_tail(&new->list, &map.row.segments);
end:
    return;
}

static __always_inline void add_beacon(int bx)
{
    for (int i = 0; i < map.row.nbeacons; ++i) {
        if (map.row.beacons[i] == bx)
            return;
    }
    map.row.beacons[map.row.nbeacons++] = bx;
}

/**
 * is_off_range() - test if a point is off range from all sensors.
 */
static __always_inline int is_off_range(struct coord *point)
{
    struct pair *pair;

    /* reverse loop, because higher manhattan means higher chances to fail */
    list_for_each_entry_reverse(pair, &pairs_head, list) {
        if ((abs(point->x - pair->sensor.x) +
             abs(point->y - pair->sensor.y)) <= pair->manhattan)
            return 0;
    }
    return 1;
}

static struct pair *parse()
{
    int ret;
    struct pair *pair = NULL, *cur;
    struct coord sensor, beacon;

    ret = scanf("%*[^-0-9]%d%*[^-0-9]%d%*[^-0-9]%d%*[^-0-9]%d",
                &sensor.x, &sensor.y, &beacon.x, &beacon.y);
    if (ret == 4) {
        pair = pool_get(pool_pair);
        pair->sensor = sensor;
        pair->beacon = beacon;
        pair->manhattan = abs(beacon.x - sensor.x) + abs(beacon.y - sensor.y);
        pair->parity = (pair->beacon.x + pair->beacon.y) % 2;
        pair->corners[TOP]    = (struct coord) {
            sensor.x, sensor.y - pair->manhattan - 1
        };
        pair->corners[BOTTOM] = (struct coord) {
            sensor.x, sensor.y + pair->manhattan + 1
        };
        pair->corners[RIGHT]  = (struct coord) {
            sensor.x + pair->manhattan + 1, sensor.y
        };
        pair->corners[LEFT]   = (struct coord) {
            sensor.x - pair->manhattan - 1, sensor.y
        };

        /* keep list ordered by manhattan */
        if (!list_empty(&pairs_head)) {
            list_for_each_entry(cur, &pairs_head, list) {
                if (cur->manhattan > pair->manhattan) {
                    list_add_tail(&pair->list, &cur->list);
                    goto end;
                }
            }
        }
        list_add_tail(&pair->list, &pairs_head);
    }
end:
    return pair;
}

/**
 *                                    /#\
 *                           /#\     /# #\
 *                          /# #\   /#   #\
 *                         /#   #\O/#        <--- O is a possible point
 *                               #X#
 *                       rhomb A /#\ rhom B
 *                              /# #\
 *                             /#   #\
 *                            /#     #\
 *                           /#       #\
 *                          /# rhombs  #\
 *                             A & B
 *                          (intersection)
 */

/**
 * intersect() - find intersection of two segments
 *
 */
static __always_inline struct coord *intersect(struct coord *p1, struct coord *p2,
                                               struct coord *q1, struct coord *q2,
                                               struct coord *ret)
{
    int a1, a2, b1, b2, x, y;

    /* a1, b1, a2, b2 are the formulas of (p1, p2) and (q1, q2), such as:
     *     y = ax + b
     * a = (y2 - y1) / (x2 - x1)     x2 ≠ x1
     * b = y - a * x                 We can take either p1 or p2 coordinates
     */
    a1 = (p2->y - p1->y) / (p2->x - p1->x);
    b1 = p1->y - p1->x * a1;
    a2 = (q2->y - q1->y) / (q2->x - q1->x);
    b2 = q1->y - q1->x * a2;

    /* Lines intersection (x,y) is at:
     * (a1 * x) + b1 = (a2 * x) + b2
     * x * (a1 - a2) = b2 - b1
     * x = (b2 - b1) / (a1 - a2)     a2 ≠ a1
     * Then we find y = ax + b
     */
    x  = (b2 - b1) / (a1 - a2);
    y  = a1 * x + b1;

    /* check if intersection is:
     * 1) Within p1-p2 and q1-q2 segments
     * 2) Within map area
     */
    if (x >= min(min(p1->x, p2->x), min(q1->x, q2->x)) &&
        x <= max(max(p1->x, p2->x), max(q1->x, q2->x)) &&
        y >= min(min(p1->y, p2->y), min(q1->y, q2->y)) &&
        y <= max(max(p1->y, p2->y), max(q1->y, q2->y)) &&
        x >= map.min.x && x <= map.max.x &&
        y >= map.min.y && y <= map.max.y) {
        *ret = (struct coord) {x, y};
    } else {
        ret = NULL;
    }
    return ret;
}

#define T_R(p) &p->corners[TOP], &p->corners[RIGHT]
#define R_B(p) &p->corners[RIGHT], &p->corners[BOTTOM]
#define B_L(p) &p->corners[BOTTOM], &p->corners[LEFT]
#define L_T(p) &p->corners[LEFT], &p->corners[TOP]

static struct coord *check_intersect(struct coord *ret)
{
    struct pair *pair, *second;

    list_for_each_entry(pair, &pairs_head, list) {
        second = list_prepare_entry(pair, &pairs_head, list);
        list_for_each_entry_continue(second, &pairs_head, list) {
            if (second->parity == pair->parity) {
                /* top right segment */
                if ((intersect(T_R(pair), R_B(second), ret) && is_off_range(ret)) ||
                    (intersect(T_R(pair), L_T(second), ret) && is_off_range(ret)))
                    return ret;
                /* bottom left segment */
                if ((intersect(B_L(pair), R_B(second), ret) && is_off_range(ret)) ||
                    (intersect(B_L(pair), L_T(second), ret) && is_off_range(ret)))
                    return ret;
                /* right bottom segment */
                if ((intersect(R_B(pair), T_R(second), ret) && is_off_range(ret)) ||
                    (intersect(R_B(pair), B_L(second), ret) && is_off_range(ret)))
                    return ret;
                /* left top segment */
                if ((intersect(L_T(pair), T_R(second), ret) && is_off_range(ret)) ||
                    (intersect(L_T(pair), B_L(second), ret) && is_off_range(ret)))
                    return ret;
            }
        }
    }
    return NULL;
}

static u64 part1(void)
{
    u64 res = 0;
    struct pair *pair;
    struct segment *cur;

    map.row.row = map.min.y = map.max.y = testmode() ? 10: 2000000;

    while ((pair = parse())) {
        if (map.row.row >= pair->sensor.y - pair->manhattan &&
            map.row.row <= pair->sensor.y + pair->manhattan) {
            int half = pair->manhattan - abs(map.row.row - pair->sensor.y);
            int x1 = max(pair->sensor.x - half, map.min.x);
            int x2 = max(pair->sensor.x + half, map.min.x);
            merge_segment(x1, x2);
            if (map.row.row == pair->beacon.y)
                add_beacon(pair->beacon.x);
        }
    }
    list_for_each_entry(cur, &map.row.segments, list)
        res += cur->end - cur->start + 1;
    return res - map.row.nbeacons;
}

static u64 part2()
{
    u64 res = 0;
    struct coord result = {0, 0};

    map.min.x = map.min.y =  0;
    map.max.x = map.max.y = testmode()? 20: 4000000;

    while (parse())
        ;
    check_intersect(&result);
    res = ((u64)result.x) * 4000000UL + (u64)result.y;
    return res;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);

    pool_segment =  pool_create("segments", 8192, sizeof(struct segment));
    pool_pair = pool_create("pair", 32, sizeof(struct pair));

    printf("%s: res=%lu\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_segment);
    pool_destroy(pool_pair);
    exit(0);
}
