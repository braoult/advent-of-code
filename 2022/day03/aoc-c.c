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

static char c1[2][NCHARS];                        /* char exists in set */
static char c2[3][NCHARS];
static char ctmp[NCHARS];

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


static void exists_print(char *ex)
{
    printf("found:");
    for (int i = 0; i < NCHARS; ++i) {
        if (ex[i])
            printf(" %c", POS2CHAR(i));
    }
    printf("\n");
}

static int *parse(int *res)
{
    size_t alloc = 0;
    char *buf[3] = { NULL, NULL, NULL };
    ssize_t buflen;
    int curc2 = 0;
    int foo;

    while ((buflen = getline(&buf[curc2], &alloc, stdin)) > 0) {
        buf[curc2][--buflen] = 0;
        int half = buflen / 2;
        //printf("half=%lu/%d\n", buflen, half);
        /* populate c1 */
        exists_populate(c1[0], buf[curc2], half);
        exists_populate(c1[1], buf[curc2] + half, half);
        //exists_print(c1[0]);
        //exists_print(c1[1]);g
        foo = exists_match(NULL, c1[0], c1[1]);
        res[0] += foo + 1;
        //printf("match=%c\n", POS2CHAR(exists_match(NULL, c1[0], c1[1])));
        //exit(0);
        /* populate c2 */
        //exists_init(c2[curc2]);
        exists_populate(c2[curc2], buf[curc2], buflen);

        /* calculate part 1 */

        if (!(++curc2 % 3)) {                   /* calculate part 2 */
            //printf("curc2=%d, part2\n", curc2);
            exists_match(ctmp, c2[0], c2[1]);
            foo = exists_match(NULL, ctmp, c2[2]);
            res[1] += foo + 1;
            //printf("match 2 =%c\n", POS2CHAR(foo));
            curc2 = 0;
        }
        //printf("\n");
    }
    for (int i = 0; i < 3; ++i)
        free(buf[i]);
    return res;
}


int main(int ac, char **av)
{
    int res[2] = { 0, 0 };
    printf("a=%d z=%d A=%d Z=%d\n", CHAR2POS('a'), CHAR2POS('z'),
           CHAR2POS('A'), CHAR2POS('Z'));
    printf("NCHARS=%d 0=%c 25=%c 26=%c 51=%c\n", NCHARS, POS2CHAR(0),
           POS2CHAR(25), POS2CHAR(26), POS2CHAR(51));
    //  printf("%s: res=%d\n", *av, parse(res)[parseargs(ac, av) - 1]);
    //int part = parseargs(ac, av);
    printf("%s: res=%d\n", *av, parse(res)[parseargs(ac, av) - 1]);

    exit(0);
}
