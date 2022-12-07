/* aoc-c.c: Advent of Code 2022, day 3
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

#include "aoc.h"

#define NCHARS ('Z' - 'A' + 1 + 'z' - 'a' + 1)

/* convert item to map position, and vice versa */
#define CHAR2POS(c) ((c) > 'Z'? (c) - 'a': (c) - 'A' + 26)
#define POS2CHAR(p) ((p) < 26? (p) + 'a': (p) - 26 + 'A')

/* set a map from char */
#define SET_MAP(ex, c) ((ex)[CHAR2POS(c)] = 1)

static void map_init(char *p)
{
    memset(p, 0, NCHARS);
}

static void map_populate(char *ex, char *p, int len)
{
    map_init(ex);
    for (; len; --len)
        SET_MAP(ex, p[len - 1]);
}

/* match map1 and map2 maps. If dest is NULL, return first match,
 * otherwise populate dest with matching chars.
 * TODO: replace map2 with the initial items string, this would avoid
 * a loop.
 */
static int map_match(char *dest, char *map1, char *map2)
{
    if (dest)
        map_init(dest);

    for (int i = 0; i < NCHARS; ++i) {
        if (map1[i] && map2[i]) {
            if (dest)
                dest[i] = 1;                      /* fill dest array */
            else
                return i;                         /* ont match only */
        }
    }
    return -1;                                    /* unused if dest != NULL */
}

static int parse(int part)
{
    size_t alloc = 0;
    char *buf = NULL;
    ssize_t buflen;
    int line = 0, res = 0, half;
    char map[3][NCHARS], ctmp[NCHARS];

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (part == 1) {
            half = buflen / 2;
            /* populate c1 */
            map_populate(map[0], buf, half);
            map_populate(map[1], buf + half, half);
            /* calculate part 1 */
            res += map_match(NULL, map[0], map[1]) + 1;
        } else {
            /* populate c2 */
            map_populate(map[line % 3], buf, buflen);
            if ((++line % 3))
                continue;
            /* calculate part 2 */
            map_match(ctmp, map[0], map[1]);
            res += map_match(NULL, ctmp, map[2]) + 1;
        }
    }
    free(buf);
    return res;
}

int main(int ac, char **av)
{
    printf("%s: res=%d\n", *av, parse(parseargs(ac, av)));
    exit(0);
}
