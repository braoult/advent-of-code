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

static int part1()
{
    return 1;
}

static int part2()
{
    return 2;
}

typedef struct monkey {
    char op;                                      /* '+' or '*' */
    long values[2];                               /* formula operands */
    int div;                                      /* divisor */
    struct list_head *dest[2];                    /* destination monkey */
    uint visits;                                  /* total visits */
    struct list_head items;                       /* monkey items */
} monkey_t;

typedef struct item {
    int item;
    struct list_head list;
} item_t;

static monkey_t monkeys[MAXMONKEYS];
static int nmonkeys;
static u64 lcm = 1, divide = 3;
static pool_t *pool_item;


static void print_monkeys()
{
    monkey_t *m = monkeys;
    item_t *item;

    for (int i = 0; i < nmonkeys; ++i, m++) {
        printf("Monkey %d visits:%u\n\titems: ", i, m->visits);
        list_for_each_entry(item, &m->items, list) {
            printf("%d ", item->item);
        }
        putchar('\n');
        printf("\ttrue=%ld false=%ld\n",
               container_of(&m->dest[0], monkey_t, dest[0]) - monkeys,
               container_of(&m->dest[0], monkey_t, dest[1]) - monkeys);

        //printf("\top: %ld %c %ld\n", m->values[0], m->op, m->values[1]);
        //printf("\tdiv = %d\n", m->div);
        //printf("\ttrue:%ld false:%ld\n", m->dest[1]-monkeys, m->dest[0]-monkeys);
    }
    //putchar('\n');
}

static u64 result()
{
    u64 max1 = 0, max2 = 0;
    monkey_t *m = monkeys;
    for (int i = 0; i < nmonkeys; ++i, m++) {
        if (m->visits > max1) {
            max2 = max1;
            max1 = m->visits;
        } else if (m->visits > max2) {
            max2 = m->visits;
        }
    }
    printf("max1=%lu max2=%lu res=%lu", max1, max2, max1*max2);
    return max1 * max2;
}

static void inspect(monkey_t *m)
{
    item_t *item, *tmp;
    long op1, op2, res;

    list_for_each_entry_safe(item, tmp, &m->items, list) {
        list_del(&item->list);
        //printf("Monkey %ld inspects %d\n", m - monkeys, item->item);
        m->visits++;
        printf("Monkey %ld inspect=%d visits=%d ", m - monkeys,
               item->item, m->visits);
        op1 = m->values[0] < 0 ? item->item: m->values[0];
        op2 = m->values[1] < 0 ? item->item: m->values[1];
        if (m->op == '+') {
            res = op1 + op2;
        } else {
            res = op1 * op2;
        }
        printf("op1=%ld op2=%ld op=%c res=%ld ", op1, op2, m->op, res);
        res = (res / divide) % lcm;
        //printf("final=%ld div=%d dest=%ld\n", res, m->div,
        //       m->dest[!!(res % m->div)] - monkeys);
        item->item = res;
        //if (res % m->div) {
        list_add_tail(&item->list, m->dest[!!(res % m->div)]);
            //}
        //print_monkeys();
    }
}

static char *skip(char *buf, int n)
{
    char *ret;
    for (; n >= 0; n--) {
        ret = strtok(buf, " ,\n");
        //printf("skip %d=%s\n", n, ret);
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
    //ssize_t buflen;
    monkey_t *m = monkeys;

    while (getline(&buf, &alloc, stdin) > 0) {
        printf("monkey %d\n", nmonkeys);
        INIT_LIST_HEAD(&m->items);

        getline(&buf, &alloc, stdin);             /* starting items */
        tok = skip(buf, 2);
        printf("skip=%s\n", tok);
        while (tok) {
            item_t *item = pool_get(pool_item);
            item->item = atoi(tok);
            list_add_tail(&item->list, &m->items);
            printf("val = %d\n", item->item);
            tok = getnext();
        }

        getline(&buf, &alloc, stdin);             /* operation */
        tok = skip(buf, 3);
        printf("skip=%s\n", tok);
        //tok = getnext();
        //printf("tok=%p %c\n", tok, *tok);
        m->values[0] = (*tok == 'o') ? -1: atoi(tok); /* first operand */
        printf("val1 = %ld\n", m->values[0]);
        //tok = getnext();
        m->op = *getnext();                       /* operator */
        tok = getnext();
        m->values[1] = *tok == 'o' ? -1: atoi(tok); /* second operand */
        printf("val2 = %ld\n", m->values[1]);

        getline(&buf, &alloc, stdin);             /* divisible */
        tok=skip(buf, 3);
        //printf("skip div=%s\n", tok);
        m->div = atoi(tok);
        lcm *= m->div;

        getline(&buf, &alloc, stdin);             /* true */
        //puts(buf);
        tok = skip(buf, 5);
        //printf("skip true=%s\n", tok);
        m->dest[0] = &(monkeys + atoi(tok))->items;
        //printf("true = %ld\n", m->dest[0] - monkeys);

        getline(&buf, &alloc, stdin);             /* false */
        tok = skip(buf, 5);
        //printf("skip false=%s\n", tok);
        m->dest[1] = &(monkeys + atoi(tok))->items;
        //printf("false = %ld\n", m->dest[1] - monkeys);

        getline(&buf, &alloc, stdin);             /* skip empty line */

        nmonkeys++;
        m++;
        printf("\n");
        //print_monkeys();
        //exit(0);
    }
    printf("lcm = %ld\n", lcm);
    print_monkeys();
    free(buf);
    return 1;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    int rounds = part == 1? 20: 10000;
    if (part == 2)
        divide = 1;
    pool_item =  pool_create("item", 64, sizeof(item_t));

    parse();
    for (int r = 0; r < rounds; ++r) {
        for (int i = 0; i < nmonkeys; ++i) {
            inspect(monkeys + i);
        }
        printf("+++++++ after round %d\n", r + 1);
        print_monkeys();
    }
    printf("%s: res=%lu\n", *av, result());
    exit(0);
}
