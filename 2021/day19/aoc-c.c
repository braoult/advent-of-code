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

#define MAX_SCANNERS   32                         /* I know, I know... */

typedef struct vector {
    s64 x, y, z;
} vector_t;

typedef struct beacon {
    int scanner;                                  /* original scanner  for beacon */
    int num;                                      /* original # in original scanner */
    int common;                                   /* has common distance with 1st scanner */
    vector_t vec;                                 /* beacon coordinates */
    struct list_head list_beacons;
} beacon_t;

typedef struct dist {
    u64 dist;                                     /* square distance... */
    beacon_t *beacon1, *beacon2;                  /* ... between these beacons */
    struct list_head list_dists;
} dist_t;

typedef struct scanner {
    int nbeacons, ndists;
    int adjusted;
    vector_t rel;                                 /* relative position to scanner 0 */
    struct list_head list_beacons;
    struct list_head list_dists;
    beacon_t *ref[3];                             /* reference beacons */
    //vector_t ref[3];
} scanner_t;

static pool_t *pool_beacon;
static pool_t *pool_dist;

static scanner_t scanners[MAX_SCANNERS];
static int nscanners;

/* Thanks to:
 * http://www.euclideanspace.com/maths/algebra/matrix/transforms/examples/index.htm
 */
vector_t rotations[] = {
    { 1,  0,  0}, { 0,  1,  0}, { 0,  0,  1},
    { 0,  0,  1}, { 0,  1,  0}, {-1,  0,  0},
    {-1,  0,  0}, { 0,  1,  0}, { 0,  0, -1},
    { 0,  0, -1}, { 0,  1,  0}, { 1,  0,  0},

    { 0, -1,  0}, { 1,  0,  0}, { 0,  0,  1},
    { 0,  0,  1}, { 1,  0,  0}, { 0,  1,  0},
    { 0,  1,  0}, { 1,  0,  0}, { 0,  0, -1},
    { 0,  0, -1}, { 1,  0,  0}, { 0, -1,  0},

    { 0,  1,  0}, {-1,  0,  0}, { 0,  0,  1},
    { 0,  0,  1}, {-1,  0,  0}, { 0, -1,  0},
    { 0, -1,  0}, {-1,  0,  0}, { 0,  0, -1},
    { 0,  0, -1}, {-1,  0,  0}, { 0,  1,  0},

    { 1,  0,  0}, { 0,  0, -1}, { 0,  1,  0},
    { 0,  1,  0}, { 0,  0, -1}, {-1,  0,  0},
    {-1,  0,  0}, { 0,  0, -1}, { 0, -1,  0},
    { 0, -1,  0}, { 0,  0, -1}, { 1,  0,  0},

    { 1,  0,  0}, { 0, -1,  0}, { 0,  0, -1},
    { 0,  0, -1}, { 0, -1,  0}, {-1,  0,  0},
    {-1,  0,  0}, { 0, -1,  0}, { 0,  0,  1},
    { 0,  0,  1}, { 0, -1,  0}, { 1,  0,  0},


    { 1,  0,  0}, { 0,  0,  1}, { 0, -1,  0},
    { 0, -1,  0}, { 0,  0,  1}, {-1,  0,  0},
    {-1,  0,  0}, { 0,  0,  1}, { 0,  1,  0},
    { 0,  1,  0}, { 0,  0,  1}, { 1,  0,  0},
};

#define NROTATIONS (sizeof(rotations) / sizeof(*rotations) / 3)

/* vector_rotate: returns vector rotation vector by "nrot"th rotations matrix
 */
static vector_t vector_rotate(const vector_t *vector, int nrot)
{
    vector_t *rot = &rotations[nrot*3], res;

    res.x = vector->x * rot[0].x + vector->y * rot[0].y + vector->z * rot[0].z;
    res.y = vector->x * rot[1].x + vector->y * rot[1].y + vector->z * rot[1].z;
    res.z = vector->x * rot[2].x + vector->y * rot[2].y + vector->z * rot[2].z;
    return res;
}

/* beacon_diff: returns difference between 2 vectors
 */
static vector_t vector_diff(const vector_t *vec1, vector_t *vec2)
{
    vector_t res;

    res.x = vec1->x - vec2->x;
    res.y = vec1->y - vec2->y;
    res.z = vec1->z - vec2->z;
    return res;
}

/* beacon_diff: calculate sum of two vectors
 */
static vector_t vector_add(const vector_t *vec1, vector_t *vec2)
{
    vector_t res;

    res.x = vec1->x + vec2->x;
    res.y = vec1->y + vec2->y;
    res.z = vec1->z + vec2->z;
    return res;
}

/* insert a new distance in scanner's (sorted) distances list
 */
static int insert_dist(scanner_t *scanner, dist_t *dist)
{
    dist_t *cur;
    uint newdist = dist->dist;

    cur = list_first_entry_or_null(&scanner->list_dists, dist_t, list_dists);
    /* special case: first distance or new dist lower than first dist */
    if (!cur || newdist < cur->dist) {
        list_add(&dist->list_dists, &scanner->list_dists);
        goto end;
    }
    /* normal case: insert before current when new dist is lower than current dist */
    list_for_each_entry(cur, &scanner->list_dists, list_dists) {
        if (newdist < cur->dist) {
            list_add_tail(&dist->list_dists, &cur->list_dists);
            goto end;
        }
    }
    /* special case: we went to end, insert at list's tail */
    list_add_tail(&dist->list_dists, &scanner->list_dists);
end:
    return ++scanner->ndists;
}

/* add distances between beacon and all others
 */
static int add_beacon_dists1(scanner_t *scanner, beacon_t *beacon)
{
    beacon_t *cur;
    dist_t *dist;
    int count = 0;

    list_for_each_entry(cur, &scanner->list_beacons, list_beacons) {
        if (cur == beacon)
            continue;
        dist = pool_get(pool_dist);
        dist->dist =
            (beacon->vec.x - cur->vec.x) * (beacon->vec.x - cur->vec.x) +
            (beacon->vec.y - cur->vec.y) * (beacon->vec.y - cur->vec.y) +
            (beacon->vec.z - cur->vec.z) * (beacon->vec.z - cur->vec.z);
        dist->beacon1 = beacon;
        dist->beacon2 = cur;
        insert_dist(scanner, dist);
        count++;
    }
    return count;
}

/* compare two beacons by x, y, and z.
 */
static inline int compare_beacons(beacon_t *b1, beacon_t *b2)
{
    vector_t *v1 = &b1->vec, *v2 = &b2->vec;
    //u64 m1 = b1->manhattan, m2 = b2->manhattan;

    if (v1->x < v2->x ||
        (v1->x == v2->x && (v1->y < v2->y ||
                            (v1->y == v2->y && v1->z < v2->z)))) {
        return -1;
    } else if (v1->x == v2->x && v1->y == v2->y && v1->z == v2->z) {
        return 0;
    }
    return 1;
}

/* insert a new beacon in scanner's list.
 * keep the list ordered by:
 * 1) manhattan distance
 * 2) x, then y, then z
 */
static int insert_unique_beacon(scanner_t *scanner, beacon_t *beacon)
{
    beacon_t *cur;

    cur = list_first_entry_or_null(&scanner->list_beacons, beacon_t, list_beacons);
    /* special case: first beacon or new beacon lower than first beacon */
    if (!cur || compare_beacons(beacon, cur) < 0) {
        list_add(&beacon->list_beacons, &scanner->list_beacons);
        goto end;
    }
    /* normal case: insert before current when new dist is lower than current dist */
    list_for_each_entry(cur, &scanner->list_beacons, list_beacons) {
        switch (compare_beacons(beacon, cur)) {
            case -1:
                list_add_tail(&beacon->list_beacons, &cur->list_beacons);
                goto end;
            case 0:
                return -1;
        }
    }
    /* special case: we went to end, insert at list's tail */
    list_add_tail(&beacon->list_beacons, &scanner->list_beacons);
end:
    return ++scanner->nbeacons;
}

/* using scanner's reference points, find the correct rotation and translation
 */
static int adjust_scanner(scanner_t *ref, scanner_t *s)
{
    beacon_t *beacon_ref[3], *beacon[3], *cur;
    int error = -1;

    for (uint i = 0; i < 3; ++i) {
        beacon_ref[i] = ref->ref[i];
        beacon[i] = s->ref[i];
    }

    for (uint rotnum = 0; rotnum < NROTATIONS; ++rotnum) {
        vector_t rot[3], diff[3];
        /* rotate the first beacon and translate to match ref's (x, y, z)
         */
        for (int bref = 0; bref < 3; ++bref) {
            rot[bref] = vector_rotate(&beacon[bref]->vec, rotnum);
            diff[bref] = vector_diff(&beacon_ref[bref]->vec, rot + bref);

            /* check that rotation/translation works for the 3 reference points
             */
            if (bref > 0 && (diff[bref].x != diff[0].x ||
                             diff[bref].y != diff[0].y ||
                             diff[bref].z != diff[0].z)) {
                goto next_rot;
            }
        }
        s->rel = diff[0];
        error = 0;
        /* adjust all beacons */
        list_for_each_entry(cur, &s->list_beacons, list_beacons) {
            cur->vec = vector_rotate(&cur->vec, rotnum);
            cur->vec = vector_add(&cur->vec, &diff[0]);
        }
        s->adjusted = 1;

        break;
    next_rot:
    }
    return error;
}


/* merge scanner s2 (already translated) beacons into scanner s1.
 * - ignore duplicate beacons
 * - recalculate all distances for new beacons
 */
static int merge_scanner(scanner_t *s1, scanner_t *s2)
{
    struct list_head *cur, *tmp;
    beacon_t *beacon;
    dist_t *dist;
    int count = 0;

    list_for_each_safe(cur, tmp, &s2->list_beacons) {
        beacon = list_entry(cur, beacon_t, list_beacons);
        list_del(cur);
        s2->nbeacons--;
        if (insert_unique_beacon(s1, beacon) > 0)
            add_beacon_dists1(s1, beacon);
    }
    /* free all dists */
    list_for_each_safe(cur, tmp, &s2->list_dists) {
        dist = list_entry(cur, dist_t, list_dists);
        list_del(cur);
        s2->ndists--;
        pool_add(pool_dist, dist);
    }
    return count;
}

/* For each scanner, calculate square distances between every beacon,
 * and generate corresponding list.
 * For N beacons, we will get (N) * (N-1) / 2 distances.
 *
 * Note: To find 12 matching beacons between scanners later, we will need
 * 66 matching distances (12 * 11 / 2).
 */
static int count_common_distances(scanner_t *s1, scanner_t *s2)
{
    struct list_head *plist1, *plist2;
    dist_t *pdist1, *pdist2;
    beacon_t *tmpbeacon;
    int cur1 = 0, cur2 = 0;
    u64 dist1, dist2;
    int nref = 0;
    uint count = 0;

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
    if (list_empty(&s1->list_dists) || list_empty(&s2->list_dists))
        return 0;
    plist1 = s1->list_dists.next;
    plist2 = s2->list_dists.next;

    /* initialize second scanner common beacons
     */
    list_for_each_entry(tmpbeacon, &s2->list_beacons, list_beacons) {
        tmpbeacon->common = 0;
    }

    while (plist1 != &s1->list_dists && plist2 != &s2->list_dists) {
        pdist1 = list_entry(plist1, dist_t, list_dists);
        pdist2 = list_entry(plist2, dist_t, list_dists);
        dist1 = pdist1->dist;
        dist2 = pdist2->dist;

        if (dist1 == dist2) {
            plist1 = plist1->next;
            plist2 = plist2->next;
            cur1++;
            cur2++;
            count++;

            pdist2->beacon1->common++;            /* mark beacons as common in 2nd scanner */
            pdist2->beacon2->common++;

            switch (nref) {
                case 0:                           /* first 2 reference points */
                    s1->ref[0] = pdist1->beacon1;
                    s1->ref[1] = pdist1->beacon2;

                    s2->ref[0] = pdist2->beacon1;
                    s2->ref[1] = pdist2->beacon2;
                    nref = 2;
                    break;

                case 2:                           /* third reference point */
                    beacon_t *beacon1 , *beacon2;
                    scanner_t *scanner;

                    /* we need to adjust references for the two pairs of
                     * 2 beacons having same distance, for both scanners.
                     */
                    for (int i = 0; i < 2; i++) {
                        if (i == 0) {
                            beacon1 = pdist1->beacon1;
                            beacon2 = pdist1->beacon2;
                            scanner = s1;
                        } else {
                            beacon1 = pdist2->beacon1;
                            beacon2 = pdist2->beacon2;
                            scanner = s2;
                        }

                        if (beacon1 == scanner->ref[0]) {
                            /* ref0----------ref1
                             * ref0----------ref2
                             */
                            scanner->ref[2] = beacon2;
                            nref++;
                        } else if (beacon1 == scanner->ref[1]) {
                            /* ref1----------ref0
                             * ref0----------ref2
                             * first & second reference beacons must be reversed,
                             * second is ok
                             */
                            beacon_t *tmp = scanner->ref[0];
                            scanner->ref[0] = scanner->ref[1];
                            scanner->ref[1] = tmp;
                            scanner->ref[2] = beacon2;
                            nref++;
                        } else if (beacon2 == scanner->ref[0]) {
                            /* ref0----------ref1
                             * ref2----------ref0
                             */
                            scanner->ref[2] = beacon1;
                            nref++;
                        } else if (beacon2 == scanner->ref[1]) {
                            /* ref1----------ref0
                             * ref2----------ref0
                             */
                            beacon_t *tmp = scanner->ref[0];
                            scanner->ref[0] = scanner->ref[1];
                            scanner->ref[1] = tmp;
                            scanner->ref[2] = beacon1;
                            nref++;
                        }
                    }

                    break;
            }


        } else if (dist1 < dist2) {
            plist1 = plist1->next;
            cur1++;
        } else {                                  /* dist1 > dist2 */
            plist2 = plist2->next;
            cur2++;
        }

    }
    return count;
}

/* match all scanners
 */
static void match_scanners()
{
    int finished = 0;

    scanners[0].adjusted = 1;
    while (!finished) {
        finished = 1;
        for (int j = 1; j < nscanners; ++j) {
            if (scanners[j].adjusted) /* already translated */
                continue;

            int count = count_common_distances(scanners, scanners + j);
            if (count >= 66) {
                adjust_scanner(scanners, scanners + j);
                merge_scanner(scanners, scanners + j);
                finished = 0;
            }
        }
    }
}

/* read input
 */
static int scanners_read()
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    beacon_t *beacon;
    scanner_t *scanner = NULL;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        switch (buf[1]) {
            case '-':
                scanner = scanners + nscanners;
                INIT_LIST_HEAD(&scanner->list_beacons);
                INIT_LIST_HEAD(&scanner->list_dists);
                nscanners++;
                scanner->nbeacons = 0;
                break;
            case '\0':
                break;
            default:
                beacon = pool_get(pool_beacon);
                beacon->scanner = nscanners;
                beacon->num = scanner->nbeacons;
                sscanf(buf, "%ld,%ld,%ld", &beacon->vec.x, &beacon->vec.y,
                       &beacon->vec.z);
                if (insert_unique_beacon(scanner, beacon) > 0)
                    add_beacon_dists1(scanner, beacon);
        }
    }
    free(buf);
    return nscanners;
}

static s64 part1()
{
    return (*scanners).nbeacons;
}

static s64 part2()
{
    vector_t *v1, *v2;
    s64 max = 0, cur;

    for (int i = 0; i < nscanners; ++i) {
        v1 = &scanners[i].rel;
        for (int j = i + 1; j < nscanners; ++j) {
            v2 = &scanners[j].rel;
            cur = labs(v2->x - v1->x)
                + labs(v2->y - v1->y)
                + labs(v2->z - v1->z);
            if (cur > max) {
                max = cur;
            }
        }
    }
    return max;
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
    if (!(pool_dist = pool_create("dists", 1024, sizeof(dist_t)))) {
        log(1, "pool create error, errno=%d\n", errno);
        exit(1);
    }
    scanners_read();
    match_scanners();
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
    pool_stats(pool_beacon);
    pool_destroy(pool_beacon);
    pool_stats(pool_dist);
    pool_destroy(pool_dist);
    exit(0);
}
