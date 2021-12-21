/* aoc-c.c: Advent of Code 2021, day 16 parts 1 & 2
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
#include <ctype.h>
#include <stdint.h>

#include "debug.h"
#include "pool.h"
#include "bits.h"
#include "list.h"

#define TYPE_SUM     0
#define TYPE_PRODUCT 1
#define TYPE_MIN     2
#define TYPE_MAX     3
#define TYPE_VALUE   4
#define TYPE_GREATER 5
#define TYPE_LESS    6
#define TYPE_EQUAL   7

#define MAX_STACK 128                             /* max stack for operators input */
static u64 totversions;                           /* sum of versions */

static u64 bitval(char *p, int length)
{
    u64 val = 0;
    for (; length; length--, p++) {
        u8 b = *p -'0';
        val += b << (length - 1);
    }
    return val;
}

static u64 decode_value(char *bits, char **end)
{
    u64 val = 0;

    for (u64 tmp = 16; tmp & 16; bits += 5) {
        tmp = bitval(bits, 5);
        val = val << 4 | (tmp & 15);
    }
    *end = bits;
    return val;
}

static s64 decode(char *start, char **end)
{
    u8 type;
    s64 count, result = 0;
    s64 stack[MAX_STACK];
    int nstack = 0;

    totversions += bitval(start, 3);              /* decode version & type */
    type = bitval(start + 3, 3);
    start += 6;

    if (type == TYPE_VALUE)
        return decode_value(start, end);

    switch (*start++) {
        case '0':
            count = bitval(start, 15);
            start += 15;
            *end = start;
            log(3, "nbits = %lu\n", count);
            while ((*end - start) < count) {
                stack[nstack++] = decode(*end, end);
                //log(3, "  end - start = %lu\n", *end - start);
                log(3, "  stack %d = %ld\n", nstack - 1, stack[nstack - 1]);
            }
            break;
        case '1':
            count = bitval(start, 11);
            start += 11;
            *end = start;
            log(3, "npackets = %ld\n", count);
            while (count--) {
                stack[nstack++] = decode(*end, end);
                log(3, "  stack %d = %ld\n", nstack - 1, stack[nstack - 1]);
            }

    }
    switch (type) {
        case TYPE_SUM:
            log(3, "OP=sum\n");
            while (--nstack > -1)
                result += stack[nstack];
            break;
        case TYPE_PRODUCT:
            result = 1;                           /* forgot this one ;-) */
            log(3, "OP=mult\n");
            while (--nstack > -1)
                result *= stack[nstack];
            break;
        case TYPE_MIN:
            result = INT64_MAX;
            log(3, "OP=min\n");
            while (--nstack > -1)
                if (stack[nstack] < result)
                    result = stack[nstack];
            break;
        case TYPE_MAX:
            log(3, "OP=max\n");
            while (--nstack > -1)
                if (stack[nstack] > result)
                    result = stack[nstack];
            break;
        case TYPE_GREATER:
            log(3, "OP=gt\n");
            result = stack[0] > stack[1]? 1: 0;
            break;
        case TYPE_LESS:
            log(3, "OP=le\n");
            result = stack[0] < stack[1]? 1: 0;
            break;
        case TYPE_EQUAL:
            log(3, "OP=eq\n");
            result = stack[0] == stack[1]? 1: 0;
    }

    //log_f(3, "bin=[%s] version=%d type=%d\n", bits, version, type);
    //log_f(3, "[%s] version=%d type=%d\n", start, version, type);
    return result;
}

/* read BITS data
 */
static char *read_input()
{
    size_t alloc = 0;
    char *buf, *bits = NULL, *p, *q, decode[2] = { 0 };
    ssize_t buflen, bits_len;

    if ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;

        bits_len = buflen * 4 + 1;
        if (!(bits = malloc(bits_len)))
            goto free;
        for (p = buf, q = bits; *p; ++p, q += 4) {
            decode[0] = *p;
            int val = strtol(decode, NULL, 16);
            sprintf(q, "%c%c%c%c", val & 8? '1': '0', val & 4? '1': '0',
                    val & 2? '1': '0', val & 1? '1': '0');
        }
    }
free:
    free(buf);
    return bits;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1;
    char *buf, *end=NULL;

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


    buf = read_input();
    s64 result = decode(buf, &end);
    printf("END: end=%lu sum_versions=%lu result=%ld\n", end - buf, totversions, result);
    //printf("%s : res=%u\n", *av, part == 1? part1(): part2());
    exit (0);
}
