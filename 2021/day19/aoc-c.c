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
    int x, y, z;
} vector_t;

typedef struct beacon {
    vector_t vec;                                 /* beacon coordinates */
    struct list_head list_beacons;
} beacon_t;

typedef struct dist {
    int common;                                    /* matched in last dists compare */
    uint dist;                                     /* square distance... */
    beacon_t *beacon1, *beacon2;                   /* ... between these beacons */
    struct list_head list_dists;
} dist_t;

typedef struct scanner {
    int nbeacons, ndists;
    int adjusted;
    beacon_t *ref[3];                             /* reference beacons */
    vector_t rel;                                 /* relative position to scanner 0 */
    struct list_head list_beacons;
    struct list_head list_dists;
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
static inline vector_t vector_rotate(const vector_t *vector, const int nrot)
{
    vector_t *rot = &rotations[nrot*3], res;

    res.x = vector->x * rot[0].x + vector->y * rot[0].y + vector->z * rot[0].z;
    res.y = vector->x * rot[1].x + vector->y * rot[1].y + vector->z * rot[1].z;
    res.z = vector->x * rot[2].x + vector->y * rot[2].y + vector->z * rot[2].z;
    return res;
}

/* beacon_diff: returns difference between 2 vectors
 */
static inline vector_t vector_diff(const vector_t *vec1, const vector_t *vec2)
{
    vector_t res;

    res.x = vec1->x - vec2->x;
    res.y = vec1->y - vec2->y;
    res.z = vec1->z - vec2->z;
    return res;
}

/* beacon_diff: calculate sum of two vectors
 */
static inline vector_t vector_add(const vector_t *vec1, const vector_t *vec2)
{
    vector_t res;

    res.x = vec1->x + vec2->x;
    res.y = vec1->y + vec2->y;
    res.z = vec1->z + vec2->z;
    return res;
}

/* compare two beacons by x, y, and z.
 */
static inline int vector_cmp(const vector_t *v1, const vector_t *v2)
{
    if (v1->x < v2->x ||
        (v1->x == v2->x && (v1->y < v2->y ||
                            (v1->y == v2->y && v1->z < v2->z)))) {
        return -1;
    } else if (v1->x == v2->x && v1->y == v2->y && v1->z == v2->z) {
        return 0;
    }
    return 1;
}

/* insert a new distance in scanner's (sorted) distances list
 */
static inline int insert_dist(scanner_t *scanner, dist_t *dist)
{
    dist_t *cur;
    uint newdist = dist->dist;

    /* normal case: insert before current when new dist is lower than current dist */
    list_for_each_entry(cur, &scanner->list_dists, list_dists) {
        if (newdist < cur->dist) {
            list_add_tail(&dist->list_dists, &cur->list_dists);
            goto end;
        }
    }
    /* special case: we went to end without inserting, insert at list's tail.
     * This includes the case list was empty.
     */
    list_add_tail(&dist->list_dists, &scanner->list_dists);
end:
    return ++scanner->ndists;
}

/* add distances between beacon and all other beacons in scanner.
 */
static int add_beacon_dists(scanner_t *scanner, beacon_t *beacon)
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

/* move non common dists from scanner s2 to scanner s1
 */
static int move_beacon_dists(scanner_t *s1, scanner_t *s2)
{
    struct list_head *cur, *tmp;
    int count = 0;

    list_for_each_safe(cur, tmp, &s2->list_dists) {
        dist_t *dist = list_entry(cur, dist_t, list_dists);
        if (!dist->common) {
            list_del(cur);
            insert_dist(s1, dist);
            count++;
        }
    }
    return count;
}

/* insert a new beacon in scanner's list.
 * keep the list ordered by x, y, then z.
 */
static int insert_unique_beacon(scanner_t *scanner, beacon_t *beacon)
{
    beacon_t *cur;

    /* normal case: insert before current when new dist is lower than current dist
     */
    list_for_each_entry(cur, &scanner->list_beacons, list_beacons) {
        switch (vector_cmp(&beacon->vec, &cur->vec)) {
            case -1:
                list_add_tail(&beacon->list_beacons, &cur->list_beacons);
                goto end;
            case 0:
                return -1;
        }
    }
    /* special case: we went to end without inserting, insert at list's tail.
     * This includes the case list was empty.
     */
    list_add_tail(&beacon->list_beacons, &scanner->list_beacons);
end:
    return ++scanner->nbeacons;
}

/* using scanner's 3 reference beacons, find the correct rotation and translation
 */
static int adjust_scanner(scanner_t *ref, scanner_t *scanner)
{
    beacon_t *cur;
    int error = -1;
    vector_t *vec_ref[3], *vec[3];

    for (uint i = 0; i < 3; ++i) {
        vec_ref[i] = &ref->ref[i]->vec;
        vec[i] = &scanner->ref[i]->vec;
    }

    for (uint rotnum = 0; rotnum < NROTATIONS; ++rotnum) {
        vector_t diff[3];
        /* rotate the first beacon and translate to match ref's (x, y, z)
         */
        for (int bref = 0; bref < 3; ++bref) {
            diff[bref] = vector_rotate(vec[bref], rotnum);
            diff[bref] = vector_diff(vec_ref[bref], diff + bref);

            /* Does rotation/translation works for the 3 reference points ?
             */
            if (bref > 0 && (diff[bref].x != diff[0].x ||
                             diff[bref].y != diff[0].y ||
                             diff[bref].z != diff[0].z)) {
                goto next_rot;
            }
        }
        scanner->rel = diff[0];                   /* we found a match */
        error = 0;
        /* adjust all beacons */
        list_for_each_entry(cur, &scanner->list_beacons, list_beacons) {
            cur->vec = vector_rotate(&cur->vec, rotnum);
            cur->vec = vector_add(&cur->vec, diff);
        }
        scanner->adjusted = 1;

        break;
    next_rot:
    }
    return error;
}


/* merge scanner s2 (already translated) beacons into scanner s1.
 * - ignore duplicate beacons
 * - move dists lists which were
 */
static int merge_scanner(scanner_t *s1, scanner_t *s2)
{
    struct list_head *cur, *tmp;
    beacon_t *beacon;
    dist_t *dist;
    int count = 0;

    move_beacon_dists(s1, s2);
    list_for_each_safe(cur, tmp, &s2->list_beacons) {
        beacon = list_entry(cur, beacon_t, list_beacons);
        list_del(cur);
        s2->nbeacons--;
        if (insert_unique_beacon(s1, beacon) < 0)
            pool_add(pool_beacon, beacon);
    }
    /* free all remaining dists from s2 */
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
    dist_t *pdist1, *pdist2, *tmpdist;
    uint dist1, dist2;
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

    /* initialize second scanner dists
     */
    list_for_each_entry(tmpdist, &s2->list_dists, list_dists) {
        tmpdist->common = 0;
    }

    while (plist1 != &s1->list_dists && plist2 != &s2->list_dists) {
        pdist1 = list_entry(plist1, dist_t, list_dists);
        pdist2 = list_entry(plist2, dist_t, list_dists);
        dist1 = pdist1->dist;
        dist2 = pdist2->dist;

        if (dist1 == dist2) {
            pdist2->common = 1;
            plist1 = plist1->next;
            plist2 = plist2->next;
            count++;

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
        } else {                                  /* dist1 > dist2 */
            plist2 = plist2->next;
        }

    }
    return count;
}

/* match all scanners with scanner 0.
 * When a match is found, merge it into scanner 0.
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
            case '-':                             /* scanner header */
                scanner = scanners + nscanners;
                INIT_LIST_HEAD(&scanner->list_beacons);
                INIT_LIST_HEAD(&scanner->list_dists);
                nscanners++;
                scanner->nbeacons = 0;
                break;
            case '\0':                            /* empty line */
                break;
            default:
                beacon = pool_get(pool_beacon);
                sscanf(buf, "%d,%d,%d", &beacon->vec.x, &beacon->vec.y,
                       &beacon->vec.z);
                if (insert_unique_beacon(scanner, beacon) > 0)
                    add_beacon_dists(scanner, beacon);
        }
    }
    free(buf);
    return nscanners;
}

static int part1()
{
    return (*scanners).nbeacons;
}

static int part2()
{
    vector_t *v1, *v2;
    int max = 0, cur;

    for (int i = 0; i < nscanners; ++i) {
        v1 = &scanners[i].rel;
        for (int j = i + 1; j < nscanners; ++j) {
            v2 = &scanners[j].rel;
            cur = abs(v2->x - v1->x)
                + abs(v2->y - v1->y)
                + abs(v2->z - v1->z);
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
    pool_destroy(pool_beacon);
    pool_destroy(pool_dist);
    printf("%s : res=%d\n", *av, part == 1? part1(): part2());
    exit(0);
}
