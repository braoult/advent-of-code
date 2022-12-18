/* aoc-c.c: Advent of Code 2022, day 10
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
#include <string.h>

#include "br.h"
#include "debug.h"

#include "aoc.h"

#define MAXARGS 1                                 /* max arguments to opcode */

typedef enum opcode {                             /* available opcodes */
    ERR  = -1,
    NOOP =  0,
    ADDX
} opcode_t;

typedef struct hw {                               /* hardware */
    int tick;                                     /* clock */
    int regx;                                     /* x register */
    int signal;                                   /* signal strength */
    int alrm;                                     /* next tick signal */
    int rearm;                                    /* rearm signal value */
    void (* tickfct)(struct hw *hw);              /* tick function */
    void (* alrmfct)(struct hw *hw);              /* signal function */
    int part;                                     /* puzzle part */
} hw_t;

typedef struct op {                               /* operators indormation */
    char *mnemo;
    opcode_t opcode;
    int ticks;
    void (* fct)(hw_t *hw, int *);
    int nargs;
} op_t;

static inline void alarm1(hw_t *hw)
{
    hw->signal += hw->tick * hw->regx;
    hw->alrm = hw->rearm;
}

static inline void alarm2(hw_t *hw)
{
    putchar('\n');                                /* '.' fixes last column garbage */
    hw->alrm = hw->rearm;
}

static inline void tick1(__unused hw_t *hw)
{
}

static inline void tick2(hw_t *hw)
{
    int pos = abs(hw->tick % 40 - hw->regx - 1);
    putchar(pos < 2? '#': '.');
}

static void bootstrap(hw_t *hw, int part)
{
    hw->tick = 0;
    hw->regx = 1;
    hw->signal = 0;
    hw->alrm = part == 1? 20: 40;
    hw->rearm = 40;
    hw->alrmfct = part == 1?  alarm1: alarm2;
    hw->tickfct = part == 1?  tick1: tick2;
    hw->part = part;
}

static inline void tick(hw_t *hw)
{
    hw->tick++;
    hw->alrm--;
    hw->tickfct(hw);
    if (!hw->alrm)
        hw->alrmfct(hw);
}

static inline void ticks(hw_t *hw, int ticks)
{
    for (int i = 0; i < ticks; ++i)
        tick(hw);
}

static void do_noop(__unused hw_t *hw, __unused int *args)
{
}

static void do_addx(hw_t *hw, int *args)
{
    hw->regx += *args;
}

static op_t opcodes[] = {
    { "noop", NOOP, 1, do_noop, 0 },
    { "addx", ADDX, 2, do_addx, 1 }
};

#define NOPS ARRAY_SIZE(opcodes)

static op_t *getop(const char *mnemo)
{
    for (ulong i = 0; i < NOPS; ++i)
        if (!strcmp(mnemo, opcodes[i].mnemo))
            return opcodes + i;
    return NULL;
}

static int part1(hw_t *hw)
{
    return hw->signal;
}

static int part2(hw_t *hw)
{
    return hw->signal;
}

static int parse(hw_t *hw)
{
    size_t alloc = 0;
    char *buf = NULL;
    ssize_t buflen;
    char *token;
    op_t *op;
    int args[MAXARGS];

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if ((token = strtok(buf, " "))) {
            if ((op = getop(token))) {
                ticks(hw, op->ticks);
                for (int i = 0; i < op->nargs; ++i)
                    args[i] = atoi(strtok(NULL, " "));
                op->fct(hw, args);
            } else {
                fprintf(stderr, "Fatal: opcode [%s] not found\n", token);
                exit(1);
            }
        }
    }
    return 1;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    hw_t hw;

    bootstrap(&hw, part);
    parse(&hw);
    printf("%s: res=%d\n", *av, part == 1? part1(&hw): part2(&hw));
    exit(0);
}
