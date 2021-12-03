/* aoc-c: Advent2021 game, day 2 parts 1 & 2
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
    /* idea : Given 3 bits input
     * 101
     * 010
     * 110
     *
     * we create an ushort array of size 2⁴ initialized at 2⁴
     *
     */
    return 1;
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
    }
    if (exercise == 2) {
        res = ex2();
        printf ("%s : res=%d\n", *av, res);
    }

    exit (0);
}
