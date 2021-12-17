/* aoc-c.c: Advent of Code 2021, day 14 parts 1 & 2
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
//#include "list.h"

/* possible 10 letters: B, C, F, H, K, N, O, P, S, V */
int weight[] =  {
    ['B'] = 0, ['C'] = 1, ['F'] =  2,
        ['H'] =  3, ['K'] =  4, ['N'] =  5, ['O'] =  6,
        ['P'] =  7, ['S'] =  8, ['V'] =  9
        };

int count[26];
/*struct weigth {
    char key;
    int val;
} weight1[] = {
    { 'A', -1 },
    { 'B',  0 },
    { 'C',  1 },
    { 'D', -1 },
    { 'E', -1 },
    { 'F',  2 },
    { 'G', -1 },
    { 'H',  3 },
    { 'I', -1 },
    { 'J', -1 },
    { 'K',  4 },
    { 'L', -1 },
    { 'M', -1 },
    { 'N',  5 },
    { 'O',  6 },
    { 'P',  7 },
    { 'Q', -1 },
    { 'R', -1 },
    { 'S',  8 },
    { 'T', -1 },
    { 'U', -1 },
    { 'V',  9 },
    { 'W', -1 },
    { 'X', -1 },
    { 'Y', -1 },
    { 'Z', -1 }
};
*/
#define MAX_RUN    10
#define MAX_SIZE   1024 * 10 * 2                        /* max resulting string size */

char seq[MAX_SIZE], seq2[MAX_SIZE];
int nseq;

int rules[10 * 10];
int nrules;

/* read data and create graph.
 */
static int read_input()
{
    ssize_t len;
    size_t alloc = 0;
    char *buf;
    char val[10] = { 0 };
    int pos;//, ins;

    nseq = getline(&buf, &alloc, stdin) - 1;
    buf[nseq] = 0;
    strcpy(seq, buf);
    printf("str = %d [%s]\n", nseq, seq);
    printf("empt = %ld\n", getline(&buf, &alloc, stdin));

    /* get rules */
    while ((len = getline(&buf, &alloc, stdin)) > 1) {
        //log(3, "len = %d [%s]\n", len, buf);
        sscanf(buf, "%c%c -> %c", val, val+1, val+2);
        //*val -= 'A';
        //*(val + 1) -= 'A';
        //*(val + 2) -= 'A';

        pos = weight[(int)*val] * 10 + weight[(int)*(val + 1)];
        //ins = weight[(int)*(val + 2)];
        rules[pos] = *(val + 2);
        //log(3, val);
        //log(3, "%s : %c %d %c %d %d %d pos=%d\n", val, *val, (int) *val - 'A', val[1], val[1] - 'A',
        //    weight[(int)*val] * 10, weight[(int)*(val + 1)], pos);
        log(3, "%.2s : array[%d]=%c\n", val, pos, rules[pos]);
        nrules++;
    }
    free(buf);
    return nrules;
}

static int part1()
{
    char *src, *dst;
    int key;//, last;
    unsigned int count[26] = {0};
    unsigned min, max;

    read_input();
    for (int i = 0; i < 10; ++i) {
        int j, k;

        src = i % 2? seq2: seq;
        dst = i % 2? seq: seq2;
        //savedst = dst;
        //savesrc = dst;
        //last = strlen(src) - 1;
        for (j = 0, k = 0; src[j+1]; ++j, k += 2) {
            key = weight[(int) src[j]] * 10 + weight[(int) src[j+1]];
            //(*src - 'A') * 10 + *(src+1) -'A';
            //log(3, "%d/%d: src=%s dst=%s s1=%c s2=%c key=%d\n", i, j, src, dst, *src, *(src + 1), key);
            dst[k] = src[j];
            dst[k+1] = rules[key];
            dst[k+2] = src[j+1];
            //src++;
            //dst += 3;
            //log_i(3, "j=%d k=%d dst=%s\n", j, k, dst);
        }
        log(3, "i = %d len=%d dst = %s\n", i, strlen(dst), dst);
        //log(3, "%d: src=%p dst=%p seq=%p seq2=%p\n", src, dst, seq, seq2);
    }
    for (int i=0; dst[i]; ++i)
        count[dst[i] - 'A']++;
    min = ~0;
    max = 0;
    for (unsigned i = 0; i < sizeof(count)/sizeof(*count); ++i)
        printf("count[%c]=%d\n", i + 'A', count[i]);
    for (unsigned i = 0; i < sizeof(count)/sizeof(*count); ++i) {
        if (count[i]) {
            if (count[i] < min)
                min = count[i];
            if (count[i] > max)
                max = count[i];
        }
    }
    printf("min = %i max=%u\n", min, max);
    return max - min;
}

static int part2()
{
    return 2;
}

static int doit(int part)
{
    //read_input();
    return part == 1? part1(): part2();
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1;

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

    printf("%s : res=%d\n", *av, doit(part));
    exit (0);
}
