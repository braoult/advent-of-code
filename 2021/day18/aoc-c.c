/* aoc-c.c: Advent of Code 2021, day 18 parts 1 & 2
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

#include "pool.h"
#include "debug.h"
#include "list.h"

/* Data structure : A snailfish number (SN) is represented as a binary tree and a
 * (doubly) linked list LL for leaves (= decimal numbers).
 * The latter will allow to easily find previous/next number.
 *
 * Example, [1,[4,2]] is represented as :
 *
 *         +-------> SN root <---------------+        Depth: 0
 *         |        /       \                |
 *         |LL     /         \               |
 *         |      /           \              |LL
 *         +---> 1 <--+       SN node        |        Depth: 1
 *                    |       /     \        |
 *                    |LL    /       \       |
 *                    |     /         \      |
 *                    +--> 4 <-------> 2 <---+        Depth: 2
 *                             LL
 */

static pool_t *pool_nodes;

#define MAX_LINES 128                             /* I know, I know... */
static char *lines[MAX_LINES];
static int nlines;

#define LEFT       0
#define RIGHT      1

#define IS_LEAF(node) (!(node)->left)

typedef struct node {
    int val;
    struct node *left, *right;
    struct list_head leaf_list;                   /* tree root is list head */
} node_t;

/* get a node from memory pool
 */
static inline node_t *node_get()
{
    node_t *node = pool_get(pool_nodes);

    node->left = NULL;
    node->right = NULL;
    return node;
}

/* recursive cleanup of node
 */
static inline void node_free(node_t *node)
{
    if (!IS_LEAF(node)) {
        node_free(node->left);
        node_free(node->right);
    }
    pool_add(pool_nodes, node);
}

/* split_node - split first node with value >= 10
 * return: 1: finished
 */
static int node_split(node_t *node)
{
    if (IS_LEAF(node)) {
        if (node->val >= 10) {                    /* eligible leaf */
            node_t *left, *right;

            /* create and populate new nodes */
            left = node_get();
            left->val = node->val / 2;

            right = node_get();
            right->val = (node->val + 1) / 2;

            node->left  = left;
            node->right = right;

            /* add new nodes in leaves list, remove current one */
            list_add(&node->right->leaf_list, &node->leaf_list);
            list_add(&node->left->leaf_list, &node->leaf_list);
            list_del(&node->leaf_list);
            return 1;
        }
    } else {
        return node_split(node->left) || node_split(node->right);
    }
    return 0;
}

/* explode node - explode first node with level == 4
 * return:
 *    0: nothing done
 *    1: an explode operation was done
 */
static int node_explode(node_t *node, int depth)
{
    static node_t *root;
    node_t *left, *right;

    if (!(left = node->left))
        return 0;
    right = node->right;

    /* Need to keep leaves list head */
    if (depth == 0)
        root = node;

    if (depth == 4) {
        /* increment left and right leaves values */
        if (!list_is_first(&left->leaf_list, &root->leaf_list))
            list_prev_entry(left, leaf_list)->val += left->val;
        if (!list_is_last(&right->leaf_list, &root->leaf_list))
            list_next_entry(right, leaf_list)->val += right->val;

        /* current node becomes a leaf */
        node->val = 0;
        node->left = NULL;
        node->right = NULL;
        list_add(&node->leaf_list, &left->leaf_list);

        /* remove children from leaves list, and put back in mem pool */
        list_del(&left->leaf_list);
        list_del(&right->leaf_list);
        pool_add(pool_nodes, left);
        pool_add(pool_nodes, right);

        return 1;
    } else {
        return node_explode(left, depth + 1) ||
            node_explode(right, depth + 1);
    }
}

/* node reduce
 */
static node_t *node_reduce(node_t *node)
{
    while (1) {
        if (! (node_explode(node, 0) || node_split(node)))
            return node;
    }
}

/* add 2 nodes
 */
static node_t *node_add(node_t *n1, node_t *n2)
{
    node_t *head = pool_get(pool_nodes);

    /* create new tree from the two nodes
     */
    INIT_LIST_HEAD(&head->leaf_list);
    head->left  = n1;
    head->right = n2;

    /* link leaves lists
     */
    list_splice(&n2->leaf_list, &head->leaf_list);
    list_splice(&n1->leaf_list, &head->leaf_list);
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
            (*p)++;                               /* skip left bracket */
            node->left  = _node_read(p, depth + 1);
            (*p)++;                               /* skip comma */
            node->right = _node_read(p, depth + 1);
            break;
        default:                                  /* number: add to tail list */
            node->val = **p - '0';
            list_add_tail(&node->leaf_list, &root->leaf_list);
    }
    (*p)++;
    return node;
}

/* wrapper for recursive function
 */
static inline node_t *node_read(char *p)
{
    char *tmp = p;
    return _node_read(&tmp, 0);
}

/* read input
 */
static void read_lines()
{
    size_t alloc = 0;
    ssize_t buflen;

    while ((buflen = getline(&lines[nlines], &alloc, stdin)) > 0) {
        lines[nlines][--buflen] = 0;
        nlines++;
    }
    free(lines[nlines]);                          /* EOF, need to be freed */
}

/* free lines memory
 */
static void free_lines()
{
    for (int i = 0; i < nlines; ++i)
        free(lines[i]);
}

static inline int node_magnitude(node_t *node)
{
    if (!node->left)
        return node->val;
    return 3 * node_magnitude(node->left) + 2 * node_magnitude(node->right);
}

static int part1()
{
    node_t *head, *next;
    int res;

    head = node_read(lines[0]);
    for (int i = 1; i < nlines; ++i) {
        next = node_read(lines[i]);
        head = node_reduce(node_add(head, next));
    }
    res = node_magnitude(head);
    node_free(head);
    return res;
}

static int part2()
{
    node_t *list;
    int max = 0, cur;

    /* calculate magnitude for any combination of two snailfish numbers */
    for (int i = 0; i < nlines; ++i) {
        for (int j = 0; j < nlines; ++j) {
            if (j != i) {
                list = node_reduce(node_add(node_read(lines[i]),
                                            node_read(lines[j])));
                if ((cur = node_magnitude(list)) > max)
                    max = cur;
                node_free(list);
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

    if (!(pool_nodes = pool_create("nodes", 1024, sizeof(node_t))))
        exit(1);
    read_lines();
    printf("%s : res=%d\n", *av, part == 1? part1(): part2());
    free_lines();
    pool_destroy(pool_nodes);
    exit(0);
}
