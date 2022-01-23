/* aoc-c.c: Advent of Code 2021, day 21 parts 1 & 2
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
#include <errno.h>

#include "pool.h"
#include "debug.h"
#include "bits.h"
#include "list.h"

typedef struct {
    int score;
    int pos;
} player_t;

static player_t players[2];

static int nrolls = 0;

static inline int do_roll()
{
    static int roll = 0;
    int res = 0;

    for (int i = 0; i < 3; ++i) {
        res += roll++ % 100 + 1;
        nrolls++;
    }
    return res % 10;
}

static int player_turn(int player)
{
    int roll;

    roll = do_roll();
    players[player].pos += roll;
    if (!(players[player].pos % 10))
        players[player].pos = 10;
    else
        players[player].pos %= 10;

    players[player].score += players[player].pos;
    log_f(1, "player %d: nrolls=%d roll=%d pos=%d score=%d\n", player+1,
          nrolls, roll,
          players[player].pos, players[player].score);
    return players[player].score;
}

static int part1()
{
    while (1) {
        if (player_turn(0) >= 1000)
            return players[1].score * nrolls;
        if (player_turn(1) >= 1000)
            return players[0].score * nrolls;
    }

    return 1;
}

static int part2()
{
    return 1;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

static void read_input()
{
    scanf("Player %*c starting position: %d\n", &players[0].pos);
    scanf("Player %*c starting position: %d\n", &players[1].pos);
}

int main(int ac, char **av)
{
    int opt, part = 1;

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

    read_input();
    printf("player1: %d\n", players[0].pos);
    printf("player1: %d\n", players[1].pos);
    printf("%s : res=%d\n", *av, part == 1? part1(): part2());

    exit(0);
}
