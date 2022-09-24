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

static struct {
    int cur;
    int length;
    int val[10];
} input, output;

typedef enum {
    ADD = 1,
    MUL = 2,
    INP = 3,
    OUT = 4,
    JMP_T = 5,
    JMP_F = 6,
    SET_LT = 7,
    SET_EQ = 8,
    HLT = 99
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
    [JMP_T]  = { JMP_T,  2, 0, __stringify(JMP_T) },
    [JMP_F]  = { JMP_F,  2, 0, __stringify(JMP_F) },
    [SET_LT] = { SET_LT, 3, 4, __stringify({SET_LT) },
    [SET_EQ] = { SET_EQ, 3, 4, __stringify(SET_EQ) },
    [HLT] = { HLT, 0, 1, __stringify(HLT) }
};

union mem {
    struct {
        u8 op: 8;
        u8 nargs;
        u8 flags: 8;
    };
    uint instr;
    int param;
    int *ind;
};

#define MAXOPS 1024
struct program {
    int length;                                   /* total program length */
    int cur;                                      /* current position */
    union mem mem[MAXOPS];                        /* should really be dynamic */
};

static int _flag_pow10[] = {1, 100, 1000, 10000};
#define OP(p, n)        ((p->mem[n].instr) % 100)
#define MODE(p, n, i)   (((p->mem[n].instr) / _flag_pow10[i]) % 10)

static int print_opcode(struct program *p, int pos)
{
    int op = OP(p, pos);
    if (ops[op].op) {
        int nargs = ops[op].len;

        log(3, "%03d [%2d][%6s]\t", pos, op, ops[op].mnemo);
        for (int i = 1; i <= nargs; ++i) {
            int direct = p->mem[pos + i].param;
            if (i > 1)
                log(3, ", ");
            if (i < 3 && !MODE(p, pos, i)) {
                log(3, "*[%d]=", direct);
                int val = p->mem[direct].param;
                log(3, "%d", val);
            } else {
                log(3, "%d", direct);
            }
        }
        log(3, "\n");
        return nargs;
    } else {
        log(3, "%03d      \t", pos);
        log(3, "%d\n", p->mem[pos].param);
        return 0;
    }
}

static void print_program_codes(struct program *p)
{
    log(3, "program codes: length=%d\n", p->length);
    for (int i = 0; i < p->length; ++i)
        log(3, "%d ", p->mem[i].param);
    log(3, "\n");
}

static void print_program(struct program *p)
{
    print_program_codes(p);
    log(3, "program: length=%d\n", p->length);
    for (int i = 0; i < p->length; ++i) {
        i += print_opcode(p, i);
    }
}

#define INDIRECT(p, n) (p->mem[n].param)

/**
 */
static int peek(struct program *p, int pos, int arg)
{
    int flag = MODE(p, pos, arg);
    int ret = p->mem[pos + arg].param;
    if (!flag)
        ret = p->mem[ret].param;
    return ret;
}

static int poke(struct program *p, int pos, int arg, int val)
{
    int addr = p->mem[pos + arg].param;
    p->mem[addr].param = val;
    return val;
}

static int run(struct program *p)
{
    while (1) {
        int op = OP(p, p->cur);
        int n1, n2;
        if (!(ops[op].op)) {
            fprintf(stderr, "PANIC: illegal instruction %d at %d.\n", op, p->cur);
            return -1;
        }
        log(3, "OP=%s\n", ops[op].mnemo?  ops[op].mnemo: "UNKNOWN");
        switch (op) {
            case ADD:
                n1 = peek(p, p->cur, 1);
                n2 = peek(p, p->cur, 2);
                poke(p, p->cur, 3, n1 + n2);
                break;
            case MUL:
                n1 = peek(p, p->cur, 1);
                n2 = peek(p, p->cur, 2);
                poke(p, p->cur, 3, n1 * n2);
                break;
            case INP:
                poke(p, p->cur, 1, input.val[input.cur++]);
                break;
            case OUT:
                output.val[output.length++] =  peek(p, p->cur, 1);
                break;
            case JMP_T:
                n1 = peek(p, p->cur, 1);
                n2 = peek(p, p->cur, 2);
                if (n1)
                    p->cur = n2;
                else
                    p->cur += ops[op].len + 1;
                break;
            case JMP_F:
                n1 = peek(p, p->cur, 1);
                n2 = peek(p, p->cur, 2);
                if (!n1)
                    p->cur = n2;
                else
                    p->cur += ops[op].len + 1;
                break;
            case SET_LT:
                n1 = peek(p, p->cur, 1);
                n2 = peek(p, p->cur, 2);
                poke(p, p->cur, 3, n1 < n2 ? 1: 0);
                break;
            case SET_EQ:
                n1 = peek(p, p->cur, 1);
                n2 = peek(p, p->cur, 2);
                poke(p, p->cur, 3, n1 == n2 ? 1: 0);
                break;
            case HLT:
                return p->mem[0].param;
            default:
                fprintf(stderr, "unknown error\n");
                exit (1);
        }
        p->cur += ops[op].next;
    }
    return -1;
}

static struct program *parse()
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL, *token;
    int input;
    struct program *prog = NULL;

    if ((buflen = getline(&buf, &alloc, stdin)) <= 0) {
        fprintf(stderr, "error reading file.\n");
        goto end;
    }

    if (!(prog = calloc(1, sizeof(struct program)))) {
        fprintf(stderr, "cannot allocate program.\n");
        goto freebuf;
    }
    for (token = strtok(buf, ","); token; token = strtok(NULL, ",")) {
        if (prog->length >= MAXOPS - 1) {
            fprintf(stderr, "overflow !\n");
            free(prog);
            prog = NULL;
            goto freebuf;
        }
        input=atoi(token);
        prog->mem[prog->length++].param = input;
    }
freebuf:
    free(buf);
end:
    return prog;
}

static int part1(struct program *p)
{
    if (!input.length)
        input.val[input.length++] = 1;
    run(p);
    return output.val[output.length - 1];
}

static int part2(struct program *p)
{
    if (!input.length)
        input.val[input.length++] = 5;
    run(p);
    return output.val[output.length - 1];
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part] [-i input]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1;
    struct program *p;

    while ((opt = getopt(ac, av, "d:p:i:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'i':
                input.val[input.length++] = atoi(optarg);
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
    p = parse();
    print_program(p);
    printf("%s : res=%d\n", *av, part == 1? part1(p): part2(p));
    exit (0);
}
