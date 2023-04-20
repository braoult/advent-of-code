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
#include "debug.h"
#include "bits.h"

#include "aoc.h"

pool_t *pool_valve;

union val {
    u32 val;
    char str[3];
};

enum state {
    CLOSED,
    OPENED
};

struct worker {
    struct valve *pos;
    int depth;
    int time;
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
    struct list_head eval;
    int worker;
    struct list_head played;
    int ntunnels, tottunnels;
    struct valve **tunnels;                       /* array */
};

static struct graph {
    struct valve *aa;                             /* head ("AA") */
    int npositive;                                /* only "AA" & working valves */
    int nvalves;
    u64 opened;                                   /* bitmask of opened valves */
    u64 openable;                                 /* bitmask of openable valves */
    struct list_head index_sorted;                /* TO REMOVE ? */
    struct list_head flow_sorted;
    struct list_head eval;
    struct list_head played[2];
    struct valve **indexed_all;
    int *dist;                                   /* 2-D array */
} graph = {
    .aa = NULL,
    .npositive = 0,
    .nvalves = 0,
    .index_sorted = LIST_HEAD_INIT(graph.index_sorted),
    .flow_sorted = LIST_HEAD_INIT(graph.flow_sorted),
    .eval = LIST_HEAD_INIT(graph.eval),
    .played[0] = LIST_HEAD_INIT(graph.played[0]),
    .played[1] = LIST_HEAD_INIT(graph.played[1]),
    .indexed_all = NULL,
    .dist = NULL
};

#define POS(a, b)  ((a)*graph.nvalves + (b))
#define DIST(a, b) (graph.dist[POS((a), (b))])

static void print_valves()
{
    struct valve *cur;
    printf("**** graph: .head=%p npositive=%d\n", graph.aa, graph.npositive);
    printf("index1: ");
    list_for_each_entry(cur, &graph.index_sorted, index_sorted) {
        printf("%d:%s ", cur->index, cur->val.str);
    }
    printf("\n");
    printf("index2: ");
    for (int i = 0; i < graph.nvalves; ++i) {
        printf("%d:%s ", graph.indexed_all[i]->index, graph.indexed_all[i]->val.str);
    }
    printf("\n");
    if (testmode()) {
        printf("distances:\n   ");
        for (int i = 0; i < graph.nvalves; ++i) {
            printf("    %s", graph.indexed_all[i]->val.str);
        }
        printf("\n");
        for (int i = 0; i < graph.nvalves; ++i) {
            printf("%s  ", graph.indexed_all[i]->val.str);
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
    printf("openable: %#lx ", graph.openable);
    int pos, tmp;
    bit_for_each64_2(pos, tmp, graph.openable) {
        printf("%d ", pos);
    }
    printf("\n");


}

#define PAD3  log(3, "%*s", _depth * 2, "")
#define PAD4 log(4, "%*s", _depth * 2, "")

/**
 * eval() - eval possible moves from @flow_sorted list.
 * @_depth: recursivity depth (for debug only, TODO: remove).
 * @nworkers: number of workers.
 * @workers: array of workers.
 * @pick: max position (in @flow_sorted) to pick moves from (-1 for all).
 * @pressure: total pressure per time unit so far.
 *
 * Find the "best" next move by evaluating only the first @pick elements
 * in @flow_sorted list.
 *
 * @Return: the current position eval.
 */
static struct valve *eval(int _depth, int nworkers, struct worker *worker, int pick, int pressure)
{
    struct valve *cur, *best = NULL, *sub;
    struct list_head *list_flow, *tmp;
    int _pick = pick, val = 0, val1, max = 0, bestworker = -1;
    int _nworkers = nworkers;
    if (nworkers == 2 && worker[0].pos->index ==  worker[1].pos->index)
        _nworkers = 1;

    PAD3; log_f(3, "EVAL _depth=%d w0={ pos=%d[%s] depth=%d time=%d }  w1={ pos=%d[%s] depth=%d time=%d } pick=%d pressure=%d \n",
              _depth,
              worker[0].pos->index, worker[0].pos->val.str,
              worker[0].depth, worker[0].time,
              worker[1].pos->index, worker[1].pos->val.str,
              worker[1].depth, worker[1].time,
              pick, pressure);
    list_for_each_safe(list_flow, tmp, &graph.flow_sorted) {
        cur = list_entry(list_flow, struct valve, flow_sorted);
        //int nworkers = worker[0].pos->index == worker[1].pos->index? 1: 2;
        if (!--_pick) {
            PAD4; log(4, "pick exhausted\n");
            break;
        }
        for (int _w = 0; _w < _nworkers; ++_w) {
            struct worker *w = worker + _w;
            int d = DIST(w->pos->index, cur->index);
            int remain = w->time - (d + 1);
            PAD3; log(3, "worker %d/%d ", _w + 1, nworkers );
            PAD3; log(3, "dist(%s,%s) = %d ", w->pos->val.str, cur->val.str, d);
            if (remain < 1) {
                PAD3; log(4, "time exhausted\n");
                continue;
            }
            val = remain * cur->rate;
            PAD3; log(3, "--> val=%d\n", val);

            if (w->depth > 0) {
                struct worker _tmp = *w;
                w->pos = cur;
                w->depth--;
                w->time = remain;
                /* do not use list_del() here, to preserve prev/next pointers */
                __list_del_entry(list_flow);
                sub = eval(_depth + 1, nworkers, worker, pick, pressure + w->pos->rate);
                list_flow->prev->next = list_flow;
                list_flow->next->prev = list_flow;
                *w = _tmp;
            } else {
                sub = NULL;
            }
            val1 = sub? sub->evalflow: 0;
            PAD3; log(3, "eval3(%s->%s)= %5d = %d + %d", w->pos->val.str, cur->val.str,
                      val+val1, val, val1);
            if (val + val1 > max) {
                max = val + val1;
                best = cur;
                bestworker = _w;
                log(3, "  NEW MAX !");
            }
            log(3, "\n");
        }
    }
    if (best) {
        best->evalflow = max;
        best->worker = bestworker;                         /* FIXME */
        PAD3; log(3, "EVAL returning best [%s] worker=%d eval=%d\n", best->val.str,
                  best->worker, max);
    }
    return best;
}

static struct valve *find_valve(union val val, int ntunnels, int rate)
{
    struct valve *cur;

    log_f(3, "val=%s ntunnels=%d rate=%d\n", val.str, ntunnels, rate);
    list_for_each_entry(cur, &graph.index_sorted, index_sorted) {
        //log(3, "\tcomparing with found, addr=%p\n", cur);
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
    cur->worker = -1;
    cur->evalflow = cur->playedflow = 0;
    cur->evaltime = cur->playedtime = 30;
    INIT_LIST_HEAD(&cur->index_sorted);
    INIT_LIST_HEAD(&cur->flow_sorted);
    INIT_LIST_HEAD(&cur->eval);
    INIT_LIST_HEAD(&cur->played);
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

/**
 * nthtok - get nth token fron string.
 * @buf: buffer to parse.
 * @sep: separators string.
 * @n: token number (0: first token).
 *
 * @Return: pointer to token.
 */
static char *nthtok(char *buf, const char *sep, int n)
{
    char *ret;
    for (; n >= 0; n--) {
        ret = strtok(buf, sep);
        buf = NULL;
    }
    return ret;
}

#define SEP " ,;="
static struct graph *parse()
{
    int index = 0, ntunnels;
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL, *tok;
    int rate;
    struct valve *v1, *v2;
    union val cur, link;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        /* valve name */
        strncpy(cur.str, nthtok(buf, SEP, 1), sizeof(cur.str));

        //printf("valve=%s ", tok);
        rate = atoi(nthtok(NULL, SEP, 3));
        //printf("rate=%s ", tok);
        tok = nthtok(NULL, SEP, 4);
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
        }
        //printf("lead=%s ntunnels=%d ", tok, ntunnels);
        do {
            link.val = *(u16 *)tok;
            v2 = find_valve(link, 0, 0);
            *(v1->tunnels + v1->ntunnels++) = v2;
            //printf(",%s", tok);
        } while ((tok = nthtok(NULL, SEP, 0)));
            //printf("\n");
    }
    graph.aa = find_valve((union val) { .str="AA" }, 0, 0);
    /* build array of indexed valves */
    graph.indexed_all = calloc(graph.nvalves, sizeof(struct valve *));
    list_for_each_entry(v1, &graph.index_sorted, index_sorted) {
        graph.indexed_all[v1->index] = v1;
    }
    return &graph;
}

static int is_neighbour(int i, int j)
{
    struct valve *v1 = graph.indexed_all[i], *v2 = graph.indexed_all[j];
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
        }
    }

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

static void print_played(int nworkers)
{
    struct valve *p;
    int total = 0;
    for (int w = 0; w < nworkers; ++w) {
        int remain = 26, i = 1;
        struct valve *prev = graph.aa;
        i = 1;
        printf("played by %d/%d:\n", w + 1, nworkers);
        list_for_each_entry(p, &graph.played[w], played) {
            printf("%2d: %s, ", i, p->val.str);
            remain -= DIST(p->index, prev->index) + 1;
            total += p->rate * remain;
            printf("dist=%d remain=%d total=%d", DIST(p->index, prev->index), remain,
                   total);
            printf("\n");
            i++;
            prev = p;
        }
    }
}

static int doit(int part)
{
    struct worker w[2];
    struct valve *best;
    int res = 0;
    //int topick = part == 1? 7: 12;
    int topick = part == 1? 12: 12;

    w[0].pos = w[1].pos = graph.aa;
    //w[0].depth = w[1].depth = part == 1? 7: 4;
    w[0].depth = w[1].depth = part == 1? 4: 4;
    w[0].time = w[1].time = part == 1? 30: 26;

    while ((best = eval(0, part, w, topick, 0))) {
        list_del(&best->flow_sorted);
        list_add_tail(&best->played, &graph.played[best->worker]);
        w[best->worker].time -= DIST(w[best->worker].pos->index, best->index) + 1;
        w[best->worker].pos = best;
        res += best->rate * w[best->worker].time;
        print_played(part);
    }
    return res;
}

static ulong part1()
{
    return doit(1);
}

static ulong part2()
{
    return doit(2);
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_valve = pool_create("valve", 512, sizeof(struct valve));
    parse();
    build_distances();
    print_valves();
    printf("%s: res=%lu\n", *av, part == 1? part1(): part2());
    exit(0);
}
