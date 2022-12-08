/* aoc-c.c: Advent of Code 2022, day 3
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

#include "aoc.h"

static int parse(int part)
{
    int res = 0, val[4];

    while (scanf("%d-%d,%d-%d", val, val+1, val+2, val+3) == 4) {
        if (part == 1) {
            if ( (val[0] >= val[2] && val[1] <= val[3] ) ||
                 (val[0] <= val[2] && val[1] >= val[3] ) ) {
                res++;
            }
        } else {
            if ( (val[0] >= val[2] && val[0] <= val[3] ) ||
                 (val[0] <= val[2] && val[1] >= val[2] ) ) {
                res++;
            }
        }
    }
    return res;
}

int main(int ac, char **av)
{
    printf("%s: res=%d\n", *av, parse(parseargs(ac, av)));
    exit(0);
}
