/* aoc-c.c: Advent of Code 2019, day 9 parts 1 & 2
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

#include "br.h"
#include "bits.h"
#include "debug.h"
#include "list.h"
#include "pool.h"

#define _unused __attribute__((unused))

/*  operators codes
 */
typedef enum {
    ADD    = 1, MUL    = 2,                       /* CALC: add and mult */
    INP    = 3, OUT    = 4,                       /* I/O: input and output value */
    JMP_T  = 5, JMP_F  = 6,                       /* JUMPS: jump if true / if false */
    SET_LT = 7, SET_EQ = 8,                       /* COND SETS: set if true/false */
    ADJ_RL = 9,                                   /* ADDRESSING: adjust relative addr */
    HLT    = 99                                   /* HALT */
} opcode_t;

/**
 * ops - array of op-codes, mnemo, and number of parameters
 * @op:      An integer, the opcode
 * @length:  Next instruction offset
 */
typedef struct {
    int      op;
    u8       length;
} ops_t;

typedef struct io {
    s64 val;
    struct list_head list;
} io_t;

#define MAXOPS   2048
typedef struct {
    s64 length;                                   /* total program length */
    s64 cur;                                      /* current instruction */
    s64 rel;                                      /* current relative memory */
    struct list_head input;                       /* process input queue */
    struct list_head output;                      /* process output queue */
    s64 mem [MAXOPS];                             /* should really be dynamic */
} program_t;

static ops_t ops[] = {
    [ADD]    = { ADD,    4 },    [MUL]    = { MUL,    4 },
    [INP]    = { INP,    2 },    [OUT]    = { OUT,    2 },
    [JMP_T]  = { JMP_T,  3 },    [JMP_F]  = { JMP_F,  3 },
    [SET_LT] = { SET_LT, 4 },    [SET_EQ] = { SET_EQ, 4 },
    [ADJ_RL] = { ADJ_RL, 2 },
    [HLT]    = { HLT,    1 }
};

typedef enum {
    IND = 0,
    DIR = 1,
    REL = 2
} param_t;

static __always_inline int getop(program_t *prg, int addr)
{
    return prg->mem[addr] % 100;
}

static __always_inline param_t paramtype(program_t *prg, int addr, int param)
{
    static int _flag_pow10[] = {1, 100, 1000, 10000};
    return prg->mem[addr] / _flag_pow10[param] % 10;
}

#define DIRECT(p, i)       ((p)->mem[i])
#define INDIRECT(p, i)     (DIRECT(p, DIRECT(p, i)))
#define RELATIVE(p, i)     (DIRECT(p, DIRECT(p, i) + p->rel))

static __always_inline s64 peek(program_t *prg, s64 cur, s64 param)
{
    switch(paramtype(prg, cur, param)) {
        case IND:
            return INDIRECT(prg, cur + param);
        case REL:
            return RELATIVE(prg, cur + param);
        case DIR:
            return DIRECT(prg, cur + param);
    }
    return 0;                                     /* not reached */
}

static __always_inline void poke(program_t *prg, int cur, int param, s64 val)
{
    if (paramtype(prg, cur, param) == REL)
        RELATIVE(prg, cur + param) = val;
    else
        INDIRECT(prg, cur + param) = val;
}

static pool_t *pool_io;

static inline int prg_add_input(program_t *prg, s64 in)
{
    io_t *input = pool_get(pool_io);
    input->val = in;
    list_add_tail(&input->list, &prg->input);
    return in;
}

static inline s64 prg_add_output(program_t *prg, s64 out)
{
    io_t *output = pool_get(pool_io);
    output->val = out;
    list_add_tail(&output->list, &prg->output);
    return out;
}

static inline int prg_get_input(program_t *prg, s64 *in)
{
    io_t *input = list_first_entry_or_null(&prg->input, io_t, list);
    if (!input)
        return 0;
    *in = input->val;
    list_del(&input->list);
    pool_add(pool_io, input);
    return 1;
}

static inline _unused int prg_get_output(program_t *prg, s64 *out)
{
    io_t *output = list_first_entry_or_null(&prg->output, io_t, list);
    if (!output)
        return 0;
    *out = output->val;
    list_del(&output->list);
    pool_add(pool_io, output);
    return 1;
}

static s64 run(program_t *p, int *end)
{
    s64 out = -1, input;

    while (1) {
        int cur = p->cur;
        opcode_t op = getop(p, p->cur);

        if (!(ops[op].op)) {
            fprintf(stderr, "PANIC: illegal instruction %d at %ld.\n", op, p->cur);
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
                if (prg_get_input(p, &input))
                    poke(p, p->cur, 1, input);
                else
                    /* we need an input which is not yet avalaible, so we need
                     * to put the program in "waiting mode": We stop it (and
                     * return output value) without setting end flag.
                     */
                    goto sleep;
                break;
            case OUT:
                prg_add_output(p, out = peek(p, p->cur, 1));
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
            case ADJ_RL:
                p->rel += peek(p, p->cur, 1);
                break;
            case HLT:
                *end = 1;
            sleep:
                return out;
        }
        if (p->cur == cur)
            p->cur += ops[op].length;
    }
}

static void parse(program_t *prog)
{
    while (scanf("%ld%*c", &prog->mem[prog->length++]) > 0)
        ;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part] [-i input]\n", prg);
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

    int end = 0;
    program_t p = { 0 };

    pool_io = pool_create("i/o", 128, sizeof(io_t));

    INIT_LIST_HEAD(&p.input);
    INIT_LIST_HEAD(&p.output);
    prg_add_input(&p, part);
    parse(&p);

    printf("%s : res=%ld\n", *av, run(&p, &end));
    pool_destroy(pool_io);
    exit(0);
}
