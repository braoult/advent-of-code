/* aoc-c.c: Advent of Code 2021, day 18 parts 1 & 2
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

#include "pool.h"
#include "debug.h"
#include "bits.h"
#include "list.h"

#define LEFT   0
#define RIGHT  1

static pool_t *pool_nodes;

#define MAX_LINES 128
static char *lines[MAX_LINES];
static int nlines;

typedef struct node {
    s64 val;
    struct node *tree[2];                         /* left & right */
    struct list_head leaf_list;                   /* head is tree root */
} node_t;

#ifdef DEBUG
/* print leaves list
 */
static void leaves_print(node_t *root)
{
    node_t *list_cur;
    log_i(2, "leaves:");
    list_for_each_entry(list_cur, &root->leaf_list, leaf_list) {
        log_i(2, " %d", list_cur->val);
    }
    log_i(2, "\n");
}

/* print node tree
 */
static void node_print(node_t *node, int depth)
{
    if (!depth)
        log_f(1, "");
    if (!node->tree[LEFT]) {                      /* leaf */
        log(1, "%ld", node->val);
    } else {
        log(1, "[");
        node_print(node->tree[LEFT], depth + 1);
        log(1, ",");
        node_print(node->tree[RIGHT], depth + 1);
        log(1, "]");
    }
    if (!depth) {
        log(1, "\n");
        leaves_print(node);
    }
}
#endif

static inline node_t *node_get()
{
    node_t *node = pool_get(pool_nodes);

    node->tree[LEFT] = NULL;
    node->tree[RIGHT] = NULL;
    return node;
}

static inline void node_free(node_t *node)
{
    if (node->tree[LEFT]) {
        node_free(node->tree[LEFT]);
        node_free(node->tree[RIGHT]);
    }
    pool_add(pool_nodes, node);
}

/* split_node - split first node with value >= 10
 * return: 1: finished
 */
static int node_split(node_t *node)
{
    if (node->tree[LEFT]) {
        if (node_split(node->tree[LEFT]) || node_split(node->tree[RIGHT]))
            return 1;
    } else {
        if (node->val >= 10) {   /* eligible leaf */
            node_t *left, *right;

            /* create and populate new nodes */
            left = node_get();
            left->val = node->val / 2;

            right = node_get();
            right->val = (node->val + 1) / 2;

            node->tree[LEFT] = left;
            node->tree[RIGHT] = right;

            /* add new nodes in leaves list, remove current one */
            list_add(&node->tree[RIGHT]->leaf_list, &node->leaf_list);
            list_add(&node->tree[LEFT]->leaf_list, &node->leaf_list);
            list_del(&node->leaf_list);
            return 1;
        }
    }
    return 0;
}

/* explode node - explode first node with level == 4
 * return: 1: finished
 */
static int node_explode(node_t *node, int depth)
{
    static node_t *root;

    if (depth == 0)
        root = node;

    if (depth == 4) {
        node_t *left = node->tree[LEFT], *right = node->tree[RIGHT];

        /* skip leaves */
        if (!left)
            return 0;

        /* increment left leaf */
        if (!list_is_first(&left->leaf_list, &root->leaf_list)) {
            node_t *tmpnode = list_prev_entry(left, leaf_list);
            tmpnode->val += left->val;
        }
        /* increment right leaf */
        if (!list_is_last(&right->leaf_list, &root->leaf_list)) {
            node_t *tmpnode = list_next_entry(right, leaf_list);
            tmpnode->val += right->val;
        }

        node->val = 0;

        list_add(&node->leaf_list, &left->leaf_list);
        list_del(&left->leaf_list);
        pool_add(pool_nodes, left);
        list_del(&right->leaf_list);
        pool_add(pool_nodes, right);

        /* remove childs links */
        node->tree[LEFT] = NULL;
        node->tree[RIGHT] = NULL;

        return 1;
    } else {
        if (node->tree[LEFT]) {
            if (node_explode(node->tree[LEFT], depth + 1) ||
                node_explode(node->tree[RIGHT], depth + 1))
                return 1;
        }
    }
    return 0;
}

static node_t *node_reduce(node_t *node)
{
    while (1) {
        if (node_explode(node, 0))
            continue;
        if (node_split(node))
            continue;
        break;
    }
    return node;
}

/* add 2 nodes
 */
static node_t *node_add(node_t *n1, node_t *n2)
{
    node_t *head = pool_get(pool_nodes);

    INIT_LIST_HEAD(&head->leaf_list);

    /* create new tree from the two
     */
    head->tree[LEFT] = n1;
    head->tree[RIGHT] = n2;

    /* link corresponding leaves
     */
    list_splice_tail(&n1->leaf_list, &head->leaf_list);
    list_splice_tail(&n2->leaf_list, &head->leaf_list);
    return head;
}

/* read node, from '[' to corresponding ']'
 */
static inline node_t *_node_read(char **p, int depth)
{
    static node_t *root;
    node_t *node = node_get();

    if (!depth) {                                  /* root node */
        root = node;
        INIT_LIST_HEAD(&node->leaf_list);
    }

    switch (**p) {
        case '[':
            (*p)++;
            node->tree[LEFT] = _node_read(p, depth + 1);
            (*p)++;
            node->tree[RIGHT] = _node_read(p, depth + 1);
            break;
        default:                                  /* number */
            node->val = **p - '0';
            list_add_tail(&node->leaf_list, &root->leaf_list);
    }
    (*p)++;
    return node;
}

static inline node_t *node_read(char *p)
{
    char *tmp = p;
    return _node_read(&tmp, 0);
}

/* read input
 */
static int read_lines()
{
    size_t alloc = 0;
    ssize_t buflen;

    while ((buflen = getline(&lines[nlines], &alloc, stdin)) > 0) {
        lines[nlines][--buflen] = 0;
        nlines++;
    }
    free(lines[nlines]);                          /* EOF */
    return nlines;
}

/* free lines memory
 */
static void free_lines()
{
    for (int i = 0; i < nlines; ++i)
        free(lines[i]);
}

static inline s64 node_magnitude(node_t *node)
{
    if (!node->tree[LEFT])
        return node->val;
    return 3 * node_magnitude(node->tree[LEFT]) +
        2 * node_magnitude(node->tree[RIGHT]);
}

static s64 part1()
{
    node_t *head = NULL, *next = NULL;
    s64 res;

    head = node_read(lines[0]);
    for (int i = 1; i < nlines; ++i) {
        next = node_read(lines[i]);
        head = node_reduce(node_add(head, next));
    }
    res = node_magnitude(head);
    node_free(head);
    return res;
}

static s64 part2()
{
    node_t *list;
    s64 max = 0, cur;

    for (int i = 0; i < nlines; ++i) {
        for (int j = 0; j < nlines; ++j) {
            if (j == i)
                continue;
            list = node_reduce(node_add(node_read(lines[i]),
                                        node_read(lines[j])));
            cur = node_magnitude(list);
            if (cur > max)
                max = cur;
            node_free(list);
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

    if (!(pool_nodes = pool_create("nodes", 1024, sizeof(node_t))))
        exit(1);
    read_lines();
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
    free_lines();
    pool_destroy(pool_nodes);
    exit(0);
}
