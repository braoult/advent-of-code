/* aoc-c.c: Advent of Code 2021, day 23 parts 1 & 2
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

/* Warning: Work in progress. Should not work before I spend a lot of time
 * on it, the original "bitboard" approach for moves generation is pretty
 * difficult to program/debug
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "pool.h"
#include "debug.h"
#include "bits.h"
#include "list.h"

typedef enum {
    A = 0,
    B,
    C,
    D
} amphipod_t;

static const u64 cost[] = {
    1, 10, 100, 1000
};

/*
 * #############
 * #abcdefghijk#
 * ###l#m#n#o###
 *   #p#q#r#s#
 *   #########
 *
 * From initial position, we can go:
 *   - to a, b, d, f, h, j, k
 *   - to destination room if room unlocked
 * To unlock a room:
 *   - Room must be empty or contain only correct amphipods
 *
 *
 *
 * From a, b, d, f, h, j, k, we can go only to correct unlocked room
 *
 * Init:
 * from start position
 * bit 63: a
 * bit 62: b
 * bit 61: c
 * ...
 * bit 59: e
 * ...
 * bit 57: g
 * ...
 * bit 55: i
 * ...
 * bit 53: k
 * ...
 * bit 61 - 16 = 45: l
 * bit 59 - 16 = 43: m
 * ...
 * bit 53 - 16 = 39: o
 * ...
 * bit 61 - 32 = 29: p
 * ...
 * bit 55 - 32 = 23: s
 *
 * move up is current << 16
 * move left is current << 1
 * move right is current >> 1
 *
 * forbidden mask:
 * abcdefghijk       l m n o         p q r s
 * ...........1111111.1.1.1.111111111.1.1.1.11111111111111111111111
 * 0000000000011111110101010111111111010101011111111111111111111111
 *
 * allowed mask:
 * abcdefghijk       l m n o         p q r s
 * 1111111111100000001010101000000000101010100000000000000000000000
 * ...........1111111.1.1.1.111111111.1.1.1.11111111111111111111111
 * 0000000000011111110101010111111111010101011111111111111111111111
 */
#define FORBIDDEN 0x1FD57FD57FFFFFUL
#define ALLOWED   (~FORBIDDEN)

typedef struct pos {
    u64 amp[4];                                   /* bitboards */
    int moves;
    u64 cost;
    u64 occupied;
    u64 available;                                /* ALLOWED & ~occupied */
    struct list_head list_pos;
} pos_t;

pool_t *pool_pos;
pos_t *pos_zero;

static inline u64 get_occupancy(pos_t *pos)
{
    return pos->amp[A] | pos->amp[B] | pos->amp[C] | pos->amp[D];
}

/* here we allow mask to be in forbidden places
 */
static void mask_print(u64 bits)
{
    //log_f(1, "A=%lx %lu\n", pos->amp[A], pos->amp[A]);
    //log(1, "               B=%lx %lu\n", pos->amp[B], pos->amp[B]);
    //log(1, "               C=%lx %lu\n", pos->amp[C], pos->amp[C]);
    //log(1, "               D=%lx %lu\n", pos->amp[D], pos->amp[D]);

    u64 tmp;
    int count;
    log(4, "%lu: popcount=%d ", bits, popcount64(bits));
    bit_for_each64_2(count, tmp, bits) {
        log(4, " %d", count);
    }
    log(4, "\n");

    log(1, "#############\n#");
    for (int i = 63; i > 21; --i) {
        u64 mask = 1UL << i;
            //log(1, "bit = %d A=%d B=%d C=%d D=%d\n");
        if (bits & mask)
            printf("X");
        else if (FORBIDDEN & mask)
            printf("#");
        else
            printf(".");

        if (i == 52) {                            /* hallway line */
            printf("\n");
            i -= 3;
        } else if (i == (52 - 16)) {              /* rooms top line */
            printf("\n  ");
            i -= 5;
        } else if (i == (52 - 32)) {              /* rooms bottom line */
            printf("\n  ");
            i -= 5;
        }
    }
    printf("\n  #########\n");
}

static void burrow_print(pos_t *pos)
{
    log_f(4, "A=%lx %lu\n", pos->amp[A], pos->amp[A]);
    log(4, "               B=%lx %lu\n", pos->amp[B], pos->amp[B]);
    log(4, "               C=%lx %lu\n", pos->amp[C], pos->amp[C]);
    log(4, "               D=%lx %lu\n", pos->amp[D], pos->amp[D]);

    for (int i = 0; i < 4; ++i) {
        u64 tmp;
        int count;
        log(4, "%c: popcount=%d ", i + 'A', popcount64(pos->amp[i]));
        bit_for_each64_2(count, tmp, pos->amp[i]) {
            log(4, " %d", count);
        }
        log(4, "\n");
    }

    log(1, "#############\n#");
    for (int i = 63; i > 21; --i) {
        u64 mask = 1UL << i;
        if (FORBIDDEN & mask)
            printf("#");
        else {
            //log(1, "bit = %d A=%d B=%d C=%d D=%d\n");
            if (pos->amp[A] & mask)
                printf("A");
            else if (pos->amp[B] & mask)
                printf("B");
            else if (pos->amp[C] & mask)
                printf("C");
            else if (pos->amp[D] & mask)
                printf("D");
            else
                printf(".");
        }
        if (i == 52) {                            /* hallway line */
            printf("\n");
            i -= 3;
        } else if (i == (52 - 16)) {              /* rooms top line */
            printf("\n  ");
            i -= 5;
        } else if (i == (52 - 32)) {              /* rooms bottom line */
            printf("\n  ");
            i -= 5;
        }
    }
    printf("\n  #########\n");
}

/* get a position from memory pool
 */
static pos_t *get_pos(pos_t *from)
{
    pos_t *new;
    if (!(new = pool_get(pool_pos)))
        return NULL;
    if (!from) {
        new->amp[0] = new->amp[1] = new->amp[2] = new->amp[3] = 0;
        new->moves = 0;
        new->cost = new->occupied = new->available = 0;
    } else {
        *new = *from;
    }
    INIT_LIST_HEAD(&new->list_pos);
    return new;
}

static u64 move_up(pos_t *pos, int amphipod)
{
    pos_t newpos = *pos;
    u64 new, tmp, occupied = 0;
    new = pos->amp[amphipod] << 16;

    log(1, "********* moving %c up\n", amphipod + 'A');
    for (int i = 0; i < 4; ++i) {
        occupied |= pos->amp[i];
    }
    log(1, "occupied:\n");
    mask_print(occupied);
    log(1, "current occupation:\n");
    mask_print(pos->amp[amphipod]);
    log(1, "new occupation:\n");
    mask_print(new);

    tmp = new & ALLOWED;
    if (tmp != new)
        log(1, "%c cannot go up (forbidden)\n", amphipod + 'A');
    tmp = new & ~occupied;
    if (tmp != new)
        log(1, "%c cannot go up (occupied)\n", amphipod + 'A');
    newpos.amp[amphipod] = new & ALLOWED & ~occupied;
    //burrow_print(&newpos);
    return newpos.amp[amphipod];
}

static u64 move_down(pos_t *pos, int amphipod)
{
    pos_t newpos = *pos;
    u64 new;
    log(1, "********* moving %c down\n", amphipod + 'A');
    new = pos->amp[amphipod] >> 16;
    printf("occupied\n");
    mask_print(pos->occupied);
    printf("available\n");
    mask_print(pos->available);
    newpos.amp[amphipod] = new & pos->available;
    return newpos.amp[amphipod];
}

static u64 move_left(pos_t *pos, int amphipod)
{
    pos_t newpos = *pos;
    u64 new, tmp, occupied = 0;
    new = pos->amp[amphipod] << 1;

    log(1, "********* moving %c left\n", amphipod + 'A');
    for (int i = 0; i < 4; ++i) {
        occupied |= pos->amp[i];
    }
    log(1, "occupied:\n");
    mask_print(occupied);
    log(1, "current occupation:\n");
    mask_print(pos->amp[amphipod]);
    log(1, "new occupation:\n");
    mask_print(new);

    tmp = new & ALLOWED;
    if (tmp != new)
        log(1, "%c cannot go left (forbidden)\n", amphipod + 'A');
    tmp = new & ~occupied;
    if (tmp != new)
        log(1, "%c cannot go left (occupied)\n", amphipod + 'A');
    newpos.amp[amphipod] = new & ALLOWED & ~occupied;
    //burrow_print(&newpos);
    return newpos.amp[amphipod];
}

static u64 move_right(pos_t *pos, int amphipod)
{
    pos_t newpos = *pos;
    u64 new;
    log(1, "********* moving %c right\n", amphipod + 'A');
    new = pos->amp[amphipod] >> 1;
    printf("occupied\n");
    mask_print(pos->occupied);
    printf("available\n");
    mask_print(pos->available);
    newpos.amp[amphipod] = new & ALLOWED & pos->available;
    return newpos.amp[amphipod];
}


/* minimal parsing: We just read the 3-5 lines to get
 * the amphipods location in side rooms
 */
static void read_input()
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    int line = 0;
    pos_t *pos = get_pos(NULL);

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        printf("line=%d str=%s\n", line, buf);
        if (line == 2 || line == 3) {
            u64 bit = 45 - 16 * (line - 2);
            //printf("bit = %d\n", bit);
            for (int i=0; i < 4; ++i) {
                int amp = buf[i * 2 + 3] - 'A';
                //printf("bit = %lu char = %c\n", bit, amp + 'A');
                pos->amp[amp] |= 1UL << bit;
                bit -= 2;
            }
        }
        line++;
    }
    pos->occupied = get_occupancy(pos);
    pos->available = ALLOWED & ~pos->occupied;
    burrow_print(pos);
    for (int i = 0; i < 4; ++i) {
        mask_print(move_up(pos, i));
        mask_print(move_left(pos, i));
        mask_print(move_down(pos, i));
        mask_print(move_right(pos, i));
    }
    free(buf);
}

static s64 part1()
{
    return 1;
}

static s64 part2()
{
    return 2;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
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

    pool_pos = pool_create("pos", 1024, sizeof(pos_t));

    read_input();

    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());

    exit(0);
}
