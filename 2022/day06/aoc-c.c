/* aoc-c.c: Advent of Code 2022, day 6
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
#include "aoc.h"

struct msg {
    char *data;
    size_t len;
};

static struct msg *parse(struct msg *msg)
{
    size_t alloc = 0;

    msg->data=NULL;
    msg->len = getline(&msg->data, &alloc, stdin);
    msg->data[--msg->len] = 0;
    return msg;
}

static int solve(struct msg *msg, int marklen)
{
    char *pcur = msg->data, *pnext = pcur;
    int len = msg->len, lmark = 0;

    for (; pnext < msg->data + len && lmark < marklen; lmark = ++pnext - pcur) {
        for (int j = 0; j < lmark; ++j) {         /* compare with prev marker chars */
            if (*(pcur+j) == *pnext) {            /* check new char with cur marker */
                pcur += j + 1;                    /* move marker to after dup char */
                goto nextchar;
            }
        }
    nextchar: ;
    }
    return pnext - msg->data;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    struct msg msg;

    printf("%s: res=%d\n", *av, solve(parse(&msg), part == 1? 4:14));
    free(msg.data);
    exit(0);
}
