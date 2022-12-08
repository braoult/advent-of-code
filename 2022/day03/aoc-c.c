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

/* match map with items list. If dest is NULL, return first match,
 * otherwise populate dest with matching chars.
 */
static int map_match(char *dest, char *map, char *items, int ilen)
{
    if (dest)
        map_init(dest);

    for (int i = 0; i < ilen; ++i) {
        int pos = CHAR2POS(items[i]);
        if (map[pos]) {                           /* already exists */
            if (dest)
                dest[pos] = 1;                    /* fill dest array */
            else
                return pos + 1;                   /* one match only: return prio */
        }
    }
    return -1;                                    /* unused if dest != NULL */
}

static int parse(int part)
{
    size_t alloc = 0;
    char *buf = NULL;
    ssize_t buflen;
    int line = 0, res = 0;
    char map[2][NCHARS];

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (part == 1) {
            int half = buflen / 2;
            map_populate(*map, buf, half);        /* populate and calculate */
            res += map_match(NULL, *map, buf + half, half);
            continue;
        }
        /* Part 2 here */
        switch (++line % 3) {                     /* group lines by 3 */
            case 1:                               /* line 1: populate *map */
                map_populate(*map, buf, buflen);
                break;
            case 2:                               /* line 2: merge #1 into map[1] */
                map_match(*(map + 1), *map, buf, buflen);
                break;
            case 0:                               /* line 3: final merge & calc */
                res += map_match(NULL, *(map + 1), buf, buflen);
                break;
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
