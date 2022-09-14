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

typedef enum {
    A, B, C, D
} amphipod_t;

static const u64 cost[] = {
    1, 10, 100, 1000
};

#define _ROOM_A    0x00000F00                     /* 000000000000111100000000*/
#define _ROOM_B    0x0000F000                     /* 000000001111000000000000*/
#define _ROOM_C    0x000F0000                     /* 000011110000000000000000*/
#define _ROOM_D    0x00F00000                     /* 111100000000000000000000*/
#define _ROOM      0x00FFFF00                     /* 111111111111111100000000 */
#define _HALLWAY   0x0000007F                     /* 000000000000000001111111 */

#define BIT(c)    (1 << (c))

#define RAND_SEED 1337                            /* seed for random generator */

static u32 rooms[4] = { _ROOM_A, _ROOM_B, _ROOM_C, _ROOM_D };
static u64 result = -1;

typedef struct pos {
    u32 amp[4];                                   /* bitmask of amphipods position */
    u32 occupied;                                 /* bitmask for all occupied cells */
    u32 final;                                    /* bitmask for correct position */
    u64 zobrist;                                  /* for zobrist_2() */
    u64 cost;                                     /* cost till this position */
    struct list_head list;                        /*  */
} pos_t;

typedef struct hash {
    u64 zobrist;                                  /* zobrist hash */
    u32 amp[4];
    u64 cost;
    struct list_head list;
} hash_t;

#define HASH_SIZE 131071
#define HASH_SEEN (HASH_SIZE + 1)

struct {
    u32 count;
    struct list_head list;
} hasht[HASH_SIZE];

static inline u32 get_occupancy(pos_t *pos)
{
    return pos->amp[A] | pos->amp[B] | pos->amp[C] | pos->amp[D];
}

pool_t *pool_pos;
pool_t *pool_hash;

LIST_HEAD(pos_queue);

static u64 zobrist_table[24][4];

/* from https://stackoverflow.com/a/33021408/3079831
 */
#define IMAX_BITS(m) ((m)/((m)%255+1) / 255%255*8 + 7-86/((m)%255+12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)
_Static_assert((RAND_MAX & (RAND_MAX + 1u)) == 0, "RAND_MAX not a Mersenne number");


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

/* note that to determine of hallway space is left or right,
 * we simply need to divide the room number by 4.
 */
typedef enum {
    _H1 = 0, _H2, _H3, _H4, _H5, _H6, _H7,        /* 0-6 */
    _WRONG = 7,                                   /* 7 */
    _A1 = 8, _A2, _A3, _A4,                       /* 8-11 */
    _B1,     _B2, _B3, _B4,                       /* 12-15 */
    _C1,     _C2, _C3, _C4,                       /* 16-19 */
    _D1,     _D2, _D3, _D4,                       /* 20-23 */
} _space_t;

#define HALLWAY(b)          ((b) & _HALLWAY)
#define ROOM(b)             ((b) & _ROOM)

#define IN_HALLWAY(c)       ((int)(c) >= _H1 && (int)(c) <= _H7)
#define IN_ROOM(c)          (((int)(c) >= _A1 && (int)(c) <= _A4) ||   \
                             ((int)(c) >= _B1 && (int)(c) <= _B4) ||  \
                             ((int)(c) >= _C1 && (int)(c) <= _C4) || \
                             ((int)(c) >= _D1 && (int)(c) <= _D4))

#define LEFT 0
#define RIGHT 1

static s32 room_exit[4][2][6] = {
    { { _H2, _H1, -1 },                           /* room A left */
      { _H3, _H4, _H5, _H6, _H7, -1 }             /* room A right */
    },
    { { _H3, _H2, _H1, -1 },                      /* room B left */
      { _H4, _H5, _H6, _H7, -1 }                  /* room B right */
    },
    { { _H4, _H3, _H2, _H1, -1 },                 /* room C left */
      { _H5, _H6, _H7, -1 }                       /* room C right */
    },
    { { _H5, _H4, _H3, _H2, _H1, -1 },            /* room D left */
      { _H6, _H7, -1 }                            /* room D right */
    }
};


char *cells[] = {
    "H1", "H2", "H3", "H4", "H5", "H6", "H7", "BAD",
    "A1", "A2", "A3", "A4",
    "B1", "B2", "B3", "B4",
    "C1", "C2", "C3", "C4",
    "D1", "D2", "D3", "D4",
};

typedef enum {
    H1 = BIT(_H1), H2 = BIT(_H2), H3 = (_H3), H4 = (_H4),
    H5 = (_H5), H6 = (_H6), H7 = (_H7),
    A1 = (_A1), A2 = (_A2), A3 = (_A3), A4 = (_A4),
    B1 = (_B1), B2 = (_B2), B3 = (_B3), B4 = (_B4),
    C1 = (_C1), C2 = (_C2), C3 = (_C3), C4 = (_C4),
    D1 = (_D1), D2 = (_D2), D3 = (_D3), D4 = (_D4),
} space_t;

/* Steps to move to hallway
 */
typedef enum {
    A1H = 1, A2H = 2, A3H = 3, A4H = 4
} out_t;

/* Mask which disallow moves to hallway
 */
typedef struct {
    uint mask, dist;
} possible_move_t;

/* Steps to move from space outside room to hallway destination
 */
typedef enum {
    AH1 = 2, AH2 = 1, AH3 = 1, AH4 = 3, AH5 = 5, AH6 = 7 , AH7 = 8,
    BH1 = 4, BH2 = 3, BH3 = 1, BH4 = 1, BH5 = 3, BH6 = 5 , BH7 = 6,
    CH1 = 6, CH2 = 5, CH3 = 3, CH4 = 1, CH5 = 1, CH6 = 3 , CH7 = 4,
    DH1 = 8, DH2 = 7, DH3 = 5, DH4 = 3, DH5 = 1, DH6 = 1 , DH7 = 2
} steps_t;

/*
 * #############
 * #ab.c.d.e.fg#
 * ###h#i#j#k###
 *   #l#m#n#o#
 *   #p#q#r#s#
 *   #t#u#v#w#
 *   #########
 */

/* set bits between 2 positions
 * setbits(2, 4) -> 0011100
 */
static u32 setbits(int from, int to)
{
    u32 ret = 0;
    for (int i = from; i < to; ++i)
        ret |= BIT(i);
    log_f(4, "(%d, %d) = %d\n", from, to, ret);
    return ret;
}

static int h2h[][7] = {
    { 2, 1, 1, 3, 5, 7, 8 },                     /* A */
    { 4, 3, 1, 1, 3, 5, 6 },                     /* B */
    { 6, 5, 3, 1, 1, 3, 4 },                     /* C */
    { 8, 7, 5, 3, 1, 1, 2 }                      /* D */
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
    if (pos) {
        list_add(&pos->list, &pos_queue);
    } else {
        exit(1);
    }
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

static u64 rand64(void) {
  u64 r = 0;
  for (int i = 0; i < 64; i += RAND_MAX_WIDTH) {
    r <<= RAND_MAX_WIDTH;
    r ^= (unsigned) rand();
  }
  return r;
}

static void zobrist_init()
{
    log_f(1, "seed=%d rand_max=%d\n", RAND_SEED, RAND_MAX);
    srand(RAND_SEED);
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 4; ++j) {
            zobrist_table[i][j] = rand64();
        }
    }
}

static inline u64 zobrist_1(pos_t *pos)
{
    u32 tmp;
    int bit;
    u64 zobrist = 0;

    for (int amp = 0; amp < 4; ++amp) {
        bit_for_each32_2(bit, tmp, pos->amp[amp]) {
            //log_f(2, "amp=%d/%c bit=%d\n", amp, amp+'A', bit);
            zobrist ^= zobrist_table[bit][amp];
        }
    }
    log_f(1, "zobrist=%lu -> %lu\n", zobrist, zobrist % HASH_SIZE);
    return zobrist;
}

/* calculate zobrist hash from previous zobrist value
 */
static inline u64 zobrist_2(pos_t *pos, int amp, u32 from, u32 to)
{
    u64 zobrist = pos->zobrist;

    zobrist ^= zobrist_table[from][amp];
    zobrist ^= zobrist_table[to][amp];
    log_f(1, "zobrist(%d, %u, %u)=%lu -> %lu (amp=%d from=%u to=%u)\n",
          amp, from, to, zobrist, zobrist % HASH_SIZE, amp, from, to);
    return zobrist;
}

static void hash_init()
{
    for (int i = 0; i < HASH_SIZE; ++i) {
        hasht[i].count = 0;
        INIT_LIST_HEAD(&hasht[i].list);
    }
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
    u32 hashpos;
    u64 zobrist = pos->zobrist;

    hashpos = zobrist % HASH_SIZE;
    list_for_each_entry(cur, &hasht[hashpos].list, list) {
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
    hasht[hashpos].count++;
    cur = get_hash(pos);

    list_add(&cur->list, &hasht[hashpos].list);
    return cur;
}


/* generate possible moves between hallway and rooms
 */
static possible_move_t moves[24][24];

/* calculate distance and move mask for all possible moves
 * (room -> hallway ans hallway -> room)
 */
static possible_move_t (*init_moves())[24]
{
    int hallway, room, dist, pos;
    u32 mask_h, mask_r;

    for (room = _A1; room <= _D4; ++room) {
        pos = (room >> 2) - 2;
        for (hallway = _H1; hallway <= _H7; ++hallway) {
            dist = h2h[pos][hallway] + room % 4 + 1;

            /* from room to hallway */
            if (room >> 2 > hallway)
                mask_h = setbits(hallway, room >> 2);
            else
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
        log(3, "\n");
    }
    return moves;
}


static pos_t *newmove(pos_t *pos, amphipod_t amp, u32 from, u32 to)
{
    int rows = popcount32(pos->amp[0]);
    possible_move_t *move = &moves[from][to];
    pos_t *newpos;
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

    if (HALLWAY(bit_from)) {                    /* from hallway */
        newpos->final |= BIT(to);
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
    amphipod_t amp;
    u32 tmp;
    int bit;
    int rows = popcount32(pos->amp[0]);

    for (amp = A; amp <= D; ++amp) {
        u32 cur = pos->amp[amp];
        bit_for_each32_2(bit, tmp, cur) {
            if (bit >= _A1) {                     /* in a room */
                if (BIT(bit) & pos->final)
                    continue;
                int room = (bit >> 2) - 2;
                for (int side = LEFT; side <= RIGHT; ++side) {
                    int *d = room_exit[room][side];
                    for (; *d != -1; ++d) {
                        possible_move_t *move = &moves[bit][*d];
                        if (move->mask & pos->occupied)
                            break;
                        newmove(pos, amp, bit, *d);
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
                    possible_move_t *move = &moves[bit][dest];
                    if (move->mask & pos->occupied)
                        break;
                    found = dest;
                    if (++count >= rows)
                        break;
                }
                if (found)
                    newmove(pos, amp, bit, found);
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
        log(1, "line=%d str=%s\n", line, buf);
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
    pos->occupied = get_occupancy(pos);
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

    printf("%s : res=%ld\n", *av, result);
    exit(0);
}
