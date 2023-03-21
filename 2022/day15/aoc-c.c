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
    INT_MAX, INT_MIN, INT_MAX, INT_MIN, hasht_rows
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

static void print_map()
{
    struct segment *s;

    printf("XXXXXXX ");
    for (int x = map.xmin; x < 0; ++x)
        putchar(' ');
    printf("0\n");
    for (int y = map.ymin; y <= map.ymax; ++y) {
        printf("%7d ", y);
        struct row *row = find_row(&map.hash[hash_32(y, HBITS)], y);
        int x = map.xmin;
        list_for_each_entry(s, &row->segments, list) {
            for (; x < s->start; ++x) {
                putchar('.');
            }
            for (; x <= s->end; ++x) {
                putchar('#');
            }
        }
        for (; x <= map.xmax; ++x) {
            putchar('.');
        }
        putchar('\n');
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

static int merge_segment(struct row *prow, int x1, int x2)
{
    struct segment *seg, *seg2, *new;
    struct list_head *cur, *tmp, *next;

    log_f(3, "merging segment (%d,%d) on row (%d)\n", x1, x2, prow->row);
    new = get_segment(prow->row, x1, x2);
    if (list_empty(&prow->segments)) {
        log_f(3, "  first segment\n");
        list_add(&new->list, &prow->segments);
        return 1;
    }
    list_for_each_safe(cur, tmp, &prow->segments) {
        seg = list_entry(cur, struct segment, list);
        log_f(3, "compare to (start=%d end=%d)\n", seg->start, seg->end);
        if (x1 > seg->end) {
            log_f(3, "  skipping to next\n");
            continue;
        }
        if (x2 < seg->start) {
            log_f(3, "  adding to left\n");
            list_add_tail(&new->list, &seg->list);
            return 2;
        }
        /* we know here there is at least an overlap */

        /* new is inside cur: do nothing */
        if (x1 >= seg->start && x2 <= seg->end) {
            log_f(3, "  overlap IN, do nothing\n");
            pool_add(pool_segment, new);
            return 3;
        }
        /* cur inside new: remove cur */
        if (x1 <= seg->start && x2 >= seg->end) {
            log_f(3, "  overlap OUT, remove current\n");
            // TODO: avoid this
            list_del(cur);
            pool_add(pool_segment, seg);
            continue;
        }

        /* exactly one overlap */
        log_f(3, "  exactly one overlap\n");

        if (x1 > seg->start) {
            log_f(3, "  overlap left new=(%d,%d)\n", new->start, new->end);
            new->start = seg->start;
        }
        if (x2 >= seg->end){
            log_f(3, "  overlap right: delete cur\n");
            list_del(cur);
            pool_add(pool_segment, seg);
            continue;
        }
        /* we stop here */
        log_f(3, "  stop here\n");
        new->end = seg->end;
        list_add_tail(&new->list, cur);
        list_del(cur);
        pool_add(pool_segment, seg);
        return 4;

        if (x2 <= seg->end) {
            log_f(3, "  extending left side %d->%d\n", seg->start, x1);
            list_add(&new->list, cur);
            list_del(cur);
        }
        if (x1 < seg->start) {
            log_f(3, "  extending left side %d->%d\n", seg->start, x1);
            seg->start = x1;
            if (x2 <= seg->end) {
            } else {
                log_f(3, "  already covered\n");
            }
            return 3;
        }
        /* here we know that 1) x2 > end and 2) x1 <= start */
        if (x1 < seg->start) {
            log_f(3, "  extending temp left side %d->%d\n", seg->start, x1);
            seg->start = x1;
        }
        /* now x1 and cur->start are fixed: only x2 is left, and > end */
        log_f(3, "  fixing end\n");
        next = cur;
        do {
            seg2 = list_entry(next, struct segment, list);

            if (x2 <= seg2->end) {                /* we stop here */
                log_f(3, "  found end\n");
                if (next != cur) {
                    log_f(3, "  extending cur end\n");
                    seg->end = seg2->end;
                    list_del(next);
                    pool_add(pool_segment, seg2);
                }
                return 6;
            }
            if (list_is_last(next, &prow->segments)) {
                if (x2 > seg2->end) {
                    log_f(3, "  extending cur end\n");
                    seg->end = x2;
                    if (next != cur) {
                        log_f(3, "  removing element\n");
                        list_del(next);
                        pool_add(pool_segment, seg2);
                    }
                    return 5;
                }
            }
            //next:
            next = next->next;
        } while (1);
    }
    log_f(3, "  adding at end of list\n");
    list_add_tail(&new->list, &prow->segments);
    return 10;
}

static int _add_segment(int row, int center, int half)
{
    int x1 = center - half, x2 = center + half;
    uint hash = row, bucket = hash_32(hash, HBITS);
    struct row *prow = find_row(&map.hash[bucket], hash);

    log_f(3, "adding segment (%d,%d) on row (%d) - bucket(%u) = %u prow=%p\n",
          x1, x2, row, hash, bucket, prow);
    if (row < map.ymin) {
        log(5, "new ymin=%d->%d\n", map.ymin, row);
        map.ymin = row;
    }
    if (row > map.ymax) {
        log(5, "new ymax=%d->%d\n", map.ymax, row);
        map.ymax = row;
    }
    if (x1 < map.xmin) {
        log(5, "new xmin=%d->%d\n", map.xmin, x1);
        map.xmin = x1;
    }
    if (x2 > map.xmax) {
        log(5, "new xmax=%d->%d\n", map.xmax, x2);
        map.xmax = x2;
    }
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
    /*
     * struct segment *segment = pool_get(pool_segment);
     * segment->start = x1;
     * segment->end = x2;
     * segment->row = row;
     * hlist_add_head(&map.hash, &segment->list, &map.hash[bucket]);
     */


    return 1;
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

static int add_segment(int sx, int sy, int bx, int by)
{
    int manhattan;

    log_f(3, "sensor4=(%d, %d) beacon=(%d, %d) - ", sx, sy, bx, by);
    manhattan = abs(bx - sx) + abs(by - sy);
    printf("manhattan=%u\n", manhattan);
    _add_segment(sy, sx, manhattan);
    for (int dy = 1, half = manhattan - 1; dy <= manhattan; ++dy, half--) {
        _add_segment(sy - dy, sx, half);
        _add_segment(sy + dy, sx, half);
    }
    add_beacon(bx, by);
    return 1;
}

static int parse(int part)
{
    int scanned, sx, sy, bx, by;
    int line = 1;
    //while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
    //    buf[--buflen] = 0;
    while (1) {
        scanned = scanf("%*[^-0-9]%d%*[^-0-9]%d%*[^-0-9]%d%*[^-0-9]%d",
                        &sx, &sy, &bx, &by);
        if (scanned != 4)
            break;
        printf("line %d scanned=%d sx=%d sy=%d bx=%d by=%d\n",
            line++, scanned, sx, sy, bx, by);
        if (part == 1) {
            int row = 2000000;
            map.xmin = -2;
            map.xmax = 25;
            int manhattan = abs(bx - sx) + abs(by - sy);
            log(3, "m=%d : ", manhattan);
            if (row >= sy && row <= (sy + manhattan)) {
                int half = manhattan - (row - sy);
                log(3, "min ok half=%d\n", half);
                _add_segment(row, sx, half);
            } else if (row < sy && row >= (sy - manhattan)) {
                int half = manhattan - (sy - row);
                log(3, "max ok half=%d\n", half);
                _add_segment(row, sx, half);
            } else {
                log(3, "OUT\n");
            }
            if (by == row)
                add_beacon(bx, by);

        }
        //add_segment(sx, sy, bx, by);
    }
    //free(buf);
    return 1;
}

static int doit(int part)
{
    int res = 0;
    parse(part);

    uint row = 2000000, bucket = hash_32(row, HBITS);
    struct row *prow = find_row(&map.hash[bucket], row);
    struct segment *cur;

    if (prow) {
        print_map();
        list_for_each_entry(cur, &prow->segments, list) {
            printf("counting segment (%d,%d) = %d nbeac=%d\n", cur->start, cur->end,
                   cur->end - cur->start + 1, prow->nbeacons);
            res += cur->end - cur->start + 1;
        }
        res -= prow->nbeacons;
    }
    print_map();
    return res;
}


int main(int ac, char **av)
{
    int part = parseargs(ac, av);

    pool_row =  pool_create("rows", 512, sizeof(struct row));
    pool_segment =  pool_create("segments", 512, sizeof(struct segment));

    printf("%s: res=%d\n", *av, doit(part));
    pool_destroy(pool_row);
    pool_destroy(pool_segment);
    exit(0);
}
