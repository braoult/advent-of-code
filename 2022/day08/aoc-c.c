/* aoc-c.c: Advent of Code 2022, day 8
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

#include "bits.h"
#include "aoc.h"

typedef struct tree {
    u16 size;
    char *h;                                      /* heigts */
    char *v;                                      /* visible (part 1) */
} tree_t;

#define HEIGHT(t, x, y)  (((t)->h[((t)->size * (y)) + (x)]))
#define VISIBLE(t, x, y) (((t)->v[((t)->size * (y)) + (x)]))

static tree_t *parse(tree_t *trees)
{
    size_t alloc = 0;
    char *buf = NULL;
    ssize_t buflen;

    buflen = getline(&buf, &alloc, stdin);
    buf[--buflen] = 0;
    trees->size = buflen;
    trees->h = malloc((buflen * buflen) * sizeof(char) + 1);
    memcpy(trees->h, buf, buflen);                   /* store first line */
    free(buf);

    for (buf = trees->h + buflen; ; buf += buflen) {
        if (scanf("%s", buf) <= 0)
            break;
    }
    trees->v = calloc(buflen * buflen, sizeof (char));
    return trees;
}

static int visible(tree_t *t, int *max, int x, int y)
{
    int h = HEIGHT(t, x, y);
    if (h > *max) {
        VISIBLE(t, x, y) = 1;
        if ((*max = h) == '9') {
            return 1;
        }
    }
    return 0;
}

static int check_tree(tree_t *t, int x, int y)
{
    int i, h = HEIGHT(t, x, y), res = 1, size = t->size, tmp;

    for (tmp = 0, i = x + 1; i < size ; ++i) {    /* east */
        tmp++;
        if (HEIGHT(t, i, y) >= h)
            break;
    }
    res *= tmp;
    for (tmp = 0, i = x - 1; i >= 0; --i) {       /* west */
        tmp++;
        if (HEIGHT(t, i, y) >= h)
            break;
    }
    res *= tmp;
    for (tmp = 0, i = y + 1; i < size; ++i) {     /* south */
        tmp++;
        if (HEIGHT(t, x, i) >= h)
            break;
    }
    res *= tmp;
    for (tmp = 0, i = y - 1; i >= 0; --i) {       /* north */
        tmp++;
        if (HEIGHT(t, x, i) >= h)
            break;
    }
    res *= tmp;
    return res;
}

static int part1(tree_t *t)
{
    int x, y, max, res = 0, size = t->size;

    for (y = 1; y < size -1; ++y) {               /* to east */
        if ((max = HEIGHT(t, 0, y)) < '9') {
            for (x = 1; x < size -1; ++x) {
                if (visible (t, &max, x, y))
                    break;
            }
        }
    }
    for (y = 1; y < size -1; ++y) {               /* to west */
        if ((max = HEIGHT(t, size - 1, y)) < '9') {
            for (x = size - 2; x > 0; --x) {
                if (visible (t, &max, x, y)) {
                    break;
                }
            }
        }
    }
    for (x = 1; x < size -1; ++x) {             /* to south */
        if ((max = HEIGHT(t, x, 0)) < '9') {
            for (y = 1; y < size -1; ++y) {
                if (visible (t, &max, x, y))
                    break;
            }
        }
    }
    for (x = 1; x < size -1; ++x) {             /* to north */
        if ((max = HEIGHT(t, x, size - 1)) < '9') {
            for (y = size - 2; y > 0; --y) {
                if (visible (t, &max, x, y))
                    break;
            }
        }
    }
    for (y = 0; y < size; ++y)
        for (x = 0; x < size; ++x)
            if (VISIBLE(t, x, y))
                res++;
    return res + size * 4 - 4;
}

static int part2(tree_t *t)
{
    int res = 0, tmp, size = t->size;

    for (int y = 1; y < size - 1; ++y) {
        for (int x = 1; x < size - 1; ++x) {
            tmp = check_tree(t, x, y);
            if (tmp > res)
                res = tmp;
        }
    }
    return res;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    tree_t trees;

    parse(&trees);
    printf("%s: res=%d\n", *av, part == 1? part1(&trees): part2(&trees));
    exit(0);
}
