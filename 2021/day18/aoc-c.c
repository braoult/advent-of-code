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

#define POISON_VALUE -232323
#define POISON_DEPTH 121212;

#define PARENT 0
#define LEFT   1
#define RIGHT  2

static pool_t *pool_nodes;

#define MAX_LINES 128
static char *lines[MAX_LINES];
static int nlines;

typedef struct node {
    s64 val;
    struct node *tree[3];                         /* parent, left, right */
    struct list_head leaf_list;                   /* head is tree root */
    u32 depth;
} node_t;


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
static void node_print(node_t *node)
{
    if (!node->depth)
        log_f(1, "");
    if (!node->tree[LEFT]) {                      /* leaf */
        log(1, "%ld", node->val);
    } else {
        log(1, "[");
        node_print(node->tree[LEFT]);
        log(1, ",");
        node_print(node->tree[RIGHT]);
        log(1, "]");
    }
    if (!node->depth) {
        log(1, "\n");
        leaves_print(node);
    }
}

static inline node_t *node_get()
{
    node_t *node = pool_get(pool_nodes);

    node->depth = POISON_DEPTH;
    node->val = POISON_VALUE;
    node->tree[LEFT] = NULL;
    node->tree[RIGHT] = NULL;
    node->tree[PARENT] = NULL;
    INIT_LIST_HEAD(&node->leaf_list);
    return node;
}

/* split_node - split first node with value >= 10
 * return: 1: finished
 */
static int node_split(node_t *node)
{
    int depth = node->depth;
    static node_t *root;

    if (depth == 0)
        root = node;
    //if (node->depth == 0)
        log_f(2, "entering node %p: val=%d left=%p right=%p\n", node, node->val,
              node->tree[LEFT], node->tree[RIGHT]);

    if (node->tree[LEFT]) {
        if (node_split(node->tree[LEFT]) || node_split(node->tree[RIGHT]))
            return 1;
    } else {
        if (node->val >= 10) {   /* eligible leaf */
            //node_t *leaf_left = list_prev_entry(node, leaf_list);
            node_t *left, *right;

            log_i(2, "splitting [%ld,%ld]\n", node->val);

            /* create and populate new nodes */
            left = node_get();
            left->depth = node->depth + 1;
            left->val = node->val / 2;

            right = node_get();
            right->depth = node->depth + 1;
            right->val = (node->val + 1) / 2;

            node->tree[LEFT] = left;
            node->tree[RIGHT] = right;
            node->val = POISON_VALUE;

            /* add new nodes in leaves list, remove current one */
            list_add(&node->tree[RIGHT]->leaf_list, &node->leaf_list);
            list_add(&node->tree[LEFT]->leaf_list, &node->leaf_list);
            list_del(&node->leaf_list);
            log_i(2, "after split: \n\t");
            node_print(root);
            return 1;
        }
    }
    if (node->depth == 0) {
        log_f(2, "return:\n\t");
        node_print(node);
    }
    return 0;
}

/* explode node - explode first node with level == 4
 * return: 1: finished
 */
static int node_explode(node_t *node)
{
    int depth = node->depth;
    static node_t *root;

    if (depth == 0) {
        root = node;

        log_f(2, "entering node %p: val=%d left=%p right=%p\n", node, node->val,
              node->tree[LEFT], node->tree[RIGHT]);
    }
    if (depth == 4) {
        node_t *left = node->tree[LEFT], *right = node->tree[RIGHT];
        node_t *tmp;
        //struct list_head *tmp;

        /* skip leaves */
        if (!left)
            return 0;

        log_i(2, "exploding [%ld,%ld]\n", left->val, right->val);
        //log_i(2, "zobu: ");
        //node_print(root);

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

        //log_i(2, "zobi: ");
        //node_print(root);
        /* remove leaves from list, and add current one */
        //tmp = list_entry(left, node_t, leaf_list);
        node->val = 0;

        list_add(&node->leaf_list, &left->leaf_list);
        //log_i(2, "zoba: n=%p l=%p) ", &node->leaf_list, &left->leaf_list);
        //node_print(root);

        //list_del(&left->leaf_list);
        //list_del(left->leaf_list.prev);

        //list_del(&node->leaf_list->prev);
        //tmp = list_prev_entry(node, leaf_list);
        //log_i(2, "zoby: n=%p l=%p) ", &node->leaf_list, &left->leaf_list);
        list_del(&left->leaf_list);
        //node->leaf_list.prev;
        //log_i(2, "zobe: ");
        //node_print(root);
        pool_add(pool_nodes, left);

        list_del(&right->leaf_list);
        //log_i(2, "zobo: ");
        //node_print(root);
        pool_add(pool_nodes, right);

        /* remove childs links */
        node->tree[LEFT] = NULL;
        node->tree[RIGHT] = NULL;

        //log_i(2, "after reduce: ");
        //node_print(root);
        return 1;
    } else {
        if (node->tree[LEFT]) {
            if (node_explode(node->tree[LEFT]) || node_explode(node->tree[RIGHT]))
                return 1;
        }
    }
    return 0;
}

static node_t *node_reduce(node_t *node)
{
    //log_f(2, "");
    //node_explode(node);
    //node_explode(node);
    //return node;
    while (1) {
        log_f(2, "\t");
        node_print(node);
        if (node_explode(node))
            continue;
        if (node_split(node))
            continue;
        break;
    }
    return node;
}

/* promote/demode node
 */
static node_t *node_promote(node_t *node, int promote)
{
    node->depth += promote;
    if (node->tree[LEFT]) {
        node_promote(node->tree[LEFT], promote);
        node_promote(node->tree[RIGHT], promote);
    }
    return node;
}

/* add 2 nodes
 */
static node_t *node_add(node_t *n1, node_t *n2)
{
    node_t *head = pool_get(pool_nodes);

    log_f(2, "\n");
    node_print(n1);
    node_print(n2);
    head->depth = 0;
    INIT_LIST_HEAD(&head->leaf_list);

    /* create new tree from the two
     */
    head->tree[LEFT] = node_promote(n1, 1);
    head->tree[RIGHT] = node_promote(n2, 1);
    n1->tree[PARENT] = head;
    n2->tree[PARENT] = head;

    /* link leaves
     */
    //head->leaf_list = n1->leaf_list;
    list_splice_tail(&n1->leaf_list, &head->leaf_list);
    list_splice_tail(&n2->leaf_list, &head->leaf_list);
    //node_print(head);
    return head;
}

/* read node, from '[' to corresponding ']'
 */
static node_t *node_read(char **p, int depth)//, node_t *parent)
{
    static node_t *root;
    node_t *node = node_get();

    node->depth = depth;

    if (!depth)                                   /* root node */
        root = node;

    //log_f(2, "str=%s depth=%d\n", *p, depth);
    switch (**p) {
        case '[':
            (*p)++;
            node->tree[LEFT] = node_read(p, depth+1);
            node->tree[LEFT]->tree[PARENT] = node;
            //log_i(2, "comma = %c\n", **p);
            (*p)++;
            node->tree[RIGHT] = node_read(p, depth+1);
            node->tree[RIGHT]->tree[PARENT] = node;
            //log_i(2, "closing = %c\n", **p);
            break;
        default:                                  /* number */
            //log_i(2, "number = %c\n", **p);
            node->val = **p - '0';
            list_add_tail(&node->leaf_list, &root->leaf_list);
    }
    (*p)++;
    return node;
}

/* read line, create a tree
 */
static node_t *read_line()
{
    node_t *head = NULL;
    size_t alloc = 0;
    char *buf, *p;
    ssize_t buflen;

    if ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        p = buf;
        buf[--buflen] = 0;
        head = node_read(&p, 0);
    }
    free(buf);
    //node_print(head);
    return head;
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
    return nlines;
}

/* free lines memory
 */
static void free_lines()
{
    for (int i = 0; i < nlines; ++i)
        free(lines[i]);
}

static s64 node_magnitude(node_t *node)
{
    if (!node->tree[LEFT])
        return node->val;
    return 3 * node_magnitude(node->tree[LEFT]) +
        2 * node_magnitude(node->tree[RIGHT]);
}

static s64 part1()
{
    node_t *head = NULL, *next = NULL;
    s64 res = 1;

    head = node_read(lines + 0, 0);
    for (int i = 1; i < nlines; ++i) {
        next = node_read(lines + i, 0);
        head = node_reduce(node_add(head, next));
    }
    node_print(head);
    return node_magnitude(head);
}

static s64 part2()
{
    s64 res = 2;
    return res;
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
    exit(0);
    node_t *n1, *n2, *n3, *n4;
    n1 = read_line();
    n2 = read_line();
    //node_print(n1);
    //node_print(n2);
    n3 = node_add(n1, n2);
    //node_print(n3);
    n4 = node_reduce(n3);
    node_print(n4);
    free_lines();
    exit(0);
    //node_print(n1);
    //node_print(n2);
    n3 = node_add(n1, n2);
    node_print(n3);
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
    exit (0);
}
