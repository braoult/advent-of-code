/* aoc-c.c: Advent of Code 2021, day 12 parts 1 & 2
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

#define START (&nodes[0])                         /* start is always nodes[0] */
#define END   (&nodes[1])                         /* end is always nodes[1] */

typedef struct node {
    char token[16];                               /* node name */
    int passed;                                   /* number of passages */
    int multiple;                                 /* can pass multiple times ? */
    struct list_head list;                        /* links list */
} node_t;

typedef struct link {                             /* links between nodes */
    node_t *node;                                 /* linked node */
    struct list_head list;                        /* next/prev link */
} link_t;

static node_t nodes[MAX_NODES];                   /* nodes list */
static int nnodes;                                /* ... and size */

static pool_t *links_pool;                        /* links memory pool */

/* find node from token value
 * return @node or NULL
 */
static node_t *find_token(char *token)
{
    for (int i = 0; i < nnodes; ++i)
        if (!strcmp(nodes[i].token, token))
            return nodes+i;
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
        INIT_LIST_HEAD(&node->list);
        nnodes++;
    }
    return node;
}

/* create a link between two nodes
 * return 1 if OK, 0 if error
 */
static int link_nodes(node_t *n1, node_t *n2)
{
    struct link *link;

    /* link from n1 to n2 */
    if (n1 != END && n2 != START) {               /* cannot link from END or to START */
        if (!(link = pool_get(links_pool)))
            return 0;
        link->node = n2;
        list_add_tail(&link->list, &n1->list);
    }
    /* link from n2 to n1 */
    if (n1 != START && n2 != END) {               /* cannot link from END or to START */
        if (!(link = pool_get(links_pool)))
            return 0;
        link->node = n1;
        list_add_tail(&link->list, &n2->list);
    }
    return 1;
}

/* read data and create graph.
 */
inline static int create_graph()
{
    ssize_t len;
    size_t alloc = 0;
    char *buf;

    add_token_maybe("start");                     /* position 0 */
    add_token_maybe("end");                       /* position 1 */
    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        node_t *n1 = add_token_maybe(strtok(buf, "-\n"));
        node_t *n2 = add_token_maybe(strtok(NULL, "-\n"));
        link_nodes(n1, n2);
    }
    free(buf);

    return nnodes;
}

/* recursively traverse graph (DFS) from a node
 * return solutions found
 */
static int dfs_run(node_t *node, int part, int svisited)
{
    int res = 0;
    struct link *link;

    if (node == END)
        return 1;

    list_for_each_entry(link, &node->list, list) {
        if (link->node->multiple || !link->node->passed || (part == 2 &&
                                                            !svisited)) {
            int inc = 0;
            link->node->passed++;
            if (!link->node->multiple && link->node->passed == 2) {
                svisited = 1;
                inc = 1;
            }
            res += dfs_run(link->node, part, svisited);
            link->node->passed--;
            if (inc) {
                svisited = 0;
            }
        }
    }
    return res;
}

static int doit(int part)
{
    create_graph();
    return part == 1? dfs_run(nodes, 1, 0): dfs_run(nodes, 2, 0);
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

    if (!(links_pool = pool_create("links", 128, sizeof (struct link))))
        return -1;

    printf("%s : res=%d\n", *av, doit(part));
    exit (0);
}
