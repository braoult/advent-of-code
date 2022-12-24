/* aoc-c.c: Advent of Code 2022, day 12
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

#include "br.h"
#include "list.h"
#include "pool.h"

#include "aoc.h"

#define MAX_LINES 128                             /* should be dynamic */

typedef struct coord {
    int x, y;
} coord_t;

typedef struct square {
    char height;
    int dfs;
    coord_t coord;
    struct list_head queue;
} square_t;

typedef struct grid {
    int nx, ny;
    coord_t start, end;

    square_t **map;
} grid_t;

pool_t *pool_lines;
LIST_HEAD(dfs);

#define CELL(g, x, y) (&(g)->map[y][x])

static inline void push(square_t *s)
{
    list_add_tail(&s->queue, &dfs);
}

static inline square_t *pop()
{
    square_t *s = list_first_entry_or_null(&dfs, square_t, queue);
    if (s)
        list_del(&s->queue);
    return s;
}

static int parse(grid_t *grid)
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    int line = 0;
    square_t *pline;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (line == 0) {
            pool_lines = pool_create("lines", 128, buflen * sizeof(square_t));
            grid->nx = buflen;
        }
        grid->map[line] = pool_get(pool_lines);
        pline = grid->map[line];
        for (int i = 0; i < buflen; ++i) {
            pline[i].coord.x = i;
            pline[i].coord.y = line;
            pline[i].dfs = 0;
            if (buf[i] == 'S') {
                grid->start.x = i;
                grid->start.y = line;
                pline[i].height = 'a';
            } else if (buf[i] == 'E') {
                grid->end.x = i;
                grid->end.y = line;
                pline[i].height = 'z';
            } else {
                pline[i].height = buf[i];
            }
        }
        line++;
    }
    grid->ny = line;
    free(buf);
    return 1;
}

static square_t *dfs_add(grid_t *g, square_t *s, int n)
{
    static coord_t dirs[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
    int x = s->coord.x + dirs[n].x, y = s->coord.y + dirs[n].y;
    square_t *ret = NULL;
    if (x >= 0 && y >= 0 && x < g->nx && y < g->ny) {
        ret = CELL(g, x, y);
        if (!ret->dfs)
            return ret;
    }
    return NULL;
}

static int part1(grid_t *g)
{
    square_t *s, *n;

    s = CELL(g, g->start.x, g->start.y);
    s->dfs = 1;
    push(s);
    while((s = pop())) {
        //log_f(3, "");
        for (int i = 0; i < 4; i++) {
            if (!(n = dfs_add(g, s, i)))
                continue;
            if (n->height <= s->height+1) {
                if (n->coord.x == g->end.x && n->coord.y == g->end.y)
                    return s->dfs;
                n->dfs = s->dfs + 1;
                push(n);
            }

        }
    }
    return -1;
}

static int part2(grid_t *g)
{
    square_t *s, *n;
    //int ret = 0;
    s = CELL(g, g->end.x, g->end.y);
    s->dfs = 1;
    push(s);
    while((s = pop())) {
        //log_f(3, "");
        for (int i = 0; i < 4; i++) {
            if (!(n = dfs_add(g, s, i)))
                continue;
            if (n->height >= s->height - 1) {
                if (n->height == 'a')
                    return s->dfs;
                n->dfs = s->dfs + 1;
                push(n);
            }

        }
    }
    return -1;
}


int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    square_t *board[MAX_LINES];
    grid_t grid = { 0, 0, { 0, 0 }, {0, 0}, board };

    parse(&grid);
    printf("%s: res=%d\n", *av, part == 1? part1(&grid): part2(&grid));
    pool_destroy(pool_lines);
    exit(0);
}
