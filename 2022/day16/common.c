/* common.c: Advent of Code 2022, common functions
 *
 * Copyright (C) 2022-2023 Bruno Raoult ("br")
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

#include "aoc.h"
#include "debug.h"

static int _testmode = 0;

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-t][-d debug_level] [-p part] [-i input]\n", prg);
    return 1;
}

int testmode(void)
{
    return _testmode;
}

int parseargs(int ac, char **av)
{
    int opt, part = 1;

    while ((opt = getopt(ac, av, "td:p:")) != -1) {
        switch (opt) {
            case 't':
                _testmode = 1;
                break;
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                part = atoi(optarg);
                if (part < 1 || part > 2)
                    return usage(*av);
                break;
            case 'i':

            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);
    return part;
}
