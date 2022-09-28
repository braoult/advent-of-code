/* aoc-c.c: Advent of Code 2019, day 4 parts 1 & 2
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
 * @data:      pointer to an object
 */
typedef struct trie {
    struct trie *child[TRIESIZE];
    //char str[8];
    int is_object;
    object_t data;
} trie_t;

/**
 * parent_t - list of parents for an object
 * @object - address of parent object
 * @list - next parent
 *
 * For example, if A orbits around B, which orbits around COM, list will be:
 * head -> COM -> B -> A
 */
typedef struct {
    object_t *object;
    struct list_head list;
} parent_t;

static pool_t *pool_tries, *pool_parents;

static trie_t *trie_get(trie_t *parent, char *name, int pos)
{
    trie_t *trie;
    static int count = 0;
    count++;

    log_f(3, "parent=%p name=%s pos=%d\n", parent, name, pos);
    if ((trie = pool_get(pool_tries))) {
        for (int i = 0; i < TRIESIZE; ++i)
            trie->child[i] = NULL;
        //*trie->str = 0;
        trie->is_object = 0;
        if (parent) {
            int index = c2index(name[pos]);
            parent->child[index] = trie;
            log(3, "setting parent %p[%c] to %p\n", parent, name[pos], trie);
            //strncpy(trie->str, name, pos + 1);
        }
    }
    log(3, "\tnew trie=%p total = %d\n", trie, count);
    return trie;
}

static void trie_print(trie_t *trie, int depth)
{
    if (depth == 0)
        log_f(3, "root=%p depth=%d\n", trie, depth);
    if (trie->is_object) {
        printf("%*sOBJECT %s parent=%s\n", depth * 4, "", trie->data.name,
               trie->data.parent? trie->data.parent->name: "NIL");
    }
    for (int i = 0; i < TRIESIZE; ++i) {
        if (trie->child[i]) {
            printf("%*s+%c\n", depth * 4, "", index2c(i));
            trie_print(trie->child[i], depth + 1);
        }
    }
}

static void tree_print(object_t *object, int depth)
{
    if (depth == 0)
        log_f(3, "root=%p depth=%d\n", object, depth);
    printf("%*sOBJECT %s\n", depth * 4, "", object->name);
    if (!list_empty(&object->child)) {
        object_t *cur;
        list_for_each_entry(cur, &object->child, sibling) {
            tree_print(cur, depth + 1);
        }
    }
}

static int orbit_count(object_t *object, int depth)
{
    int ret = depth;
    if (!list_empty(&object->child)) {
        object_t *cur;
        list_for_each_entry(cur, &object->child, sibling) {
            ret += orbit_count(cur, depth + 1);
        }
    }
    return ret;
}

static trie_t *trie_find(trie_t *root, char *name)
{
    int len = strlen(name);
    trie_t *cur = root;

    for (int i = 0; i < len; ++i) {
        int ind = c2index(name[i]);
        cur = cur->child[ind] ?
            cur->child[ind]:
            trie_get(cur, name, i);
    }
    if (!cur->is_object) {
        cur->data.parent = NULL;
        cur->is_object = 1;
        strcpy(cur->data.name, name);
        INIT_LIST_HEAD(&cur->data.child);
        INIT_LIST_HEAD(&cur->data.sibling);
    }
    return cur;
}

static object_t *object_find(trie_t *root, char *object)
{
    object_t *ret = &trie_find(root, object)->data;
    log_f(3, "object = %p - %s\n", ret, ret->name);
    return ret;
}

static int count_path(trie_t *root, char *name1, char *name2)
{
    object_t *obj1 = object_find(root, name1);
    object_t *obj2 = object_find(root, name2);
    parent_t *parent, *parent1, *parent2;
    LIST_HEAD(list1);
    LIST_HEAD(list2);
    int count1 = 0, count2 = 0;
    while (obj1->parent) {
        count1++;
        obj1 = obj1->parent;
        parent = pool_get(pool_parents);
        parent->object = obj1;
        list_add(&parent->list, &list1);
    }
    while (obj2->parent) {
        count2++;
        obj2 = obj2->parent;
        parent = pool_get(pool_parents);
        parent->object = obj2;
        list_add(&parent->list, &list2);
    }
    parent1 = list_first_entry_or_null(&list1, parent_t, list);
    parent2 = list_first_entry_or_null(&list2, parent_t, list);
    while (parent1->object == parent2->object) {
        count1--;
        count2--;
        parent = parent1;
        parent1 = list_next_entry(parent1, list);
        //list_del(parent1->list.prev);
        parent2 = list_next_entry(parent2, list);
        //list_del(parent2->list.prev);
    }
    printf ("common ancestor = %s\n", parent->object->name);
    printf("count1 = %d count2 = %d\n", count1, count2);
    return count1 + count2;
}

static void object_link(trie_t *root, char *star, char *planet)
{
    object_t *st, *pl;
    log_f(3, "linking planet %s to star %s\n", planet, star);
    st = object_find(root, star);
    pl = object_find(root, planet);
    pl->parent = st;
    list_add(&pl->sibling, &st->child);
}

static void parse(trie_t *root)
{
    char *star, *planet;

    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        star = strtok(buf, ")\n");
        planet = strtok(NULL, ")\n");
        printf ("read [%s][%s]\n", star, planet);
        object_link(root, star, planet);
        //hash = hash_string(NULL, star, strlen(star));
        //log(3, "hash(%s) = %u\n", star, hash);

    }
    free(buf);
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
    pool_parents = pool_create("parents", 1024, sizeof(parent_t));

    trie_t *root = trie_get(NULL, NULL, 0);
    /*
    printf("size = %d\n", TRIESIZE);
    printf("0 = %d\n", c2index('0'));
    printf("  0 = %c\n", index2c(0));
    printf("1 = %d\n", c2index('1'));
    printf("  1 = %c\n", index2c(1));
    printf("9 = %d\n", c2index('9'));
    printf("  8 = %c\n", index2c(8));
    printf("A = %d\n", c2index('A'));
    printf("  A = %c\n", index2c(9));
    printf("Z = %d\n", c2index('Z'));
    printf("  Z = %c\n", index2c(34));
    */
    //trie_find(root, "COM");
    //trie_find(root, "CAM");
    //trie_print(root, 0);
    //exit(0);
    parse(root);
    object_t *root_obj = object_find(root, "COM");
    trie_print(root, 0);
    tree_print(root_obj, 0);
    printf("%s : res=%d\n", *av,
           part == 1 ? orbit_count(object_find(root, "COM"), 0) :
           count_path(root, "YOU", "SAN"));
    //ancestor_find(root, "YOU", "SAN");
    pool_destroy(pool_tries);
    exit (0);
}
