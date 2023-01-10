/* aoc-c.c: Advent of Code 2022, day 13
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
#include <string.h>
#include <ctype.h>

#include "br.h"
#include "debug.h"
#include "list.h"
#include "pool.h"

#include "aoc.h"

typedef enum {
    NIL,
    LIST,
    INT
} type_t;

#define CAR 0                                     /* always NUM or SUBLIST */
#define CDR 1                                     /* always LIST */

typedef struct node {
    type_t car_t;
    union {
        struct list_head sub;
        int value;
    } car;
    struct list_head cdr;
} node_t;

typedef struct nodes {
    struct list_head node;
    struct list_head list;
} nodes_t;
LIST_HEAD(nodes);

/* dummy nodes for integer vs list */
static struct list_head dummy_list;               /* tentative definition */
static node_t dummy = {
    .car_t = INT, .car.value = 0, .cdr = LIST_HEAD_INIT(dummy_list)
};
static struct list_head dummy_list = LIST_HEAD_INIT(dummy.cdr);

pool_t *pool_node, *pool_nodes;

static node_t *getnode()
{
    node_t *node = pool_get(pool_node);
    node->car_t = NIL;
    INIT_LIST_HEAD(&node->cdr);
    log_f(4, "\n");
    return node;
}

static void print_tree(struct list_head *head, int level)
{
    struct node *cur;
    log_f(4, "head=%p lev=%d\n", head, level);
    printf("( ");
    //if (!list_empty(&node->cdr)) {
    list_for_each_entry(cur, head, cdr) {
        if (cur->car_t == INT)
            printf("%d ", cur->car.value);
        else if (cur->car_t == NIL)
            printf("ERROR ");
        else
            print_tree(&cur->car.sub, level + 2);
    }
    printf(") ");
    if (!level)
        printf("\n");
}

static int compare_tree(struct list_head *h1, struct list_head *h2)
{
    struct list_head *cur1, *cur2;
    node_t *n1, *n2;
    int res;
    log_f(3, "h1=%p h2=%p\n", h1, h2);

    /* get lists first entries */
    cur1 = h1->next;
    cur2 = h2->next;

     while (cur1 != h1 && cur2 != h2) {
        n1 = container_of(cur1, node_t, cdr);
        n2 = container_of(cur2, node_t, cdr);

        if (n1->car_t == n2->car_t) {
            if (n1->car_t == INT) {
                if (n1->car.value < n2->car.value) {
                    log(3, "car1=%d < car2=%d, returning 1\n", n1->car.value,
                        n2->car.value);
                    return 1;
                } else if (n1->car.value > n2->car.value) {
                    log(3, "car1=%d > car2=%d, returning -1\n", n1->car.value,
                        n2->car.value);
                    return -1;
                }
                log(3, "car1 == car2 == %d, skipping\n", n1->car.value);
            } else {                              /* both sublists */
                res = compare_tree(&n1->car.sub, &n2->car.sub);
                if (res)
                    return res;
            }
        } else {                                  /* one number, one list */
            if (n1->car_t == INT) {
                log(3, "car1 == INT, using dummy\n");
                dummy.car.value = n1->car.value;
                res = compare_tree(&dummy_list, &n2->car.sub);
            } else {
                log(3, "car2 == INT, using dummy\n");
                dummy.car.value = n2->car.value;
                res = compare_tree(&n1->car.sub, &dummy_list);
            }
            if (res)
                return res;
        }
        cur1 = cur1->next;
        cur2 = cur2->next;
    }
    /* at least one list came to end */
     if (cur1 == h1 && cur2 == h2) {              /* both are ending */
        log(3, "Both sides Left side ran out of items, undecided\n");
        return 0;
     } else if (cur1 == h1) {                         /* first list did end */
        log(3, "Left side ran out of items, so inputs are in the right order\n");
        return 1;
    } else {
        log(3, "Right side ran out of items, so inputs are not in the right order\n");
        return -1;
    }
}

static void print_nodes()
{
    nodes_t *cur;
    log_f(3, "+++++++++++\n");
    list_for_each_entry(cur, &nodes, list) {
        print_tree(&cur->node, 0);
    }
}

static int add_node(nodes_t *h)
{
    nodes_t *first, *iter, *prev;
    struct list_head *node_next = &nodes;
    int num = 1;

    log_f(3, "adding ");
    print_tree(&h->node, 0);

    if (list_empty(&nodes)) {
        log_f(3, "adding first entry\n");
        goto ins_node;
    }
    first = iter = list_first_entry(&nodes, nodes_t, list);
    do {
        if (compare_tree(&h->node, &iter->node) > 0) {
            node_next = &iter->list;
            break;
        }
        prev = iter;
        iter = list_entry(iter->list.next, nodes_t, list);
        num++;
    } while (iter != first);
ins_node:
    log_f(3, "adding entry before ");
    if (node_next == &nodes)
        log(3, "head\n");
    else
        print_tree(&list_entry(node_next, nodes_t, list)->node, 0);
    //&n->node, 0);
    //            list_add_tail(&h->list, &n->list);
    log_f(3, "position = %d\n", num);
    list_add_tail(&h->list, node_next);
    return num;
}


static struct list_head *create_tree(char *s, int *consumed, int level, struct list_head *head)
{
    node_t *node = NULL;
    LIST_HEAD(sub);
    int num = 0, val, depth = 0, subconsumed;
    //LIST_HEAD(head);
    *consumed = 1;
    //if (*s != '[') {
    //    printf("error 0\n");
    //    exit(0);
    //}
    INIT_LIST_HEAD(head);
    //head = getnode();
    log_f(3, "create_tree(%s)\n", s);
    for (; *s; s++, (*consumed)++) {
        log(4, "L=%d examining %c num=%d depth=%d\n", level, *s, num, depth);
        val = 0;
        switch (*s) {
            case '[':
                switch (++depth) {
                    case 1:                       /* first level */
                        break;
                    case 2:                       /* second level (nested list) */
                        node = getnode();
                        node->car_t = LIST;
                        INIT_LIST_HEAD(&node->car.sub);
                        create_tree(s, &subconsumed, level + 1, &node->car.sub);
                        s += subconsumed - 1;
                        *consumed += subconsumed - 1;
                        log(4, "after create_tree: cons=%d s=%s\n", subconsumed, s);
                        list_add_tail(&node->cdr, head); /* add node to current level */
                        //print_tree(&node->car.sub, 0);
                        //print_tree(head, 0);
                        depth--;
                        num++;
                        break;
                    default:                      /* should not happen */
                        printf("error 1\n");
                        exit(0);
                        break;                    /* not reached */
                }
                //num++;
                break;
            case ',':
                break;
            case ']':
                if (!--depth)
                    goto end;
                break;
            default:                              /* number */
                sscanf(s, "%d%n", &val, &subconsumed);
                *consumed += subconsumed - 1;
                s += subconsumed - 1;
                log(4, "got integer=%d num=%d depth=%d chars=%d\n", val, num, depth, subconsumed);
                node = getnode();
                node->car_t = INT;
                node->car.value = val;
                list_add_tail(&node->cdr, head);
                num++;
                break;
        }
    }
end:
    log(4, "returning from create_tree, consumed=%d head=%p\n", *consumed, head);
    return head;
}

static int parse()
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    int i = 0;
    //struct list_head h[2];
    nodes_t *head[2];
    int dummy, group = 1, res = 0, tmp;

    while (1) {
        buflen = getline(&buf, &alloc, stdin);
        if (--buflen > 0) {
            head[ i % 2] = pool_get(pool_nodes);
            create_tree(buf, &dummy, 0, &head[i % 2]->node);
            add_node(head[i %2]);
            //create_tree(buf, &dummy, 0, &head[i % 2]);

            printf("setting node %d\n\n", i % 2);
        }
        if (buflen != 0) {
            if (i % 2) {
                printf("++++ node 0 = %p\n", &head[0]);
                print_tree(&head[0]->node, 0);
                printf("++++ node 1 = %p\n", &head[1]);
                print_tree(&head[1]->node, 0);
                tmp = compare_tree(&head[0]->node, &head[1]->node);
                printf("group = %d compare_tree() = %d\n", group, tmp);
                if (tmp == 1) {
                    res += group;
                    printf("group = %d: right order res:%d -> %d\n", group, res - group, res);
                    printf("\n\n");
                }
                group++;
            }
            i++;

        }
        if (buflen < 0)
            break;
    }
    return res;
}

static int part1()
{
    return parse();
}

static int part2()
{
    int dummy, res;
    struct nodes *h;
    parse();
    h = pool_get(pool_nodes);
    create_tree("[2]", &dummy, 0, &h->node);
    res = add_node(h);
    h = pool_get(pool_nodes);
    create_tree("[6]", &dummy, 0, &h->node);
    res *= add_node(h);
    return res;
}

/*
static struct list_head dummy_head;
static node_t dummy = {
    .car_t = INT, .car.value = 0, .cdr = LIST_HEAD_INIT(dummy_head)
};
static struct list_head dummy_head = LIST_HEAD_INIT(dummy.car.sub);
*/

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_node =  pool_create("node", 512, sizeof(node_t));
    pool_nodes =  pool_create("nodes", 512, sizeof(nodes_t));
    //u32 foo=0x12345678;
    printf("&dummy->cdr=%p next=%p prev=%p\n",
           &dummy.cdr, dummy.cdr.next, dummy.cdr.prev);
    printf("&dummy_head=%p next=%p prev=%p\n",
           &dummy_list, dummy_list.next, dummy_list.prev);
    //parse();
    //printf("&dummy->cdr=%p next=%p prev=%p\n",
    //       &dummy.cdr, dummy.cdr.next, dummy.cdr.prev);
    printf("%s: res=%d\n", *av, part == 1? part1(): part2());
    print_nodes();
    pool_destroy(pool_node);
    pool_destroy(pool_nodes);
    exit(0);
}
