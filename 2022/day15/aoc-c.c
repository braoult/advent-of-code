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

static pool_t *pool_segment, *pool_row;

#define HBITS 20                                  /* 20 bits: 1,048,576 buckets */
static DEFINE_HASHTABLE(hasht_rows, HBITS);

/**
 * struct map - full map
 * @xmin, @xmax: most left/right x-coordinates.
 * @ymin, @ymax: most top/bottom y-coordinates.
 * @hash: rows hash table
 */
struct map {
    int xmin, xmax, ymin, ymax;
    struct hlist_head *hash;
} map = {
    //INT_MAX, INT_MIN, INT_MAX, INT_MIN, hasht_rows
    INT_MIN, INT_MAX, INT_MIN, INT_MAX, hasht_rows
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

static void print_segments()
{
    struct row *prow;
    struct segment *s;

    //log_f(1, "xmin=%d xmax=%d\n", map.xmin, map.xmax);
    if (! testmode())
        return;

    for (int y = -3; y <= 22; ++y) {
        if ((prow = find_row(&map.hash[hash_32(y, HBITS)], y))) {
            log(1, "%2d ", y);
            log(5, "prow(%d->%d)=%p\n", y, hash_32(y, HBITS), prow);
            int count = 0;
            list_for_each_entry(s, &prow->segments, list) {
                log(1, "%s (%d,%d)", count? " -> ":"", s->start, s->end);
            }
            log(1, "\n");
        }
    }
}

static void print_map()                           /* for test mode only */
{
    struct row *prow;
    struct segment *s;

    //log_f(1, "xmin=%d xmax=%d\n", map.xmin, map.xmax);
    if (! testmode())
        return;
    log(1, "    -              1    1    2    2    3\n");
    log(1, "    5    0    5    0    5    0    5    0\n");

    for (int y = -3; y <= 22; ++y) {
        if ((prow = find_row(&map.hash[hash_32(y, HBITS)], y))) {
            log(1, "%2d ", y);
            log(5, "prow(%d->%d)=%p\n", y, hash_32(y, HBITS), prow);
            int x = -6;
            list_for_each_entry(s, &prow->segments, list) {
                //log_f(1, "segment start=%d end=%d\n", s->start, s->end);
                for (; x <= 30 && x < s->start; ++x) {
                    log(1, ".");
                }
                for (; x <= 30 && x <= s->end; ++x) {
                    log(1, "#");
                }
            }
            for (; x <= 30; ++x) {
                log(1, ".");
            }
            log(1, "\n");
        }
    }
}

static struct segment *get_segment(int row, int x1, int x2)
{
    struct segment *new = pool_get(pool_segment);

    log_f(5, "alloc segment (%d,%d) on row (%d)\n", x1, x2, row);
    new->row=row;
    new->start=x1;
    new->end=x2;
    INIT_LIST_HEAD(&new->list);
    return new;
}

static int merge_segment(struct row *prow, int start, int end)
{
    struct segment *seg, *new;
    struct list_head *cur, *tmp;
    int l;

    l = debug_level_get();
    //if (prow->row != 9)
        l++;

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

static void add_beacon(int bx, int by)
{
    uint hash = by, bucket = hash_32(hash, HBITS);
    struct row *prow = find_row(&map.hash[bucket], hash);

    if (!prow) {
        puts("fuck)");
        exit(1);
    }
    for (int i = 0; i < prow->nbeacons; ++i) {
        if (prow->beacons[i] == bx)
            return;
    }
    prow->beacons[prow->nbeacons++] = bx;
}

static int add_segment(int row, int center, int half)
{
    int x1, x2;
    uint hash = row, bucket = hash_32(hash, HBITS);
    struct row *prow = find_row(&map.hash[bucket], hash);

    x1 = max(center - half, map.xmin);
    x2 = min(center + half, map.xmax);
    if (x1 != center - half || x2 != center + half)
        log(1, "adjust x: min:%d->%d max:%d->%d\n",
            center - half, x1, center + half, x2);
    log_f(3, "adding segment (%d,%d) on row (%d) - bucket(%u) = %u prow=%p\n",
          x1, x2, row, hash, bucket, prow);
    /*
     * if (row < map.ymin) {
     *     log(5, "new ymin=%d->%d\n", map.ymin, row);
     *     map.ymin = row;
     * }
     * if (row > map.ymax) {
     *     log(5, "new ymax=%d->%d\n", map.ymax, row);
     *     map.ymax = row;
     * }
     * if (x1 < map.xmin) {
     *     log(5, "new xmin=%d->%d\n", map.xmin, x1);
     *     map.xmin = x1;
     * }
     * if (x2 > map.xmax) {
     *     log(5, "new xmax=%d->%d\n", map.xmax, x2);
     *     map.xmax = x2;
     * }
     */
    log(3, "map borders: xmin=%d xmax=%d ymin=%d ymax=%d\n",
        map.xmin, map.xmax, map.ymin, map.ymax);
    if (!prow) {
        prow = pool_get(pool_row);
        prow->row = row;
        prow->nbeacons = 0;
        INIT_HLIST_NODE(&prow->hlist);
        INIT_LIST_HEAD(&prow->segments);
        hlist_add_head(&prow->hlist, &map.hash[bucket]);
    }
    merge_segment(prow, x1, x2);
    return 1;
}

static int add_segments(int sx, int sy, int bx, int by)
{
    int manhattan = abs(bx - sx) + abs(by - sy);
    int ymin = max(sy - manhattan, map.ymin);
    int ymax = min(sy + manhattan, map.ymax);

    log_f(2, "sensor4=(%d, %d) beacon=(%d, %d) - ", sx, sy, bx, by);
    log(2, "manhattan=%u ymin=%d ymax=%d\n", manhattan, ymin, ymax);
    //add_segment(sy, sx, manhattan);

    for (int y = ymin; y <= ymax; ++y) {
        int half = manhattan - abs(y - sy);
        add_segment(y, sx, half);
        if (y == by)
            add_beacon(bx, by);
        //add_segment(y, sx, half);
    }
    //for (int dy = 1, half = manhattan - 1; dy <= manhattan; ++dy, half--) {
    //       add_segment(sy - dy, sx, half);
    //  add_segment(sy + dy, sx, half);
    //}
    //add_beacon(bx, by);
    return 1;
}

static inline int parse(int *sx, int *sy, int *bx, int *by)
{
    int ret = scanf("%*[^-0-9]%d%*[^-0-9]%d%*[^-0-9]%d%*[^-0-9]%d",
                    sx, sy, bx, by);

    return ret;
}

static ulong part1()
{
    ulong res = 0;
    int row = testmode() ? 10: 2000000;
    uint bucket = hash_32(row, HBITS);
    int sx, sy, bx, by;

    map.ymin = row - 1;
    map.ymax = row + 1;
    while (parse(&sx, &sy, &bx, &by) > 0) {
        int manhattan = abs(bx - sx) + abs(by - sy);
        add_segments(sx, sy, bx, by);
        log(3, "m=%d : ", manhattan);
    }
    struct row *prow = find_row(&map.hash[bucket], row);
    if (prow) {
        struct segment *cur;
        print_map();
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
    int sx, sy, bx, by;

    map.xmin = map.ymin =  0;
    map.xmax = map.ymax = testmode()? 20: 4000000;
    while ((parse(&sx, &sy, &bx, &by)) > 0) {
        int manhattan = abs(bx - sx) + abs(by - sy);

        add_segments(sx, sy, bx, by);
        log(3, "m=%d : ", manhattan);
    }
    for (int row = map.ymin; row <= map.ymax; ++row) {
        uint bucket = hash_32(row, HBITS);
        struct row *prow = find_row(&map.hash[bucket], row);
        if (!prow) {
            printf("fuck 1: prow(%d)=NULL\n", row);
            exit(1);
        }
        struct segment *cur;
        if (list_empty(&prow->segments)) {
            puts("fuck 2\n");
            continue;
        }

        cur = list_first_entry(&prow->segments, struct segment, list);
        if (cur->end != map.xmax) {
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

    printf("%s: res=%lu\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_row);
    pool_destroy(pool_segment);
    exit(0);
}
