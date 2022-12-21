/* aoc-c.c: Advent of Code 2022, day 11
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

#include "br.h"
#include "debug.h"
#include "list.h"
#include "pool.h"

#include "aoc.h"

#define MAXMONKEYS 8

typedef struct monkey {
    char op;                                      /* '+' or '*' */
    long values[2];                               /* formula operands */
    uint visits;                                  /* total visits */
    int div;                                      /* divisor */
    struct list_head *dest[2];                    /* destination monkey */
    struct list_head items;                       /* monkey items */
} monkey_t;

typedef struct item {
    long item;
    struct list_head list;
} item_t;

static pool_t *pool_item;

/* TODO: the following 3 variables should not be global
 */
static monkey_t monkeys[MAXMONKEYS];
static int nmonkeys;
static u64 lcm = 1;

static char *getnth(char *buf, int n)
{
    char *ret;
    for (; n >= 0; n--) {
        ret = strtok(buf, " ,\n");
        buf = NULL;
    }
    return ret;
}

static char *getnext()
{
    return strtok(NULL, " ,\n");
}

static int parse()
{
    size_t alloc = 0;
    char *buf = NULL, *tok;
    monkey_t *m = monkeys;

    while (getline(&buf, &alloc, stdin) > 0) {
        INIT_LIST_HEAD(&m->items);

        getline(&buf, &alloc, stdin);             /* starting items */
        tok = getnth(buf, 2);
        while (tok) {
            item_t *item = pool_get(pool_item);
            item->item = atoi(tok);
            list_add_tail(&item->list, &m->items);
            tok = getnext();
        }

        getline(&buf, &alloc, stdin);             /* operation */
        tok = getnth(buf, 3);
        m->values[0] = (*tok == 'o') ? -1: atoi(tok); /* first operand */
        m->op = *getnext();                       /* operator */
        tok = getnext();
        m->values[1] = *tok == 'o' ? -1: atoi(tok); /* second operand */

        getline(&buf, &alloc, stdin);             /* divisible */
        m->div = atoi(getnth(buf, 3));
        lcm *= m->div;

        getline(&buf, &alloc, stdin);             /* true */
        m->dest[0] = &(monkeys + atoi(getnth(buf, 5)))->items;

        getline(&buf, &alloc, stdin);             /* false */
        m->dest[1] = &(monkeys + atoi(getnth(buf, 5)))->items;

        getline(&buf, &alloc, stdin);             /* skip empty line */

        nmonkeys++;
        m++;
    }
    free(buf);
    return 1;
}

static __always_inline void inspect(monkey_t *m, int divide)
{
    item_t *item, *tmp;
    long op1, op2;

    list_for_each_entry_safe(item, tmp, &m->items, list) {
        m->visits++;
        /* I wonder if we could not find some mathematical properties to
         * simplify the following three lines
         */
        op1 = m->values[0] < 0 ? item->item: m->values[0];
        op2 = m->values[1] < 0 ? item->item: m->values[1];
        item->item = (((m->op == '+')? op1 + op2: op1 * op2) / divide ) % lcm;
        list_move_tail(&item->list, m->dest[!!(item->item % m->div)]);
    }
}

static u64 doit(int rounds, int divide)
{
    u64 max1 = 0, max2 = 0;
    monkey_t *m = monkeys;

    for (int r = 0; r < rounds; ++r)
        for (int i = 0; i < nmonkeys; ++i)
            inspect(monkeys + i, divide);

    for (int i = 0; i < nmonkeys; ++i, m++) {
        if (m->visits > max1) {
            max2 = max1;
            max1 = m->visits;
        } else if (m->visits > max2) {
            max2 = m->visits;
        }
    }
    return max1 * max2;
}

static u64 part1()
{
    return doit(20, 3);
}

static u64 part2()
{
    return doit(10000, 1);
}


int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_item =  pool_create("item", 64, sizeof(item_t));

    parse();
    printf("%s: res=%lu\n", *av, part == 1? part1(): part2());
    pool_destroy(pool_item);
    exit(0);
}
