/* aoc-c.c: Advent of Code 2021, day 25 part 1
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

/* Warning: Work in progress. Should not work before I spend a lot of time
 * on it, the original "bitboard" approach for moves generation is pretty
 * difficult to program/debug
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


struct map {
    int ncols, nrows;
    char *map, *next;
};

static inline char *mapline(struct  map *m, int row)
{
    return m->map + row * m->ncols + row;
}
static inline char *nextline(struct  map *m, int row)
{
    return m->next + row * m->ncols + row;
}
static inline char *mapc(struct  map *m, int row, int col)
{
    return mapline(m, row) + col;
}
static inline char* nextc(struct  map *m, int row, int col)
{
    return nextline(m, row) + col;
}

/*
static void print_map(struct map *map)
{
    int i;
    printf ("map: cols=%d rows=%d\ncur:\n", map->ncols, map->nrows);
    for  (i = 0; i < map->nrows; ++i)
        puts(mapline(map, i));
    printf ("next:\n");
    for  (i = 0; i < map->nrows; ++i)
        puts(nextline(map, i));
}
*/

static int step(struct map *map)
{
    int r, c, mod = map->ncols, moves=0;
    char *tmp;

    /* move right */
    for (r = 0; r < map->nrows; r++) {
        for (c = 0; c < map->ncols; ++c) {
            int next = (c + 1) % mod;
            if (*mapc(map, r, c) == '>' && *mapc(map, r, next) == '.') {
                *nextc(map, r, c) = '.';
                *nextc(map, r, next) = '>';
                moves++;
                c++;
            } else {
                *nextc(map, r, c) = *mapc(map, r, c);
            }
        }
    }
    tmp = map->map;
    map->map = map->next;
    map->next = tmp;

    /* move down */
    mod = map->nrows;
    for (c = 0; c < map->ncols; ++c) {
        for (r = 0; r < map->nrows; r++) {
            int next = (r + 1) % mod;
            if (*mapc(map, r, c) == 'v' && *mapc(map, next, c) == '.') {
                *nextc(map, r, c) = '.';
                *nextc(map, next, c) = 'v';
                moves++;
                r++;
            } else {
                *nextc(map, r, c) = *mapc(map, r, c);
            }
        }
    }
    tmp = map->map;
    map->map = map->next;
    map->next = tmp;
    //print_map(map);
    return moves;
}

/* minimal parsing: We just read the 3-5 lines to get
 * the amphipods location in side rooms
 */
static void read_input(struct map *map)
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    int line = 0;

    buflen = getline(&buf, &alloc, stdin);
    buf[buflen - 1] = 0;
    map->map = malloc(buflen * buflen);
    map->next = malloc(buflen * buflen);
    map->ncols = buflen - 1;
    strcpy(map->map, buf);
    strcpy(map->next, buf);
    line++;
    free(buf);
    buf = mapline(map, line);
    while (fgets(buf, buflen + 1, stdin)) {
        buf[buflen - 1] = 0;
        strcpy(nextline(map, line), buf);
        line++;
        buf = mapline(map, line);
    }
    map->nrows = line;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1, moves, cur=1;
    struct map map = { 0, 0, NULL, NULL};

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

    read_input(&map);
    while ((moves = step(&map))) {
        cur++;
    }

    printf("%s : res=%d\n", *av, cur);

    exit(0);
}
