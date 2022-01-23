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

#define ALGO 512
int algolen;
char algo[ALGO + 1];

#define WIDTH 256
int universe_size = 0;
int left;
int curlen;
char **universe[2];

int left, right;
int current = 0, next = 1;

/* read input
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

/* make one step
 */
static int count()
{
    int res = 0;

    for (int i = left; i < right; ++i) {
        for (int j = left; j < right; ++j) {
            res += universe[current][i][j];
        }
    }
    return res;
}

/* make one step
 */
static void step()
{
    int newleft = left - 1, newright = right + 1;
    int index;

    for (int j = newleft; j < newright; ++j) {
        for (int i = newleft; i < newright; ++i) {
            index = universe[current][i-1][j-1] << 8 |
                universe[current][i-1][j  ] << 7 |
                universe[current][i-1][j+1] << 6 |

                universe[current][i  ][j-1] << 5 |
                universe[current][i  ][j  ] << 4 |
                universe[current][i  ][j+1] << 3 |

                universe[current][i+1][j-1] << 2 |
                universe[current][i+1][j  ] << 1 |
                universe[current][i+1][j+1] << 0;
            log_i(1, "i=%d, j=%d, index=%d\n", i, j, index);
            universe[next][i][j] = algo[index];
        }
    }
    left = newleft;
    right = newright;
    current = !current;
    next = !next;
}

/* read input
 */
static int read_data(int steps)
{
    int i = 0, cur = 0, verif;
    //char *buf, *buf1;
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;

    //scanf("%m[#.]%n%*c", &buf, &algolen);
    buflen = getline(&buf, &alloc, stdin) - 1;
    printf("len=%d\n", algolen);
    algolen = buflen;
    for (int i = 0; i < buflen; ++i)
        algo[i] = buf[i] == '#';

    getline(&buf, &alloc, stdin);

    /* We consider input will be a square, first line determines
     * the width
     */
    buflen = getline(&buf, &alloc, stdin) - 1;
    verif = buflen;
    universe_size = buflen + 2 * steps + 2;
    left = steps + 1;
    right = universe_size - left;
    //scanf("%m[#.]s%n%*c", &buf1, &spacelen);
    log_f(1, "buflen=%ld universe_size=%d left=%d\n",
          buflen, universe_size, left);
    universe[0] = malloc(universe_size * sizeof(char *));
    universe[1] = malloc(universe_size * sizeof(char *));
    for (i = 0; i < universe_size; ++i) {
        universe[0][i] = calloc(universe_size, sizeof(char));
        universe[1][i] = calloc(universe_size, sizeof(char));
    }
    /* they got me on this one ;-)
     */
    if (*algo) {
        log(1, "init second array\n");
        for (i = 0; i < universe_size; ++i)
            memset(universe[1][i], 1, universe_size);
        current=1;
        print_data(-1);
        current=0;
    }
    do {
        for (int i = 0; i < buflen; ++i) {
            universe[0][cur + left][i + left] = buf[i] == '#';
        }
        cur++;
        buflen = getline(&buf, &alloc, stdin) - 1;
        if (buflen != verif) {
            log(1, "verif: cur=%d buflen=%ld\n", cur, buflen);
        }
    } while (cur < universe_size && buflen == verif);

    log_f(1, "cur=%d buflen=%ld universe_size=%d\n",
          cur, buflen, universe_size);

    //spacelen = getline(&buf, &alloc, stdin) - 1;
    //scanf("%m[#.]%n%*c", &buf1, &spacelen);
    //printf("len=%d\n", spacelen);

    //for (int i = 0; i < algolen; ++i)
    //    algo[i] &= 1;                             /* lol */
    //print_data();


    free(buf);
    //algo[--algolen] = 0;
    return algolen - 1;
}

static int doit(int iter)
{
    for (int i = 0; i < iter; ++i) {
        step();
        print_data(i + 1);
    }
    return count();
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1, iter;

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

    iter = part == 1? 2: 50;
    read_data(iter);
    print_data(0);
    //step();
    //print_data(1);

    printf("%s : res=%d\n", *av, doit(iter));
    for (int i = 0; i < universe_size; ++i) {
        free(universe[0][i]);
        free(universe[1][i]);
    }
    free(universe[0]);
    free(universe[1]);
    exit(0);
}
