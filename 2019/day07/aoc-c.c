/* aoc-c.c: Advent of Code 2019, day 7 parts 1 & 2
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

/*  operators codes
 */
typedef enum {
    ADD    = 1, MUL    = 2,                       /* CALC: add and mult */
    INP    = 3, OUT    = 4,                       /* I/O: input and output value */
    JMP_T  = 5, JMP_F  = 6,                       /* JUMPS: jump if true / if false */
    SET_LT = 7, SET_EQ = 8,                       /* COND SETS: set if true/false */
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

typedef struct input {
    int val;
    struct list_head list;
} input_t;

#define MAXOPS   1024
typedef struct {
    int length;                                   /* total program length */
    int cur;                                      /* current position */
    struct list_head input;                       /* process input queue */
    int mem [MAXOPS];                             /* should really be dynamic */
} program_t;

static ops_t ops[] = {
    [ADD]    = { ADD,    4 },    [MUL]    = { MUL,    4 },
    [INP]    = { INP,    2 },    [OUT]    = { OUT,    2 },
    [JMP_T]  = { JMP_T,  3 },    [JMP_F]  = { JMP_F,  3 },
    [SET_LT] = { SET_LT, 4 },    [SET_EQ] = { SET_EQ, 4 },
    [HLT]    = { HLT,    1 }
};


static int _flag_pow10[] = {1, 100, 1000, 10000};
#define OP(p, n)           ((p->mem[n]) % 100)
#define ISDIRECT(p, n, i)  ((((p->mem[n]) / _flag_pow10[i]) % 10))
#define DIRECT(p, i)       ((p)->mem[i])
#define INDIRECT(p, i)     (DIRECT(p, DIRECT(p, i)))

#define peek(p, n, i)      (ISDIRECT(p, n, i)? DIRECT(p, n + i): INDIRECT(p, n + i))
#define poke(p, n, i, val) do {       \
        INDIRECT(p, n + i) = val; }   \
    while (0)


static pool_t *pool_input;
static __always_inline int prg_add_input(program_t *prg, int in)
{
    input_t *input = pool_get(pool_input);
    input->val = in;
    list_add_tail(&input->list, &prg->input);
    return in;
}

static __always_inline int prg_get_input(program_t *prg, int *out)
{
    input_t *input = list_first_entry_or_null(&prg->input, input_t, list);
    if (!input)
        return 0;
    *out = input->val;
    list_del(&input->list);
    pool_add(pool_input, input);
    return 1;
}

/**
 * permute - get next permutation of an array of integers
 * @len:   length of array
 * @array: address of array
 *
 * Algorithm: lexicographic permutations
 * https://en.wikipedia.org/wiki/Permutation#Generation_in_lexicographic_order
 * Before the initial call, the array must be sorted (e.g. 0 2 3 5)
 *
 * Return: 1 if next permutation was found, 0 if no more permutation.
 *
 */
static int permute_next(int len, int *array)
{
    int k, l;

    /* 1. Find the largest index k such that a[k] < a[k + 1] */
    for (k = len - 2; k >= 0 && array[k] >= array[k + 1]; k--)
        ;
    /*  No more permutations */
    if (k < 0)
        return 0;
    /* 2. Find the largest index l greater than k such that a[k] < a[l] */
    for (l = len - 1; array[l] <= array[k]; l--)
        ;
    /* 3. Swap the value of a[k] with that of a[l] */
    swap(array[k], array[l]);
    /* 4. Reverse sequence from a[k + 1] up to the final element */
    for (l = len - 1, k++; k < l; k++, l--)
        swap(array[k], array[l]);
    return 1;
}

static int run(program_t *p, int *end)
{
    int out = -1;
    while (1) {
        int op = OP(p, p->cur), cur = p->cur, input;

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
    int phase1[] = {0, 1, 2, 3, 4}, phase2[] = {5, 6, 7, 8, 9}, *phase;
    int opt, max = 0, part = 1;
    program_t p = { 0 }, prg[5];

    while ((opt = getopt(ac, av, "d:p:o:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'o':
                for (ulong i = 0; i < strlen(optarg); ++i)
                    phase1[i] = optarg[i] - '0';
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

    pool_input = pool_create("input", 128, sizeof(input_t));

    if (optind < ac)
        return usage(*av);

    phase = part == 1? phase1: phase2;
    parse(&p);

    do {
        int out = 0, end = 0;
        /* reset programs initial state, and add phase to their input
         */
        for (unsigned i = 0; i < ARRAY_SIZE(prg); ++i) {
            prg[i] = p;
            INIT_LIST_HEAD(&prg[i].input);
            prg_add_input(&prg[i], phase[i]);
        }

        /* run the 5 processes in order (0, 1, 2, 3, 4, 0, 1, etc...),
         * until end flag is set by the process 4 (HLT instruction)
         */
        while (!end) {
            for (int i = 0; i < 5; ++i) {
                /* add last process output in current process input queue
                 */
                prg_add_input(&prg[i], out);
                out = run(&prg[i], &end);
            }
        }
        max = max(max, out);
    } while (permute_next(5, phase));

    printf("%s : res=%d\n", *av, max);
    pool_destroy(pool_input);
    exit(0);
}
