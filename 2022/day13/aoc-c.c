/* aoc-c.c: Advent of Code 2022, day 13
 *
 * Copyright (C) 2022-2023 Bruno Raoult ("br")
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
#include "list.h"
#include "pool.h"

#include "aoc.h"

typedef enum { SUBLIST, INT } type_t;

typedef struct node {                             /* node */
    type_t car_t;
    union {                                       /* CAR */
        struct list_head sub;                     /* sublist */
        int value;                                /* value */
    };
    struct list_head cdr;                         /* CDR */
} node_t;

typedef struct {                                  /* packets ordered list */
    struct list_head node;                        /* packet head */
    struct list_head list;                        /* packets lists */
} packets_t;
LIST_HEAD(packets);

/* dummy node for integer vs list */
static struct list_head dummy_list;               /* tentative definition */
static node_t dummy = {
    .car_t = INT, .value = 0, .cdr = LIST_HEAD_INIT(dummy_list)
};
static struct list_head dummy_list = LIST_HEAD_INIT(dummy.cdr);

pool_t *pool_node, *pool_packets;

/* int getnode - allocate and initialize a new node
 * @type: The node type_t (INT/LIST)
 * @val:  The value if @type is INT
 *
 * Return: The new node.
 */
static node_t *getnode(type_t type, int val)
{
    node_t *node = pool_get(pool_node);
    node->car_t = type;
    INIT_LIST_HEAD(&node->cdr);
    if (type == INT)
        node->value = val;
    else
        INIT_LIST_HEAD(&node->sub);
    return node;
}

/* int compare tree - compare two packets trees
 * @h1: The first packet list head
 * @h2: The second packet list head
 *
 * Return: 1 if h1 and h2 are ordered, -1 if not ordered, 0 if undecided
 */
static int compare_tree(struct list_head *h1, struct list_head *h2)
{
    struct list_head *cur1, *cur2;
    node_t *n1, *n2;
    int res;

    /* get lists first entries */
    cur1 = h1->next;
    cur2 = h2->next;

    while (cur1 != h1 && cur2 != h2) {
        n1 = container_of(cur1, node_t, cdr);
        n2 = container_of(cur2, node_t, cdr);

        if (n1->car_t == n2->car_t) {
            if (n1->car_t == INT) {
                if (n1->value < n2->value) {
                    return 1;
                } else if (n1->value > n2->value) {
                    return -1;
                }
            } else {                              /* both sublists */
                if ((res = compare_tree(&n1->sub, &n2->sub)))
                    return res;
            }
        } else {                                  /* one number, one list */
            if (n1->car_t == INT) {
                dummy.value = n1->value;
                res = compare_tree(&dummy_list, &n2->sub);
            } else {
                dummy.value = n2->value;
                res = compare_tree(&n1->sub, &dummy_list);
            }
            if (res)
                return res;
        }
        cur1 = cur1->next;
        cur2 = cur2->next;
    }
    /* at least one list came to end */
    if (cur1 == h1 && cur2 == h2)                /* both are ending */
        return 0;
    else if (cur1 == h1)                         /* first list did end */
        return 1;
    else
        return -1;
}

/* int add_node - add a packet to the sorted packets list
 * @new: The new packets list head
 *
 * Return: The new packet position in list (first is 1)
 */
static int add_node(packets_t *new)
{
    packets_t *first, *iter;
    struct list_head *node_next = &packets;
    int num = 1;

    if (list_empty(&packets))
        goto ins_node;
    first = iter = list_first_entry(&packets, packets_t, list);
    do {
        if (compare_tree(&new->node, &iter->node) > 0) {
            node_next = &iter->list;
            break;
        }
        iter = list_entry(iter->list.next, packets_t, list);
        num++;
    } while (iter != first);
ins_node:
    list_add_tail(&new->list, node_next);
    return num;
}

static struct list_head *create_tree(char *s, int *consumed, struct list_head *head)
{
    node_t *node = NULL;
    LIST_HEAD(sub);
    int val, depth = 0, subconsumed;

    *consumed = 1;
    INIT_LIST_HEAD(head);
    for (; *s; s++, (*consumed)++) {
        switch (*s) {
            case '[':
                if (++depth == 2) {               /* we skip first depth level */
                    node = getnode(SUBLIST, 0);
                    list_add_tail(&node->cdr, head);
                    create_tree(s, &subconsumed, &node->sub);
                    s += subconsumed - 1;
                    *consumed += subconsumed - 1;
                    depth--;
                }
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
                node = getnode(INT, val);
                list_add_tail(&node->cdr, head);
                break;
        }
    }
end:
    return head;
}

static int parse()
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    packets_t *head[2];
    int i = 0, dummy, res = 0;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (buflen > 0) {                         /* non empty line */
            head[i % 2] = pool_get(pool_packets);
            create_tree(buf, &dummy, &head[i % 2]->node);
            add_node(head[i % 2]);
        }
        if (buflen != 0) {                        /* non empty line or EOF */
            if (i % 2 && compare_tree(&head[0]->node, &head[1]->node) == 1)
                res += (i >> 1) + 1;              /* (i / 2) + 1 */
            i++;
        }
    }
    return res;
}

static int part1()
{
    return parse();
}

static int part2()
{
    char *dividers[] = { "[[2]]", "[[6]]" };
    int dummy, res = 1;
    parse();
    for (ulong i = 0; i < ARRAY_SIZE(dividers); ++i) {
        packets_t *h = pool_get(pool_packets);
        create_tree(dividers[i], &dummy, &h->node);
        res *= add_node(h);
    }
    return res;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_node =  pool_create("node", 512, sizeof(node_t));
    pool_packets =  pool_create("packets", 512, sizeof(packets_t));

    printf("%s: res=%d\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_node);
    pool_destroy(pool_packets);
    exit(0);
}
