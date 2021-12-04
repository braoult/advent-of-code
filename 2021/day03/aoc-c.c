/* aoc-c: Advent2021 game, day 3 parts 1 & 2
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "debug.h"
#include "bits.h"

static int ex1()
{
    u32 length, gamma = 0, epsilon = 0;
    char buffer[80];
    s32 *values;

    /* get length of binary number */
    scanf("%s", buffer);
    length = strlen(buffer);

    values = calloc(length, sizeof(*values));

    do {
        for (u32 i = 0; i < length; ++i) {
            switch (buffer[i]) {
                case '0':
                    values[i]--;
                    break;
                case '1':
                    values[i]++;
                    break;
            }
        }
    } while (scanf("%s", buffer) != EOF);

    for (u32 i = 0; i < length; ++i) {
        if (values[i] > 0)
            gamma += 1 << (length-i-1);
        else
            epsilon += 1 << (length-i-1);
    }
    free (values);
    return gamma * epsilon;
}

static int ex2()
{
    u32 length, size, *values, min, max, oxygen, co2;
    u32 zero, one, lastone, lastzero, bit;
    char buffer[80];
    u64 val;

    /* get length of binary number */
    scanf("%s", buffer);
    length = strlen(buffer);
    size = 1 << length;

    values = calloc(size, sizeof(*values));

    do {
        val = strtoul(buffer, NULL, 2);
        values[val] = val;
    } while (scanf("%s", buffer) != EOF);

    min = 0;
    max = size;
    bit = size >> 1;
    while (1) {
        zero = 0;
        one = 0;
        for (u32 i = min; i < max; ++i) {
            if (values[i]) {
                if (values[i] & bit) {
                    one++;
                    lastone = i;
                } else {
                    zero++;
                    lastzero = i;
                }
            }
        }
        if (one >= zero) {
            if (one == 1) {
                oxygen = lastone;
                break;
            }
            min = (max + min) / 2;
        } else {
            if (zero == 1) {
                oxygen = lastzero;
                break;
            }
            max = (max + min) / 2;
        }
        bit >>= 1;
    }

    min = 0;
    max = size;
    bit = size >> 1;
    while (1) {
        zero = 0;
        one = 0;
        for (u32 i = min; i < max; ++i) {
            if (values[i]) {
                if (values[i] & bit) {
                    one++;
                    lastone = i;
                } else {
                    zero++;
                    lastzero = i;
                }
            }
        }
        if (zero <= one) {
            if (zero == 1) {
                co2 = lastzero;
                break;
            }
            max = (max + min) / 2;
        } else {
            if (one == 1) {
                co2 = lastone;
                break;
            }
            min = (max + min) / 2;
        }
        bit >>= 1;
    }
    free(values);
    return oxygen * co2;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    u32 exercise = 1, res;

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
    } else {
        res = ex2();
        printf ("%s : res=%d\n", *av, res);
    }

    exit (0);
}
