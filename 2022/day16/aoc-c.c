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
    u16 val;
    char str[2];
};

enum state {
//    BLOCKED = 0,
    CLOSED,
    OPENED
};

struct valve {
    int index;                                    /* -1 for zero flow rate */
    union val val;
    enum state state;
    int rate;
    struct hlist_node hlist;
    struct list_head index_sorted;
    struct list_head flow_sorted;
    struct list_head permute;
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
    .played = LIST_HEAD_INIT(graph.played),
    .indexed = NULL,
    .dist = NULL
};

#define pos(a, b)  ((a)*graph.nvalves + (b))
#define dist(a, b) (graph.dist[pos((a), (b))])

static void print_valves()
{
    ulong  bucket;
    struct valve *cur;
    printf("**** graph: .head=%p npositive=%d nzero=%d\n", graph.aa, graph.npositive,
        graph.nzero);
    hash_for_each(hasht_valves, bucket, cur, hlist) {
        printf("Valve %2.2s: rate=%d ntunnels=%d tottunnels=%d ( ",
               cur->val.str, cur->rate, cur->ntunnels, cur->tottunnels);
        for (int i=0; i < cur->ntunnels; ++i)
            printf("%2s ", cur->tunnels[i]->val.str);
        printf(")\n");
    }
    printf("index1: ");
    list_for_each_entry(cur, &graph.index_sorted, index_sorted) {
        printf("%d:%2.2s ", cur->index, cur->val.str);
    }
    printf(")\n");
    printf("index2: ");
    for (int i = 0; i < graph.nvalves; ++i) {
        printf("%d:%d:%2.2s ", i, graph.indexed[i]->index, graph.indexed[i]->val.str);
    }
    printf(")\n");
    if (testmode()) {
        printf("distances:\n   ");
        for (int i = 0; i < graph.nvalves; ++i) {
            printf("    %2.2s", graph.indexed[i]->val.str);
        }
        printf("\n");
        for (int i = 0; i < graph.nvalves; ++i) {
            printf("%2.2s  ", graph.indexed[i]->val.str);
            for (int j = 0; j < graph.nvalves; ++j) {
                printf("%5d ", dist(i, j));
            }
            printf("\n");
        }
    }
    printf("flow_sorted: ");
    list_for_each_entry(cur, &graph.flow_sorted, flow_sorted) {
        printf("%2.2s:%d ", cur->val.str, cur->rate);
    }
    printf("\n");
    printf("permute: ");
    list_for_each_entry(cur, &graph.permute, permute) {
        printf("%2.2s:%d ", cur->val.str, cur->rate);
    }
    printf("\n");
    printf("openable: %#lx ", graph.openable);
    int pos, tmp;
    bit_for_each64_2(pos, tmp, graph.openable) {
        printf("%d ", pos);
    }
    printf("\n");


}

static struct valve *valve_nth(struct list_head *start, struct list_head *head,
                               int n)
{
    struct valve *cur = list_first_entry_or_null(start, struct valve, flow_sorted);
    int i = 1;

    if (cur) {
        list_for_each_entry_from(cur, head, flow_sorted) {
            if (i == n || cur->flow_sorted.next == head)
                break;
            i++;
        }
    }
    return cur;
}

#define flow2valve(p) list_entry(p, struct valve, flow_sorted)

static struct valve *list_nth(struct list_head *start, struct list_head *head,
                               int n)
{
    struct list_head *cur = NULL;

    if (n == 0 || start->next == head)
        return NULL;
    list_for_each_continue(cur, start) {
        if (!--n || cur == head)
            break;
    }
    return cur ? flow2valve(cur): NULL;
}

static void list_reverse(struct list_head *start, struct list_head *head, int n)
{
    struct list_head *cur = start->next, *tmp;

    list_for_each_safe(cur, tmp, start) {
        if (!--n || cur == head)
            break;
        list_move_tail(cur, start);
        start = cur;
    }
}

static void permute_prepare(int n)
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
 * Will construct next permutation in graph.permute list, following the
 * "lexicographic order algorithm" :
 * https://en.wikipedia.org/wiki/Permutation#Generation_in_lexicographic_order
 *
 * Before first call for a given graph.permute list:
 * 1) the graph.flow_sorted should be (decreasing) sorted.
 * 2) permute_prepare() should have been called.
 * @Return: 0 if no more permutation, 1 otherwise.
 */
static int permute(int n)
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

    log_f(3, "val=%2.2s ntunnels=%d rate=%d h=%u b=%d\n", val.str, ntunnels,
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
        cur.val = *(u16 *)getnth(buf, 1);
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
        if (rate || v1->val.val == ('A' << 8 | 'A')) {
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
        } while ((tok = getnth(NULL, 1)));
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
        for (j = i; j < graph.nvalves; ++j) {
            if (i != j) {
                if (is_neighbour(i, j))
                    dist(i, j) = dist(j, i) = 1;
                else
                    dist(i, j) = dist(j, i) = 10000;
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
                dist(i, j) = dist(j, i) = min(dist(i, j), dist(i, k) + dist(k, j));
        }
    }
    print_valves();
    /*  first, build an array */
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
    struct valve *v;
    for (int i = 1; i < 10; ++i) {
        v = valve_nth(&graph.flow_sorted, &graph.flow_sorted, i);
        printf("sorted(%d): i=%d rate=%d\n", i, v->index, v->rate);
    }
    permute_prepare(4);
    print_valves();
    struct valve *cur;

    printf("permutation 0: ");
    list_for_each_entry(cur, &graph.permute, permute) {
        printf("%d ", cur->rate);
    }
    printf("\n");
    for (int i = 0; permute(i); ++i) {
        printf("permutation %d: ", i);
        list_for_each_entry(cur, &graph.permute, permute) {
            printf("%d ", cur->rate);
        }
        printf("\n");
    }
    //res = do_1(cur, 0, 0);
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
