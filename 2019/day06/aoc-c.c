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
 *
 * Example: if N1 and N2 orbit around O and S orbits around N1, we will
 * have :
 *                             +---------+
 *                       +---->|    0    |<---------+
 *                       |     |---------|          |
 *                       |     | parent  |--->NIL   |
 *                       |     |---------|          |
 *   +-------------------+---->| child   |<---------+-----------------+
 *   |                   |     |---------|          |                 |
 *   |                   |     | sibling |          |                 |
 *   |                   |     +---------+          |                 |
 *   |                   |                          |                 |
 *   |    +---------+    |                          |  +---------+    |
 *   |    |   N1    |<---+-----+                    |  |   N2    |    |
 *   |    |---------|    |     |                    |  |---------|    |
 *   |    | parent  |----+     |                    +--| parent  |    |
 *   |    |---------|          |                       |---------|    |
 *   | +->| child   |<---------+----+           NIL<---| child   |    |
 *   | |  |---------|          |    |                  |---------|    |
 *   +-+->| sibling |<---------+----+----------------->| sibling |<---+
 *     |  +---------+          |    |                  +---------+
 *     |                       |    |
 *     |  +---------+          |    |
 *     |  |    S    |          |    |
 *     |  |---------|          |    |
 *     |  | parent  |----------+    |
 *     |  |---------|               |
 *     |  | child   |--->NIL        |
 *     |  |---------|               |
 *     +->| sibling |<--------------+
 *        +---------+
 *
 */
typedef struct object {
    struct object *parent;
    struct list_head sibling, child;
    char name[8];
} object_t;

/**
 * trie_t - trie node
 * @child:     array of pointers to trie_t children of current node
 * @object:    pointer to object data (NULL if node only)
 *
 * For example, if objects N1, N2, and S exist, the structure will be:
 *
 *      Root trie
 *      +--------+-------------------------------------+
 *      | object | 0 | 1 | ... | N | ... | S | ... | Z |
 *      +--------+---------------+---------------------+
 *          |      |             |         |         |
 *          v      v             |         |         v
 *         NIL    NIL            |         |        NIL
 *  +----------------------------+   +-----+
 *  |   "N" trie                     |  "S" trie
 *  |   +--------+-------------+     |  +--------------------------+
 *  +-->| object | 0 | ... | Z |     +->| object | 0 | 1 | 2 | ... |
 *      +--------+-------------+        +--------------------------+
 *          |      |         |              |      |   |   |
 *          |      v         v              v      v   |   |
 *          |     NIL       NIL            NIL    NIL  |   |
 *          |        +---------------------------------+   |
 *          |        |                         +-----------+
 *          |        |   "S1" trie             |   "S2" trie
 *          |        |   +------------------+  |   +------------------+
 *          |        +-->| object | 0 | ... |  +-->| object | 0 | ... |
 *          |            +------------------+      +------------------+
 *          |                |      |                  |      |
 *          |                |      v                  |      v
 *          v                v     NIL                 v     NIL
 *     +-----------+   +-----------+             +-----------+
 *     | Object N  |   | Object S1 |             | Object S2 |
 *     +-----------+   +-----------+             +-----------+
 */
typedef struct trie {
    struct trie *child[TRIESIZE];
    object_t *object;
} trie_t;

static pool_t *pool_tries, *pool_objects;

static trie_t *trie_get(trie_t *parent, char *name, int pos)
{
    trie_t *trie;

    if ((trie = pool_get(pool_tries))) {
        for (int i = 0; i < TRIESIZE; ++i)
            trie->child[i] = NULL;
        trie->object = NULL;
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
    if (!trie->object) {
        trie->object = pool_get(pool_objects);
        trie->object->parent = NULL;
        strcpy(trie->object->name, name);
        INIT_LIST_HEAD(&trie->object->child);
        INIT_LIST_HEAD(&trie->object->sibling);
    }
    return trie->object;
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

static int get_depth(object_t *obj)
{
    int res = 0;
    for (; obj; obj = obj->parent)
        res++;
    return res;
}

static int part2(trie_t *root, char *name1, char *name2)
{
    object_t *obj1, *obj2;
    int count = 0, depth1, depth2;

    /* build a list of nodes from root to the two objects
     */
    obj1 = object_find(root, name1);
    depth1 = get_depth(obj1);
    obj2 = object_find(root, name2);
    depth2 = get_depth(obj2);
    while (obj1 != obj2) {
        if (depth1 > depth2) {
            obj1 = obj1->parent;
            depth1--;
            count++;
        } else if (depth2 > depth1) {
            obj2 = obj2->parent;
            depth2--;
            count++;
        } else {
            obj1 = obj1->parent;
            depth1--;
            obj2 = obj2->parent;
            depth2--;
            count +=2;
        }
    }
    return count - 2;                             /* coz' we want objects parents */
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
    pool_objects = pool_create("objects", 1024, sizeof(object_t));
    trie_t *root = trie_get(NULL, NULL, 0);
    parse(root);
    printf("%s : res=%d\n", *av,
           part == 1 ? part1(object_find(root, "COM"), 0) :
           part2(root, "YOU", "SAN"));
    pool_destroy(pool_tries);
    pool_destroy(pool_objects);
    exit (0);
}
