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
#include "list.h"

typedef struct {
    int len;
    char *code;
} token;

typedef struct {
    token unique[10];
    token output[4];
} code;

//#ifdef DEBUG
static void print_code(code *code)
{
    int i = 0;

    //printf("crabs=%d max=%d\n", ncrabs, crab_max);
    printf("unique: ");
    for (i = 0; i < 10; ++i)
        printf("[%d]%s ", code->unique[i].len, code->unique[i].code);
    printf("\n");
    printf("output: ");
    for (i = 0; i < 4; ++i)
        printf("[%d]%s ", code->output[i].len, code->output[i].code);
    printf("\n");
}
//#endif

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
        code.unique[i].code = token;
        code.unique[i].len = strlen(token);
        i++;
        token = strtok(NULL, " \n");
    }
    //printf("cont = %c\n", *token);
    //print_code(&code);
    i = 0;
    while ((token = strtok(NULL, " \n"))) {
        //printf("output %d = [%s]\n", i, token);
        code.output[i].code = token;
        code.output[i].len = strlen(token);
        i++;
    }
    if (i != 4)
        printf("output = %d\n", i);

    free(buf);
    return &code;
}

static u64 doit(int part)
{
    code *code;
    int res = 0;

    while ((code = read_code())) {
        if (part == 1) {
            for (int i = 0; i < 4; ++i) {
                int len = code->output[i].len;
                /* digits: 1           4           7           8 */
                if (len == 2 || len == 4 || len == 3 || len == 7) {
                    printf("%d : %s\n", len, code->output[i].code);
                    res++;
                }
            }
        }
    }
    return res;
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
