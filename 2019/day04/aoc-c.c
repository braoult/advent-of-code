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
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#include "debug.h"

static int is_valid(int number, int part)
{
    int valid = 0, dups[10] = { 0 };
    int digit, dec;

    for (digit = number % 10; number > 10; digit = dec) {
        number /= 10;
        dec = number % 10;
        if (dec > digit)
            return 0;
        if (dec == digit) {
            valid = 1;
            dups[digit] += 2;
        }
    }
    if (!valid || part == 1)
        return valid;
    for (int i = 0; i < 10; ++i)
        if (dups[i] == 2)
            return 1;
    return 0;
}

static int doit(int *nums, int part)
{
    int res = 0;

    /* There is surely a way to avoid 99% of useless calls to is_valid.
     */
    for (int i = nums[0]; i < nums[1]; ++i)
        if (is_valid(i, part))
            res++;
    return res;
}

static int *parse(int *res)
{
    size_t alloc = 0;
    char *buf = NULL;

    getline(&buf, &alloc, stdin);
    *res = atoi(strtok(buf, "-"));
    *(res+1) = atoi(strtok(NULL, "-"));
    free(buf);
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
    int nums[2];

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

    parse(nums);
    printf("%s : res=%d\n", *av, doit(nums, part));

    exit (0);
}
