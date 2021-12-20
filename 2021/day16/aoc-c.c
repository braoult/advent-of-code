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

#define MAX_SUBPACKETS 1024

#define TYPE_VAL 4

struct packet {
    u8 version;
    u8 type;
    u64 value;
};

static u64 totversions;

static u64 bitval(char *p, int length)
{
    u64 val = 0;
    for (; length; length--, p++) {
        u8 b = *p -'0';
        val += b << (length - 1);
        //printf("%c -> %d, %lu\n", *p, b, val);
    }
    //printf("%lu\n", val);
    return val;
}

static u64 decode_value(char *bits, char **end)
{
    u64 val = 0;

    for (u64 tmp = 16; tmp & 16; bits += 5) {
        tmp = bitval(bits, 5);
        val = val << 4 | (tmp & 15);
        //printf("bits =%5s  tmp=%lu xor=%lu val=%lu\n", bits, tmp, tmp & 15, val);
    }
    *end = bits;
    return val;
}

int weight[] =  {
    ['B'] = 0, ['C'] = 1, ['F'] =  2,
    ['H'] =  3, ['K'] =  4, ['N'] =  5, ['O'] =  6,
    ['P'] =  7, ['S'] =  8, ['V'] =  9
};

static struct packet decode(char *start, char **end)
{
    struct packet packet;
    char *cur;
    s64 count;

    //printf("decode(%.4s...)\n", start);
    packet.version = bitval(start, 3);
    totversions += packet.version;
    packet.type = bitval(start + 3, 3);
    log_f(3, "[%.6s...] version=%d type=%d\n", start, packet.version, packet.type);
    cur = start + 6;
    switch (packet.type) {
        case TYPE_VAL:
            packet.value = decode_value(cur, end);
            printf("val = %lu len=%lu\n", packet.value, *end - cur);
            break;
        default:
            switch (*cur++) {
                case '0':
                    count = bitval(cur, 15);
                    cur += 15;
                    *end = cur;
                    printf("nbits = %lu\n", count);
                    while ((*end - cur) < count) {
                        decode(*end, end);
                        printf("  end - cur = %lu\n", *end - cur);
                    }
                    break;
                case '1':
                    count = bitval(cur, 11);
                    cur += 11;
                    *end = cur;
                    printf("npackets = %ld\n", count);
                    while (count--) {
                        decode(*end, end);
                        printf("  count = %ld\n", count);
                    }

            }

    }
    //log_f(3, "bin=[%s] version=%d type=%d\n", bits, version, type);
    //log_f(3, "[%s] version=%d type=%d\n", start, packet.version, packet.type);
    return packet;
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

/*
static u32 doit()
{
    return 1;
}


static u32 part1()
{
    return doit();
}

static u32 part2()
{
    return doit();
}
*/

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
    decode(buf, &end);
    printf("END: end=%lu sum_versions=%lu\n", end - buf, totversions);
    //printf("%s : res=%u\n", *av, part == 1? part1(): part2());
    exit (0);
}
