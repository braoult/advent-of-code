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
 */
typedef union coord {
    u64 val;
    struct {
        s32 x, y;
    };
} coord_t;

typedef struct point {
    coord_t pos;
    struct hlist_node coll;                       /* entry in hash table */
    struct list_head all;
} point_t;

#define HBITS 12                                  /* in bits: 12 bits = 4096 */
#define HSIZE (1 << HBITS)

DEFINE_HASHTABLE(hash1, HBITS);
DEFINE_HASHTABLE(hash2, HBITS);
struct hlist_head *cur = hash1, *next = hash2;

pool_t *pt_pool;

static __always_inline u32 hash(coord_t p)
{
    return hash_64(p.val, HBITS);
}

static point_t *find_point(struct hlist_head *head, coord_t p)
{
    point_t *point;
    hlist_for_each_entry(point, head, coll) {
        if (point->pos.val == p.val)
            return point;
    }
    return NULL;
}

static point_t *add_point(struct hlist_head *p, coord_t pos)
{
    point_t *new;
    u32 h;

    h = hash(pos);
    if (!(new = find_point(p + h, pos))) {
        new = pool_get(pt_pool);
        new->pos.val = pos.val;
        hlist_add_head(&new->coll, p + h);
    }
    return new;
}

static point_t *flip_point(struct hlist_head *p, coord_t pos)
{
    point_t *new;
    u32 h;

    log_f(3, "val=%lu x=%d y=%d ", pos.val, pos.x, pos.y);
    h = hash(pos);
    log(3, "hash=%d ", h);
    if ((new = find_point(p + h, pos))) {
        log(3, "removing tile\n");
        hlist_del(&new->coll);
        pool_add(pt_pool, new);
        new = NULL;
    } else {
        log(3, "adding tile\n");
        new = pool_get(pt_pool);
        new->pos.val = pos.val;
        hlist_add_head(&new->coll, p + h);
    }
    return new;
}

static int count_points(struct hlist_head *h)
{
    point_t *cur;
    int res = 0;
    for (int bkt = 0; bkt < HSIZE; ++bkt) {
        hlist_for_each_entry(cur, &h[bkt], coll) {
            res++;
        }
    }
    return res;
}

static void parse()
{
    size_t alloc;
    ssize_t len;
    char *buf = NULL;

    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        buf[len - 1] = 0;
        coord_t p = { .val = 0 };
        puts(buf);
        char *c = buf;
        while (*c) {
            switch (*c) {
                case 'e':
                    ++p.x;
                    break;
                case 'w':
                    --p.x;
                    break;
                case 's':
                    --p.y;
                    ++c;
                    break;
                case 'n':
                    ++p.y;
                    ++c;
            }
            if (*c == 'e')
                ++p.x;
            else if (*c == 'w')
                --p.x;
            c++;
            //printf("pos=%ld x=%d y=%d\n", c - buf, p.x, p.y);
        }
        flip_point(cur, p);
    }
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
    ulong res = 0;

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
    pt_pool = pool_create("pool_points", 256, sizeof(point_t));

    parse();
    printf("count=%d\n", count_points(cur));
    printf("%s : res=%lu\n", *av, res);
    exit (0);
}
