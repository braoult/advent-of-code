/* ex1-c: Advent2020, day 24/part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bits.h"
#include "pool.h"
#include "debug.h"
#include "hashtable.h"
#include "debug.h"

/* In day 24 tasks, we do not care the order of tiles, as we regenerate
 * a new list after each process.
 * So we will rely on a hashtable, which will allow to quickly find a
 * given point.
 * We use here two hash-tables: One to keep the current black tiles, and
 * one for the neighbours count.
 * My first try with the Linux kernel hashtables implementation.
 */
typedef union coord {
    u64 val;
    struct {
        s32 x, y;
    };
} coord_t;

typedef struct point {
    coord_t pos;
    int count;
    struct hlist_node coll;                       /* entry in hash table */
} point_t;

#define HBITS 11                                  /* in bits: 12 bits = 4096 */

DEFINE_HASHTABLE(hasht_black, HBITS);             /* current black tiles */
DEFINE_HASHTABLE(hasht_count, HBITS);             /* count of neighbours */

pool_t *pt_pool;

static __always_inline u32 hash(coord_t p)
{
    return hash_64(p.val, HASH_BITS(hasht_black));
}

/**
 * find_point - find entry in an hashtable bucket
 */
static point_t *find_point(struct hlist_head *head, coord_t p)
{
    point_t *point;
    hlist_for_each_entry(point, head, coll) {
        if (point->pos.val == p.val)
            return point;
    }
    return NULL;
}

/**
 * add_point - add point in hasht_count hashtable (used to count neighbours)
 */
static point_t *add_point(coord_t pos)
{
    point_t *new;
    u32 h;

    h = hash(pos);
    if (!(new = find_point(&hasht_count[h], pos))) {
        new = pool_get(pt_pool);
        new->pos.val = pos.val;
        new->count = 0;
        hlist_add_head(&new->coll, &hasht_count[h]);
    }
    new->count++;
    return new;
}

/**
 * init_point - add point in hasht_black hashtable, remove if it exists (init)
 */
static point_t *init_point(coord_t pos)
{
    point_t *new;
    u32 h;

    h = hash(pos);
    if ((new = find_point(&hasht_black[h], pos))) {
        hlist_del(&new->coll);
        pool_add(pt_pool, new);
        new = NULL;
    } else {
        new = pool_get(pt_pool);
        new->pos.val = pos.val;
        new->count = 0;
        hlist_add_head(&new->coll, &hasht_black[h]);
    }
    return new;
}

/**
 * count_black - count elements in hasht_black
 */
static int count_black()
{
    point_t *cur;
    int res = 0;
    ulong bucket;

    hash_for_each(hasht_black, bucket, cur, coll)
        res++;
    return res;
}

static const coord_t neighbours [] = {
    { .x =  2, .y =  0 }, { .x = -2, .y = 0 },    /* east and west */
    { .x =  1, .y = -1 }, { .x =  1, .y = 1 },    /* SE and NE */
    { .x = -1, .y = -1 }, { .x = -1, .y = 1 }     /* SW and NW */
};

/**
 * count_neighbours - count hasht_black neighbours, result in hasht_next
 */
static void count_neighbours()
{
    point_t *cur;
    u32 bucket;

    hash_for_each(hasht_black, bucket, cur, coll) {
        for (int i = 0; i < (int) ARRAY_SIZE(neighbours); ++i) {
            coord_t neigh = cur->pos;
            neigh.x += neighbours[i].x;
            neigh.y += neighbours[i].y;
            add_point(neigh);
        }
    }
}

/**
 * adjust_neighbours - adjust hasht_next according to rules
 */
static void adjust_neighbours()
{
    point_t *pt_cur, *pt_count;
    u32 bucket;
    struct hlist_node *tmp;

    /* 1) check hasht_black tiles (currently black)
     */
    hash_for_each_safe(hasht_black, bucket, tmp, pt_cur, coll) {
        int h = hash(pt_cur->pos);
        point_t *pt_count = find_point(&hasht_count[h], pt_cur->pos);
        if (!pt_count || pt_count->count > 2) {
            hash_del(&pt_cur->coll);
            pool_add(pt_pool, pt_cur);
        }
        /* we do not want to re-consider this point in next loop
         */
        if (pt_count) {
            hash_del(&pt_count->coll);
            pool_add(pt_pool, pt_count);
        }
    }
    /* 2) check remaining points in hasht_next (currently white)
     */
    hash_for_each_safe(hasht_count, bucket, tmp, pt_count, coll) {
        hash_del(&pt_count->coll);
        if (pt_count->count == 2) {
            hash_add(hasht_black, &pt_count->coll, hash(pt_count->pos));
        } else {
            pool_add(pt_pool, pt_count);
        }
    }
}

static void parse()
{
    size_t alloc;
    ssize_t len;
    char *buf = NULL;

    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        buf[len - 1] = 0;
        coord_t p = { .val = 0 };
        char *c = buf;
        while (*c) {
            switch (*c) {
                case 'e': ++p.x; break;
                case 'w': --p.x; break;
                case 's': --p.y; ++c; break;
                case 'n': ++p.y; ++c;
            }
            if (*c == 'e')
                ++p.x;
            else if (*c == 'w')
                --p.x;
            c++;
        }
        init_point(p);
    }
    free(buf);
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(ac, av)
    int ac;
    char **av;
{
    int opt, part = 1;
    int res = 0;

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
    pt_pool = pool_create("pool_points", 512, sizeof(point_t));
    parse();
    if (part == 2) {
        for (int i = 0; i < 100; ++i) {
            count_neighbours();
            adjust_neighbours();
        }
    }
    res = count_black();
    printf("%s : res=%d\n", *av, res);
    pool_destroy(pt_pool);
    exit (0);
}
