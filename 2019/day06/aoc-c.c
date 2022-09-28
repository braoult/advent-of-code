/* aoc-c.c: Advent of Code 2019, day 6 parts 1 & 2
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
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <getopt.h>
#include <stdlib.h>

#include "debug.h"
#include "pool.h"

/**
 * As the character set is [1-9A-Z], the trie arrays size will be 26 + 9 = 35,
 * organized as:
 *    char:        1  2 ... 8  9  A  B ...  Y  Z
 *    index:       0  1 ... 7  8  9 10 ... 33 34
 */
#define TRIESIZE    ('Z' - 'A' + 1 + '9' - '1' + 1)
#define c2index(c)  ((c) >= 'A'? (c) - 'A' + 9: (c) - '1')
#define index2c(c)  ((c) >= 9? (c) + 'A' - 9: (c) + '1')

/**
 * object_t - object representation
 * @parent:   a pointer to the object we orbit around
 * @sibling:  a list of objects orbiting around @parent
 * @child:    a list of object orbiting around this object
 * @name:     the object name
 */
typedef struct object {
    struct object *parent;
    struct list_head sibling, child;
    char name[8];
} object_t;

/**
 * trie_t - trie node
 * @child:     array of pointers to node_t children of current node
 * @str:       current string (for debug)
 * @is_object: 1: this is an object, 0 if not
 * @data:      object data
 */
typedef struct trie {
    struct trie *child[TRIESIZE];
    int is_object;
    object_t data;
} trie_t;

/**
 * parent_t - list of parents for an object
 * @object: address of object
 * @list:   next parent
 *
 * For example, if A orbits around B, which orbits around COM, list will be:
 * head -> COM -> B -> A
 */
typedef struct {
    object_t *object;
    struct list_head list;
} parent_t;

static pool_t *pool_tries;

static trie_t *trie_get(trie_t *parent, char *name, int pos)
{
    trie_t *trie;

    if ((trie = pool_get(pool_tries))) {
        for (int i = 0; i < TRIESIZE; ++i)
            trie->child[i] = NULL;
        trie->is_object = 0;
        if (parent)
            parent->child[c2index(name[pos])] = trie;
    }
    return trie;
}

static trie_t *trie_find(trie_t *root, char *name)
{
    int len = strlen(name);

    for (int i = 0; i < len; ++i) {
        int ind = c2index(name[i]);
        root = root->child[ind] ? root->child[ind]: trie_get(root, name, i);
    }
    return root;
}

static object_t *object_find(trie_t *root, char *name)
{
    trie_t *trie = trie_find(root, name);
    if (!trie->is_object) {
        trie->data.parent = NULL;
        trie->is_object = 1;
        strcpy(trie->data.name, name);
        INIT_LIST_HEAD(&trie->data.child);
        INIT_LIST_HEAD(&trie->data.sibling);
    }
    return &trie->data;
}

static int part1(object_t *object, int depth)
{
    int ret = depth;
    object_t *cur;

    if (!list_empty(&object->child))
        list_for_each_entry(cur, &object->child, sibling)
            ret += part1(cur, depth + 1);
    return ret;
}

static int part2(trie_t *root, char *name1, char *name2)
{
    object_t *obj;
    parent_t *parent, *parent1, *parent2;
    LIST_HEAD(list1);
    LIST_HEAD(list2);
    pool_t *pool_parents = pool_create("parents", 1024, sizeof(parent_t));
    int count = 0;

    /* build a list of nodes from root to the two objects
     */
    obj = object_find(root, name1);
    while (obj->parent) {
        count++;
        obj = obj->parent;
        parent = pool_get(pool_parents);
        parent->object = obj;
        list_add(&parent->list, &list1);
    }
    obj = object_find(root, name2);
    while (obj->parent) {
        count++;
        obj = obj->parent;
        parent = pool_get(pool_parents);
        parent->object = obj;
        list_add(&parent->list, &list2);
    }
    /* skip common parents in the two lists
     */
    parent1 = list_first_entry_or_null(&list1, parent_t, list);
    parent2 = list_first_entry_or_null(&list2, parent_t, list);
    while (parent1->object == parent2->object) {
        count -= 2;
        parent1 = list_next_entry(parent1, list);
        parent2 = list_next_entry(parent2, list);
    }
    pool_destroy(pool_parents);
    return count;
}

static void parse(trie_t *root)
{
    char str1[8], str2[8];

    while (scanf(" %7[^)])%s", str1, str2) == 2) {
        object_t *star = object_find(root, str1);
        object_t *planet = object_find(root, str2);
        /* link planet to star, add planet to star's planets list
         */
        planet->parent = star;
        list_add(&planet->sibling, &star->child);
    }
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
            default:
                    return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    pool_tries = pool_create("tries", 1024, sizeof(trie_t));
    trie_t *root = trie_get(NULL, NULL, 0);
    parse(root);
    printf("%s : res=%d\n", *av,
           part == 1 ? part1(object_find(root, "COM"), 0) :
           part2(root, "YOU", "SAN"));
    pool_destroy(pool_tries);
    exit (0);
}
