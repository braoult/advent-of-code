/* aoc-c.c: Advent of Code 2021, day 14 parts 1 & 2
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
#include <string.h>
#include <malloc.h>
#include <stdint.h>

#include "debug.h"
#include "bits.h"

#define NPOLYMERS   10                     /* max number of polymers */

/* possible 10 letters: B, C, F, H, K, N, O, P, S, V, reduced to 10 integers */
static int cindex[] =  {
    ['B'] = 0, ['C'] = 1, ['F'] = 2, ['H'] = 3, ['K'] = 4,
    ['N'] = 5, ['O'] = 6, ['P'] = 7, ['S'] = 8, ['V'] = 9
};

#define INDEX(c) cindex[(int)c]

static u64 polymers[NPOLYMERS][NPOLYMERS];
static u64 rules[NPOLYMERS][NPOLYMERS];
static u64 count[NPOLYMERS] = {0};

/* read data and create graph.
 */
static void read_input()
{
    ssize_t len;
    size_t alloc = 0;
    char *buf;
    char x, y, z;

    len = getline(&buf, &alloc, stdin) - 1;

    /* first and last characters need to be counted, as they will not appear
     * in final count
     */
    count[INDEX(*buf)]++;
    count[INDEX(*(buf + len - 1))]++;
    for (int i = 0; i < len - 1; ++i)
        polymers[INDEX(buf[i])][INDEX(buf[i+1])]++;

    /* get rules */
    while ((len = getline(&buf, &alloc, stdin)) >= 0) {
        if (sscanf(buf, "%c%c -> %c", &x, &y, &z) == 3)
            rules[INDEX(x)][INDEX(y)] = z;
    }
    free(buf);
}

static u64 doit(int steps)
{
    u64 min = UINT64_MAX, max = 0;

    read_input();
    for (int step = 0; step < steps; ++step) {
        int new;
        u64 new_polymers[NPOLYMERS][NPOLYMERS];

        memset(new_polymers, 0, sizeof(new_polymers));
        for (int i = 0; i < NPOLYMERS; ++i) {
            for (int j = 0; j < NPOLYMERS; ++j) {
                new = INDEX(rules[i][j]);
                new_polymers[i][new] += polymers[i][j];
                new_polymers[new][j] += polymers[i][j];
            }
        }
        memcpy(polymers, new_polymers, sizeof(polymers));
    }
    /* attention here: polymers are counted twice.
     */
    for (int i = 0; i < NPOLYMERS; ++i) {
        for (int j = 0; j < NPOLYMERS; ++j) {
            count[i] += polymers[i][j];
            count[j] += polymers[i][j];
        }
    }
    for (int i = 0; i < NPOLYMERS; ++i) {
        if (count[i] && count[i] < min)
            min = count[i];
        if (count[i] > max)
            max = count[i];
    }
    return (max - min) / 2;
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

    printf("%s : res=%lu\n", *av, doit(part == 1? 10: 40));
    exit (0);
}
