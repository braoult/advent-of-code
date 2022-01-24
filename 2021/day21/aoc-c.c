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

/* Possible rolls in part 2:
 * 1,1,1=3 2,1,1=4 3,1,1=5
 * 1,1,2=4 2,1,2=5 3,1,2=6
 * 1,1,3=5 2,1,3=6 3,1,3=7
 *
 * 1,2,1=4 2,2,1=5 3,2,1=6
 * 1,2,2=5 2,2,2=6 3,2,2=7
 * 1,2,3=6 2,2,3=7 3,2,3=8
 *
 * 1,3,1=5 2,3,1=6 3,3,1=7
 * 1,3,2=6 2,3,2=7 3,3,2=8
 * 1,3,3=7 2,3,3=8 3,3,3=9
 */
typedef struct {
    u16 roll;
    u16 count;
} multi_t;

multi_t multi[] = {
    { 3, 1 },
    { 4, 3 },
    { 5, 6 },
    { 6, 7 },
    { 7, 6 },
    { 8, 3 },
    { 9, 1 },
};
#define NUNIVERSE (sizeof multi / sizeof(*multi))

static pool_t *pool_queue;

typedef struct {
    int score;
    int pos;
} player_t;

static ulong count1, count2;

typedef struct {
    player_t players[2];
    u64 count;
    struct list_head list_queue;
} queue_t;

static LIST_HEAD(list_queue);

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

static void enqueue(queue_t *new)
{
    log_f(2, "count=%lu score1=%d pos1=%d score2=%d pos2=%d\n",
          new->count,
          new->players[0].score, new->players[0].pos,
          new->players[1].score, new->players[1].pos);
    list_add_tail(&new->list_queue, &list_queue);
}

static queue_t *dequeue()
{
    queue_t *ret = NULL;

    if (!list_empty(&list_queue)) {
        struct list_head *ptr;
        ptr = list_queue.next;
        list_del(ptr);
        ret = list_entry(ptr, queue_t, list_queue);
        log_f(2, "count=%lu score1=%d pos1=%d score2=%d pos2=%d\n",
              ret->count,
              ret->players[0].score, ret->players[0].pos,
              ret->players[1].score, ret->players[1].pos);
    }

    return ret;
}

static u64 part1()
{
    while (1) {
        if (player_turn(0) >= 1000)
            return players[1].score * nrolls;
        if (player_turn(1) >= 1000)
            return players[0].score * nrolls;
    }
    return 1;                                     /* not reached */
}

static u64 part2()
{
    queue_t *queue, *new;

    pool_queue = pool_create("queue", 1024, sizeof(queue_t));
    /* first queue seed */
    queue = pool_get(pool_queue);
    queue->players[0].score = 0;
    queue->players[0].pos = players[0].pos;
    queue->players[1].score = 0;
    queue->players[1].pos = players[1].pos;
    queue->count = 1;
    enqueue(queue);

    while ((queue = dequeue())) {
        //log_i(1, "C1=%lu\n", queue->count);
        for (uint i = 0; i < NUNIVERSE; ++i) {
            log_i(2, "%d/%lu: roll1=%d count1=%d\n", i, NUNIVERSE,
                  multi[i].roll, multi[i].count);
            /* calculate new position */
            int pos1 = (queue->players[0].pos + multi[i].roll) % 10;
            if (!pos1)
                pos1 = 10;
            int score1 = queue->players[0].score + pos1;
            log_i(2, "pos1=%d score1=%d\n", pos1, score1);
            if (score1 >= 21) {
                count1 += queue->count * multi[i].count;
                log_i(2, "count1=%lu\n", count1);
            } else {
                for (uint j = 0; j < NUNIVERSE; ++j) {
                    log_i(3, "%d/%lu: roll2=%d count2=%d\n", j, NUNIVERSE,
                          multi[j].roll, multi[j].count);
                    int pos2 = (queue->players[1].pos + multi[j].roll) % 10;
                    if (!pos2)
                        pos2 = 10;
                    int score2 = queue->players[1].score + pos2;
                    log_i(3, "pos2=%d score2=%d\n", pos2, score2);
                    if (score2 >= 21) {
                        count2 += queue->count * multi[i].count *
                            multi[j].count;
                        log_i(3, "count2=%lu\n", count2);
                    } else {                      /* game continues */
                        new = pool_get(pool_queue);
                        //log_i(1, "C111=%lu\n", queue->count);

                        *new = *queue;
                        new->players[0].pos = pos1;
                        new->players[0].score = score1;
                        new->players[1].pos = pos2;
                        new->players[1].score = score2;
                        new->count = queue->count * multi[i].count *
                            multi[j].count;
                        log_i(3, "C1=%lu C2=%lu\n", queue->count, new->count);
                        enqueue(new);
                    }
                }

            }

        }
    }
    return count1 > count2? count1: count2;
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
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());

    exit(0);
}
