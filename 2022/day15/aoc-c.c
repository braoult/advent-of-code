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
#include "hashtable.h"
#include "pjwhash-inline.h"
#include "debug.h"

#include "aoc.h"

static pool_t *pool_segment, *pool_row, *pool_pair;

#define HBITS 20                                  /* 20 bits: 1,048,576 buckets */
static DEFINE_HASHTABLE(hasht_rows, HBITS);

struct coord {
    int x, y;
};

/**
 * struct pair - input file pair list
 * @sensor, @beacon: struct coord sensor and beacon coordinates.
 * @manhattan: manhattan distance between sensor and beacon.
 * @parity: beacon coordinates parity (as bishop color in chess).
 * @top, @right, @bottom, @left: coordinates of rhombus immediately out
 *                        range of sensor.
 * @list: list of pairs.
 */
struct pair {
    struct coord sensor, beacon;
    int manhattan;
    int parity;
    struct coord top, bottom, left, right;
    struct list_head list;
};
LIST_HEAD(pairs_head);

/**
 * struct map - full map
 * @xmin, @xmax: most left/right x-coordinates.
 * @ymin, @ymax: most top/bottom y-coordinates.
 * @hash: rows hash table
 */
struct map {
    struct coord min, max; //int xmin, xmax, ymin, ymax;
    struct hlist_head *hash;
} map = {
    //INT_MAX, INT_MIN, INT_MAX, INT_MIN, hasht_rows
    .min = { INT_MIN, INT_MIN }, .max = {INT_MAX, INT_MAX }, hasht_rows
};

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
    struct hlist_node hlist;
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

static struct row *find_row(struct hlist_head *head, int row)
{
    struct row *cur;
    hlist_for_each_entry(cur, head, hlist)
        if (cur->row == row)
            return cur;
    return NULL;
}

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

static int merge_segment(struct row *prow, int start, int end)
{
    struct segment *seg, *new;
    struct list_head *cur, *tmp;
    static int l = 9;

    //l = debug_level_get();
    //if (prow->row != 9)
    //    l++;

    log_f(l, "merging segment (%d,%d) on row (%d)\n", start, end, prow->row);
    new = get_segment(prow->row, start, end);
    if (list_empty(&prow->segments)) {
        log_f(l, "  first segment\n");
        list_add(&new->list, &prow->segments);
        goto end;
    }
    list_for_each_safe(cur, tmp, &prow->segments) {
        seg = list_entry(cur, struct segment, list);
        log_f(l, "compare to (start=%d end=%d)\n", seg->start, seg->end);

        /* 1) check for disjoint segments */
        if (start > seg->end + 1) {
            log_f(l, "  skipping (%d,%d)\n", seg->start, seg->end);
            continue;
        }
        if (end < seg->start - 1) {
            log_f(l, "  adding before (%d,%d)\n", seg->start, seg->end);
            list_add_tail(&new->list, &seg->list);
            goto end;
        }

        /* new is inside cur: do nothing */
        if (start >= seg->start && end <= seg->end) {
            log_f(l, "  overlap IN, do nothing\n");
            pool_add(pool_segment, new);
            goto end;
        }
        /* cur inside new: remove cur */
        if (start <= seg->start && end >= seg->end) {
            log_f(l, "  overlap OUT, remove current\n");
            // TODO: avoid this
            list_del(cur);
            pool_add(pool_segment, seg);
            continue;
        }

        /* 2) adjacent block */
        if (start >= seg->start && start <= seg->end + 1) {
            log_f(l, "  setting new start to %d\n", seg->start);
            new->start = seg->start;
            list_del(cur);
            pool_add(pool_segment, seg);
            continue;
        }
        if (end == seg->start - 1) {
            seg->start = start;
            pool_add(pool_segment, new);
            goto end;
        }

        /* we know here there is at least one overlap or contiguous */
        log_f(l, "  could merge new=(%d,%d) with cur=(%d,%d)\n",
              start, end, seg->start, seg->end);

        /* exactly one overlap */
        log_f(3, "  exactly one overlap\n");

        if (start >= seg->start) {
            log_f(l, "  overlap left new=(%d,%d)\n", new->start, new->end);
            new->start = seg->start;
        }
        if (end >= seg->end){
            log_f(l, "  overlap right: delete cur\n");
            list_del(cur);
            pool_add(pool_segment, seg);
            continue;
        }
        /* we stop here */
        log_f(l, "  stop here\n");
        new->end = seg->end;
        list_add_tail(&new->list, cur);
        list_del(cur);
        pool_add(pool_segment, seg);
        goto end;
    }
    log_f(l, "  adding at end of list\n");
    list_add_tail(&new->list, &prow->segments);
end:
    //print_segments();
    return 10;
}

static void add_beacon(struct row *prow, int bx)
{
    for (int i = 0; i < prow->nbeacons; ++i) {
        if (prow->beacons[i] == bx)
            return;
    }
    prow->beacons[prow->nbeacons++] = bx;
}

static struct row *add_segment(int row, int center, int half)
{
    int x1, x2;
    uint hash = row, bucket = hash_32(hash, HBITS);
    struct row *prow = find_row(&map.hash[bucket], hash);

    x1 = max(center - half, map.min.x);
    x2 = min(center + half, map.max.x);
    if (x1 != center - half || x2 != center + half)
        log(1, "adjust x: min:%d->%d max:%d->%d\n",
            center - half, x1, center + half, x2);
    log_f(3, "adding segment (%d,%d) on row (%d) - bucket(%u) = %u prow=%p\n",
          x1, x2, row, hash, bucket, prow);
    log(3, "map borders: xmin=%d xmax=%d ymin=%d ymax=%d\n",
        map.min.x, map.max.x, map.min.y, map.max.y);
    if (!prow) {
        prow = pool_get(pool_row);
        prow->row = row;
        prow->nbeacons = 0;
        INIT_HLIST_NODE(&prow->hlist);
        INIT_LIST_HEAD(&prow->segments);
        hlist_add_head(&prow->hlist, &map.hash[bucket]);
    }
    merge_segment(prow, x1, x2);
    return prow;
}

/**
 * is_off_range() - test if a point is off range from a sensor
 */
static int is_off_range(struct pair *pair, struct coord *point)
{
    return (abs(point->x - pair->sensor.x)
            + abs(point->y - pair->sensor.y)) > pair->manhattan;
}

static int add_segments(struct pair *pair)
{
    int manhattan = pair->manhattan,
        ymin = max(pair->sensor.y - manhattan, map.min.y),
        ymax = min(pair->sensor.y + manhattan, map.max.y);
    struct row *prow;

    log_f(2, "sensor4=(%d, %d) beacon=(%d, %d) - ",
          pair->sensor.x, pair->sensor.y, pair->beacon.x, pair->beacon.y);
    log(2, "manhattan=%u ymin=%d ymax=%d\n", manhattan, ymin, ymax);

    for (int y = ymin; y <= ymax; ++y) {
        int half = pair->manhattan - abs(y - pair->sensor.y);
        prow = add_segment(y, pair->sensor.x, half);
        if (y == pair->beacon.y)
            add_beacon(prow, pair->beacon.x);
    }
    return 1;
}

static void print_pairs()
{
    struct pair *pair;
    log_f(1, "**************** pairs\n");
    list_for_each_entry(pair, &pairs_head, list) {
        printf("m=%d p=%s s=(%d,%d) b=(%d,%d)\n",
               pair->manhattan,
               pair->parity? "WHITE": "BLACK",
               pair->sensor.x, pair->sensor.y,
               pair->beacon.x, pair->beacon.y);
        printf("  top=(%d,%d) bottom=(%d,%d) left=(%d,%d) right=(%d,%d)\n",
               pair->top.x, pair->top.y, pair->bottom.x, pair->bottom.y,
               pair->left.x, pair->left.y, pair->right.x, pair->right.y);
    }
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
        pair->top    = (struct coord) { sensor.x, sensor.y - pair->manhattan - 1 };
        pair->bottom = (struct coord) { sensor.x, sensor.y + pair->manhattan + 1 };
        pair->right  = (struct coord) { sensor.x + pair->manhattan + 1, sensor.y };
        pair->left   = (struct coord) { sensor.x - pair->manhattan - 1, sensor.y };

        if (list_empty(&pairs_head)) {
            list_add(&pair->list, &pairs_head);
            goto end;
        }
        list_for_each_entry(cur, &pairs_head, list) {
            if (cur->manhattan > pair->manhattan) {
                list_add_tail(&pair->list, &cur->list);
                goto end;
            }
        }
        list_add_tail(&pair->list, &pairs_head);
    }
end:
    return pair;
}

static ulong part1()
{
    ulong res = 0;
    int row = testmode() ? 10: 2000000;
    uint bucket = hash_32(row, HBITS);
    struct pair *pair;

    map.min.y = map.max.y = row;

    while ((pair = parse())) {
        add_segments(pair);
    }
    struct row *prow = find_row(&map.hash[bucket], row);
    if (prow) {
        struct segment *cur;
        list_for_each_entry(cur, &prow->segments, list) {
            printf("counting segment (%d,%d) = %d nbeac=%d\n", cur->start, cur->end,
                   cur->end - cur->start + 1, prow->nbeacons);
            res += cur->end - cur->start + 1;
        }
        res -= prow->nbeacons;
    }
    return res;
}

static ulong part2()
{
    ulong res = 0;
    struct pair *pair;

    map.min.x = map.min.y =  0;
    map.max.x = map.max.y = testmode()? 20: 4000000;
    while ((pair = parse())) {
        add_segments(pair);
    }
    for (int row = map.min.y; row <= map.max.y; ++row) {
        uint bucket = hash_32(row, HBITS);
        struct row *prow = find_row(&map.hash[bucket], row);
        struct segment *cur;
        cur = list_first_entry(&prow->segments, struct segment, list);
        if (cur->end != map.max.x) {
            res = ((u64)cur->end + 1UL) * 4000000UL + (u64)row;
            break;
        }
    }
    return res;
}

/**
 * intersect() - return two segments intersection.
 */
static int intersect(float p0_x, float p0_y, float p1_x, float p1_y,
                     float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)
{
    float s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
    s10_x = p1_x - p0_x;
    s10_y = p1_y - p0_y;
    s32_x = p3_x - p2_x;
    s32_y = p3_y - p2_y;

    denom = s10_x * s32_y - s32_x * s10_y;
    if (denom == 0)
        return 0; // Collinear
    bool denomPositive = denom > 0;

    s02_x = p0_x - p2_x;
    s02_y = p0_y - p2_y;
    s_numer = s10_x * s02_y - s10_y * s02_x;
    if ((s_numer < 0) == denomPositive)
        return 0; // No collision

    t_numer = s32_x * s02_y - s32_y * s02_x;
    if ((t_numer < 0) == denomPositive)
        return 0; // No collision

    if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
        return 0; // No collision
    // Collision detected
    t = t_numer / denom;
    if (i_x != NULL)
        *i_x = p0_x + (t * s10_x);
    if (i_y != NULL)
        *i_y = p0_y + (t * s10_y);

    return 1;
}

//static int is_inside(int x, int y, )
static ulong part2_new()
{
    ulong res = 0;
    struct pair *pair;

    map.min.x = map.min.y =  0;
    map.max.x = map.max.y = testmode()? 20: 4000000;

    while ((pair = parse())) {
        //int outside = pair->manhattan + 1;
        //add_segments(pair);
        //log(3, "m=%d : ", manhattan);
    }
    print_pairs();
    return res;
    for (int row = map.min.y; row <= map.max.y; ++row) {
        uint bucket = hash_32(row, HBITS);
        struct row *prow = find_row(&map.hash[bucket], row);
        if (!prow) {
            printf("fuck 1: prow(%d)=NULL\n", row);
            exit(1);
        }
        struct segment *cur;

        cur = list_first_entry(&prow->segments, struct segment, list);
        if (cur->end != map.max.x) {
            res = ((u64)cur->end + 1UL) * 4000000UL + (u64)row;
            break;
        }
    }
    return res;
}


int main(int ac, char **av)
{
    int part = parseargs(ac, av);

    pool_row =  pool_create("rows", 8192, sizeof(struct row));
    pool_segment =  pool_create("segments", 8192, sizeof(struct segment));
    pool_pair = pool_create("pair", 32, sizeof(struct pair));

    part2_new();
    exit(1);
    printf("%s: res=%lu\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_row);
    pool_destroy(pool_segment);
    pool_destroy(pool_pair);
    exit(0);
}
