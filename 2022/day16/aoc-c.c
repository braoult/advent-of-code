/* aoc-c.c: Advent of Code 2022, day 16
 *
 * Copyright (C) 2023 Bruno Raoult ("br")
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

#include "br.h"
#include "list.h"
#include "pool.h"
#include "hashtable.h"
#include "debug.h"
#include "bits.h"

#include "aoc.h"

#define SEP " ,;="
#define HBITS 6                                   /* 6 bits: 64 entries */
static DEFINE_HASHTABLE(hasht_valves, HBITS);
pool_t *pool_valve;

union val {
    u32 val;
    char str[3];
};

enum state {
    CLOSED,
    OPENED
};

struct valve {
    int index;                                    /* -1 for zero flow rate */
    union val val;
    enum state state;
    int rate;
    int evalflow, evaltime;
    int playedflow, playedtime;
    struct hlist_node hlist;
    struct list_head index_sorted;
    struct list_head flow_sorted;
    struct list_head permute;
    struct list_head eval;
    struct list_head played;
    int ntunnels, tottunnels;
    struct valve **tunnels;                       /* array */
};

static struct graph {
    struct valve *aa;                             /* head ("AA") */
    int npositive;                                /* only "AA" & working valves */
    int nzero;                                    /* TO REMOVE ? */
    int nvalves;
    u64 opened;                                   /* bitmask of opened valves */
    u64 openable;                                 /* bitmask of openable valves */
    struct list_head index_sorted;                /* TO REMOVE ? */
    struct list_head flow_sorted;
    struct list_head permute;
    struct list_head eval;
    struct list_head played;
    struct valve **indexed;
    int *dist;                                   /* 2-D array */
} graph = {
    .aa = NULL,
    .npositive = 0,
    .nzero = 0,
    .nvalves = 0,
    .index_sorted = LIST_HEAD_INIT(graph.index_sorted),
    .flow_sorted = LIST_HEAD_INIT(graph.flow_sorted),
    .permute = LIST_HEAD_INIT(graph.permute),
    .eval = LIST_HEAD_INIT(graph.eval),
    .played = LIST_HEAD_INIT(graph.played),
    .indexed = NULL,
    .dist = NULL
};

#define POS(a, b)  ((a)*graph.nvalves + (b))
#define DIST(a, b) (graph.dist[POS((a), (b))])

static void print_valves()
{
    ulong  bucket;
    struct valve *cur;
    printf("**** graph: .head=%p npositive=%d nzero=%d\n", graph.aa, graph.npositive,
        graph.nzero);
    hash_for_each(hasht_valves, bucket, cur, hlist) {
        printf("Valve %s: rate=%d ntunnels=%d tottunnels=%d ( ",
               cur->val.str, cur->rate, cur->ntunnels, cur->tottunnels);
        for (int i=0; i < cur->ntunnels; ++i)
            printf("%s ", cur->tunnels[i]->val.str);
        printf(")\n");
    }
    printf("index1: ");
    list_for_each_entry(cur, &graph.index_sorted, index_sorted) {
        printf("%d:%s ", cur->index, cur->val.str);
    }
    printf("\n");
    printf("index2: ");
    for (int i = 0; i < graph.nvalves; ++i) {
        printf("%d:%s ", graph.indexed[i]->index, graph.indexed[i]->val.str);
    }
    printf("\n");
    if (testmode()) {
        printf("distances:\n   ");
        for (int i = 0; i < graph.nvalves; ++i) {
            printf("    %s", graph.indexed[i]->val.str);
        }
        printf("\n");
        for (int i = 0; i < graph.nvalves; ++i) {
            printf("%s  ", graph.indexed[i]->val.str);
            for (int j = 0; j < graph.nvalves; ++j) {
                printf("%5d ", DIST(i, j));
            }
            printf("\n");
        }
    }
    printf("flow_sorted: ");
    list_for_each_entry(cur, &graph.flow_sorted, flow_sorted) {
        printf("%s:%d ", cur->val.str, cur->rate);
    }
    printf("\n");
    printf("permute: ");
    list_for_each_entry(cur, &graph.permute, permute) {
        printf("%s:%d ", cur->val.str, cur->rate);
    }
    printf("\n");
    printf("openable: %#lx ", graph.openable);
    int pos, tmp;
    bit_for_each64_2(pos, tmp, graph.openable) {
        printf("%d ", pos);
    }
    printf("\n");


}

//#define flow2valve(p) list_entry(p, struct valve, flow_sorted)


/**
 * eval() - eval possible moves from @flow_sorted list.
 * @valve: &starting valve (where we are).
 * @depth: remaining depth (-1: full depth).
 * @pick: max position (in @flow_sorted) to pick moves from (-1 for all).
 * @time: remaining time.
 *
 * Find the "best" next move by evaluating up to @depth moves, using only the
 * first @pick elements in @flow_sorted list, and within @time remaining time.
 *
 * @depth and @picked may be linked, for instance to fully explore the first N
 * possibilities in @flow_sorted with a N depth.
 *
 * @Return: the current position eval.
 */
#define PAD3  log(3, "%*s", _depth, "")
#define PAD4 log(4, "%*s", _depth, "")

static struct valve *eval(int _depth, struct valve *pos, int depth, int pick, int time, int pressure)
{
    struct valve *cur, *best = NULL, *sub;
    struct list_head *list_flow, *tmp;
    int _pick = pick, val = 0, val1, max = 0;

    PAD3;
    log(3, "EVAL _depth=%d pos=%d[%s] depth=%d pick=%d time=%d pressure=%d\n",
        _depth, pos->index, pos->val.str, depth, pick, time, pressure);
    list_for_each_safe(list_flow, tmp, &graph.flow_sorted) {
        cur = list_entry(list_flow, struct valve, flow_sorted);
        int d = DIST(pos->index, cur->index);
        PAD4; log(4, "dist(%s,%s) = %d\n", pos->val.str, cur->val.str, d);
        if (!--_pick) {
            PAD4; log(4, "pick exhausted\n");
            continue;
        }
        if (time - (d + 1 + 1) < 0) {
            PAD4; log(4, "time exhausted\n");
            continue;
        }
        val = (time - (d + 1)) * cur->rate;
        PAD4; log(4, "val=%d\n", val);

        if (depth > 0) {
            /* do not use list_del() here, to preserve prev/next pointers */
            __list_del_entry(list_flow);
            sub = eval(_depth + 2, cur, depth - 1, pick - 1, time - d - 1, pressure + pos->rate);
            list_flow->prev->next = list_flow;
            list_flow->next->prev = list_flow;
        } else {
            sub = NULL;
        }
        val1 = sub? sub->evalflow: 0;
        PAD3; log(3, "eval(%s->%s)= %5d = %d + %d", pos->val.str, cur->val.str,
                  val+val1, val, val1);
        if (val + val1 > max) {
            max = val + val1;
            best = cur;
            log(3, "  NEW MAX !");
        }
        log(3, "\n");
    }
    if (best) {
        best->evalflow = max;
        PAD3; log(3, "EVAL returning best [%s] eval=%d\n", best->val.str, max);
        //best->evaltime = time - (d + 2);
    }
    return best;
}

static __unused void permute_prepare(int n)
{
    struct valve *cur;
    INIT_LIST_HEAD(&graph.permute);
    list_for_each_entry(cur, &graph.flow_sorted, flow_sorted) {
        if (!n--)
            break;
        list_add_tail(&cur->permute, &graph.permute);
    }
}

/**
 * permute() - get next permutation in graph.permute list.
 * @n: permutation number (0 first first one)
 *
 * Construct next permutation in graph.permute list, following the
 * "lexicographic order algorithm" :
 * https://en.wikipedia.org/wiki/Permutation#Generation_in_lexicographic_order
 *
 * Before first call for a given graph.permute list:
 * 1) the graph.flow_sorted should be (decreasing) sorted.
 * 2) permute_prepare() should have been called.
 * @Return: 0 if no more permutation, 1 otherwise.
 */
static __unused int permute(int n)
{
    struct valve *last, *first, *k, *l;

    if (!n)
        return 1;
    last = list_last_entry(&graph.permute, struct valve, permute);
    first = list_first_entry(&graph.permute, struct valve, permute);
    l = last;
    k = list_prev_entry(l, permute);

    while (k->rate <= l->rate) {
        if ((l = k) == first)
            return 0;
        k = list_prev_entry(l, permute);
    }
    l = last;
    while (l != k && k->rate <= l->rate) {
        l = list_prev_entry(l, permute);
    }
    printf("found k=%d l=%d ", k->rate, l->rate);
    list_swap(&l->permute, &k->permute);
    struct list_head *anchor = l->permute.next, *cur, *tmp;
    list_for_each_prev_safe(cur, tmp, &graph.permute) {
        if (cur == anchor)
            break;
        list_move_tail(cur, anchor);
    }
    return 1;
}


static struct valve *find_valve(union val val, int ntunnels, int rate)
{
    struct valve *cur;
    uint hash = val.val, bucket = hash_32(hash, HBITS);

    log_f(3, "val=%s ntunnels=%d rate=%d h=%u b=%d\n", val.str, ntunnels,
          rate, hash, bucket);
    hlist_for_each_entry(cur, &hasht_valves[bucket], hlist) {
        if (cur->val.val == val.val) {
            log(3, "\tfound, addr=%p\n", cur);
            if (ntunnels)
                goto init;
            goto end;
        }
    }
    cur = pool_get(pool_valve);
    cur->val.val = val.val;
    cur->ntunnels = 0;
    cur->state = CLOSED;
    cur->evalflow = cur->playedflow = 0;
    cur->evaltime = cur->playedtime = 30;
    INIT_LIST_HEAD(&cur->index_sorted);
    INIT_LIST_HEAD(&cur->flow_sorted);
    INIT_LIST_HEAD(&cur->permute);
    INIT_LIST_HEAD(&cur->played);
    hlist_add_head(&cur->hlist, &hasht_valves[bucket]);
    log(3, "\talloc new, addr=%p\n", cur);
init:
    if (ntunnels) {
        cur->rate = rate;
        cur->tottunnels = ntunnels;
        cur->tunnels = calloc(ntunnels, sizeof(struct valve *));
    }
end:
    return cur;
}

static char *getnth(char *buf, int n)
{
    char *ret;
    for (; n >= 0; n--) {
        ret = strtok(buf, SEP);
        buf = NULL;
    }
    return ret;
}

static struct graph *parse()
{
    int index = 0, ntunnels;
    ulong  bucket;
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL, *tok;
    int rate;
    struct valve *v1, *v2;
    union val cur, link;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        strncpy(cur.str, getnth(buf, 1), sizeof(cur.str));
        //printf("valve=%s ", tok);
        rate = atoi(getnth(NULL, 3));
        //printf("rate=%s ", tok);
        tok = getnth(NULL, 4);
        ntunnels = (buf + buflen - tok) / 4 + 1;
        v1 = find_valve(cur, ntunnels, rate);
        v1->index = index++;
        // TODO: remove this list ?
        list_add_tail(&v1->index_sorted, &graph.index_sorted);
        graph.nvalves++;
        //if (rate || v1->val.val == ('A' << 8 | 'A')) {
        if (rate) {
            struct valve *cur;
            graph.npositive++;
            /* keep this list sorted by flow decrasing */
            list_for_each_entry(cur, &graph.flow_sorted, flow_sorted) {
                if (rate > cur->rate) {
                    list_add_tail(&v1->flow_sorted, &cur->flow_sorted);
                    goto inserted;
                }
            }
            list_add_tail(&v1->flow_sorted, &graph.flow_sorted);
        inserted: ;
            if (rate) {
                //printf("adjust openable(%d): %#lx", v1->index, graph.openable);
                graph.openable |= (1 << v1->index);
                //printf("->%#lx", graph.openable);
            }
        } else {
            graph.nzero++;
        }
        //printf("lead=%s ntunnels=%d ", tok, ntunnels);
        do {
            link.val = *(u16 *)tok;
            v2 = find_valve(link, 0, 0);
            *(v1->tunnels + v1->ntunnels++) = v2;
            //printf(",%s", tok);
        } while ((tok = getnth(NULL, 0)));
            //printf("\n");
    }
    graph.aa = find_valve((union val) { .str="AA" }, 0, 0);
    /* build array of indexed valves */
    graph.indexed = calloc(graph.nvalves, sizeof(struct valve *));
    index = 0;
    hash_for_each(hasht_valves, bucket, v1, hlist) {
        graph.indexed[v1->index] = v1;
        index++;
    }
    return &graph;
}

static int is_neighbour(int i, int j)
{
    struct valve *v1 = graph.indexed[i], *v2 = graph.indexed[j];
    for (int i = 0; i < v1->ntunnels; ++i)
        if (v1->tunnels[i]->val.val == v2->val.val)
            return 1;
    return 0;
}

static void build_distances()
{
    int i, j, k;
    graph.dist = calloc(graph.nvalves * graph.nvalves, sizeof(int));
    /* initialize values */
    for (i = 0; i < graph.nvalves; ++i) {
        for (j = 1; j < graph.nvalves; ++j) {
            if (i != j) {
                if (is_neighbour(i, j))
                    DIST(i, j) = DIST(j, i) = 1;
                else
                    DIST(i, j) = DIST(j, i) = 10000;
            }
            //printf("pos(%d,%d)=%d\n", i, j, pos(i, j));
        }
    }
    //print_valves();

    /* get all distances using Floyd-Warshall
     * see https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
     *
     * Add all vertices one by one to the set of intermediate vertices.
     * ---> Before start of an iteration, we have shortest distances between all
     * pairs of vertices such that the shortest distances consider only the
     * vertices in set {0, 1, 2, .. k-1} as intermediate vertices.
     * ----> After the end of an iteration, vertex no. k is added to the set of
     * intermediate vertices and the set becomes {0, 1, 2, .. k}
     */
    for (k = 0; k < graph.nvalves; k++) {
        /* Pick all vertices as source one by one */
        for (i = 0; i < graph.nvalves; i++) {
            /* Pick all vertices as destination for the above picked source */
            for (j = i + 1; j < graph.nvalves; j++)
                DIST(i, j) = DIST(j, i) = min(DIST(i, j), DIST(i, k) + DIST(k, j));
        }
    }
    return;
}

//static ulong do_1(struct valve *cur, int min, int pressure)
//{
//    ulong tmp;

    //  if (cur->state == CLOSED) {

    //}
//}

//static union val start = { .str = "AA" };

static ulong part1()
{
    ulong res = 1;
    //struct valve *cur = graph.aa;

    printf("part1\n");
    build_distances();
    print_valves();

    puts("zob1");
    eval(0, graph.aa, 7, 7, 30, 0);
    puts("zob2");
    /*
    permute_prepare(4);
    for (int i = 0; permute(i); ++i) {
        struct valve *cur;
        printf("permutation %d: ", i);
        list_for_each_entry(cur, &graph.permute, permute) {
            printf("%d ", cur->rate);
        }
        printf("\n");
    }
    */

    return res;
}

static ulong part2()
{
    ulong res = 2;
    return res;
}


int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_valve = pool_create("valve", 512, sizeof(struct valve));
    parse();
    printf("%s: res=%lu\n", *av, part == 1? part1(): part2());
    exit(0);
}
