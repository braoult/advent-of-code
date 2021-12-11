/* aoc-c.c: Advent of Code 2021, day 6 parts 1 & 2
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

#include "debug.h"
#include "bits.h"

#define NFISH 9
static u64 fish[NFISH];

inline static u64 fish_count()
{
    u64 nfish = 0;

    for (int i = 0; i < NFISH; ++i)
        nfish += fish[i];
    return nfish;
}

#ifdef DEBUG
static void print_fish()
{
    int i = 0;

    printf("fish: %lu\n", fish_count());
    for (i = 0; i < NFISH; ++i)
        printf("%10d", i);
    printf("\n");
    for (i = 0; i < NFISH; ++i)
        printf("%10lu", fish[i]);
    printf("\n");
}
#endif

static u64 read_fish()
{
    char *buf, *token;
    size_t alloc = 0;
    u64 nfish = 0;

    if (getline(&buf, &alloc, stdin) < 0)
        return -1;

    token = strtok(buf, ",\n");
    while (token) {
        fish[atoi(token)]++;
        nfish++;
        token = strtok(NULL, ",\n");
    }
    free(buf);
    return nfish;
}

static void doit(int part, int iter)
{
    u64 toadd;

    if (!iter)
        iter = part == 1 ? 80: 256;

    for (; iter; --iter) {
        toadd = fish[0];
        for (int i = 1; i < NFISH; ++i)
            fish[i - 1] = fish[i];
        fish[NFISH - 1] = toadd;
        fish[6] += toadd;
    }
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part] [-i iter]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, iter = 0;
    u32 exercise = 1;

    while ((opt = getopt(ac, av, "d:p:i:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                exercise = atoi(optarg);
                if (exercise < 1 || exercise > 2)
                    return usage(*av);
                break;
            case 'i':                             /* 1 or 2 */
                iter = atoi(optarg);
                break;
            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    read_fish();
    doit(exercise, iter);
    printf ("%s : res=%lu\n", *av, fish_count());
    exit (0);
}
