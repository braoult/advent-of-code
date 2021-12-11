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
    int value_corrupted;
    int value_incomplete;
} syntax[] = {
    { '(', ')', 3, 1 },
    { '[', ']', 57, 2 },
    { '{', '}', 1197, 3 },
    { '<', '>', 25137, 4 },
        };

inline static int match_corrupted(char c)
{
    int co;
    size_t i;

    if (!(co = pop()))
        return 0;
    for (i = 0; i < sizeof(syntax); ++i)
        if (syntax[i].close == c && co != syntax[i].open)
            return syntax[i].value_corrupted;
    return 0;
}

static LIST_HEAD(incomplete_head);

inline static s64 match_incomplete(char c)
{
    int co;
    size_t i;
    s64 res = 0;

    if (c) {
        if (!(co = pop()))
            return 0;
        for (i = 0; i < sizeof(syntax); ++i)
            if (syntax[i].close == c && co != syntax[i].open)
                return syntax[i].value_corrupted;
        return 0;
    } else {
        while ((co = pop())) {
            //printf("zobi = %c\n", co);
            for (i = 0; i < sizeof(syntax); ++i) {
                if (co == syntax[i].open) {
                    res = res * 5 + syntax[i].value_incomplete;
                    printf("match c=%c res=%ld\n", co, res);
                    break;
                }
            }
        }
    }
    printf("incomplete = %ld\n", res);
    return res;
}

static s64 part1()
{
    char *buf = NULL, *p;
    size_t alloc;
    ssize_t len;
    u64 res = 0, tmp;

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
                        res += tmp;
                        goto next;
                    }
            }
            p++;
        }
    next:
    }
    return res;
}

static void ins_sort(s64 *array, int n)
{
    int i, j;
    s64 cur;

    for (i = 1; i < n; i++) {
        cur = array[i]; j = i - 1;
        while (j >= 0 && array[j] > cur) {
            array[j + 1] = array[j];
            j = j - 1;
        }
        array[j + 1] = cur;
    }
}

static void bubble(s64 *array, int n)
{
    int i, j, flag = 0;;
    s64 swap;

    for (i = 0 ; i < n - 1; i++) {
        flag = 0;
        for (j = 0 ; j < n - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                swap = array[j];
                array[j] = array[j + 1];
                array[j + 1] = swap;
                flag = 1;
            }
            if (!flag) {
                break;
            }
        }
    }
}

static s64 part2()
{
    char *buf = NULL, *p;
    size_t alloc;
    ssize_t len;
    u64 res = 0;
    s64 inc[1024];
    int ninc = 0;

    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        nstack = 0;
        buf[len - 1] = 0;
        printf("line=%s p=%d\n", buf, *buf);
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
                    if (match_corrupted(*p)) {
                        printf("corrupted\n");
                        goto next;
                    }
            }
            p++;
        }
        inc[ninc++] = match_incomplete(0);
    next:
    }
    printf("results = ");
    for (int i=0; i<ninc; ++i) {
        printf("%ld ", inc[i]);
    }
    ins_sort(inc, ninc);
    printf("\nsorted = ");
    for (int i=0; i<ninc; ++i) {
        printf("%ld ", inc[i]);
    }
    printf("\n");
    return inc[ninc / 2];
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
