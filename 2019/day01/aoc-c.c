/* aoc-c.c: Advent of Code 2019, day 1 parts 1 & 2
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
#include <unistd.h>

#include "debug.h"
#include "pool.h"
#include "list.h"
#include "bits.h"

static u64 part1()
{
    u64 res = 0;
    int cur;

    while (scanf("%d", &cur) != EOF)
        res += cur / 3 - 2;
    return res;
}

static u64 part2()
{
    u64 res = 0;
    int cur;

    while (scanf("%d", &cur) != EOF)
        while ((cur = cur / 3 - 2) > 0)
            res += cur;
    return res;
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
                    return usage(*av);
                break;
            default:
                return usage(*av);
        }
    }

    if (optind < ac)
        return usage(*av);

    printf("%s : res=%lu\n", *av, part == 1? part1(): part2());

    exit (0);
}
