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
#include <unistd.h>

#include "br.h"
#include "debug.h"
#include "list.h"
#include "pool.h"

#include "aoc.h"

typedef enum {
    NIL,
    LIST,
    //SUBLIST,
    INT
} type_t;

char *types[] = {
    "NIL",
    "LIST",
    //"SUBLIST",
    "INT"
};

struct node;
typedef struct cons {
    type_t type;
    union {
        //struct list_head node;                    /* same level node */
        struct node *sub;
        int value;
    };
} cons_t;

#define CAR 0                                     /* always NUM or SUBLIST */
#define CDR 1                                     /* always LIST */

typedef struct node {
    type_t car_t;
    union {
        struct list_head sub;
        int value;
    } car;
    struct list_head cdr;
    //cons_t cons[2];
} node_t;

typedef struct line {
    int pos;                                      /* position to parse */
    char *s;
} line_t;

pool_t *pool_nodes;

static node_t *getnode()
{
    node_t *node = pool_get(pool_nodes);
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
    //} else {
    //printf("nil ");
    //}
    printf(") ");
    if (!level)
        printf("\n");
}

static int compare_tree(struct list_head *h1, struct list_head *h2)
{
    struct list_head *cur1, *cur2;
    //struct list_head *h1, *h2;
    node_t *tmp, *n1, *n2;
    int res;
    log_f(3, "h1=%p h2=%p\n", h1, h2);

    /* check for NIL */
    /*
     * if (n1->car_t == NIL) {
     *     log(3, "n1 is NIL\n");
     *     return n2->car_t == NIL? 0: 1;
     * }
     * if (n2->car_t == NIL) {
     *     log(3, "n2 is NIL\n");
     *     return -1;
     * }
     */

    //h1 = &n1->cdr;
    //h2 = &n2->cdr;
    /* get lists first entries */
    cur1 = h1->next;
    cur2 = h2->next;

    //if (!list_empty(h1) && !list_empty(h2)) {
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
            tmp = getnode();
            //INIT_LIST_HEAD(&tmp->car.sub);
            tmp->car_t = INT;
            if (n1->car_t == INT) {
                log(3, "car1 == INT, adding a node\n");
                tmp->car.value = n1->car.value;
                n1->car_t = LIST;
                INIT_LIST_HEAD(&n1->car.sub);
                list_add(&tmp->cdr, &n1->car.sub);
                //n1->car.sub = tmp;
                //print_tree()
            } else {
                log(3, "car2 == INT, adding a node\n");
                tmp->car.value = n2->car.value;
                n2->car_t = LIST;
                INIT_LIST_HEAD(&n2->car.sub);
                list_add(&tmp->cdr, &n2->car.sub);
                //n2->car.sub = tmp;
            }
            res = compare_tree(&n1->car.sub, &n2->car.sub);
            if (res)
                return res;
            //continue;
        }
        //next:
        cur1 = cur1->next;
        cur2 = cur2->next;
    }
//} else {
    //      log(3, "some list empty\n");
//}
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

static struct list_head *create_tree(char *s, int *consumed, int level, struct list_head *head)
{
    node_t *node = NULL;
    LIST_HEAD(sub);
    int num = 0, val, depth = 0, subconsumed;
    //LIST_HEAD(head);
    *consumed = 1;
    if (*s != '[') {
        printf("error 0\n");
        exit(0);
    }
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
                        //if (num == 0) {
                        //    head->car_t = SUBLIST;
                        //    head->car.sub = sub;
                        //} else {
                        //list_add(&sub, &node->car.sub); /* add sublist to new node */
                        list_add_tail(&node->cdr, head); /* add node to current level */
                        puts("ZOBI");
                        print_tree(&node->car.sub, 0);
                        puts("ZOBa");
                        //sleep(1);
                        print_tree(head, 0);
                        //}
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
                //if (num == 0) {
                //    head->car_t = INT;
                //    head->car.value = val;
                //} else {
                node = getnode();
                node->car_t = INT;
                node->car.value = val;
                list_add_tail(&node->cdr, head);
                //}
                //print_tree(head, 0);
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
    struct list_head head[2];
    int consumed;
    int group = 1, res = 0, tmp;
    //line_t line;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (!buflen) {
            //i = 0;
            //printf("++++ node 1 = %p\n", node[1]);
            printf("++++ node 0 = %p\n", &head[0]);
            print_tree(&head[0], 0);
            printf("++++ node 1 = %p\n", &head[1]);
            print_tree(&head[1], 0);
            tmp = compare_tree(&head[0], &head[1]);
            printf("group = %d compare_tree() = %d\n", group, tmp);
            if (tmp == 1) {
                res += group;
                printf("group = %d: right order res:%d -> %d\n", group, res - group, res);
            }
            group++;
            printf("\n\n");
            //pool_add(pool_nodes, node[0]);
            //pool_add(pool_nodes, node[1]);
            //node[0] = node[1] = NULL;
            //exit(0);
        } else {
            //node[i % 2] = getnode();
            create_tree(buf, &consumed, 0, &head[i % 2]);
            printf("setting node %d\n\n", i%2);
            i++;
        }
    }
    printf("++++ node 0 = %p\n", &head[0]);
    print_tree(&head[0], 0);
    printf("++++ node 1 = %p\n", &head[1]);
    print_tree(&head[1], 0);
    tmp = compare_tree(&head[0], &head[1]);
    printf("group = %d compare_tree() = %d\n", group, tmp);
    if (tmp == 1) {
        res += group;
        printf("group = %d: right order res:%d -> %d\n", group, res - group, res);
    }
    printf("\n\n");
    return 1;
}

static u64 doit()
{
    return 1;
}

static u64 part1()
{
    return doit();
}

static u64 part2()
{
    return doit();
}


int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_nodes =  pool_create("nodes", 512, sizeof(node_t));
    //u32 foo=0x12345678;
    parse();
    printf("%s: res=%lu\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_nodes);
    exit(0);
}
