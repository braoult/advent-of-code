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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "pool.h"
#include "debug.h"
#include "bits.h"
#include "list.h"

static const u32 cost[] = {
    1, 10, 100, 1000
};

#define ROOM_A    0x00000F00                      /* 000000000000111100000000 */
#define ROOM_B    0x0000F000                      /* 000000001111000000000000 */
#define ROOM_C    0x000F0000                      /* 000011110000000000000000 */
#define ROOM_D    0x00F00000                      /* 111100000000000000000000 */
#define ROOMS     0x00FFFF00                      /* 111111111111111100000000 */
#define HALLWAY   0x0000007F                      /* 000000000000000001111111 */

#define BIT(c)    (1 << (c))

#define RAND_SEED 1337                            /* seed for random generator */

static u32 rooms[4] = { ROOM_A, ROOM_B, ROOM_C, ROOM_D };
static u32 result = -1;

/* full position description
 */
typedef struct pos {
    u32 amp[4];                                   /* bitmask of amphipods positions */
    u32 occupied;                                 /* bitmask for all occupied cells */
    u32 final;                                    /* bitmask for correct position */
    u32 zobrist;                                  /* for zobrist_2() */
    u32 cost;                                     /* cost till this position */
    struct list_head list;                        /* positions list */
} pos_t;

/* zobrist hash used to ignore positions already seen
 */
typedef struct hash {
    u32 zobrist;                                  /* zobrist hash */
    u32 amp[4];                                   /* bitmask of amphipods positions */
    u32 cost;                                     /* cost for this hash */
    struct list_head list;                        /* collision list */
} hash_t;

//#define HASH_SIZE 131072
#define HASH_SIZE 65536

struct list_head hasht[HASH_SIZE];

pool_t *pool_pos;
pool_t *pool_hash;

LIST_HEAD(pos_queue);

static u32 zobrist_table[24][4];

/*
 * #############
 * #ab.c.d.e.fg#
 * ###h#i#j#k###
 *   #l#m#n#o#
 *   #p#q#r#s#
 *   #t#u#v#w#
 *   #########
 *
 * We name a-g H1-H7, h & l are A1-A2, i & m are B1 & B2, etc...
 */

/* note that to determine if hallway space is left or right,
 * we simply need to divide the room number by 4.
 * Example: Room C cells are 16-19.
 * We have: 16 / 4 = 16 >> 2 = 4, and 19 / 4 = 19 >> 2 = 4, meaning
 * that H1 (0), H2 (1), H3 (2), H4 (3) are left.
 */
enum squares {
    H1 = 0, H2, H3, H4, H5, H6, H7 = 6,           /* 0-6 */
    A1 = 8, A2, A3, A4,                           /* 8-11 */
    B1,     B2, B3, B4,                           /* 12-15 */
    C1,     C2, C3, C4,                           /* 16-19 */
    D1,     D2, D3, D4,                           /* 20-23 */
};

#define LEFT  0
#define RIGHT 1

/**
 * set bits between 2 positions, excluding MSB one:
 * setbits(2, 4) -> 0001100
 *              pos:  4 2
 */
#define setbits(i, j) (((1 << ((j) - (i))) - 1) << (i))

/* distance between position over rooms to other hallway positions.
 */
static int h2h[][7] = {
    /* H1 H2 H3 H4 H5 H6 H7 */
    {  2, 1, 1, 3, 5, 7, 8 },                     /* from virtual cell on top of A */
    {  4, 3, 1, 1, 3, 5, 6 },                     /* B */
    {  6, 5, 3, 1, 1, 3, 4 },                     /* C */
    {  8, 7, 5, 3, 1, 1, 2 }                      /* D */
};

/* get a position from memory pool
 */
static pos_t *get_pos(pos_t *from)
{
    pos_t *new;
    if (!(new = pool_get(pool_pos)))
        return NULL;
    if (!from) {
        new->amp[0] = new->amp[1] = new->amp[2] = new->amp[3] = 0;
        new->cost = new->occupied = 0;
    } else {
        *new = *from;
    }
    INIT_LIST_HEAD(&new->list);
    return new;
}

/* release a position from list
 */
static void free_pos(pos_t *pos)
{
    if (pos) {
        pool_add(pool_pos, pos);
    } else {
        exit(1);
    }
}

/* push position to stack
 */
static void push_pos(pos_t *pos)
{
    if (pos)
        list_add(&pos->list, &pos_queue);
}

/* pop a position from stack
 */
static pos_t *pop_pos()
{
    pos_t *pos;
    pos = list_first_entry_or_null(&pos_queue, pos_t, list);
    if (pos)
        list_del(&pos->list);
    return pos;
}

static void zobrist_init()
{
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 4; ++j) {
            zobrist_table[i][j] = rand();
        }
    }
}

static inline u32 zobrist_1(pos_t *pos)
{
    u32 tmp;
    int bit;
    u32 zobrist = 0;

    for (int amp = 0; amp < 4; ++amp) {
        bit_for_each32_2(bit, tmp, pos->amp[amp]) {
            zobrist ^= zobrist_table[bit][amp];
        }
    }
    return zobrist;
}

/* calculate zobrist hash from previous zobrist value
 */
static inline u32 zobrist_2(pos_t *pos, int amp, u32 from, u32 to)
{
    u32 zobrist = pos->zobrist;

    zobrist ^= zobrist_table[from][amp];
    zobrist ^= zobrist_table[to][amp];
    return zobrist;
}

static void hash_init()
{
    for (int i = 0; i < HASH_SIZE; ++i)
        INIT_LIST_HEAD(&hasht[i]);
}

/* get a position from memory pool
 */
static hash_t *get_hash(pos_t *pos)
{
    hash_t *new;
    if (!(new = pool_get(pool_hash)))
        return NULL;
    for (int i = 0; i < 4; ++i)
        new->amp[i] = pos->amp[i];
    new->cost = pos->cost;
    new->zobrist = pos->zobrist;

    INIT_LIST_HEAD(&new->list);
    return new;
}

static hash_t *hash(pos_t *pos)
{
    hash_t *cur;
    u32 hashpos, zobrist = pos->zobrist;

    hashpos = zobrist % HASH_SIZE;
    list_for_each_entry(cur, &hasht[hashpos], list) {
        if (zobrist == cur->zobrist) {
            if (pos->amp[0] == cur->amp[0] &&
                pos->amp[1] == cur->amp[1] &&
                pos->amp[2] == cur->amp[2] &&
                pos->amp[3] == cur->amp[3] &&
                pos->cost   == cur->cost) {
                return NULL;
            }
        }
    }
    cur = get_hash(pos);

    list_add(&cur->list, &hasht[hashpos]);
    return cur;
}

/* Next table shows the hallway possible moves from rooms, nearest to farthest.
 * When generating moves, this allows to stop one direction as soon as move
 * is impossible.
 * Example: Moving right from room C, if H6 is occupied, we won't try to
 * evaluate a move on H7.
 */
static int room_exit[4][2][6] = {
    { { H2, H1, -1 },                             /* room A left */
      { H3, H4, H5, H6, H7, -1 } },               /*        right */
    { { H3, H2, H1, -1 },                         /* room B left */
      { H4, H5, H6, H7, -1 } },                   /*        right */
    { { H4, H3, H2, H1, -1 },                     /* room C left */
      { H5, H6, H7, -1 } },                       /*        right */
    { { H5, H4, H3, H2, H1, -1 },                 /* room D left */
      { H6, H7, -1 } }                            /*        right */
};

/* Mask which disallow moves to hallway
 */
typedef struct {
    uint mask;
    uint dist;
} possible_move_t;

static possible_move_t moves[24][24];

/* Generate all possible moves and moves masks
 * (rooms -> hallway and hallway -> rooms)
 */
static void init_moves(void)
{
    int hallway, room, dist, pos;
    u32 mask_h, mask_r;

    for (room = A1; room <= D4; ++room) {
        pos = (room >> 2) - 2;
        for (hallway = H1; hallway <= H7; ++hallway) {
            dist = h2h[pos][hallway] + room % 4 + 1;

            /* from room to hallway */
            if (room >> 2 > hallway)              /* left */
                mask_h = setbits(hallway, room >> 2);
            else                                  /* right */
                mask_h = setbits(room >> 2, hallway + 1);
            mask_r = setbits(room & ~3, room);
            moves[room][hallway].mask = mask_r | mask_h;
            moves[room][hallway].dist = dist;

            /* from hallway to room */
            if (room >> 2 > hallway)
                mask_h = setbits(hallway + 1, room >> 2);
            else
                mask_h = setbits(room >> 2, hallway);
            mask_r = setbits(room & ~3, room + 1);
            moves[hallway][room].mask = mask_r | mask_h;
            moves[hallway][room].dist = dist;
        }
    }
}

static pos_t *newmove(pos_t *pos, int amp, u32 from, u32 to)
{
    pos_t *newpos;
    int rows = popcount32(pos->amp[0]);
    possible_move_t *move = &moves[from][to];
    u32 bit_from = BIT(from), bit_to = BIT(to);

    if (pos->cost + move->dist * cost[amp] >= result)
        return NULL;
    if (!(newpos = get_pos(pos)))
        return NULL;

    newpos->amp[amp] ^= bit_from;
    newpos->amp[amp] |= bit_to;
    newpos->occupied ^= bit_from;
    newpos->occupied |= bit_to;
    newpos->cost += move->dist * cost[amp];

    if (bit_to & ROOMS) {                         /* to room => final position */
        newpos->final |= bit_to;
        if (popcount32(newpos->final) == rows * 4) {
            if (newpos->cost < result)
                result = newpos->cost;
            free_pos(newpos);
            return NULL;
        }
    }

    newpos->zobrist = zobrist_2(newpos, amp, from, to);
    if (! hash(newpos)) {                         /* collision */
        free_pos(newpos);
        return NULL;
    }

    push_pos(newpos);
    return newpos;
}

/* generate all moves from a given position
 */
static void genmoves(pos_t *pos)
{
    int amp, cell, rows = popcount32(pos->amp[0]);
    u32 tmp;

    for (amp = 0; amp < 4; ++amp) {
        u32 cur_amp = pos->amp[amp];
        bit_for_each32_2(cell, tmp, cur_amp) {
            if (cell >= A1) {                      /* in a room */
                if (BIT(cell) & pos->final)
                    continue;
                int room = (cell >> 2) - 2;
                for (int side = LEFT; side <= RIGHT; ++side) {
                    int *d = room_exit[room][side];
                    for (; *d != -1; ++d) {
                        possible_move_t *move = &moves[cell][*d];
                        if (move->mask & pos->occupied)
                            break;
                        newmove(pos, amp, cell, *d);
                    }
                }

            } else {                              /* hallway */
                u32 room = rooms[amp], found = 0;
                int dest, count = 0, tmp;

                /* we cannot enter final room if some amphipods were correctly
                 * placed from initial position AND we still have different
                 * amphipods in that room.
                 */
                if ((room & pos->final) != (room & pos->occupied))
                    continue;
                bit_for_each32_2(dest, tmp, room) {
                    possible_move_t *move = &moves[cell][dest];
                    if (move->mask & pos->occupied)
                        break;
                    found = dest;
                    if (++count >= rows)
                        break;
                }
                if (found)
                    newmove(pos, amp, cell, found);
            }
        }
    }
    return;
}

/* minimal parsing: We just read the 3-5 lines to get
 * the amphipods location in side rooms
 */
static char *part2str[] = { "  #D#C#B#A#", "  #D#B#A#C#" };

static pos_t *read_input(int part)
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;
    int line = 0, adjline = 0;
    pos_t *pos = get_pos(NULL);
    u32 bit;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        if (line == 2 || line == 3) {

            if (part == 2 && line == 3) {
                for (int i = 0; i < 2; ++i) {
                    bit = 8 + adjline - 2;

                    for (int j = 0; j < 4; ++j) {
                        int amp = part2str[i][j * 2 + 3] - 'A';
                        pos->amp[amp] |= BIT(bit);
                        bit += 4;
                    }
                    adjline++;
                }
            }
            bit = 8 + adjline - 2;
            for (int i = 0; i < 4; ++i) {
                int amp = buf[i * 2 + 3] - 'A';
                pos->amp[amp] |= BIT(bit);
                bit += 4;
            }
        }
        line++;
        adjline++;
    }
    pos->occupied = pos->amp[0] | pos->amp[1] | pos->amp[2] | pos->amp[3];
    /* check if some amphipods are already in correct place
     */
    for (int room = 0; room < 4; ++room) {
        u32 mask = pos->amp[room];
        if (mask & rooms[room]) {
            mask &= rooms[room];
            int room1 = 8 + room * 4;
            for (int cell = part * 2 - 1; cell >= 0; --cell) {
                if (BIT(room1 + cell) & mask) {
                    pos->final |= BIT(room1 + cell);
                } else {
                    break;
                }
            }
        }

    }
    free(buf);
    return pos;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1;
    pos_t *pos;

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
    pool_hash = pool_create("hash", 1024, sizeof(hash_t));

    //printf("b1 = %d\n", setbits(2, 4));
    //printf("b2 = %d\n", setbits(2, 4));
    zobrist_init();
    hash_init();
    init_moves();
    pos = read_input(part);

    pos->zobrist = zobrist_1(pos);
    push_pos(pos);
    while ((pos = pop_pos())) {
        genmoves(pos);
        free_pos(pos);
    }

    printf("%s : res=%d\n", *av, result);
    exit(0);
}
