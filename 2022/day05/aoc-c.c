/* aoc-c.c: Advent of Code 2022, day 5
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
#include "list.h"
#include "pool.h"
#include "aoc.h"

#define MAXSTACKS 128

typedef struct crate {
    int code;
    struct list_head list;
} crate_t;

static pool_t *pool_crates;

static void move_stack(struct list_head *stacks, int from, int to, int nb)
{
    for (int i = 0; i < nb; ++i)
        /* Attention !! we should test if &stacks[from - 1] is not empty here */
        list_move(stacks[from - 1].next, &stacks[to - 1]);
}

static void move_bulk(struct list_head *stacks, int from, int to, int nb)
{
    struct list_head *tail;
    LIST_HEAD(bulk);

    int count = 1;
    list_for_each(tail, &stacks[from - 1])
        if (count++ == nb)
            break;
    list_cut_position(&bulk, &stacks[from - 1], tail);
    list_splice(&bulk, &stacks[to - 1]);
}

static void parse(struct list_head *stacks, int *nstacks, int part)
{
    size_t alloc = 0;
    char *buf = NULL;
    ssize_t buflen;
    int state = 0;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;

        if (!buflen || buf[1] == '1') {
            state = 1;
            continue;
        }
        if (!state) {                             /* stacks */
            int stack = 0, pos, code;

            for(pos = 1; pos < buflen; pos += 4, stack++) {
                if ((code = buf[pos]) != ' ') {
                    crate_t *new = pool_get(pool_crates);
                    new->code = code;
                    list_add_tail(&new->list, stacks + stack);
                    if (stack == *nstacks)
                        ++*nstacks;
                }
            }
        } else {                                  /* moves */
            int nb, from, to;
            if ((sscanf(buf, "%*s%d%*s%d%*s%d", &nb, &from, &to)) == 3) {
                if (part == 1)
                    move_stack(stacks, from, to, nb);
                else
                    move_bulk(stacks, from, to, nb);
            }
        }
    }
}

static char *build_res(struct list_head *stacks, int nstacks, char *res)
{
    int i;
    for (i = 0; i < nstacks; ++ i) {
        crate_t *cur = list_first_entry(&stacks[i], crate_t, list);
        res[i] = cur->code;
    }
    res[i] = 0;
    return res;
}

int main(int ac, char **av)
{
    char res[MAXSTACKS + 1];
    int part = parseargs(ac, av);
    struct list_head stacks[MAXSTACKS];
    int nstacks = 0;

    for (ulong ul = 0; ul < ARRAY_SIZE(stacks); ++ul)
        INIT_LIST_HEAD(&stacks[ul]);
    pool_crates = pool_create("crates", 64, sizeof (crate_t));

    parse(stacks, &nstacks, part);
    printf("%s: res=%s\n", *av, build_res(stacks, nstacks, res));
    pool_destroy(pool_crates);
    exit(0);
}
