/* aoc-c: Advent2021 game, day 2 parts 1 & 2
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

#include "debug.h"
#include "bits.h"

int ex1()
{
    u32 val;
    char buffer[80];
    s32 forward = 0, depth = 0;

    while (scanf("%s %d", buffer, &val) != EOF) {
        switch (*buffer) {
            case 'f':                             /* forward */
                forward += val;
                break;
            case 'd':                             /* down */
                depth += val;
                break;
            case 'u':                             /* up */
                depth -= val;
                break;
        }
    }
    return depth * forward;
}

int ex2()
{
    u32 val;
    char buffer[80];
    s32 aim = 0, forward = 0, depth = 0;

    while (scanf("%s %d", buffer, &val) != EOF) {
        switch (*buffer) {
            case 'f':                             /* forward */
                forward += val;
                depth += val * aim;
                break;
            case 'd':                             /* down */
                aim += val;
                break;
            case 'u':                             /* up */
                aim -= val;
                break;
        }
    }
    return depth * forward;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    u32 exercise = 1, res;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p':                             /* 1 or 2 */
                exercise = atoi(optarg);
                break;
            default:
                return usage(*av);
        }
    }

    if (optind < ac)
        return usage(*av);

    if (exercise == 1) {
        res = ex1();
        printf ("%s : res=%d\n", *av, res);
    } else {
        res = ex2();
        printf ("%s : res=%d\n", *av, res);
    }

    exit (0);
}
