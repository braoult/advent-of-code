/* aoc-c.c: Advent of Code 2019, day 4 parts 1 & 2
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
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#include "debug.h"
#include "likely.h"

/**
 * next_number() - finds next suitable number after a faulty right digit
 * @number: the number
 * @faulty: the faulty digit position (1, 10, 100, etc...)
 * @left:   the left digit of faulty one
 *
 * This function is called when rule 4 is violated:
 *    "Going from left to right, the digits never decrease."
 * Example: 453456
 *            ^
 * Here we will replace the faulty digit and next ones with its left digit
 * value (5 here). Returned number will be 455555.
 * This function allowed to save 546,495/548,022 calls to is_valid(), which
 * is 99.7%.
 */
static int next_number(int number, int faulty, int left)
{
    int next = number - number % (faulty * 10);

    for (; faulty; faulty /= 10)
        next += left * faulty;
    return next;
}

static int is_valid(int number, int part, int *next)
{
    int valid = 0, dups[10] = { 0 }, work = number;
    int digit, dec = number + 1, faulty = 1;
    *next = number + 1;

    for (digit = number % 10; number > 10; digit = dec, faulty *= 10) {
        number /= 10;
        dec = number % 10;
        if (dec > digit) {
            *next = next_number(work, faulty, dec);
            return 0;
        }
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
    int res = 0, next = 0;

    for (int i = nums[0]; i < nums[1]; i = next) {
        if (unlikely(is_valid(i, part, &next)))
            res++;
    }
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
