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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"

struct map {
    int ncols, nrows;
    char *cur, *next;
};

/**
 * mapc - gives address of cuncumber at position (row, col)
 * @ptr:    char array
 * @ncols:  number of chars in one row
 * @row:    cuncumber row
 * @col:    cuncumber column
 *
 * @return: pointer to cuncumber position
 */
static inline char *mapc(char *p, int ncols, int row, int col)
{
    return p + row * (ncols + 1) + col;
}

/*
 * static void print_map(struct map *map)
 * {
 *     printf("map: cols=%d rows=%d\ncur:\n%snext:\n%s",
 *            map->ncols, map->nrows, map->cur, map->next);
 * }
 */

static inline int maybe_move(char *cur, char *next, int ncols,
                             char dir,
                             int r, int c, int r1, int c1)
{
    int ret = 0;
    if (*mapc(cur, ncols, r, c) == dir && *mapc(cur, ncols, r1, c1) == '.') {
        *mapc(next, ncols, r, c) = '.';
        *mapc(next, ncols, r1, c1) = dir;
        ret = 1;
    } else {
        *mapc(next, ncols, r, c) = *mapc(cur, ncols, r, c);
    }
    return ret;
}

static int step(struct map *map)
{
    int moves = 0, ncols = map->ncols, nrows = map->nrows;
    char *cur = map->cur, *next = map->next;

    /* move east */
    for (int r = 0; r < nrows; r++) {
        for (int c = 0; c < ncols; ++c) {
            if (maybe_move(cur, next, ncols, '>', r, c, r, (c + 1) % ncols)) {
                moves++;
                c++;
            }
        }
    }
    /* move south - here we move data from next to cur */
    for (int c = 0; c < ncols; ++c) {
        for (int r = 0; r < nrows; r++) {
            if (maybe_move(next, cur, ncols, 'v', r, c, (r + 1) % nrows, c)) {
                moves++;
                r++;
            }
        }
    }
    return moves;
}

/**
 * read-input() - read cuncumber map into memory.
 *
 */
static struct map *read_input()
{
    size_t alloc = 0;
    ssize_t buflen;
    struct map *map=malloc(sizeof(struct map));

    if (map) {
        map->cur = NULL;
        /* read whole input */
        buflen = getdelim(&map->cur, &alloc, '\0', stdin);
        map->next = strdup(map->cur);
        map->ncols = strchr(map->cur, '\n') - map->cur;
        map->nrows = buflen / (map->ncols + 1);
    }
    return map;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1, moves, cur=1;
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

    struct map *map = read_input();
    if (map)
        while ((moves = step(map)))
            cur++;

    printf("%s : res=%d\n", *av, cur);

    exit(0);
}
