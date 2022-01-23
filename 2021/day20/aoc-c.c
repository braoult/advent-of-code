/* aoc-c.c: Advent of Code 2021, day 20 parts 1 & 2
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
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "pool.h"
#include "debug.h"
#include "bits.h"
#include "list.h"

#define ALGOLEN 512
static char algorithm[ALGOLEN];                   /* input algorithm */

static int universe_size = 0;                     /* universe width/height */
static char **universe[2];

static int left, right;                           /* universe current borders */
static int current = 0;                           /* active universe */

#ifdef DEBUG
/* print current universe
 */
static void print_data(int step)
{
    for (int i = 0; i < algolen; ++i)
        log(5, "%c", algo[i]? '#': '.');
    log(5, "\n");
    log_f(1, "step %d, current=%d\n", step, current);
    for (int i = left; i < right; ++i) {
        for (int j = left; j < right; ++j)
            log(1, "%c", universe[current][i][j]? '#': '.');
        log(1, "\n");
    }
}
#endif

/* make one step
 */
static inline int count()
{
    int res = 0;

    for (int i = left; i < right; ++i)
        for (int j = left; j < right; ++j)
            res += universe[current][i][j];
    return res;
}

/* make one step
 */
static inline void step()
{
    int index;

    left--;
    right++;
    for (int j = left; j < right; ++j) {
        for (int i = left; i < right; ++i) {
            index = universe[current][i-1][j-1] << 8 |
                universe[current][i-1][j  ] << 7 |
                universe[current][i-1][j+1] << 6 |

                universe[current][i  ][j-1] << 5 |
                universe[current][i  ][j  ] << 4 |
                universe[current][i  ][j+1] << 3 |

                universe[current][i+1][j-1] << 2 |
                universe[current][i+1][j  ] << 1 |
                universe[current][i+1][j+1] << 0;
            universe[!current][i][j] = algorithm[index];
        }
    }
    current = !current;
}

/* read input
 */
static int read_data(int steps)
{
    int i = 0, cur = 0;
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;

    buflen = getline(&buf, &alloc, stdin) - 1;
    for (int i = 0; i < buflen; ++i)
        algorithm[i] = buf[i] == '#';

    /* We consider input will be a square, first line determines
     * the width
     */
    getline(&buf, &alloc, stdin);                 /* skip second line */
    buflen = getline(&buf, &alloc, stdin) - 1;

    /* universe grows by 1 on 4 side (= +2 width and +2 height),
     * and we need also to keep one extra border for calculation.
     */
    universe_size = buflen + 2 * steps + 2;
    left = steps + 1;
    right = universe_size - left;
    universe[0] = malloc(universe_size * sizeof(char *));
    universe[1] = malloc(universe_size * sizeof(char *));
    for (i = 0; i < universe_size; ++i) {
        universe[0][i] = calloc(universe_size, sizeof(char));
        universe[1][i] = calloc(universe_size, sizeof(char));
    }

    /* they got me on this one, different from given example ;-)
     * We need to fill the odd universe with '1' if algorithm[0] is 1
     * (whole universe becomes lit).
     * It would be better to consider also the case algorithm[511] is 1,
     * and algorithm[0] is 0 (we could only count after odd steps).
     */
    if (*algorithm) {
        for (i = 0; i < universe_size; ++i)
            memset(universe[1][i], 1, universe_size);
        if (algorithm[ALGOLEN - 1]) {
            log(0, "cannot display infinity ;-)\n");
            return 0;
        }
    }

    do {
        for (int i = 0; i < buflen; ++i)
            universe[0][cur + left][i + left] = buf[i] == '#';
        cur++;
        buflen = getline(&buf, &alloc, stdin) - 1;
    } while (cur < universe_size && buflen > 0);

    free(buf);
    return 1;
}

static int doit(int iter)
{
    for (int i = 0; i < iter; ++i)
        step();
    return count();
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

    int iter = part == 1? 2: 50;

    if (read_data(iter))
        printf("%s : res=%d\n", *av, doit(iter));

    for (int i = 0; i < universe_size; ++i) {
        free(universe[0][i]);
        free(universe[1][i]);
    }
    free(universe[0]);
    free(universe[1]);
    exit(0);
}
