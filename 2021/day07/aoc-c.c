/* aoc-c: Advent2021 game, day 6 parts 1 & 2
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

static int ncrabs, crab_grow, crab_alloc, crab_max;

typedef struct crab {
    int pos;
    int ncrabs;
} crab_t;

crab_t *crabs = NULL;

//#ifdef DEBUG
static void print_crab()
{
    int i = 0;

    printf("crabs=%d max=%d\n", ncrabs, crab_max);
    for (i = 0; i < ncrabs; ++i)
        printf("%s%d/%d", i? ", ": "", crabs[i].pos, crabs[i].ncrabs);
    printf("\n");
}
//#endif

static crab_t *grow_crabs()
{
    //static int crab_max = 0;
    int old = crab_alloc;

    crab_alloc += crab_grow;
    crabs = realloc(crabs, crab_alloc * sizeof (*crabs));
    for (int i = old; i < crab_alloc; ++i) {
        crabs[i].pos = -1;
        crabs[i].ncrabs = 0;
    }
    return crabs;
}

static int add_crab(int crab)
{
    int i;

    if (crab > crab_max)
        crab_max = crab;
    if (ncrabs == crab_alloc)                     /* conservative grow */
        grow_crabs();
    for (i = 0; i < ncrabs; ++i) {
        if (crabs[i].pos == crab) {
            crabs[i].ncrabs++;
            return i;
        }
    }
    crabs[i].pos = crab;
    crabs[i].ncrabs = 1;
    ncrabs++;
    return i;
}

static u64 read_crab()
{
    char *buf, *token;
    size_t alloc = 0;
    ssize_t length;

    if ((length = getline(&buf, &alloc, stdin)) < 0)
        return -1;
    crab_grow = length / 2;

    token = strtok(buf, ",\n");
    while (token) {
        add_crab(atoi(token));
        token = strtok(NULL, ",\n");
    }
    free(buf);
    return ncrabs;
}

static int doit(int part)
{
    int try, best_dist = 0, best_fuel = crab_max * crab_max;

    if (part == 1) {
        for (try = 0; try < crab_max; ++try) {
            int fuel = 0;
            for (int crab = 0; crab < ncrabs; ++crab)
                fuel += abs(crabs[crab].pos - try) * crabs[crab].ncrabs;
            if (fuel < best_fuel) {
                best_dist = try;
                best_fuel = fuel;
            }
        }
    }
    return best_fuel;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    u32 exercise = 1;
    int res;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                exercise = atoi(optarg);
                if (exercise < 1 || exercise > 2)
                    return usage(*av);
                break;
            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    read_crab();
    res = doit(exercise);
    printf ("%s : res=%d\n", *av, res);
    exit (0);
}
