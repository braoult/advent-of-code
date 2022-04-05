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

#define _HALLWAY   0x7F                           /* 0x000001111111 */
#define _ROOM_A    0xF00                          /* 0x111100000000*/
#define _ROOM_B    0xF000                         /* 0x1111000000000000*/
#define _ROOM_C    0xF0000                        /* 0x11110000000000000000*/
#define _ROOM_D    0xF00000                       /* 0x111100000000000000000000*/
#define _ROOM      0xFFFF00

#define BIT(c)    (1u << (c))

#define RAND_SEED 1                               /* seed for random generator */
#define HASH_SIZE 3000017                         /* prime */

static u32 rooms[4] = { _ROOM_A, _ROOM_B, _ROOM_C, _ROOM_D };
static u64 result = -1;

typedef struct {
    u32 from, to;
} move_t;

typedef struct pos {
    u32 amp[4];                                   /* bitboards */
    u32 final;                                    /* bitboard: 1 if final destination */
    int moves;
    u64 cost;                                     /* cost till this position */
    u64 eval;                                     /* cost + estimation to final pos */
    u32 occupied;
    u64 zobrist;                                  /* for zobrist_2() */
    move_t moves_list[64];
    struct list_head list;
} pos_t;

/* All possible moves between hallway and rooms
 */
typedef struct {
    uint mask;                                    /* blocking spaces */
    uint dist;
} possible_move_t;

static possible_move_t moves[24][24];

typedef struct hash {
    u64 zobrist;                                  /* zobrist hash */
    u32 amp[4];
    move_t moves_list[64];
    int moves;
    struct list_head list;
} hash_t;

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

char *cells[] = {
    "H1", "H2", "H3", "H4", "H5", "H6", "H7", "BAD",
    "A1", "A2", "A3", "A4",
    "B1", "B2", "B3", "B4",
    "C1", "C2", "C3", "C4",
    "D1", "D2", "D3", "D4",
};

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
#define IN_ROOM(c)          (((int)(c) >= _A1 && (int)(c) <= _A4) || \
                             ((int)(c) >= _B1 && (int)(c) <= _B4) || \
                             ((int)(c) >= _C1 && (int)(c) <= _C4) || \
                             ((int)(c) >= _D1 && (int)(c) <= _D4))

#define HASH_SEEN  (HASH_SIZE + 1)
#define LEFT       0
#define RIGHT      1

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

static char *int2bin(u32 mask)
{
    static char ret[64];

    for (int i = 0; i < 32; ++i) {
        ret[31-i] = mask & BIT(i)? '1': '0';
    }
    ret[32] = 0;
    return ret;
}

static char *int2pos(u32 mask)
{
    static char res[1024];
    char *p = res;

    for (int i = 0; i < 32; ++i) {
        if (mask & BIT(i)) {
            *p++ = ' ';
            strcpy(p, cells[i]);
            p += 2;
        }
    }
    return res;
}

static void moves_print(move_t *moves, int nmoves)
{
    for (int i = 0; i < nmoves; ++i)
        printf(" %s-%s", cells[moves[i].from], cells[moves[i].to]);
    printf("\n");
}

static void amp_print_flat(u32 *amp)
{
    int rows = popcount32(*amp);

    for (int i = _H1; i <= _H7; ++i) {            /* hallway */
        if (i >= 2 && i <= 5)
            log(1, "$");
        log(1, "%c",
            BIT(i) & amp[A]? 'A':
            BIT(i) & amp[B]? 'B':
            BIT(i) & amp[C]? 'C':
            BIT(i) & amp[D]? 'D': '.');
    }
    for (int j = 0; j < 4; ++j) {
        log(1, " ");
        for (int i = 0; i < rows; ++i) {
            int offset = 8 + i;

            log(1, "%c",
                BIT(offset + 4 * j) & amp[A]? 'A':
                BIT(offset + 4 * j) & amp[B]? 'B':
                BIT(offset + 4 * j) & amp[C]? 'C':
                BIT(offset + 4 * j) & amp[D]? 'D': '.');
        }
    }
    log(1, "\n");

}

static void burrow_print_flat(pos_t *pos)
{
    int rows = popcount32(pos->amp[0]);

    log_f(1, "rows=%d moves=%2d cost=%6lu: ", rows, pos->moves, pos->cost);
    amp_print_flat(pos->amp);
}

static void burrow_print(pos_t *pos)
{
    u32 tmp;
    int count;
    int rows = popcount32(pos->amp[0]);

    log_f(1, "rows=%d moves=%d cost=%lu\n", rows, pos->moves, pos->cost);
    for (int i = 0; i < 4; ++i) {
        bit_for_each32_2(count, tmp, pos->amp[i]) {
            log(1, " %d", count);
        }
    }

    log(1, "\n");

    log(1, "#############\n#");
    for (int i = _H1; i <= _H7; ++i) {            /* hallway */
        if (i >= 2 && i <= 5)
            log(1, "$");
        log(1, "%c",
            BIT(i) & pos->amp[A]? 'A':
            BIT(i) & pos->amp[B]? 'B':
            BIT(i) & pos->amp[C]? 'C':
            BIT(i) & pos->amp[D]? 'D': '.');
    }
    log(1, "#\n");
    for (int i = 0; i < rows; ++i) {
        int offset = 8 + i;
        log(1, i? "  #": "###");
        for (int j = 0; j < 4; ++j) {
            log(1, "%c#",
                BIT(offset + 4 * j) & pos->amp[A]? 'A':
                BIT(offset + 4 * j) & pos->amp[B]? 'B':
                BIT(offset + 4 * j) & pos->amp[C]? 'C':
                BIT(offset + 4 * j) & pos->amp[D]? 'D': '.');
            //log(4, "%c#", bits & BIT(pos + 4 * j)? 'X': '.');
        }
        log(1, i? "\n": "##\n");
    }
    log(1, "  #########\n");
}

/* from https://stackoverflow.com/a/33021408/3079831
 */
#define IMAX_BITS(m) ((m)/((m)%255+1) / 255%255*8 + 7-86/((m)%255+12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)
_Static_assert((RAND_MAX & (RAND_MAX + 1u)) == 0, "RAND_MAX not a Mersenne number");

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
            log(10, "%lu ", zobrist_table[i][j]);
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
    log_f(1, "zobrist old=%lu new=%lu (%lu) (amp=%d from=%u to=%u)\n",
          pos->zobrist, zobrist, zobrist % HASH_SIZE, amp, from, to);

    if (zobrist % HASH_SIZE == 2321581) {
        log(1, "Zobi: ");
        burrow_print_flat(pos);
        log(1, "Zobi moves: ");
        moves_print(pos->moves_list, pos->moves);
        //for (int i = 0; i < pos->moves; ++i) {
        //    log(1, " %s-%s", cells[pos->moves_list[i].from], cells[pos->moves_list[i].to]);
        //}
        //log(1, "\n");

    }

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
    /* copy moves list
     */
    for (int i = 0; i < 64; ++i)
        new->moves_list[i] = pos->moves_list[i];
    new->moves = pos->moves;
    INIT_LIST_HEAD(&new->list);
    return new;
}

static void hash_stats()
{
    u64 ncollisions = 0;
    u32 min = -1, max = 0, minpos = 0, maxpos = 0;

    for (int i = 0; i < HASH_SIZE; ++i) {
        ncollisions += hasht[i].count;
        if (hasht[i].count < min) {
            min = hasht[i].count;
            minpos = i;
        } else if (hasht[i].count > max) {
            max = hasht[i].count;
            maxpos = i;
        }
    }
    printf("hash: size=%d min=%u (pos=%u) max=%u (pos=%u) total=%lu average=%lu\n",
           HASH_SIZE,
           min, minpos, max, maxpos, ncollisions, ncollisions / HASH_SIZE);
}

static u64 hash(pos_t *pos, int amp, u32 from, u32 to)
{
    hash_t *cur;
    u64 zobrist;
    int hashpos;
    //pos_t pos_tmp = *pos;

    log(1, "Zobi0A: amp=%c from=%s to=%s bfrom=%d bto=%d\n",
        amp + 'A', cells[from], cells[to], BIT(from), BIT(to));
    //log(1, "Zobi00: ");
    //amp_print_flat(pos_tmp.amp);
    //pos_tmp.amp[amp] ^= BIT(from);
    //pos_tmp.amp[amp] |= BIT(to);
    //pos_tmp.moves_list[pos_tmp.moves].from = from;
    //pos_tmp.moves_list[pos_tmp.moves].to = to;
    //pos_tmp.moves++;
    log(1, "Zobi01: ");
    amp_print_flat(pos->amp);

    /* we use the 2 first chars of the amps 32, this should be enough
     * to avoid most collisions
     */
    zobrist = zobrist_2(pos, amp, from, to);
    pos->zobrist = zobrist;
    hashpos = zobrist % HASH_SIZE;
    log_f(1, "zobrist=%lu->%u, count=%d\n", zobrist, hashpos, hasht[hashpos].count);
    list_for_each_entry(cur, &hasht[hashpos].list, list) {
        if (zobrist == cur->zobrist) {
            if (pos->amp[0] == cur->amp[0] &&
                pos->amp[1] == cur->amp[1] &&
                pos->amp[2] == cur->amp[2] &&
                pos->amp[3] == cur->amp[3]) {
                log(1, "zobrist collision for same positions\n");
                log(1, "Zobirst=%lu->%u, count=%d\n", zobrist, hashpos,
                    hasht[hashpos].count);
                log(1, "Zobi cur: ");
                amp_print_flat(cur->amp);
                log(1, "Zobi cur: ");
                moves_print(cur->moves_list, cur->moves);

                log(1, "Zobi tmp: ");
                amp_print_flat(pos->amp);
                log(1, "Zobi tmp: ");
                moves_print(pos->moves_list, pos->moves);
                log(1, "Returning zobrist=HASH_SEEN\n");
                return HASH_SEEN;
            } else {
                log(1, "zobrist collision for different positions:\n");
                log(1, "Zobirst=%lu->%u, count=%d\n", zobrist, hashpos,
                    hasht[hashpos].count);
                log(1, "Zobi cur: ");
                amp_print_flat(cur->amp);
                log(1, "Zobi cur: ");
                moves_print(cur->moves_list, cur->moves);
                log(1, "Zobi tmp: ");
                amp_print_flat(pos->amp);
                log(1, "Zobi tmp: ");
                moves_print(pos->moves_list, pos->moves);
            }
        }
    }
    hasht[hashpos].count++;
    log(1, "adding hash count=%u\n", hasht[hashpos].count);
    cur = get_hash(pos);
    cur->zobrist = zobrist;
    cur->amp[amp] ^= BIT(from);
    cur->amp[amp] |= BIT(to);
    cur->moves_list[cur->moves].from = from;
    cur->moves_list[cur->moves].to = to;
    cur->moves++;

    list_add(&cur->list, &hasht[hashpos].list);
    log(1, "Returning zobrist=%lu\n", zobrist);
    return zobrist;
}

/* evaluate current position :
 * eval = cost + eval_to_dest
 * cost is current cost
 * eval is cost to join top of destination room
 */
static int room2room_dist[][4] = {
    { 4, 4, 6, 8}, { 4, 4, 4, 6}, { 6, 4, 4, 4}, { 8, 6, 4, 4}
};

/* static u64 eval_amp(pos_t *pos, int amp, u32 cell) */
/* { */
/*     u32 bit = BIT(cell); */
/*     int rows = popcount32(pos->amp[0]); */
/*     int dist; */
/*     int left = rows - popcount32(rooms[amp] & pos->final); */

/*     if (ROOM(bit)) {                              /\* in a room *\/ */
/*         int curroom = cell / 4 - 2; */

/*         dist = cell % 4;                          /\* distance to top of room *\/ */
/*         dist += room2room_dist[curroom][amp]; */
/*         dist += --left;                   /\* distance to final room *\/ */
/*         log(1, "eval %c room(%s) = %d\n", amp + 'A', cells[cell], dist); */
/*     } else {                              /\* in hallway *\/ */
/*         int dest = (amp + 2) * 4; */
/*         dist = moves[cell][dest].dist; */
/*         dist += --left;                   /\* distance to final room *\/ */
/*         log(1, "eval %c hallway(%s) = %d\n", amp + 'A', cells[cell], dist); */
/*     } */
/*     return cost[amp] * dist; */
/* } */

static u64 eval(pos_t *pos)
{
    u32 amp, tmp;
    int cell;
    int rows = popcount32(pos->amp[0]);
    u64 eval = 0;

    for (amp = A; amp <= D; ++amp) {
        u32 todo = pos->amp[amp] & ~pos->final;   /* ignore finished ones */
        int destroom = amp, dist;
        int left = rows - popcount32(rooms[destroom] & pos->final);

        printf("amp=%d %u final=%u\n", amp, pos->amp[amp], pos->final);
        printf("amp=%c pos=%s\n", amp + 'A', int2bin(pos->amp[amp]));
        printf("amp=  tod=%s\n", int2bin(todo));
        printf("amp=  fin=%s\n", int2bin(pos->final));
        bit_for_each32_2(cell, tmp, todo) {
            u32 bit = BIT(cell);
            if (ROOM(bit)) {                      /* in a room */
                int curroom = cell / 4 - 2;

                dist = cell % 4;                   /* distance to top of room */
                dist += room2room_dist[curroom][destroom];
                dist += --left;                   /* distance to final room */
                log(1, "eval %c room(%s) = %d\n", amp + 'A', cells[cell], dist);
            } else {                              /* in hallway */
                int dest = (destroom + 2) * 4;
                dist = moves[cell][dest].dist;
                dist += --left;                   /* distance to final room */
                log(1, "eval %c hallway(%s) = %d\n", amp + 'A', cells[cell], dist);
            }
            //printf("eval = %lu %lu\n", cost[amp] * dist, eval_amp(pos, amp, cell));
            eval += cost[amp] * dist;
        }
    }
    //log_f(1, "pos=%p amp=%d from=%u to=%u\n", pos, amp, from, to);
    return eval;
}

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
        new->final = new->occupied = new->zobrist = 0;
        new->cost = new->eval = 0;
        new->moves = 0;
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
        log(1, "Fatal bar\n");
        exit(1);
    }
}

/* push position to stack
 */
static void push_pos(pos_t *pos)
{
    pos_t *cur;

    if (!pos) {
        log(1, "Fatal foo\n");
        exit(1);
    }
    if (!list_empty(&pos_queue)) {
        //list_add_tail(&pos->list, &pos_queue);
        //} else {
        list_for_each_entry(cur, &pos_queue, list) {
            if (cur->eval > pos->eval) {
                log(1, "adding %lu before %lu\n", pos->eval, cur->eval);
                list_add_tail(&pos->list, &cur->list);
                return;
            } else {
                log(1, "not adding %lu before %lu\n", pos->eval, cur->eval);
            }
        }
    }
    list_add_tail(&pos->list, &pos_queue);
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

static void print_moves(possible_move_t (*moves)[24])
{
    int hallway, room;

    log(4, "From rooms to hallway\n");
    for (room = _A1; room <= _D4; ++room) {
        for (hallway = _H1; hallway <= _H7; ++hallway) {
            log(4, "%s -> %s ", cells[room], cells[hallway]);
            log(4, "dist=%d ", moves[room][hallway].dist);
            log(4, "mask=");
            log(4, "%s\n", int2pos(moves[room][hallway].mask));
        }
    }
    log(4, "From hallway to rooms\n");
    for (hallway = _H1; hallway <= _H7; ++hallway) {
        for (room = _A1; room <= _D4; ++room) {
            log(4, "%s -> %s ", cells[hallway], cells[room]);
            log(4, "dist=%d ", moves[hallway][room].dist);
            log(4, "mask=");
            log(4, "%s\n", int2pos(moves[hallway][room].mask));
        }
    }
}

static void mask_print(u32 bits)
{
    u32 tmp;
    int count;
    int rows = popcount32(bits);

    if (rows > 4)
        rows /= 4;
    log(10, "%u: popcount=%d ", bits, popcount32(bits));
    bit_for_each32_2(count, tmp, bits) {
        log(10, " %d", count);
    }
    log(10, "\n");

    log(10, "#############\n#");
    for (int i = _H1; i <= _H7; ++i) {            /* hallway */
        if (i >= 2 && i <= 5)
            log(10, "$");
        log(10, "%c", bits & BIT(i)? 'X': '.');
    }
    log(10, "#\n");
    for (int i = 0; i < rows; ++i) {
        int pos = 8 + i;
        log(10, i? "  #": "###");
        for (int j = 0; j < 4; ++j) {
            log(10, "%c#", bits & BIT(pos + 4 * j)? 'X': '.');
        }
        log(10, i? "\n": "##\n");
    }
    log(10, "  #########\n");
}


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
            log(3, "\ndist(%s, %s) = %d - h2h=%d pos=%d\n", cells[room],
                cells[hallway], dist,
                h2h[pos][hallway], pos);

            /* from rool to hallway */
            if (room >> 2 > hallway)
                mask_h = setbits(hallway, room >> 2);
            else
                mask_h = setbits(room >> 2, hallway + 1);
            mask_r = setbits(room & ~3, room);
            log(5, "\tmask_r=%d <%s>", mask_r, int2bin(mask_r));
            log(5, "\tmask_h=%d <%s>\n", mask_h, int2bin(mask_h));
            log(3, "%s\n", int2pos(mask_r | mask_h));
            moves[room][hallway].mask = mask_r | mask_h;
            moves[room][hallway].dist = dist;

            /* from hallway to room */
            if (room >> 2 > hallway)
                mask_h = setbits(hallway + 1, room >> 2);
            else
                mask_h = setbits(room >> 2, hallway);
            mask_r = setbits(room & ~3, room + 1);
            log(5, "\tmask_r=%d <%s>", mask_r, int2bin(mask_r));
            log(5, "\tmask_h=%d <%s>\n", mask_h, int2bin(mask_h));
            log(3, "%s\n", int2pos(mask_r | mask_h));
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
    pos_t *newpos, pos_tmp = *pos;
    u64 newcost = pos->cost + move->dist * cost[amp];
    u32 bit_from = BIT(from), bit_to = BIT(to);
    int ok = popcount32(pos->final);
    u64 zobrist;

    log_f(1, "rows=%d amp=%c from=%s to=%s dist=%u ok=%d cost=%lu zobrist=%lu\n",
          rows, amp + 'A',
          cells[from], cells[to],
          move->dist, ok, move->dist * cost[amp], pos->zobrist);

    /*
    if ((IN_HALLWAY(from) && !IN_ROOM(to)) ||
        (IN_ROOM(from) && !IN_HALLWAY(to)) ||
        (HALLWAY(bit_from) && !ROOM(bit_to)) ||
        (ROOM(bit_from) && !HALLWAY(bit_to))) {
        log(1, "BUG genmove!\n");
    }
    */
    if (newcost >= result)
        return NULL;
    pos_tmp = *pos;
    pos_tmp.amp[amp] ^= bit_from;
    pos_tmp.amp[amp] |= bit_to;
    pos_tmp.occupied ^= bit_from;
    pos_tmp.occupied |= bit_to;
    pos_tmp.moves_list[pos_tmp.moves].from = from;
    pos_tmp.moves_list[pos_tmp.moves].to = to;
    pos_tmp.moves++;
    pos_tmp.cost = newcost;
    if (ROOM(bit_to)) {                    /* to room */
        //newpos->ok++;
        pos_tmp.final |= bit_to;
        log(1, "Final destination %s, ok=%d, final=%s\n", cells[to],
            popcount32(pos_tmp.final), int2pos(pos_tmp.final));
        if (popcount32(pos_tmp.final) == rows * 4) {
            log(1, "found solution! cost=%lu\n", pos_tmp.cost);
            if (pos_tmp.cost < result) {
                result = pos_tmp.cost;
                log(1, "New best=%lu moves=%u List:", result, pos_tmp.moves);
                moves_print(pos_tmp.moves_list, pos_tmp.moves);
            }
            return NULL;
        }
    }
    /*  final move, cannot fail */
    /*
    if (ok == rows * 4 - 1) {
        u64 final = newcost;
        log(1, "found solution! cost=%lu\nZobi10 ", final);
        burrow_print_flat(pos);
        if (final < result) {
            result = final;
            log(1, "New best=%lu moves=%u+1 List:", result, pos->moves);
            for (int i = 0; i < pos->moves; ++i) {
                log(1, " %s-%s", cells[pos->moves_list[i].from],
                    cells[pos->moves_list[i].to]);
            }
            log(1, "\n");
        }
    }
    */
    //if (zobrist != zobrist_1(&pos_tmp)) {
    //    printf("Zobi Fuck1\n");
    //    exit(1);
    //}
    //if (zobrist != pos_tmp.zobrist) {
    //    printf("Zobi Fuck2\n");
    //    exit(1);
    //}
    //zobrist = hash1(pos);
    zobrist = hash(&pos_tmp, amp, from, to);
    if (ok < rows * 4 - 1) {                                 /*  */
        if (zobrist == HASH_SEEN) {
            log(1, "collision, skipping move : ");
            burrow_print_flat(&pos_tmp);
            return NULL;
        }
    }
    pos_tmp.eval = eval(&pos_tmp) + pos_tmp.cost;
    if (!(newpos = get_pos(&pos_tmp)))
        return NULL;

    burrow_print_flat(newpos);
    push_pos(newpos);
    log(1, "New position: moves=%d cost=%lu\n", newpos->moves, newpos->cost);
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

    if (pos->cost >= result)
        return;
    log_f(1, "position :\n");
    burrow_print(pos);
    for (amp = A; amp <= D; ++amp) {
        u32 cur = pos->amp[amp];
        bit_for_each32_2(bit, tmp, cur) {
            log(3, "toto -> %s %d\n", int2bin(bit), bit);

            if (bit >= _A1) {                     /* in a room */
                if (BIT(bit) & pos->final) {
                    log(1, "position already ok\n");
                    continue;
                }
                /* TODO: already in dest room */
                int room = (bit >> 2) - 2;
                log(3, "room = %d\n", room);
                for (int side = LEFT; side <= RIGHT; ++side) {
                    int *d = room_exit[room][side];
                    for (; *d != -1; ++d) {
                        log(3, "checking %c from %s to %s\n",
                            amp + 'A', cells[bit], cells[*d]);
                        possible_move_t *move = &moves[bit][*d];
                        log(3, "mask=%s dist=%d cost=%lu\n",
                            int2bin(move->mask), move->dist,
                            move->dist * cost[amp]);
                        if (move->mask & pos->occupied) {
                            log(3, "blocked!\n");
                            break;
                        }
                        newmove(pos, amp, bit, *d);
                    }
                }

            } else {                              /* hallway */
                u32 room = rooms[amp], found = 0;
                int dest, count = 0, tmp;

                log(1, "%c from %s to %#x (%s)\n", amp + 'A', cells[bit], room,
                    int2bin(room));
                if (room & pos->occupied && !(room & pos->final)) {
                    log(1, "room is occupied\n");
                    continue;                     /* skip current amp pos */
                }
                log(1, "room is available\n");
                bit_for_each32_2(dest, tmp, room) {
                    possible_move_t *move = &moves[bit][dest];
                    log(1, " %d(%s) -> %d(%s) : ", bit, cells[bit],
                        dest, cells[dest]);
                    if (move->mask & pos->occupied) {
                        log(1, "blocked\n");
                        break;
                    }
                    log(1, "valid\n");
                    found = dest;
                    if (++count >= rows)
                        break;
                }
                if (found) {
                    log(1, "found %s -> %s\n", cells[bit], cells[found]);
                    newmove(pos, amp, bit, found);
                }

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
        log(3, "line=%d str=%s\n", line, buf);
        if (line == 2 || line == 3) {

            if (part == 2 && line == 3) {
                for (int i = 0; i < 2; ++i) {
                    bit = 8 + adjline - 2;

                    for (int j = 0; j < 4; ++j) {
                        int amp = part2str[i][j * 2 + 3] - 'A';
                        pos->amp[amp] |= BIT(bit);
                        log(3, "setting bit %d to %c\n", bit, amp + 'A');
                        bit += 4;
                    }
                    adjline++;
                }
            }
            bit = 8 + adjline - 2;
            for (int i = 0; i < 4; ++i) {
                int amp = buf[i * 2 + 3] - 'A';
                pos->amp[amp] |= BIT(bit);
                log(3, "setting bit %d to %c\n", bit, amp + 'A');
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
            log(3, "found amp %c in room %d\n", room + 'A', room);
            mask &= rooms[room];
            int room1 = 8 + room * 4;
            for (int cell = part * 2 - 1; cell >= 0; --cell) {
                if (BIT(room1 + cell) & mask) {
                    log(3, " -> Match for cell %d\n", cell + 1);
                    pos->final |= BIT(room1 + cell);
                    //pos->ok++;
                } else {
                    log(3, " -> No Match for cell %d\n", cell + 1);
                    break;
                }
                //if (mask && mask & rooms[cell])
            }
        }

    }
    free(buf);
    return pos;
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
    print_moves(moves);

    pos = read_input(part);

    pos->zobrist = zobrist_1(pos);
    push_pos(pos);
    /*
    for (int i = 0; i < 4; ++ i) {
        log(1, "------------ %c\n", 'A' + i);
        mask_print(pos->amp[i]);
        }
    */
    mask_print(pos->occupied);

    burrow_print(pos);

    while ((pos = pop_pos())) {
        genmoves(pos);
        free_pos(pos);
    }
    hash_stats();

    printf("%s : res=%ld\n", *av, result);
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());

    exit(0);
    /* dummy calls for flycheck */
    eval(pos);
}
