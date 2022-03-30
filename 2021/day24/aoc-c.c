/* aoc-c.c: Advent of Code 2021, day 24 parts 1 & 2
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
#include <errno.h>

#include "debug.h"
#include "bits.h"

/*
 * All 14 blocks have the same code, with a1, b1, c1 differing :
 * inp w
 * mul x 0
 * add x z                         # x = z        (0)
 * mod x 26                        # x %= 26      (0)
 *                                 #    = z % 26
 * div z 1          a1             # z = z / a1   (0)
 * add x 13         b1             # x += b1      (13)
 *                                 #    = (z % 26) + b1
 * eql x w                         # x = (x == w) (0)
 * eql x 0                         # x = !!x      (1)
 *                                 # if (w == (z % 26 + b1)
 *                                 #     x = 0
 *                                 # else
 *                                 #     x = 1
 * mul y 0
 * add y 25                        # y = 25      (25)
 * mul y x                         # y *= x       (25)
 *                                 # if (w == (z % 26 + b1)
 *                                 #     y = 0
 *
 * add y 1                         # y++          (26)
 *                                 # if (w == (z % 26 + b1)
 *                                 #     y = 1
 *                                 # else
 *                                 #     y = 26
 *
 * mul z y                         # z *= y       (0)
 *                                 # if (w != (z % 26 + b1)
 *                                 #     z *= 26
 * mul y 0
 * add y w                         # y = w
 * add y 10         c1             # y += c1      (w + 10)
 *                                 #    = w + c1
 * mul y x                         # y *= x       (w + 10) * 1
 *                                 # if (w == (z % 26 + b1)
 *                                 #    y = 0
 * add z y                         # z += y       (w + 10) * 1
 *                                 # if (w != (z % 26 + b1))
 *                                 #    z = z / a1 * 26 + w + c1
 *                                 # else
 *                                 #    z = z / a1
 *
 * So we end up with :
 * if (w == (z % 26 + b))
 *    z = z / a
 * else
 *    z = z / a * 26 + w + c
 *
 * a1 is 1 or 26. it means we can have an upper limit MAXZ at each of
 * the 14 steps : if z > MAXZ, final will not be able to end at zero.
 * MAXZ = 26^n, n being the number of A=26 from current step to 14th.
 */

struct var {
    int a, b, c;
    s64 maxz;
} var[14];

static inline s64 step(int n, int w, s64 z)
{
    return z % 26 + var[n].b - w ? z / var[n].a * 26 + w + var[n].c : z / var[n].a;
}

static void read_input()
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    int line = 0;
    u64 maxz = 1;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;

        /* each block is 18 lines */
        switch (line % 18) {
            case 4:                               /* a */
                sscanf(buf, "%*s %*s %d", &(var[line / 18].a));
                break;
            case 5:                               /* b */
                sscanf(buf, "%*s %*s %d", &(var[line / 18].b));
                break;
            case 15:                               /* c */
                sscanf(buf, "%*s %*s %d", &(var[line / 18].c));
                break;
        }
        line++;
    }
    /* adjust maxz */
    for (int i = 13; i >= 0; --i) {
        maxz *= var[i].a;
        var[i].maxz = maxz;
    }
    free(buf);
}


static char *part1(int n, s64 z, char *s)
{
    if (n == 14)
        return z? NULL: s;
    else if (z > var[n].maxz)
        return NULL;

    for(int i = 9; i >= 1; --i) {
        if (part1(n + 1, step(n, i, z), s)) {
            s[n] = '0' + i;
            return s;
        }
    }
    return NULL;
}

static char *part2(int n, s64 z, char *s)
{
    if (n == 14)
        return z? NULL: s;
    else if (z > var[n].maxz)
        return NULL;

    for (int i = 1; i <= 9; ++i) {
        if (part2(n + 1, step(n, i, z), s)) {
            s[n] = '0' + i;
            return s;
        }
    }
    return NULL;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1;
    static char res[16];

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

    read_input();

    printf("%s : res=%s\n", *av, part == 1? part1(0, 0, res): part2(0, 0, res));

    exit(0);
}
