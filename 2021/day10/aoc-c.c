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

static char stack[1024];
static size_t nstack;

struct match {
    char open;
    char close;
    int value_corrupted;
    int value_incomplete;
} syntax[] = {
    { '(', ')', 3,     1 },
    { '[', ']', 57,    2 },
    { '{', '}', 1197,  3 },
    { '<', '>', 25137, 4 }
};

inline static int push(char c)
{
    return ((stack[nstack++] = c));
}

inline static int pop()
{
    if (!nstack)
        return 0;
    return (stack[--nstack]);
}

static s64 match_corrupted(char c)
{
    int co;
    s64 res = 0;

    if ((co = pop())) {
        for (size_t i = 0; i < sizeof(syntax) / sizeof(struct match); ++i) {
            if (syntax[i].close == c && co != syntax[i].open) {
                res = syntax[i].value_corrupted;
                break;
            }
        }
    }
    return res;
}

static s64 match_incomplete()
{
    int co;
    s64 res = 0;

    while ((co = pop())) {
        for (size_t i = 0; i < sizeof(syntax); ++i) {
            if (co == syntax[i].open) {
                res = res * 5 + syntax[i].value_incomplete;
                break;
            }
        }
    }
    return res;
}

static s64 *ins_sort(s64 *array, int n)
{
    for (int i = 1, j = 0; i < n; i++, j = i - 1) {
        s64 cur = array[i];
        while (j >= 0 && array[j] > cur) {
            array[j + 1] = array[j];
            j = j - 1;
        }
        array[j + 1] = cur;
    }
    return array;
}

static s64 doit(int part)
{
    char *buf = NULL, *p;
    size_t alloc;
    ssize_t len;
    s64 res_corrupted = 0, inc[1024], tmp;
    int ninc = 0;

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
                    if ((tmp = match_corrupted(*p))) {
                        res_corrupted += tmp;
                        goto next;
                    }
            }
            p++;
        }
        inc[ninc++] = match_incomplete(0);
    next:
    }
    free(buf);
    return part == 1? res_corrupted : ins_sort(inc, ninc)[ninc / 2];
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
    s64 res;

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
