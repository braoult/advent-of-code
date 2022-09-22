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

struct {
    int cur;
    int length;
    int val[10];
} input;

typedef enum {
    VAL = 1,                                      /* param is direct value */
    PTR = 0                                       /* param is indirect value */
} param_t;

typedef enum {
    ADD = 1,
    MUL = 2,
    INP  = 3,
    OUT = 4,
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
    char     *mnemo;
} ops[] = {
    [ADD] = { ADD, 3, __stringify(ADD) },
    [MUL] = { MUL, 3, __stringify(MUL) },
    [INP] = { INP, 1, __stringify(INP)  },
    [OUT] = { OUT, 1, __stringify(OUT) },
    [HLT] = { HLT, 0, __stringify(HLT) }
};

static void print_avail_instr()
{
    log_f(3, "ops size: %zu\n", ARRAY_SIZE(ops));
    for (uint i = 0; i < ARRAY_SIZE(ops); ++i) {
        if (ops[i].op)
            log(3, "%02u [%3s] %2d\n", i, ops[i].mnemo, ops[i].len);
    }
}

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

static union mem *parse_opcode(struct program *p, int pos, union mem *res)
{
    u8 op = p->mem[pos].instr % 100;
    int flags = p->mem[pos].instr / 100;
    int nargs = ops[op].len;

    log_f(5, "opcode=%02d, flags=%03d", op, flags);
    if (!ops[op].op) {
        log(5, "fatal: unknown opcode %d at [%03d]\n", op, pos);
        return NULL;
    }
    res->op = op;
    res->nargs = nargs;
    res->flags = 0;
    for (int i = 0; i < nargs; ++i) {
        res->flags |= flags % 10 << i;
        log(3, " flag(0)=%d %d\n", flags % 10 << i, MODE(p, pos, i + 1));
        flags /= 10;
        (res + i + 1)->param = p->mem[pos + i + 1].param;
        log(5, "\t%2d = %d\n", i+1, (res + 1)->param);
    }
    log_f(3, "%d %c%c%c %d %d %d\n", res->op,
          res->flags&4, res->flags&2, res->flags&1, (res + 1)->param,
          (res + 2)->param, (res + 3)->param);
    return res;
}

static int print_opcode(struct program *p, int pos)
{
    //union mem mem[10], *instr;
    int op = OP(p, pos);
    if (ops[op].op) {
        int nargs = ops[op].len;
        //instr->nargs;
        //int flags = instr->flags;

        log(3, "%03d [%2d][%3s]\t", pos, op, ops[op].mnemo);
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
        //p->mem[pos].op, ops[p->mem[pos].op]);
        //log_f(3, "program size: %d\n", p->length);
        //for (uint i = 0; i < ARRAY_SIZE(ops); ++i) {
        //if (ops[i].len)
        //}
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

//#define INDIRECT(p, n) (*(&p->mem[0].param + p->mem[n].param))
#define INDIRECT(p, n) (p->mem[n].param)
//#define INDIRECT(p, n)  (p->mem[_INDIRECT(p, n)].param)

//#define OP(p) ((p)->ops + (p)->cur)

//#define A1(p) ((p)->ops + *((p)->ops + (p)->cur + 1))
//#define A2(p) ((p)->ops + *((p)->ops + (p)->cur + 2))
//#define A3(p) ((p)->ops + *((p)->ops + (p)->cur + 3))

/**
 */
static int peek(struct program *p, int pos, int arg)
{
    int flag = MODE(p, pos, arg);
    int ret = p->mem[pos + arg].param;
    if (flag) {
        log_f(3, "getting direct value ");
    } else {
        log_f(3, "getting indirect (%d) value ", ret);
        ret = p->mem[ret].param;
    }
    log(3, "%d\n", ret);
    return ret;
}

static int poke(struct program *p, int pos, int arg, int val)
{
    int addr = p->mem[pos + arg].param;
    log_f(3, "val(%d)=%d\n", pos+arg, addr);
    //print_program(p);
    log_f(3, "poking %d at %d+%d=%d\n", val, pos, arg, addr);
    p->mem[addr].param = val;
    print_program(p);
    return val;
}

static int run(struct program *p)
{
    //union mem mem[10], *instr;
    while (1) {
        int op = OP(p, p->cur);
        int n1, n2;
        log_f(3, "cur position=%d\n", p->cur);
        if (!(ops[op].op)) {
            log(3, "PANIC: illegal instruction %d.\n", op);
            return -1;
        }
        log(3, "%d, cur=%d len=%d, %d, %d, %d\n", op, p->cur,
            ops[op].len, p->mem[p->cur+1].instr,
            p->mem[p->cur+2].instr, p->mem[p->cur+3].instr);

        switch (op) {
            case ADD:
                log(3, "ADD\n");
                n1 = peek(p, p->cur, 1);
                n2 = peek(p, p->cur, 2);
                poke(p, p->cur, 3, n1 + n2);
                //*A3(p) = *A1(p) + *A2(p);
                break;
            case MUL:
                log(3, "MUL\n");
                n1 = peek(p, p->cur, 1);
                n2 = peek(p, p->cur, 2);
                poke(p, p->cur, 3, n1 * n2);
                //*A3(prog) = *A1(prog) * *A2(prog);
                break;
            case HLT:
                return p->mem[0].param;
            case INP:
                poke(p, p->cur, 1, input.val[input.cur++]);
                break;
            case OUT:
                printf("***OUT: %d\n", peek(p, p->cur, 0));
                break;
            default:
                fprintf(stderr, "unknown error\n");
                exit (1);
        }
        p->cur += ops[op].len + 1;
        log_f(3, "+++ cur position=%d\n", p->cur);
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

/*
 * size_t alloc = 0;
 *     ssize_t buflen;
 *     char *buf = NULL, *token;
 *     union mem input;
 *     u8 opcode, flags, nargs;
 *     struct program *prog = NULL;
 *
 *     if ((buflen = getline(&buf, &alloc, stdin)) <= 0) {
 *         fprintf(stderr, "error reading file.\n");
 *         goto end;
 *     }
 *
 *     if (!(prog = calloc(1, sizeof(struct program)))) {
 *         fprintf(stderr, "cannot allocate program.\n");
 *         goto freebuf;
 *     }
 *     for (token = strtok(buf, ","); token; token = strtok(NULL, ",")) {
 *         if (prog->length >= MAXOPS - 1) {
 *             fprintf(stderr, "overflow !\n");
 *             free(prog);
 *             prog = NULL;
 *             goto freebuf;
 *         }
 *         input.param = atoi(token);
 *         /\* we get opcode and parameters types *\/
 *         opcode = input.param % 100;
 *         flags  = input.param / 100;
 *         //log(3, "opcode=%02d, flags=%03d", opcode, flags);
 *         if (!ops[opcode].op) {
 *             log(3, "fatal: unknown opcode %d\n", opcode);
 *             exit(1);
 *         }
 *         prog->mem[prog->length].op = opcode;
 *         /\* get op arguments *\/
 *         if ((nargs = ops[opcode].len)) {
 *             log(3, "opcode=%02d, flags=%03d\n", opcode, flags);
 *             for (int i = 0; i < nargs; ++i) {
 *                 if (!(token = strtok(NULL, ","))) {
 *                     fprintf(stderr, "overflow !\n");
 *                     free(prog);
 *                     prog = NULL;
 *                     goto freebuf;
 *                 }
 *                 prog->mem[prog->length].flags |= flags % 10 << i;
 *                 flags /= 10;
 *                 prog->mem[prog->length + 1 + i].param = atoi(token);
 *                 log(3, "\t%2d = %d\n", i+1, atoi(token));
 *             }
 *         }
 *         prog->length += nargs +1;
 *         //if (prog->length >= MAXOPS - 1) {
 *         //        if ()
 *         //prog->ops[prog->length++] = input;
 *     }
 * freebuf:
 *     free(buf);
 * end:
 *     return prog;
 * }
 */

static int part1(struct program *p)
{
    input.val[input.length++] = 1;
    run(p);
//    p->ops[1] = 12;
//    p->ops[2] = 2;

    return p->length;
}

static int part2(struct program *p)
{
    /*
     * struct program work;
     *
     * for (int noun = 0; noun < 100; ++noun) {
     *     for (int verb = 0; verb < 100; ++verb) {
     *         work = *p;
     *         work.ops[1] = noun;
     *         work.ops[2] = verb;
     *         if (run(&work) == 19690720)
     *             return noun * 100 + verb;
     *     }
     * }
     * return -1;
     */
    return p->length;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1;
    struct program *p;

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
    //print_avail_instr();
    p = parse();
    print_program(p);
    //return 0;
    //union mem mem[10], *instr;
    //instr = parse_opcode(p, p->cur, mem);
    //peek(p, mem, 0);
    //exit(0);
    part1(p);
    print_program_codes(p);
    printf("mem[0] = %d\n", p->mem[0].param);
    exit(0);
    printf("%s : res=%d\n", *av, part == 1? part1(p): part2(p));
    //free(p);
    exit (0);
}
