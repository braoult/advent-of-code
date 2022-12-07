/* aoc-c.c: Advent of Code 2022, day 1
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

#include "plist.h"
#include "debug.h"
#include "pool.h"
#include "aoc.h"


PLIST_HEAD(plist);

static int calc_top_plist(int n)
{
    int res = 0;

    struct plist_node *node;
    plist_for_each_reverse(node, &plist) {
        res += node->prio;
        if (!--n)
            break;
    }
    return res;
}

static void parse(pool_t *pool)
{
    size_t alloc = 0;
    char *buf = NULL;
    ssize_t buflen;
    int total = 0;
    struct plist_node *node;

    while (1) {
        buflen = getline(&buf, &alloc, stdin);
        switch (buflen) {
            case 1:
            case -1:                              /* EOF */
                node = pool_get(pool);
                plist_node_init(node, total);
                plist_add(node, &plist);
                total = 0;
                if (buflen == -1)
                    goto end;
                break;
            default:
                total += atoi(buf);
        }
    }
end:
    free(buf);
    return;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);

    pool_t *pool_tot = pool_create("total", 128, sizeof(struct plist_node));
    parse(pool_tot);

    printf("%s: res=%d\n", *av, calc_top_plist(part == 1? 1: 3));
    pool_destroy(pool_tot);
    exit(0);
}
