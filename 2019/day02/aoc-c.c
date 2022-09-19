/* aoc-c.c: Advent of Code 2019, day 2 parts 1 & 2
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

#include "debug.h"

typedef enum {
    ADD = 1,
    MUL = 2,
    RET = 99
} opcode_t;

#define MAXOPS 1024
struct program {
    int length;                                   /* total program length */
    int cur;                                      /* current position */
    int ops[MAXOPS];                              /* should really be dynamic */
};

#define OP(p) ((p)->ops + (p)->cur)

#define A1(p) ((p)->ops + *((p)->ops + (p)->cur + 1))
#define A2(p) ((p)->ops + *((p)->ops + (p)->cur + 2))
#define A3(p) ((p)->ops + *((p)->ops + (p)->cur + 3))

static int run(struct program *prog)
{
    while (1) {
        opcode_t opcode = *OP(prog);
        switch (opcode) {
            case ADD:
                *A3(prog) = *A1(prog) + *A2(prog);
                break;
            case MUL:
                *A3(prog) = *A1(prog) * *A2(prog);
                break;
            case RET:
                return prog->ops[0];
            default:
                fprintf(stderr, "wrong opcode %d at %d.\n", opcode, prog->cur);
                exit (1);
        }
        prog->cur += 4;
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
        prog->ops[prog->length++] = input;
    }
freebuf:
    free(buf);
end:
    return prog;
}

static int part1(struct program *p)
{
    p->ops[1] = 12;
    p->ops[2] = 2;

    return run(p);
}

static int part2(struct program *p)
{
    struct program work;

    for (int noun = 0; noun < 100; ++noun) {
        for (int verb = 0; verb < 100; ++verb) {
            work = *p;
            work.ops[1] = noun;
            work.ops[2] = verb;
            if (run(&work) == 19690720)
                return noun * 100 + verb;
        }
    }
    return -1;
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
    p = parse();
    printf("%s : res=%d\n", *av, part == 1? part1(p): part2(p));
    free(p);
    exit (0);
}
