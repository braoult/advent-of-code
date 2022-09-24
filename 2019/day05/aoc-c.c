/* aoc-c.c: Advent of Code 2019, day 5 parts 1 & 2
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

#include "br.h"
#include "bits.h"
#include "debug.h"

typedef enum {
    ADD    = 1, MUL    = 2,
    INP    = 3, OUT    = 4,
    JMP_T  = 5, JMP_F  = 6,
    SET_LT = 7, SET_EQ = 8,
    HLT    = 99
} opcode_t;

/**
 * ops - array of op-codes mnemoand number of parameters
 * @len: number of parameters
 * @mnemo: op mnemo
 */
static struct {
    opcode_t op;
    u8       len;
    u8       next;
    char     *mnemo;
} ops[] = {
    [ADD]    = { ADD,    3, 4, __stringify(ADD) },
    [MUL]    = { MUL,    3, 4, __stringify(MUL) },
    [INP]    = { INP,    1, 2, __stringify(INP) },
    [OUT]    = { OUT,    1, 2, __stringify(OUT) },
    [JMP_T]  = { JMP_T,  2, 3, __stringify(JMP_T) },
    [JMP_F]  = { JMP_F,  2, 3, __stringify(JMP_F) },
    [SET_LT] = { SET_LT, 3, 4, __stringify({SET_LT) },
    [SET_EQ] = { SET_EQ, 3, 4, __stringify(SET_EQ) },
    [HLT]    = { HLT,    0, 1, __stringify(HLT) }
};

#define MAXOPS 1024
struct program {
    int length;                                   /* total program length */
    int cur;                                      /* current position */
    int mem [MAXOPS];                             /* should really be dynamic */
};

static int _flag_pow10[] = {1, 100, 1000, 10000};
#define OP(p, n)           ((p->mem[n]) % 100)
#define ISDIRECT(p, n, i)  ((((p->mem[n]) / _flag_pow10[i]) % 10))
#define DIRECT(p, i)       ((p)->mem[i])
#define INDIRECT(p, i)     (DIRECT(p, DIRECT(p, i)))

#define peek(p, n, i)      (ISDIRECT(p, n, i)? DIRECT(p, n + i): INDIRECT(p, n + i))
#define poke(p, n, i, val) do { INDIRECT(p, n + i) = val; } while (0)

static int run(struct program *p, int in)
{
    int out = -1;
    while (1) {
        int op = OP(p, p->cur), _cur = p->cur;

        if (!(ops[op].op)) {
            fprintf(stderr, "PANIC: illegal instruction %d at %d.\n", op, p->cur);
            return -1;
        }
        switch (op) {
            case ADD:
                poke(p, p->cur, 3, peek(p, p->cur, 1) +  peek(p, p->cur, 2));
                break;
            case MUL:
                poke(p, p->cur, 3, peek(p, p->cur, 1) *  peek(p, p->cur, 2));
                break;
            case INP:
                poke(p, p->cur, 1, in);
                break;
            case OUT:
                out = peek(p, p->cur, 1);
                break;
            case JMP_T:
                if (peek(p, p->cur, 1))
                    p->cur =  peek(p, p->cur, 2);
                break;
            case JMP_F:
                if (!peek(p, p->cur, 1))
                    p->cur = peek(p, p->cur, 2);
                break;
            case SET_LT:
                poke(p, p->cur, 3, peek(p, p->cur, 1) < peek(p, p->cur, 2) ? 1: 0);
                break;
            case SET_EQ:
                poke(p, p->cur, 3, peek(p, p->cur, 1) == peek(p, p->cur, 2) ? 1: 0);
                break;
            case HLT:
                return out;
        }
        if (p->cur == _cur)
            p->cur += ops[op].next;
    }
}

static void parse(struct program *prog)
{
    while (scanf("%d%*c", &prog->mem[prog->length++]) > 0)
           ;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part] [-i input]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1, in = -1;
    struct program p = { 0 };

    while ((opt = getopt(ac, av, "d:p:i:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'i':
                in = atoi(optarg);
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
    if (in == -1)
        in = part == 1? 1: 5;
    parse(&p);
    printf("%s : res=%d\n", *av, run(&p, in));
    exit (0);
}
