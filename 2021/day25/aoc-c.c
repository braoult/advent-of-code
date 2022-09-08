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
#include "bits.h"

#define SWAP(x, y) do { typeof(x) __x = x; x = y; y = __x; } while (0)

#define EAST  '>'
#define SOUTH 'v'
#define EMPTY '.'

struct map {
    uint ncols, nrows;
    char *cur, *next;
};

struct move {
    uchar r, c;
    uchar r1, c1;
    uchar dir;
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

static void print_map(struct map *map, int details)
{
    if (details) {
        log(2, "map: cols=%d rows=%d\ncur:\n%snext:\n%s",
            map->ncols, map->nrows, map->cur, map->next);
    } else {
        log(2, "cur:\n%s\n", map->cur);
    }
}

static inline int maybe_move(struct map *map, struct move m)
{
    int ret = 0, ncols = map->ncols;
    char *cur = map->cur, *next = map->next;

    if (*mapc(cur, ncols, m.r, m.c) == m.dir
        && *mapc(cur, ncols, m.r1, m.c1) == EMPTY) {
        *mapc(next, ncols, m.r, m.c) = EMPTY;
        *mapc(next, ncols, m.r1, m.c1) = m.dir;
        ret = 1;
    } else {
        *mapc(next, ncols, m.r, m.c) = *mapc(cur, ncols, m.r, m.c);
    }
    return ret;
}

static int step(struct map *map)
{
    int moves = 0, ncols = map->ncols, nrows = map->nrows;
    struct move m;

    /* move east */
    m.dir = EAST;
    for (m.r = 0; m.r < nrows; ++m.r) {
        m.r1 = m.r;
        for (m.c = 0; m.c < ncols; ++m.c) {
            m.c1 = (m.c + 1) % ncols;
            if (maybe_move(map, m)) {
                moves++;
                m.c++;
            }
        }
    }

    /* move south - here we move data from next to cur (by swapping them) */
    SWAP(map->cur, map->next);
    m.dir = SOUTH;
    for (m.c = 0; m.c < ncols; ++m.c) {
        m.c1 = m.c;
        for (m.r = 0; m.r < nrows; ++m.r) {
            m.r1 = (m.r + 1) % nrows;
            if (maybe_move(map, m)) {
                moves++;
                m.r++;
            }
        }
    }
    SWAP(map->cur, map->next);
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
        /* read whole input, we will keep '\n' and avoit useless splitting */
        buflen = getdelim(&map->cur, &alloc, '\0', stdin);
        map->next = strdup(map->cur);
        map->ncols = strchr(map->cur, '\n') - map->cur;
        /* we suppose there is nothing after the last input data last line
         * Therefore last char of input is '\n', at position (bufflen - 1)
         */
        map->nrows = (buflen - 1) / map->ncols;

        log(2, "buflen=%ld ncols=%d nrows=%d lastnl=%ld\n", buflen, map->ncols,
            map->nrows, strrchr(map->cur, '\n') - map->cur);
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
    int opt, part = 1;
    struct map *map;
    int moves, cur = 1;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                part = atoi(optarg);
                if (part == 1 || part == 2)
                    break;
                /* fall through */
            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    if ((map = read_input())) {
        while ((moves = step(map))) {
            cur++;
            log(2, "+++ after step %d\n", cur);
            print_map(map, 0);
        }
    }

    printf("%s : res=%d\n", *av, cur);

    exit(0);
}
