/* aoc-c.c: Advent of Code 2022, day 2
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

#include "plist.h"
#include "debug.h"
#include "pool.h"
#include "aoc.h"

/* we will use the following convention, for input line "a x":
 * position in array=(a-'A' * 3) + x - 'X'
 */
enum {
    A = 0, B, C,
    X = 0, Y, Z
};
#define pos(x, y) ((x) * 3 + (y))

int outcome[2][9] = {                             /* shape is known */
    {
        [pos(A, X)] = 3 + 1, [pos(A, Y)] = 6 + 2, [pos(A, Z)] = 0 + 3,
        [pos(B, X)] = 0 + 1, [pos(B, Y)] = 3 + 2, [pos(B, Z)] = 6 + 3,
        [pos(C, X)] = 6 + 1, [pos(C, Y)] = 0 + 2, [pos(C, Z)] = 3 + 3
    },
    {                                             /* result is known */
        [pos(A, X)] = 0 + 3, [pos(A, Y)] = 3 + 1, [pos(A, Z)] = 6 + 2,
        [pos(B, X)] = 0 + 1, [pos(B, Y)] = 3 + 2, [pos(B, Z)] = 6 + 3,
        [pos(C, X)] = 0 + 2, [pos(C, Y)] = 3 + 3, [pos(C, Z)] = 6 + 1
    }
};

static int *parse(int *res)
{
    size_t alloc = 0;
    char *buf = NULL;
    ssize_t buflen;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        for (int i = 0; i < 2; ++i)
            res[i] += outcome[i][pos(buf[0] - 'A', buf[2] - 'X')];
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
