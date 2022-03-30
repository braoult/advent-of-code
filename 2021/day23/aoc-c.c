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

#define FORBIDDEN 0x1FD57FD57FFFFFUL
#define ALLOWED   (~FORBIDDEN)
#define HALLWAY   0x7F                            /* 0x000001111111 */
#define ROOM_A    0xF00                           /* 0x111100000000*/
#define ROOM_B    0xF000                          /* 0x1111000000000000*/
#define ROOM_C    0xF0000                         /* 0x11110000000000000000*/
#define ROOM_D    0xF00000                        /* 0x111100000000000000000000*/
#define ROOM      0xFFFF00
#define BIT(c)    (1 << (c))

#define RAND_SEED 1337                            /* seed for random generator */

static u32 rooms[4] = { ROOM_A, ROOM_B, ROOM_C, ROOM_D };
static u64 result = -1;
typedef struct pos {
    u32 amp[4];                                   /* bitboards */
    int moves;
    u64 cost;
    u32 occupied;
    u32 final;                                    /* 1 if final destination */
    int ok;                                       /* amphipods in correct place */
    u64 zobrist;                                  /* for zobrist_2() */
    struct {
        u32 from, to;
    } move_list[64];
    struct list_head list;
} pos_t;

typedef struct hash {
    u64 zobrist;                                  /* zobrist hash */
    u32 amp[4];
    struct list_head list;
} hash_t;

#define HASH_SIZE 131071
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

static u32 zobrist_table[24][4];

static void zobrist_init()
{
    log_f(1, "zobrist init. RAND_MAX=%d seed=%d\n", RAND_MAX, RAND_SEED);
    srand(RAND_SEED);
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 4; ++j) {
            zobrist_table[i][j] = rand();
            log(10, "%d ", zobrist_table[i][j]);
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
    log_f(1, "zobrist=%lu -> %lu (amp=%d from=%u to=%u)\n",
          zobrist, zobrist % HASH_SIZE, amp, from, to);
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
    INIT_LIST_HEAD(&new->list);
    return new;
}

static void hash_stats()
{
    u64 ncollisions = 0;
    u32 min = -1, max = 0;

    for (int i = 0; i < HASH_SIZE; ++i) {
        ncollisions += hasht[i].count;
        if (hasht[i].count < min)
            min = hasht[i].count;
        if (hasht[i].count > max)
            max = hasht[i].count;
    }
    log_f(1, "hash stats: size=%d min=%u max=%u total=%lu average=%lu\n", HASH_SIZE,
          min, max, ncollisions, ncollisions / HASH_SIZE);
}

static u64 hash(pos_t *pos, int amp, u32 from, u32 to)
{
    hash_t *cur;
    u64 zobrist;
    u32 val;

    /* we use the 2 first chars of the amps 32, this should be enough
     * to avoid most collisions
     */
    zobrist = zobrist_2(pos, amp, from, to);
    val = zobrist % HASH_SIZE;
    log_f(1, "zobrist=%lu->%u, count=%d\n", zobrist, val, hasht[val].count);
    list_for_each_entry(cur, &hasht[val].list, list) {
        if (zobrist == cur->zobrist) {
            u32 amp_tmp[4];
            for (int i = 0; i < 4; ++i) {
                amp_tmp[i] = pos->amp[i];
            }
            amp_tmp[amp] ^= BIT(from);
            amp_tmp[amp] |= BIT(to);
            if (amp_tmp[0] == cur->amp[0] &&
                amp_tmp[1] == cur->amp[1] &&
                amp_tmp[2] == cur->amp[2] &&
                amp_tmp[3] == cur->amp[3])
                return 0;
            log(1, "zobrist collision for different positions\n");
        }
    }
    hasht[val].count++;
    log(1, "adding hash count=%u\n", hasht[val].count);
    cur = get_hash(pos);
    cur->zobrist = zobrist;
    cur->amp[amp] ^= BIT(from);
    cur->amp[amp] |= BIT(to);

    list_add(&cur->list, &hasht[val].list);
    return zobrist;
}

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

static char *int2bin(u32 mask, char *ret)
{
    for (int i = 0; i < 32; ++i) {
        ret[31-i] = mask & BIT(i)? '1': '0';
    }
    ret[32] = 0;
    return ret;
}

char *cells[] = {
    "H1", "H2", "H3", "H4", "H5", "H6", "H7", "BAD",
    "A1", "A2", "A3", "A4",
    "B1", "B2", "B3", "B4",
    "C1", "C2", "C3", "C4",
    "D1", "D2", "D3", "D4",
};

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
    u32 from, to;                                 /* unused */
    uint mask, dist;
} move_t;

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
        new->moves = new->ok = 0;
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
        log(1, "Fatal bar\n");
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
        log(1, "Fatal foo\n");
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

static void print_moves(move_t (*moves)[24])
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
    log(1, "%u: popcount=%d ", bits, popcount32(bits));
    bit_for_each32_2(count, tmp, bits) {
        log(1, " %d", count);
    }
    log(1, "\n");

    log(1, "#############\n#");
    for (int i = _H1; i <= _H7; ++i) {            /* hallway */
        if (i >= 2 && i <= 5)
            log(1, "$");
        log(1, "%c", bits & BIT(i)? 'X': '.');
    }
    log(1, "#\n");
    for (int i = 0; i < rows; ++i) {
        int pos = 8 + i;
        log(1, i? "  #": "###");
        for (int j = 0; j < 4; ++j) {
            log(1, "%c#", bits & BIT(pos + 4 * j)? 'X': '.');
        }
        log(1, i? "\n": "##\n");
    }
    log(1, "  #########\n");
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

/* generate possible moves between hallway and rooms
 */
static move_t moves[24][24];

/* calculate distance and move mask for all possible moves
 * (room -> hallway ans hallway -> room)
 */
static move_t (*init_moves())[24]
{
    int hallway, room, dist, pos;
    u32 mask_h, mask_r;
    char bin[64];

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
            log(5, "\tmask_r=%d <%s>", mask_r, int2bin(mask_r, bin));
            log(5, "\tmask_h=%d <%s>\n", mask_h, int2bin(mask_h, bin));
            log(3, "%s\n", int2pos(mask_r | mask_h));
            moves[room][hallway].mask = mask_r | mask_h;
            moves[room][hallway].dist = dist;

            /* from hallway to room */
            if (room >> 2 > hallway)
                mask_h = setbits(hallway + 1, room >> 2);
            else
                mask_h = setbits(room >> 2, hallway);
            mask_r = setbits(room & ~3, room + 1);
            log(5, "\tmask_r=%d <%s>", mask_r, int2bin(mask_r, bin));
            log(5, "\tmask_h=%d <%s>\n", mask_h, int2bin(mask_h, bin));
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
    move_t *move = &moves[from][to];
    pos_t *newpos;
    u64 collision;

    log_f(1, "rows=%d amp=%c from=%s to=%s dist=%u ok=%d cost=%lu\n",
          rows, amp + 'A',
          cells[from], cells[to],
          move->dist, pos->ok, move->dist * cost[amp]);

    if (pos->ok < 0) {
        collision = hash(pos, amp, from, to);
        if (!collision) {
            log(1, "collision, skipping move :\n");
            pos->amp[amp] ^= BIT(from);
            pos->amp[amp] |= BIT(to);
            burrow_print(pos);
            pos->amp[amp] ^= BIT(to);
            pos->amp[amp] |= BIT(from);
            return NULL;
        }
    }
    if (!(newpos = get_pos(pos)))
        return NULL;

    newpos->move_list[newpos->moves].from = from;
    newpos->move_list[newpos->moves].to = to;
    newpos->amp[amp] ^= BIT(from);
    newpos->amp[amp] |= BIT(to);
    newpos->occupied ^= BIT(from);
    newpos->occupied |= BIT(to);
    newpos->moves++;
    newpos->cost += move->dist * cost[amp];

    if (to >= A1) {                               /* final destination */
        newpos->ok++;
        newpos->final |= BIT(to);
        log(1, "Final destination %s, ok=%d, final=%s\n", cells[to],
            newpos->ok, int2pos(newpos->final));
        if (newpos->ok == rows * 4) {
            log(1, "found solution! cost=%lu\n", newpos->cost);
            burrow_print(newpos);
            free_pos(newpos);
            if (newpos->cost < result) {
                result = newpos->cost;
                log(1, "New best=%lu moves=%u List:", result, newpos->moves);
                for (int i = 0; i < newpos->moves; ++i) {
                    log(1, " %s-%s", cells[newpos->move_list[i].from],
                        cells[newpos->move_list[i].to]);
                }
                log(1, "\n");
            }
            return NULL;
        }

    }

    burrow_print(newpos);
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

    log_f(1, "position :\n");
    burrow_print(pos);
    for (amp = A; amp <= D; ++amp) {
        u32 cur = pos->amp[amp];
        bit_for_each32_2(bit, tmp, cur) {
            char s1[64], s2[64];
            int2bin(bit, s1);
            int2bin(ROOM, s2);
            log(3, "-> %s %d\n   %s\n", s1, bit, s2);

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
                        move_t *move = &moves[bit][*d];
                        log(3, "mask=%s dist=%d cost=%lu\n",
                            int2bin(move->mask, s1), move->dist,
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
                char foo[64];

                log(1, "%c from %s to %#x (%s)\n", amp + 'A', cells[bit], room,
                    int2bin(room, foo));
                if (room & pos->occupied && !(room & pos->final)) {
                    log(1, "room is occupied\n");
                    continue;                     /* skip current amp pos */
                }
                log(1, "room is available\n");
                bit_for_each32_2(dest, tmp, room) {
                    move_t *move = &moves[bit][dest];
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
        log(1, "line=%d str=%s\n", line, buf);
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

    zobrist_1(pos);
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

    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());

    exit(0);
}
