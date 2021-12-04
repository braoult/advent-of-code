/* aoc-c: Advent2021 game, day 1 parts 1 & 2
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

#include "debug.h"
#include "bits.h"
#include "pool.h"

struct ranges {
    u32 val;
    struct list_head list;
};

LIST_HEAD(list_head);

int ex1()
{
    u32 count = 0, res = 0, prev, cur;

    while (scanf("%d", &cur) != EOF) {
        if (count && cur > prev)
            res++;
        count++;
        prev = cur;
    }
    return res;
}

int ex2()
{
    u32 count = 0, res = 0;
    u32 val;
    pool_t *pool;
    struct ranges *input;
    struct ranges *list_cur;

    if (!(pool = pool_init("pool", 10, sizeof (struct ranges))))
        return -1;

    while (scanf("%d", &val) != EOF) {
        if (!(input = pool_get(pool)))
            return -1;
        input->val = val;
        list_add_tail(&input->list, &list_head);

        if (count > 2) {
            u32 loop = 0, v1 = 0, v2 = 0;
            struct ranges *first = list_entry(list_head.next, struct ranges, list);

            list_for_each_entry(list_cur, &list_head, list) {
                if (loop < 3)
                    v1 += list_cur->val;
                if (loop > 0)
                    v2 += list_cur->val;
                ++loop;
            }
            list_del(&first->list);
            pool_add(pool, first);
            if (v2 > v1)
                res++;
        }
        count++;
    }
    return res;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    u32 exercise = 2, res;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                exercise = atoi(optarg);
                break;
            default:
                return usage(*av);
        }
    }

    if (optind < ac)
        return usage(*av);

    if (exercise == 1) {
        res = ex1();
        printf ("%s : res=%d\n", *av, res);
    }
    if (exercise == 2) {
        res = ex2();
        printf ("%s : res=%d\n", *av, res);
    }

    exit (0);
}
