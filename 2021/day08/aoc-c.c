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

/* bitmask (last 7 bits): ...  g f e d c b a
 */
typedef u64 token;

typedef struct {
    token unique[10];
    token output[4];
} code;

#ifdef DEBUG
inline static char *bits_str(u64 c)
{
    static char str[9];

    for (int i = 7; i >= 0; --i)
        str[7 - i] = c & (1 << i) ? '1': '0';
    return str;
}

static void print_code(code *code)
{
    int i = 0;

    //printf("crabs=%d max=%d\n", ncrabs, crab_max);
    printf("unique: ");
    for (i = 0; i < 10; ++i)
        printf("[%d]%s ", popcount64(code->unique[i]), bits_str(code->unique[i]));
    printf("\n");
    printf("output: ");
    for (i = 0; i < 4; ++i)
        printf("[%d]%s ", popcount64(code->unique[i]), bits_str(code->unique[i]));
    printf("\n");
}
#endif

#define BIT(c) (1 << ((c) - 'a'))

inline static u64 a2bit(char *token)
{
    u64 res = 0;
    while (*token) {
        res |= BIT(*token);
        token++;
    }
    return res;
}

static code *read_code()
{
    int i = 0;
    static char *buf = NULL;
    char *token;
    size_t alloc = 0;
    static code code;

    if (getline(&buf, &alloc, stdin) < 0)
        return NULL;

    /* read unique segment data
     */
    token = strtok(buf, " \n");
    while (token) {
        if (*token == '|')
            break;
        code.unique[i] = a2bit(token);
        i++;
        token = strtok(NULL, " \n");
    }
    i = 0;
    while ((token = strtok(NULL, " \n"))) {
        code.output[i] = a2bit(token);
        i++;
    }

    free(buf);
    return &code;
}


static u64 part1()
{
    code *code;
    int res = 0;

    while ((code = read_code())) {
        for (int i = 0; i < 4; ++i) {
            int len = popcount64(code->output[i]);
            /* digits: 1           4           7           8 */
            if (len == 2 || len == 4 || len == 3 || len == 7)
                res++;
        }
    }
    return res;
}


static u64 part2()
{
    code *code;
    u64 bits;
    int res = 0, tmp;

    while ((code = read_code())) {
        u64 digits[10] = { 0 };

        /* find digits 1, 4, 7, 8 */
        for (int i = 0; i < 10; ++i) {
            bits = code->unique[i];
            switch (popcount64(bits)) {
                case 2:
                    digits[1] = bits;
                    code->unique[i] = 0;
                    break;
                case 3:
                    digits[7] = bits;
                    code->unique[i] = 0;
                    break;
                case 4:
                    digits[4] = bits;
                    code->unique[i] = 0;
                    break;
                case 7:
                    digits[8] = bits;
                    code->unique[i] = 0;
                    break;
            }
        }
        /* find digits 3 & 6: 2 and 1 bits in common with 1 */
        for (int i = 0; i < 10; ++i) {
            bits = code->unique[i];
            if (popcount64(bits) == 5) {
                if (popcount64(bits & digits[1]) == 2) {
                    digits[3] = bits;
                    code->unique[i] = 0;
                }
            }
            if (popcount64(bits) == 6) {
                if (popcount64(bits & digits[1]) == 1) {
                    digits[6] = bits;
                    code->unique[i] = 0;
                }
            }
        }
        /* find digits 9 and 0: 9 has 5 bits in common with 5 */
        for (int i = 0; i < 10; ++i) {
            bits = code->unique[i];
            if (popcount64(bits) == 6) {
                if (popcount64(bits & digits[3]) == 5)
                    digits[9] = bits;
                else
                    digits[0] = bits;
                code->unique[i] = 0;
            }
        }
        /* find digits 2 and 5: 2 has 4 bits in common with 9 */
        for (int i = 0; i < 10; ++i) {
            bits = code->unique[i];
            if (popcount64(bits) == 5) {
                if (popcount64(bits & digits[9]) == 4)
                    digits[2] = bits;
                else
                    digits[5] = bits;
                code->unique[i] = 0;
            }
        }
        tmp = 0;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 10; ++j) {
                if (code->output[i] == digits[j]) {
                    tmp = tmp * 10 + j;
                    break;
                }
            }
        }
        res += tmp;
    }

    return res;
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
    printf ("%s : res=%lu\n", *av, res);
    exit (0);
}
