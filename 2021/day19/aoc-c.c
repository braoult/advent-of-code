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
    u64 manhattan;                                /* manhattan distance from scanner */
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
    struct list_head list_beacons;
    struct list_head list_dists;
    beacon_t *ref[3];                             /* reference beacons */
    //vector_t ref[3];
} scanner_t;

static pool_t *pool_beacon;
static pool_t *pool_dist;

static scanner_t scanners[MAX_SCANNERS];
static int nscanners;

static void scanners_print_dists()
{
    dist_t *cur;
    beacon_t *beacon1, *beacon2;

    log_f(1, "nscanners: %d\n", nscanners);
    for (int i = 0; i < nscanners; ++i) {
        log(1, "scanner %d: %d dists\n", i, scanners[i].ndists);
        list_for_each_entry(cur, &scanners[i].list_dists, list_dists) {
            beacon1 = cur->beacon1;
            beacon2 = cur->beacon2;
            log_i(3, "%lu : %d-%d (%ld,%ld,%ld)-(%ld,%ld,%ld)\n", cur->dist,
                  beacon1->num, beacon2->num,
                  beacon1->vec.x, beacon1->vec.y, beacon1->vec.z,
                  beacon2->vec.x, beacon2->vec.y, beacon2->vec.z);
        }
    }
}

static void scanners_print_refs(scanner_t *s1, scanner_t *s2)
{
    log_f(1, "s1:%ld s2:%ld\n", s1 - scanners, s2 - scanners);
    log_i(2, "scanner %ld:", s1 - scanners);
    for (int i = 0; i < 3; ++i) {
        beacon_t *beacon = s1->ref[i];
        log(1, " (%ld,%ld,%ld)", beacon->vec.x, beacon->vec.y, beacon->vec.z);
    }
    log(1, "\n");
    log_i(2, "\nscanner %ld:", s2 - scanners);
    for (int i = 0; i < 3; ++i) {
        beacon_t *beacon = s2->ref[i];
        log(1, " (%ld,%ld,%ld)", beacon->vec.x, beacon->vec.y, beacon->vec.z);
    }
    log(1, "\n");
}

static void scanner_print(scanner_t *s)
{
    beacon_t *cur;

    log(1, "scanner %ld: %d beacons\n", s - scanners, s->nbeacons);
    list_for_each_entry(cur, &s->list_beacons, list_beacons) {
        log_i(3, " m=%lu %ld/%ld/%ld\n", cur->manhattan,
            cur->vec.x, cur->vec.y, cur->vec.z);
    }
    //log(1, "\n");
}

static void scanners_print()
{
    log_f(1, "nscanners: %d\n", nscanners);
    for (int i = 0; i < nscanners; ++i) {
        scanner_print(scanners + i);
    }
}

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

    log_f(7, "dist=%u\n", newdist);

    cur = list_first_entry_or_null(&scanner->list_dists, dist_t, list_dists);
    /* special case: first distance or new dist lower than first dist */
    if (!cur || newdist < cur->dist) {
        list_add(&dist->list_dists, &scanner->list_dists);
        log_i(7, "first entry\n");
        goto end;
    }
    /* normal case: insert before current when new dist is lower than current dist */
    list_for_each_entry(cur, &scanner->list_dists, list_dists) {
        log_i(7, "comp=%lu\n", cur->dist);
        if (newdist < cur->dist) {
            list_add_tail(&dist->list_dists, &cur->list_dists);
            log_i(7, "add before\n");
            goto end;
        }
    }
    /* special case: we went to end, insert at list's tail */
    list_add_tail(&dist->list_dists, &scanner->list_dists);
    log_i(7, "add end\n");
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
        log_f(7, "scanner %lu new dist : %lu (%ld,%ld,%ld) / (%ld,%ld,%ld)\n",
              scanner - scanners,
              dist->dist,
              beacon->vec.x, beacon->vec.y, beacon->vec.z,
              cur->vec.x, cur->vec.y, cur->vec.z);
        insert_dist(scanner, dist);
        count++;
    }
    log_f(3, "scanner %lu : %d new dists\n", scanner - scanners, count);
    return count;
}

/* compare two beacons by manhattan distance, then x, y, and z.
 */
static inline int compare_beacons(beacon_t *b1, beacon_t *b2)
{
    vector_t *v1 = &b1->vec, *v2 = &b2->vec;
    u64 m1 = b1->manhattan, m2 = b2->manhattan;

    if (m1 < m2 ||
        (m1 == m2 &&
         (v1->x < v2->x ||
          (v1->x == v2->x && v1->y < v2->y) ||
          ((v1->x == v2->x && v1->y == v2->y) && v1->z < v2->z)))) {
        return -1;
    } else if (m1 == m2 && v1->x == v2->x && v1->y == v2->y && v1->z == v2->z) {
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
    vector_t *vec = &beacon->vec;
    beacon_t *cur;
    u64 manhattan = labs(vec->x) + labs(vec->y) + labs(vec->z);

    beacon->manhattan = manhattan;

    log_f(3, "manhattan=%lu\n", manhattan);

    cur = list_first_entry_or_null(&scanner->list_beacons, beacon_t, list_beacons);
    /* special case: first beacon or new beacon lower than first beacon */
    if (!cur || compare_beacons(beacon, cur) < 0) {
        list_add(&beacon->list_beacons, &scanner->list_beacons);
        log_i(7, "first entry\n");
        goto end;
    }
    /* normal case: insert before current when new dist is lower than current dist */
    list_for_each_entry(cur, &scanner->list_beacons, list_beacons) {
        log_i(7, "comp=%lu\n", cur->manhattan);
        switch (compare_beacons(beacon, cur)) {
            case -1:
                list_add_tail(&beacon->list_beacons, &cur->list_beacons);
                log_i(7, "add before\n");
                goto end;
            case 0:
                return -1;
        }
    }
    /* special case: we went to end, insert at list's tail */
    list_add_tail(&beacon->list_beacons, &scanner->list_beacons);
    log_i(7, "add end\n");
end:
    return ++scanner->nbeacons;
}

/* return 1 if b exists in s beacons's list, 0 otherwise.
 */
static int beacon_exists(scanner_t *s, beacon_t *b)
{
    beacon_t *cur;

    list_for_each_entry(cur, &s->list_beacons, list_beacons) {
        if (b->vec.x == cur->vec.x &&
            b->vec.y == cur->vec.y &&
            b->vec.z == cur->vec.z) {
            return 1;
        }
    }
    return 0;
}

/* using scanner's reference points, find the correct rotation and translation
 */
static int adjust_scanner(scanner_t *ref, scanner_t *s)
{
    beacon_t *beacon_ref[3], *beacon[3], *cur;
    int error = -1;

    //log_f(1, "sizeof(rotations)=%lu\n", NROTATIONS);
    for (uint i = 0; i < 3; ++i) {
        beacon_ref[i] = ref->ref[i];
        beacon[i] = s->ref[i];
    }

    for (uint rotnum = 0; rotnum < NROTATIONS; ++rotnum) {
        vector_t rot[3], diff[3];
        //int match = 1;
        /* rotate the first beacon and translate to match ref's (x, y, z)
         */
        for (int bref = 0; bref < 3; ++bref) {
            rot[bref] = vector_rotate(&beacon[bref]->vec, rotnum);
            diff[bref] = vector_diff(&beacon_ref[bref]->vec, rot + bref);

            log(2, "ref %d diff=(%ld,%ld,%ld)\n", bref,
                diff[bref].x, diff[bref].y, diff[bref].z);
            /* check that rotation/translation works for the 3 reference points
             */
            if (bref > 0 && (diff[bref].x != diff[0].x ||
                             diff[bref].y != diff[0].y ||
                             diff[bref].z != diff[0].z)) {
                log(2, "skipping this translation\n");
                goto next_rot;
            }
        }
        log(2, "Got it: scanner %lu is (%ld,%ld,%ld) from reference\n",
            s - scanners, diff[0].x, diff[0].y, diff[0].z);
        error = 0;
        /* adjust all beacons */
        list_for_each_entry(cur, &s->list_beacons, list_beacons) {
            //vector_t tmp;
            log(2, "translating beacon: (%ld,%ld,%ld) ->", cur->vec.x,
                cur->vec.y, cur->vec.z);
            cur->vec = vector_rotate(&cur->vec, rotnum);
            cur->vec = vector_add(&cur->vec, &diff[0]);
            //cur->vec.y = tmp.y + diff[0].y;
            //cur->vec.z = tmp.z + diff[0].z;
            log(2, " (%ld,%ld,%ld)\n", cur->vec.x, cur->vec.y, cur->vec.z);
        }
        scanner_print(s);
        s->adjusted = 1;

        break;
    next_rot:
        log(2, "\n");
    }
    if (error) {
        log_f(1, "BUG!!\n");
    }
    return error;
}

/* add distances between b1 and all following beacons in scanner's list
 */
static int add_beacon_dists(scanner_t *scanner, beacon_t *b1)
{
    beacon_t *b2 = b1;
    dist_t *dist;
    int count = 0;

    list_for_each_entry_continue(b2, &scanner->list_beacons, list_beacons) {
        dist = pool_get(pool_dist);
        dist->dist =
            (b1->vec.x - b2->vec.x) * (b1->vec.x - b2->vec.x) +
            (b1->vec.y - b2->vec.y) * (b1->vec.y - b2->vec.y) +
            (b1->vec.z - b2->vec.z) * (b1->vec.z - b2->vec.z);
        dist->beacon1 = b1;
        dist->beacon2 = b2;
        log_f(1, "scanner %lu new dist : %lu (%ld,%ld,%ld) / (%ld,%ld,%ld)\n",
              scanner - scanners,
              dist->dist,
              b1->vec.x, b1->vec.y, b1->vec.z,
              b2->vec.x, b2->vec.y, b2->vec.z);
        insert_dist(scanner, dist);
        count++;
    }
    return count;
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

    //log_f(1, "merging scanner %lu into %lu\n", s2 - scanners, s1 - scanners);
    log_f(3, "before(%lu -> %lu): count1=%d count2=%d dist1=%d dist2=%d\n\n",
          s2 - scanners, s1 - scanners,
          s1->nbeacons, s2->nbeacons, s1->ndists, s2->ndists);

    list_for_each_safe(cur, tmp, &s2->list_beacons) {
        beacon = list_entry(cur, beacon_t, list_beacons);
        list_del(cur);
        s2->nbeacons--;
        if (insert_unique_beacon(s1, beacon) > 0)
            add_beacon_dists1(s1, beacon);
    }
    /*
        if (beacon->common) {
            if (!beacon_exists(s1, beacon))
                log(2, "BUG 1\n");
            log(2, "common beacon ignored (%ld,%ld,%ld) - count=%d\n",
                beacon->vec.x, beacon->vec.y, beacon->vec.z, beacon->common);
            pool_add(pool_beacon, beacon);
        } else {
            if (beacon_exists(s1, beacon))
                log(2, "BUG 2\n");
            log(2, "add new beacon (%ld,%ld,%ld) - count=%d\n",
                beacon->vec.x, beacon->vec.y, beacon->vec.z, beacon->common);
            list_add(cur, &s1->list_beacons);
            s1->nbeacons++;
            add_beacon_dists(s1, beacon);
            count++;
        }
    }
    */
    /* free all dists */
    list_for_each_safe(cur, tmp, &s2->list_dists) {
        dist = list_entry(cur, dist_t, list_dists);
        list_del(cur);
        s2->ndists--;
        pool_add(pool_dist, dist);
    }
    log_f(3, "after(%lu -> %lu):  count1=%d count2=%d dist1=%d dist2=%d added=%d\n",
          s2 - scanners, s1 - scanners,
          s1->nbeacons, s2->nbeacons, s1->ndists, s2->ndists, count);
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
    int ref_triangle = 0, nref = 0;
    uint count = 0;

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
            log(1, " %lu: (%d,%d)= %d-%d %d-%d triangle=%d\n",
                dist1,
                cur1, cur2,
                pdist1->beacon1->num, pdist1->beacon2->num,
                pdist2->beacon1->num, pdist2->beacon2->num,
                ref_triangle);
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
                    log_i(3, "s1_ref=%d,%d,-1 s2_ref=%d,%d,-1\n",
                          s1->ref[0]->num, s1->ref[1]->num,
                          s2->ref[0]->num, s2->ref[1]->num);
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

                        log(1, "s%c: beacon1=%p beacon2=%p\n",
                            i + '1', beacon1, beacon2);
                        log(1, "     ref1   =%p ref2   =%p\n",
                            scanner->ref[0], scanner->ref[1]);

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
                        /*else {
                            log_f(1, "BUG!!\n");
                        }
                        */
                    }

                    log_i(3, "nref=%d s1_ref=%d,%d,%d s2_ref=%d,%d,%d\n",
                          nref,
                          s1->ref[0]->num,
                          s1->ref[1]->num,
                          s1->ref[2]? s1->ref[2]->num: -1,
                          s2->ref[0]->num,
                          s2->ref[1]->num,
                          s2->ref[2]? s2->ref[2]->num: -1);
                    break;
            }


        } else if (dist1 < dist2) {
            log(7, "    dist1=%lu < dist2=%lu : (%d,%d)= %d-%d %d-%d triangle=%d\n",
                dist1, dist2,
                cur1, cur2,
                pdist1->beacon1->num, pdist1->beacon2->num,
                pdist2->beacon1->num, pdist2->beacon2->num,
                ref_triangle);
            plist1 = plist1->next;
            cur1++;
        } else {                                  /* dist1 > dist2 */
            log(7, "    dist1=%lu > dist2=%lu : (%d,%d)= %d-%d %d-%d triangle=%d\n",
                dist1, dist2,
                cur1, cur2,
                pdist1->beacon1->num, pdist1->beacon2->num,
                pdist2->beacon1->num, pdist2->beacon2->num,
                ref_triangle);
            plist2 = plist2->next;
            cur2++;
        }

    }
    return count;
}

/* For each scanner, calculate square distances between every beacon,
 * and generate corresponding list.
 * For N beacons, we will get (N) * (N-1) / 2 distances.
 *
 * Note: To find 12 matching beacons between scanners later, we will need
 * 66 matching distances (= 12 * 11 / 2).
 */
static void calc_square_distances()
{
    scanner_t *scanner;
    beacon_t *beacon;

    log_f(1, "nscanners: %d\n", nscanners);
    for (int i = 0; i < nscanners; ++i) {
        scanner = scanners + i;
        list_for_each_entry(beacon, &scanner->list_beacons, list_beacons) {
            add_beacon_dists(scanner, beacon);
        }
    }
    scanners_print_dists();
}

/* match all scanners
 */
static void match_scanners()
{
    int finished = 0;

    scanners[0].adjusted = 1;
    log_f(1, "nscanners=%d\n", nscanners);
    while (!finished) {
        finished = 1;
        for (int j = 1; j < nscanners; ++j) {
            if (scanners[j].adjusted) /* already translated */
                continue;

            int count = count_common_distances(scanners, scanners + j);
            log(1, "common(%d, %d) = %d\n\n", 0, j, count);
            if (count >= 66) {
                adjust_scanner(scanners, scanners + j);
                scanners_print_refs(scanners, scanners + j);

                merge_scanner(scanners, scanners + j);
                finished = 0;
            }
        }
    }
    scanners_print_dists();
}

static void match_scanners1()
{
    int finished = 0;

    scanners[0].adjusted = 1;
    log_f(1, "nscanners=%d\n", nscanners);
    while (!finished) {
        finished = 1;
        for (int i = 0; i < nscanners - 1; ++i) {
            if (!scanners[i].adjusted)            /* skip un-translated scanners */
                continue;

            for (int j = i+1; j < nscanners; ++j) {
                if (i == j || scanners[j].adjusted) /* already translated */
                    continue;

                int count = count_common_distances(scanners + i, scanners + j);
                log(1, "common(%d, %d) = %d\n", i, j, count);
                if (count >= 66) {
                    adjust_scanner(scanners + i, scanners + j);
                    scanners_print_refs(scanners + i, scanners + j);

                    //merge_scanner(scanners + i, scanners + j);
                    finished = 0;
                }
            }
        }
    }
    scanners_print_dists();
    //scanners_print_dists(scanners + 2);
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
                log_f(9, "[%c] nscanners = %d %p/%p\n", *buf, nscanners,
                      scanner, scanners);
                INIT_LIST_HEAD(&scanner->list_beacons);
                INIT_LIST_HEAD(&scanner->list_dists);
                nscanners++;
                scanner->nbeacons = 0;
                break;
            case '\0':
                log_f(9, "NULL line\n");
                break;
            default:
                //log_f(2, "[%c] beacon = %d\n", *buf, nscanners);
                beacon = pool_get(pool_beacon);
                beacon->scanner = nscanners;
                beacon->num = scanner->nbeacons;
                sscanf(buf, "%ld,%ld,%ld", &beacon->vec.x, &beacon->vec.y,
                       &beacon->vec.z);
                if (insert_unique_beacon(scanner, beacon) > 0)
                    add_beacon_dists1(scanner, beacon);
                //log(1, "zobi\n");
                //list_add_tail(&beacon->list_beacons, &scanner->list_beacons);
        }
    }
    free(buf);
    scanners_print();
    return nscanners;
}

static s64 part1()
{
    return (*scanners).nbeacons;
}

static s64 part2()
{
    return list_last_entry(&(*scanners).list_beacons, beacon_t, list_beacons)->manhattan;
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

    log_f(1, "sizeof(rotations)=%lu, sizeof2=%lu sizeof3=%lu rot=%lu\n",
          sizeof(rotations), sizeof(*rotations),
          sizeof(rotations) / sizeof(*rotations),
          NROTATIONS);
    if (!(pool_beacon = pool_create("beacons", 1024, sizeof(beacon_t)))) {
        log(1, "pool create error, errno=%d\n", errno);
        exit(1);
    }
    if (!(pool_dist = pool_create("dists", 1024, sizeof(dist_t)))) {
        log(1, "pool create error, errno=%d\n", errno);
        exit(1);
    }
    scanners_read();
    //calc_square_distances();
    match_scanners();
    scanners_print();
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
    pool_stats(pool_beacon);
    pool_destroy(pool_beacon);
    pool_stats(pool_dist);
    pool_destroy(pool_dist);
    exit(0);
}
