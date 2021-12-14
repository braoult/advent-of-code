/* aoc-c.c: Advent of Code 2021, day 11 parts 1 & 2
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
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
#include <string.h>
#include <malloc.h>
#include <ctype.h>

#include "debug.h"
#include "pool.h"
#include "bits.h"
#include "list.h"


/* graph representation used :
 *
 *                NODE ARRAY           LINK (doubly linked)
 *                -------------        --------------
 *                | node[x]   |        | link x_1   |
 * prev link <----> list_head <--------> list head  <----> next link <----> ...
 *                | ...       |        |            |
 *                -------------    ----< node y ptr |
 *                                 |   | ...        |
 *                ...              |   --------------
 *                                 |
 *                ------------- <---   --------------
 *                | node[y]   |        | link y_1   |
 * prev link <----> list_head <--------> list head  <----> next link
 *                | ...       |        |            |
 *                -------------        | node ptr   |
 *                                     | ...        |
 *                                     --------------
 *
 */
#define MAX_LINKS 10
#define MAX_NODES 128

#define START 0                                   /* start is always nodes[0] */
#define END   1                                   /* end is always nodes[1] */

typedef struct node {
    char token[16];                               /* node name */
    int val;                                      /* unused ? */
    int passed;                                   /* number of passages */
    int multiple;                                 /* can pass multiple times ? */
    int nlinks;                                   /* number of links */
    struct list_head list;                        /* links list */
} node_t;

typedef struct link {                             /* links between nodes */
    node_t *node;                                 /* linked node */
    int val;                                      /* unused ? */
    struct list_head list;                        /* next/prev link */
} link_t;

static node_t nodes[MAX_NODES];                   /* nodes list */
static int nnodes;                                /* ... and size */

static pool_t *links_pool;                        /* links memory pool */

static node_t* stack[1024];                       /* moves stack */
static int nstack;                                /* ... and size */
static int npaths;                                /* number of solutions */

static void print_tokens()
{
    log_f(3, "nodes = %d\n", nnodes);
    for (int i = 0; i < nnodes; ++i) {
        log(3, "%2d: %6s mult=%d passed=%d\n", i, nodes[i].token,
            nodes[i].multiple, nodes[i].passed);
    }
}

static void print_links()
{
    struct link *link;
    node_t *node;

    log_f(3, "links = %d\n", nnodes);
    for (int i = 0; i < nnodes; ++i) {
        node = nodes+i;
        log(3, "[%6s]", node->token);
        list_for_each_entry(link, &node->list, list) {
            printf(" --> %s", link->node->token);
        }
        log(3, "\n");
    }
}

/* find node from token value
 * return @node or NULL
 */
static node_t *find_token(char *token)
{
    log_f(2, "token=[%s]\n", token);
    for (int i = 0; i < nnodes; ++i) {
        log_i(4, "compare=[%s]\n", nodes[i].token);
        if (!strcmp(nodes[i].token, token)) {
            return nodes+i;
        }
    }
    return NULL;
}

/* get node from token value (create if does not exist)
 * return @node
 */
static node_t *add_token_maybe(char *token)
{
    node_t *node;

    if (!(node = find_token(token))) {
        node = nodes + nnodes;
        strcpy(node->token, token);
        node->multiple = isupper(*token)? 1: 0;
        node->passed = 0;
        node->nlinks = 0;
        INIT_LIST_HEAD(&node->list);
        nnodes++;
    }
    return node;
}

/* set all nodes with passed > pass (-1: all) "passed" value to zero
 */
static void clean_node_passed(int pass)
{
    for (int i = 0; i < nnodes; ++i) {
        if (pass == -1 || nodes[i].passed >= pass)
            nodes[i].passed = 0;
    }
}

/* create a link between two nodes
 * return 1 if OK, 0 if error
 */
static int link_nodes(node_t *n1, node_t *n2)
{
    struct link *link1, *link2;

    if (!((link1 = pool_get(links_pool)) && (link2 = pool_get(links_pool))))
        return 0;
    link1->node = n2;
    list_add_tail(&link1->list, &n1->list);
    link2->node = n1;
    list_add_tail(&link2->list, &n2->list);
    return 1;
}

/* read data and create tree.
 */
static int read_graph()
{
    ssize_t len;
    size_t alloc = 0;
    char *buf = NULL, *token1, *token2;
    node_t *node1, *node2;

    if (!(links_pool = pool_init("links", 128, sizeof (struct link))))
        return -1;

    add_token_maybe("start");
    add_token_maybe("end");
    print_tokens();
    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        token1 = strtok(buf, "-\n");
        token2 = strtok(NULL, "-\n");
        node1 = add_token_maybe(token1);
        node2 = add_token_maybe(token2);
        link_nodes(node1, node2);
        //log_f(2, "token1=[%s] token2=[%s]\n", token1, token2);
    }
    free(buf);

    print_tokens();
    clean_node_passed(-1);
    print_tokens();
    print_links();
    return nnodes;
}

/* push link ptr to stack
 */
inline static node_t *push(node_t *node)
{
    return ((stack[nstack++] = node));
}

/* pop link ptr from stack
 */
inline static node_t *pop()
{
    return nstack? stack[--nstack]: NULL;
}

/* traverse graph from a node, return
 */
static int bfs_run(node_t *node)
{
    int i, res = 0;
    struct link *link;

    log_f(3, "node=%s npaths=%d\n", node->token, npaths);
    //log_f(3, "node=%p nodes=%p start=%p end=%p end1=%p\n",
    //      node, nodes, nodes+START, nodes+END, &nodes[END]);
    /* we found a path */
    if (node == nodes+END) {
        npaths++;
        log(3, "solution :");
        for (i = 0; i < nstack; ++i)
            log(3, " -> %s", stack[i]->token);
        log(3, "\n");
        return 1;
    }

    if (list_empty(&node->list))
        return 0;

    i = 0;
    list_for_each_entry(link, &node->list, list) {
        if (link->node->multiple || !link->node->passed) {
            link->node->passed++;
            push(link->node);
            res += bfs_run(link->node);
            pop();
            link->node->passed--;
        }
    }
    return res;
}

/* run 1 full step.
 * return number of flashed octopuses
 */

static int part1()
{
    nodes[START].passed = 1;
    push(nodes + START);
    return bfs_run(nodes);
    pop();
}

static int part2()
{
    return 2;
}

static int doit(int part)
{
    read_graph();
    return part == 1? part1(): part2();
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

    printf("%s : res=%d npaths=%d\n", *av, doit(part), npaths);
    exit (0);
}
