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
#include <unistd.h>
#include <string.h>

#include "plist.h"
#include "debug.h"
#include "pool.h"
#include "aoc.h"

#define NCHARS ('Z' - 'A' + 1 + 'z' - 'a' + 1)

#define CHAR2POS(c) ((c) > 'Z'? (c) - 'a': (c) - 'A' + 26)
#define POS2CHAR(p) ((p) < 26? (p) + 'a': (p) - 26 + 'A')

#define SET_EXISTS(ex, c) ((ex)[CHAR2POS(c)] = 1)

static void exists_init(char *p)
{
    memset(p, 0, NCHARS);
}

static void exists_populate(char *ex, char *p, int len)
{
    exists_init(ex);
    for (; len; --len) {
        char c = p[len - 1];
        SET_EXISTS(ex, c);
    }
}

/* match c1 and c2 maps. If dest is NULL, return first match,
 * otherwise populate dest with matching chars
 */
static int exists_match(char *dest, char *c1, char *c2)
{
    int count = 0;
    if (dest)
        exists_init(dest);

    for (int i = 0; i < NCHARS; ++i) {
        if (c1[i] && c2[i]) {
            if (dest) {
                dest[i] = 1;
            } else {
                return i;
            }
        }
    }
    return count;
}

static int *parse(int *res)
{
    size_t alloc = 0;
    char *buf = NULL;
    ssize_t buflen;
    int curc2 = 0;                                   /* counter for part2 */
    char c1[2][NCHARS], c2[3][NCHARS], ctmp[NCHARS]; /* char exists in set */

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        int half = buflen / 2;

        /* populate c1 */
        exists_populate(c1[0], buf, half);
        exists_populate(c1[1], buf + half, half);
        /* calculate part 1 */
        res[0] += exists_match(NULL, c1[0], c1[1]) + 1;

        /* populate c2 */
        exists_populate(c2[curc2], buf, buflen);

        if (!(++curc2 % 3)) {                     /* calculate part 2 */
            exists_match(ctmp, c2[0], c2[1]);
            res[1] += exists_match(NULL, ctmp, c2[2]) + 1;
            curc2 = 0;
        }
    }
    free(buf);
    return res;
}


int main(int ac, char **av)
{
    int res[2] = { 0, 0 };

    printf("%s: res=%d\n", *av, parse(res)[parseargs(ac, av) - 1]);
    exit(0);
}
