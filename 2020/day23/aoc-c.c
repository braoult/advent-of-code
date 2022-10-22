/* aoc-c.c: Advent2020, day 23, part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "list.h"
#include "debug.h"

/* Here we will simply use an array for the numbers (as key is the number
 * itself). All elements make a next/prev ring.
 * Note: we will ignore array[0] to avoid millions of useless "position -1"
 * calculations
 */

struct list_head *curcup;                         /* curcup cup */
struct list_head *cups;                           /* the cups cups */
int lastnum;                                      /* last cup number */

#define CUR_CUP (&*curcup)
#define NUM(pcup)  ((pcup) - cups)

static __always_inline void step()
{
    struct list_head *first, *last, *dest = CUR_CUP;
    int num[3] = { 0 };

    first = CUR_CUP->next;
    last  = first->next->next;
    num[0] = NUM(first);
    num[1] = NUM(first->next);
    num[2] = NUM(last);

    do {
        if (NUM(--dest) <= 0)
            dest = &cups[lastnum];
    } while (NUM(dest) == num[0] || NUM(dest) == num[1] || NUM(dest) == num[2]);

    //list_bulk_move_tail(dest->next, first, last);
    list_bulk_move(dest, first, last);
    curcup = CUR_CUP->next;
}

/**
 * parse - initialize cups list.
 */
static void parse(int last)
{
    int count = 0, c;
    cups = malloc(sizeof(struct list_head) * (last + 1));

    if (cups) {
        for (c = getchar();  isdigit(c); c = getchar(), ++count) {
            struct list_head *next = &cups[c - '0'];
            if (!count)                           /* first cup */
                INIT_LIST_HEAD(curcup = next);
            else
                list_add_tail(&cups[c - '0'], CUR_CUP);
        }
        for (++count; count <= last; ++count)     /* add remaining cups */
            list_add_tail(&cups[count], CUR_CUP);
    }
}

static long part1()
{
    struct list_head *cur;
    int res = 0;
    list_for_each(cur, &cups[1]) {
        res = res * 10 + NUM(cur);
    }
    return res;
}

static long part2()
{
    return NUM((&cups[1])->next) * NUM((&cups[1])->next->next);
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(ac, av)
    int ac;
    char **av;
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

    lastnum = part == 1? 9: 1000000;
    int loops = part == 1? 100: 10000000;
    parse(lastnum);
    for (int i = 0; i < loops; ++i)
        step();
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
    free(cups);
    exit (0);
}
