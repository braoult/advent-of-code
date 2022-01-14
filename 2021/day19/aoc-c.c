/* aoc-c.c: Advent of Code 2021, day 19 parts 1 & 2
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

#include "pool.h"
#include "debug.h"
#include "bits.h"
#include "list.h"

/*
  typedef struct beacon {
  int x, y, z;
  struct list_head list_beacons;
  } beacon_t;

  typedef struct scanner {
  struct list_head list_beacons;
  } scanner_t;
*/

#define MAX_SCANNERS   32                         /* I know, I know... */
#define MAX_BEACONS    32
#define MAX_DISTANCES  ((MAX_BEACONS - 1) * MAX_BEACONS)

typedef struct beacon {
    int x, y, z;                                  /* beacon coordinates */
    uint count;                                   /* common scanner distance count */
    //struct list_head list_beacons;
} beacon_t;

typedef struct dist {
    uint dist;                                    /* square distance */
    int b1, b2;                                   /* beacons */
} dist_t;

typedef struct scanner {
    //struct list_head list_beacons;
    int nbeacons, ndists;
    beacon_t beacons[MAX_BEACONS];
    dist_t dists[MAX_DISTANCES];                  /* sorted */
} scanner_t;

static pool_t *pool_beacon;

static scanner_t scanners[MAX_SCANNERS];
static int nscanners;

static void scanners_print_dists()
{
    dist_t *cur;

    log_f(1, "nscanners: %d\n", nscanners);
    for (int i = 0; i < nscanners; ++i) {
        log(1, "scanner %d:\n", i);
        for (int j = 0; j < scanners[i].ndists; ++j) {
            cur = &scanners[i].dists[j];
            log(1, "\t%u : %d,%d\n", cur->dist, cur->b1, cur->b2);
        }
        log(1, "\n");
    }
}

static void scanners_print()
{
    beacon_t *cur;

    log_f(1, "nscanners: %d\n", nscanners);
    for (int i = 0; i < nscanners; ++i) {
        log(1, "scanner %d:\n\t", i);
        /*
          list_for_each_entry(cur, &scanners[i].list_beacons, list_beacons) {
          log(1, " %d/%d/%d", cur->x, cur->y, cur->z);
          }
        */
        for (int j = 0; j < scanners[i].nbeacons; ++j) {
            cur = &scanners[i].beacons[j];
            log(1, " %d/%d/%d", cur->x, cur->y, cur->z);
        }
        log(1, "\n");
    }
}

static void merge(array, left, mid, right)
    dist_t *array;
    int left, mid, right;
{
    dist_t temp[right - left + 1];
    int pos = 0, lpos = left, rpos = mid + 1, iter;

    while(lpos <= mid && rpos <= right) {
        if(array[lpos].dist < array[rpos].dist) {
            temp[pos++] = array[lpos++];
        }
        else {
            temp[pos++] = array[rpos++];
        }
    }
    while(lpos <= mid)
        temp[pos++] = array[lpos++];
    while(rpos <= right)
        temp[pos++] = array[rpos++];
    for(iter = 0; iter < pos; iter++) {
        array[iter+left] = temp[iter];
    }
    return;
}

static void mergesort(array, left, right)
    dist_t *array;
    int left, right;
{
    int mid = (left + right) / 2;
    if(left < right) {
        mergesort(array, left, mid);
        mergesort(array, mid + 1, right);
        merge(array, left, mid, right);
    }
}

static int count_common_distances(scanner_t *s1, scanner_t *s2)
{
    dist_t *d1 = s1->dists, *d2 = s2->dists;
    int cur1 = 0, cur2 = 0, i;
    uint count = 0;
    beacon_t *beacon1 = s1->beacons, *beacon2 = s2->beacons;

    /* initialize s1 and s2 beacons count
     */
    for (i = 0; i < s1->nbeacons; ++i)
        beacon1[i].count = 0;
    for (i = 0; i < s2->nbeacons; ++i)
        beacon2[i].count = 0;

    log_f(1, "(%ld, %ld): ", s1 - scanners, s2 - scanners);
    while (cur1 < s1->ndists && cur2 < s2->ndists) {
        if (d1[cur1].dist == d2[cur2].dist) {
            log(1, " (%d,%d)=%u", cur1, cur2, d1[cur1].dist);
            ++beacon1[d1[cur1].b1].count;
            ++beacon1[d1[cur1].b2].count;
            ++beacon2[d2[cur2].b1].count;
            ++beacon2[d2[cur2].b2].count;
            ++count;
            ++cur1;
            ++cur2;
        }
        else if (d1[cur1].dist < d2[cur2].dist) {
            ++cur1;
        } else {
            ++cur2;
        }
    }
    log_i(3, "\n");
    return count;
}

static void calc_square_distances()
{
    scanner_t *scanner;
    beacon_t *b1, *b2;
    uint dist;

    log_f(1, "nscanners: %d\n", nscanners);
    for (int i = 0; i < nscanners; ++i) {
        scanner = scanners + i;
        for (int j = 0; j < scanner->nbeacons; ++j) {
            b1 = scanner->beacons + j;
            for (int k = j + 1; k < scanner->nbeacons; ++k) {
                scanner->dists[scanner->ndists].b1 = j;
                scanner->dists[scanner->ndists].b2 = k;

                b2 = scanner->beacons + k;
                dist = (b1->x - b2->x) * (b1->x - b2->x) +
                    (b1->y - b2->y) * (b1->y - b2->y) +
                    (b1->z - b2->z) * (b1->z - b2->z);
                scanner->dists[scanner->ndists].dist = dist;
                scanner->ndists++;
                log(5, "\tdist(%d/%d) = %u\n",
                    j, k,
                    dist);
            }

        }
        scanners_print_dists();
        mergesort(scanner->dists, 0, scanner->ndists - 1);
    }
    scanners_print_dists();
    log(1, "\n");
}

/* read input
 */
static int read_lines()
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    beacon_t *beacon;
    scanner_t *scanner = NULL;
    //int nbeacon = 0;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        //buf[--buflen] = 0;
        switch (buf[1]) {
            case '-':
                scanner = scanners + nscanners;
                log_f(2, "[%c] nscanners = %d %p/%p\n", *buf, nscanners,
                      scanner, scanners);
                //INIT_LIST_HEAD(&scanner->list_beacons);
                nscanners++;
                scanner->nbeacons = 0;
                break;
            case '\0':
                log_f(2, "NULL line\n");
                break;
            default:
                //log_f(2, "[%c] beacon = %d\n", *buf, nscanners);
                //beacon = pool_get(pool_beacon);
                //list_add_tail(&beacon->list_beacons, &scanner->list_beacons);
                beacon = &scanner->beacons[scanner->nbeacons];
                sscanf(buf, "%d,%d,%d", &beacon->x, &beacon->y, &beacon->z);
                //log_f(2, "beacon %d = %d/%d/%d\n", nbeacon, beacon->x, beacon->y, beacon->z);
                scanner->nbeacons++;

        }
    }
    free(buf);
    scanners_print();
    calc_square_distances();
    for (int i = 0; i < nscanners - 1; ++i) {
        for (int j = i + 1; j < nscanners; ++j) {
            int count = count_common_distances(scanners + i, scanners + j);
            if (count >= 66) {
                log(1, "common(%d, %d) = %d\n", i, j, count);
                for (int k = 0; k < scanners[i].nbeacons; ++k) {
                    beacon_t *beacon = scanners[i].beacons + k;
                    if (beacon->count >= 11)
                        log(1, "s1(%d, %d, %d): %d\n", beacon->x, beacon->y,
                            beacon->z, beacon->count);
                }
                for (int k = 0; k < scanners[j].nbeacons; ++k) {
                    beacon_t *beacon = scanners[j].beacons + k;
                    if (beacon->count >= 11)
                        log(1, "s2(%d, %d, %d): %d\n", beacon->x, beacon->y,
                            beacon->z, beacon->count);
                }
            }
        }
    }
    return nscanners;
}

static s64 part1()
{
    return 1;
}

static s64 part2()
{
    return 2;
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

    if (!(pool_beacon = pool_create("beacons", 1024, sizeof(beacon_t))))
        exit(1);
    read_lines();
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_beacon);
    exit(0);
}
