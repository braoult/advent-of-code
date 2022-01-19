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
#include <errno.h>

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

// #define SQUARE(x)      ((x) * (x))

#define MAX_SCANNERS   32                         /* I know, I know... */
#define MAX_BEACONS    32
#define MAX_DISTANCES  ((MAX_BEACONS - 1) * MAX_BEACONS)

typedef struct vector {
    int x, y, z;                                  /* beacon coordinates */
} vector_t;

typedef struct beacon {
    int x, y, z;                                  /* beacon coordinates */
    int count;
} beacon_t;

typedef struct rotmatrix {
    vector_t a, b, c;
} rotmatrix_t;

typedef struct dist {
    uint dist;                                    /* square distance */
    int beacon1, beacon2;                         /* beacons */
} dist_t;

typedef struct scanner {
    int nbeacons, ndists, adjusted;
    beacon_t beacons[MAX_BEACONS];
    uint beacons_count[MAX_BEACONS];              /* common scanner distance count */
    dist_t dists[MAX_DISTANCES];                  /* sorted */
    int reference[3];                             /* reference beacons */
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
            log(1, "\t%u : %d,%d\n", cur->dist, cur->beacon1, cur->beacon2);
        }
        // log(1, "zobi\n");
    }
}

static void scanners_print_refs(scanner_t *s1, scanner_t *s2)
{
    log_f(1, "s1:%ld s2:%ld\n", s1 - scanners, s2 - scanners);
    log(1, "scanner %ld:", s1 - scanners);
    for (int i = 0; i < 3; ++i) {
        beacon_t *beacon = s1->beacons + s1->reference[i];
        log(1, " (%d,%d,%d)", beacon->x, beacon->y, beacon->z);
    }
    log(1, "\nscanner %ld:", s2 - scanners);
    for (int i = 0; i < 3; ++i) {
        beacon_t *beacon = s2->beacons + s2->reference[i];
        log(1, " (%d,%d,%d)", beacon->x, beacon->y, beacon->z);
    }
    log(1, "\n");
}

/*
static void scanners_print()
{
    beacon_t *cur;

    log_f(1, "nscanners: %d\n", nscanners);
    for (int i = 0; i < nscanners; ++i) {
        log(1, "scanner %d:\n\t", i);
        for (int j = 0; j < scanners[i].nbeacons; ++j) {
            cur = &scanners[i].beacons[j];
            log(1, " %d/%d/%d", cur->x, cur->y, cur->z);
        }
        log(1, "\n");
    }
}
*/

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

/* Thanks to:
 * http://www.euclideanspace.com/maths/algebra/matrix/transforms/examples/index.htm
 */
rotmatrix_t rotations[24] = {
    {{ 1,  0,  0}, { 0,  1,  0}, { 0,  0,  1}},
    {{ 0,  0,  1}, { 0,  1,  0}, {-1,  0,  0}},
    {{-1,  0,  0}, { 0,  1,  0}, { 0,  0, -1}},
    {{ 0,  0, -1}, { 0,  1,  0}, { 1,  0,  0}},

    {{ 0, -1,  0}, { 1,  0,  0}, { 0,  0,  1}},
    {{ 0,  0,  1}, { 1,  0,  0}, { 0,  1,  0}},
    {{ 0,  1,  0}, { 1,  0,  0}, { 0,  0, -1}},
    {{ 0,  0, -1}, { 1,  0,  0}, { 0, -1,  0}},

    {{ 0,  1,  0}, {-1,  0,  0}, { 0,  0,  1}},
    {{ 0,  0,  1}, {-1,  0,  0}, { 0, -1,  0}},
    {{ 0, -1,  0}, {-1,  0,  0}, { 0,  0, -1}},
    {{ 0,  0, -1}, {-1,  0,  0}, { 0,  1,  0}},

    {{ 1,  0,  0}, { 0,  0, -1}, { 0,  1,  0}},
    {{ 0,  1,  0}, { 0,  0, -1}, {-1,  0,  0}},
    {{-1,  0,  0}, { 0,  0, -1}, { 0, -1,  0}},
    {{ 0, -1,  0}, { 0,  0, -1}, { 1,  0,  0}},

    {{ 1,  0,  0}, { 0, -1,  0}, { 0,  0, -1}},
    {{ 0,  0, -1}, { 0, -1,  0}, {-1,  0,  0}},
    {{-1,  0,  0}, { 0, -1,  0}, { 0,  0,  1}},
    {{ 0,  0,  1}, { 0, -1,  0}, { 1,  0,  0}},


    {{ 1,  0,  0}, { 0,  0,  1}, { 0, -1,  0}},
    {{ 0, -1,  0}, { 0,  0,  1}, {-1,  0,  0}},
    {{-1,  0,  0}, { 0,  0,  1}, { 0,  1,  0}},
    {{ 0,  1,  0}, { 0,  0,  1}, { 1,  0,  0}},
};
#define NROTATIONS (sizeof(rotations) / sizeof(*rotations))

/* beacon_rotate: rotate beacon "in" with "nrot"th rotations matrix
 */
static beacon_t *beacon_rotate(const beacon_t *in, beacon_t *res, rotmatrix_t *rot)
{
    res->x = in->x * rot->a.x + in->y * rot->a.y + in->z * rot->a.z;
    res->y = in->x * rot->b.x + in->y * rot->b.y + in->z * rot->b.z;
    res->z = in->x * rot->c.x + in->y * rot->c.y + in->z * rot->c.z;
    return res;
}

/* beacon_diff: calculate difference between 2 beacons
 */
static beacon_t *beacon_diff(const beacon_t *b1, beacon_t *b2, beacon_t *res)
{
    res->x = b1->x - b2->x;
    res->y = b1->y - b2->y;
    res->z = b1->z - b2->z;
    return res;
}

/* using scanner's reference points, find the correct rotation and translation
 */
static int adjust_scanner(scanner_t *ref, scanner_t *s)
{
    beacon_t *beacon_ref[3], *beacon[3];

    //log_f(1, "sizeof(rotations)=%lu\n", NROTATIONS);
    for (uint i = 0; i < 3; ++i) {
        beacon_ref[i] = ref->beacons + ref->reference[i];
        beacon[i] = s->beacons + s->reference[i];
    }

    for (uint i = 0; i < NROTATIONS; ++i) {
        beacon_t rot[3], diff[3];
        //int match = 1;
        /* rotate the first beacon and translate to match ref's (x, y, z)
         */
        for (int bref = 0; bref < 3; ++bref) {
            beacon_rotate(beacon[bref], &rot[bref], rotations + i);
            beacon_diff(beacon_ref[bref], &rot[bref], &diff[bref]);
            log(2, "ref %d diff=(%d,%d,%d)\n", bref, diff[bref].x, diff[bref].y,
                diff[bref].z);
            /* check that rotation/translation works for the 3 reference points
             */
            if (bref > 0 && (diff[bref].x != diff[0].x ||
                             diff[bref].y != diff[0].y ||
                             diff[bref].z != diff[0].z)) {
                log(2, "skipping this translation\n");
                goto next_rot;
            }
        }
        log(2, "Got it: scanner %lu is (%d,%d,%d) from reference\n",
            s - scanners, diff[0].x, diff[0].y, diff[0].z);
        /* adjust all beacons */
        for (int b = 0; b < s->nbeacons; ++ b) {
            beacon_rotate(s->beacons + b, rot, rotations + i);
            s->beacons[b] = *rot;
            s->beacons[b].x += (*diff).x;
            s->beacons[b].y += (*diff).y;
            s->beacons[b].z += (*diff).z;
            log(2, "translated beacon: (%d,%d,%d)\n", s->beacons[b].x,
                s->beacons[b].y, s->beacons[b].z);
            //beacon_diff(beacon_ref[bref], &rot[bref], &diff[bref]);
        }
        break;
        next_rot:
        log(2, "\n");
    }
    return 1;
}

static int count_common_distances(scanner_t *s1, scanner_t *s2)
{
    dist_t *d1 = s1->dists, *d2 = s2->dists;
    int cur1 = 0, cur2 = 0, i;
    int ref_triangle = 0;
    uint count = 0;
    //beacon_t *beacon1 = s1->beacons, *beacon2 = s2->beacons;

    /* initialize s1 and s2 beacons count
     */
    for (i = 0; i < MAX_BEACONS; ++i) {
        s1->beacons_count[i] = 0;
        s2->beacons_count[i] = 0;
    }

    log_f(1, "(%ld, %ld): \n", s1 - scanners, s2 - scanners);
    /* We need to find common references A, B, C such as:
     *
     *                     d1
     *             A------------------B
     *              \                /
     *             d2\   +----------/
     *                \ /    (d3)
     *                 C
     * To do so, we find common d1 and d2 such as both have
     * a common A point.
     */
    while (cur1 < s1->ndists && cur2 < s2->ndists) {
        if (d1[cur1].dist == d2[cur2].dist) {
            log(1, " %u: (%d,%d)= %d-%d %d-%d triangle=%d\n",
                d1[cur1].dist, cur1, cur2,
                d1[cur1].beacon1, d1[cur1].beacon2,
                d2[cur2].beacon1, d2[cur2].beacon2,
                ref_triangle);
            if (ref_triangle == 0) {
                /* both s1 s2 refs may be reversed */
                s1->reference[0] = d1[cur1].beacon1;
                s1->reference[1] = d1[cur1].beacon2;

                s2->reference[0] = d2[cur2].beacon1;
                s2->reference[1] = d2[cur2].beacon2;
                ref_triangle = 2;
                log(2, "\ts1_ref=%d,%d,%d s2_ref=%d,%d,%d\n",
                    s1->reference[0], s1->reference[1], s1->reference[2],
                    s2->reference[0], s2->reference[1], s2->reference[2]);
            } else if (ref_triangle == 2) {
                int beacon1 , beacon2;

                /* we need to adjust references for the two pairs of
                 * 2 beacons having same distance, for scanner 1.
                 */
                beacon1 = d1[cur1].beacon1;
                beacon2 = d1[cur1].beacon2;
                log(2, "\ts1 bea1=%d bea2=%d\n\t   ref0=%d ref1=%d\n",
                    beacon1, beacon2, s1->reference[0], s1->reference[1]);

                if (beacon1 == s1->reference[0]) {
                    /* ref0----------ref1
                     * ref0----------ref2
                     */
                    s1->reference[2] = beacon2;
                    ref_triangle++;
                } else if (beacon1 == s1->reference[1]) {
                    /* ref1----------ref0
                     * ref0----------ref2
                     * first & second reference beacons must be reversed,
                     * second is ok
                     */
                    int tmp = s1->reference[0];
                    s1->reference[0] = s1->reference[1];
                    s1->reference[1] = tmp;
                    s1->reference[2] = beacon2;
                    ref_triangle++;
                } else if (beacon2 == s1->reference[0]) {
                    /* ref0----------ref1
                     * ref2----------ref0
                     */
                    s1->reference[2] = beacon1;
                    ref_triangle++;
                } else if (beacon2 == s1->reference[1]) {
                    /* ref1----------ref0
                     * ref2----------ref0
                     */
                    int tmp = s1->reference[0];
                    s1->reference[0] = s1->reference[1];
                    s1->reference[1] = tmp;
                    s1->reference[2] = beacon1;
                    ref_triangle++;
                }

                /* we do the same for second scanner */
                beacon1 = d2[cur2].beacon1;
                beacon2 = d2[cur2].beacon2;
                if (beacon1 == s2->reference[0]) {
                    /* ref0----------ref1
                     * ref0----------ref2
                     */
                    s2->reference[2] = beacon2;
                    ref_triangle++;
                } else if (beacon1 == s2->reference[1]) {
                    /* ref1----------ref0
                     * ref0----------ref2
                     * first & second reference beacons must be reversed,
                     * second is ok
                     */
                    int tmp = s2->reference[0];
                    s2->reference[0] = s2->reference[1];
                    s2->reference[1] = tmp;
                    s2->reference[2] = beacon2;
                    ref_triangle++;
                } else if (beacon2 == s2->reference[0]) {
                    /* ref0----------ref1
                     * ref2----------ref0
                     */
                    s2->reference[2] = beacon1;
                    ref_triangle++;
                } else if (beacon2 == s2->reference[1]) {
                    /* ref1----------ref0
                     * ref2----------ref0
                     */
                    int tmp = s2->reference[0];
                    s2->reference[0] = s2->reference[1];
                    s2->reference[1] = tmp;
                    s2->reference[2] = beacon1;
                    ref_triangle++;
                }

                log(2, "\ttriangle=%d s1_ref=%d,%d,%d s2_ref=%d,%d,%d\n",
                    ref_triangle,
                    s1->reference[0], s1->reference[1], s1->reference[2],
                    s2->reference[0], s2->reference[1], s2->reference[2]);
            }
            /*
            if (ref_triangle == 3) {
                if (d1[cur1].beacon1 == s1->reference[0] ||
                    d1[cur1].beacon1 == s1->reference[1]) {
                    s1->reference[2] = d2[cur1].beacon2;
                    ref_triangle++;
                } else if (d1[cur1].beacon2 == s1->reference[0] ||
                           d1[cur1].beacon2 == s1->reference[1]) {
                    s1->reference[2] = d1[cur1].beacon1;
                    ref_triangle++;
                }
            }
            */
            ++s1->beacons_count[d1[cur1].beacon1];
            ++s1->beacons_count[d1[cur1].beacon2];

            ++s2->beacons_count[d2[cur2].beacon1];
            ++s2->beacons_count[d2[cur2].beacon2];
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
    log_i(1, "\n");
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
                scanner->dists[scanner->ndists].beacon1 = j;
                scanner->dists[scanner->ndists].beacon2 = k;

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
        //scanners_print_dists();
        mergesort(scanner->dists, 0, scanner->ndists - 1);
    }
    scanners_print_dists();
    //log(1, "\n");
}

/* match all scanners
 */
static void match_scanners()
{
    int finished = 0;

    scanners[0].adjusted = 1;

    while (!finished) {
        finished = 1;
        for (int i = 0; i < nscanners - 1; ++i) {
            if (!scanners[i].adjusted)            /* skip un-translated scanners */
                continue;

            for (int j = 0; j < nscanners; ++j) {
                if (i == j || scanners[j].adjusted) /* already translated */
                    continue;

                int count = count_common_distances(scanners + i, scanners + j);
                log(1, "common(%d, %d) = %d\n", i, j, count);
                if (count >= 66) {
                    /*
                    beacon_t *beacon;
                    for (int k = 0; k < scanners[i].nbeacons; ++k) {
                        beacon = scanners[i].beacons + k;
                        //if (beacon->count >= 11)
                        log(1, "s1(%d, %d, %d): %d\n", beacon->x, beacon->y,
                            beacon->z, scanners[i].beacons_count[k]);
                    }
                    for (int k = 0; k < scanners[j].nbeacons; ++k) {
                        beacon = scanners[j].beacons + k;
                        //if (beacon->count >= 11)
                        log(1, "s2(%d, %d, %d): %d\n", beacon->x, beacon->y,
                            beacon->z, scanners[j].beacons_count[k]);
                    }
                    */
                    adjust_scanner(scanners + i, scanners + j);
                    scanners[j].adjusted = 1;
                    finished = 0;
                    //scanners_print_refs(scanners + i, scanners + j);
                }
            }
        }
    }
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
    calc_square_distances();
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

    if (!(pool_beacon = pool_create("beacons", 1024, sizeof(beacon_t)))) {
        log(1, "pool create error, errno=%d\n", errno);
        exit(1);
    }
    read_lines();
    match_scanners();
    log_f(1, "sizeof(rotations)=%lu, sizeof2=%lu sizeof2=%lu\n", sizeof(rotations),
          sizeof(*rotations), sizeof(rotations) / sizeof(*rotations));
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_beacon);
    exit(0);
}
