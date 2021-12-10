/* aoc-c: Advent2021 game, day 6 parts 1 & 2
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
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
#include <malloc.h>

#include "debug.h"
#include "bits.h"
#include "list.h"
#include "pool.h"

static char stack[1024];
static size_t nstack;

inline static int push(char c)
{
    stack[nstack++] = c;
    printf("push(%c)\n", c);
    return c;
}

inline static int pop()
{
    int c;
    if (!nstack)
        return 0;
    c = stack[--nstack];
    printf("pop(%c)\n", c);

    return c;
}

struct match {
    char open;
    char close;
    int value;
} syntax[] = {
    { '(', ')', 3 },
    { '[', ']', 57 },
    { '{', '}', 1197 },
    { '<', '>', 25137 },
};

inline static int match(char c)
{
    int co;
    size_t i;

    if (!(co = pop()))
        return 0;
    for (i = 0; i < sizeof(syntax); ++i)
        if (syntax[i].close == c && co != syntax[i].open)
            return syntax[i].value;
    return 0;
}

static u64 part1()
{
    char *buf = NULL, *p;
    size_t alloc;
    ssize_t len;
    u64 res = 0;

    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        nstack = 0;
        buf[len - 1] = 0;
        p = buf;
        while (*p) {
            switch (*p) {
                case '{':
                case '(':
                case '[':
                case '<':
                    push(*p);
                    break;
                case '}':
                case ')':
                case ']':
                case '>':
                    res += match(*p);
            }
            p++;
        }
    }
    return res;
}

static u64 part2()
{
    return 1;
}

static u64 doit(int part)
{
    return part == 1? part1(): part2();
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    u32 exercise = 1;
    u64 res;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                exercise = atoi(optarg);
                if (exercise < 1 || exercise > 2)
                    return usage(*av);
                break;
            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    res = doit(exercise);
    printf("%s : res=%lu\n", *av, res);
    exit (0);
}
