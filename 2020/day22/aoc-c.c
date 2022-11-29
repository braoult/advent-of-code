/* aoc-c.c: Advent2020, day 22, part 1
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "hashtable.h"
#include "list.h"
#include "pool.h"

typedef struct card {
    u8 card;               /* card value */
    struct list_head list; /* list of cards */
} card_t;

typedef struct player {
    int ncards;            /* player cards # */
    struct list_head head; /* head of cards list */
} player_t;

/* zobrist hash used to find duplicate positions
 */
typedef struct hash {
    u32 zobrist;
    struct list_head players[2];
    struct hlist_node hlist;
} hash_t;

#define HBITS 10 /* 10 bits: hash size is 1024 */
#define CARDS 50

pool_t *pool_cards;
pool_t *pool_hash;
static u32 zobrist_table[2][CARDS][CARDS];

static void zobrist_init()
{
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 50; ++j)
            for (int k = 0; k < 50; ++k)
                zobrist_table[i][j][k] = rand();
}

static u32 zobrist(player_t *players)
{
    u32 zobrist = 0;
    card_t *card;

    for (int p = 0; p < 2; ++p) {
        int pos = 0;
        list_for_each_entry(card, &players[p].head, list) {
            zobrist ^= zobrist_table[p][pos][card->card - 1];
            pos++;
        }
    }
    return zobrist;
}

static __always_inline u32 hash(u32 h)
{
    return hash_32(h, HBITS);
}

static void print_cards(player_t *players)
{
    struct card *card;

    for (int player = 0; player < 2; ++player) {
        int c = 0;
        log(2, "Player %d's deck: ", player + 1);
        list_for_each_entry(card, &players[player].head, list) {
            if (c++)
                log(2, ",");
            log(2, " %d", card->card);
        }
        log(2, "\n");
    }
}

static int equal_decks(hash_t *hasht, player_t *new)
{
    //int i = 0;
    for (int i = 0; i < 2; ++i) {
        card_t *c1 = list_first_entry_or_null(&hasht->players[i], card_t, list);
        card_t *c2 = list_first_entry_or_null(&new[i].head, card_t, list);
        log_f(3, "p=%d c1=%p c2=%p", i + 1, c1, c2);

        if (!c1 || !c2) {
        /* one list is empty and one is not */
        //if ((!c1 && c2) || (c1 && !c2)) {
            log(3, "NULL\n");
            return 0;
        }
        log_f(3, "\nplayer=%d ", i + 1);

        while (!list_entry_is_head(c1, &hasht->players[i], list) &&
               !list_entry_is_head(c2, &new[i].head, list) &&
               c1->card == c2->card) {
            log(3, "c1=%d c2=%d ", c1->card, c2->card);
            if (c1->card != c2->card) {
                log(3, "\n");
                return 0;
            }
            c1 = list_next_entry(c1, list);
            c2 = list_next_entry(c2, list);
        }
        log(3, "\nd1=%p c1=%p/%d\nd2=%p c2=%p/%d ",
            &hasht[i], c1, list_entry_is_head(c1, &hasht->players[i], list),
            &new[i], c2, list_entry_is_head(c2, &new[i].head, list));
        //log(3, "\nZOBI\n");
        if (!list_entry_is_head(c1, &hasht->players[i], list) ||
            !list_entry_is_head(c2, &new[i].head, list))
            return 0;
        log(3, "\n");
        //log(3, "\nZOBI\n");
    }
    return 1;
}

static hash_t *create_hash(player_t *players, u32 h)
{
    struct card *card;
    hash_t *hash = pool_get(pool_hash);
    INIT_HLIST_NODE(&hash->hlist);
    hash->zobrist = h;

    for (int i = 0; i < 2; ++i) {
        log_f(4, "player %d: ", i + 1);
        INIT_LIST_HEAD(&hash->players[i]);
        list_for_each_entry(card, &players[i].head, list) {
            struct card *new = pool_get(pool_cards);
            new->card = card->card;
            list_add_tail(&new->list, &hash->players[i]);
            log(4, "%d ", card->card);
        }
        log(4, "\n");
    }
    log_f(3, "hash=%p p1-next=%p p2-next=%p\n", hash,
          hash->players[0].next, hash->players[1].next);
    for (int i = 0; i < 2; ++i) {
        log_f(4, "duped player %d: ", i + 1);
        list_for_each_entry(card, &hash->players[i], list) {
            log(4, "%d ", card->card);
        }
        log(4, "\n");
    }
    return hash;
}

static player_t *create_subgame(player_t *from, player_t *to)
{
    struct card *card;

    for (int i = 0; i < 2; ++i) {
        int n = 0, ncards;

        to[i].ncards = from[i].ncards - 1;
        INIT_LIST_HEAD(&to[i].head);
        list_for_each_entry(card, &from[i].head, list) {
            if (!n) {
                to[i].ncards = ncards = card->card;
            } else {
                struct card *new = pool_get(pool_cards);
                new->card = card->card;
                list_add_tail(&new->list, &to[i].head);
                log(4, "%d ", card->card);
                if (!--ncards)
                    break;
            }
            n++;
        }
    }
    return to;
}

/**
 * find_deck - find deck in an hashtable bucket
 */
static hash_t *find_deck(struct hlist_head *hasht, player_t *players)
{
    hash_t *cur;
    u32 z = zobrist(players);
    u32 h = hash(z);
    log_f(3, "zobrist = %u/%u ", z, h);
    hlist_for_each_entry(cur, hasht + h, hlist) {
        log(3, "[%u]\n", cur->zobrist);
        if (cur->zobrist == z && equal_decks(cur, players)) {
            // && equal_decks(&cur->players[1], &players[1].head)) {
            log_f(3, "found\n");
            return cur;
        }
    }
    log_f(3, "not found\n");
    cur = create_hash(players, z);
    hlist_add_head(&cur->hlist, &hasht[h]);
    return NULL;
}


static player_t *parse(player_t *players)
{
    size_t alloc;
    ssize_t len;
    char *buf = NULL;
    int player = 0;
    struct card *card;

    INIT_LIST_HEAD(&players[0].head);
    INIT_LIST_HEAD(&players[1].head);
    players[0].ncards = players[1].ncards = 0;
    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        buf[--len] = 0;
        if (len == 0) {
            player++;
            continue;
        }
        if (isdigit(*buf)) { /* card */
            card = pool_get(pool_cards);
            card->card = atoi(buf);
            players[player].ncards++;
            list_add_tail(&card->list, &players[player].head);
        }
    }
    free(buf);
    return players;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

static void winmove(player_t *winner, player_t *loser)
{
    card_t *win, *lose;

    win = list_first_entry(&winner->head, struct card, list);
    lose = list_first_entry(&loser->head, struct card, list);
    list_move_tail(&win->list, &winner->head);
    list_move_tail(&lose->list, &winner->head);
    loser->ncards--;
    winner->ncards++;
}

static long part1(player_t *players)
{
    int round = 0, winner = 0;

    while (players[0].ncards > 0 && players[1].ncards > 0) {
        winner = 0;
        /*  we can use list_first_entry() macro, as both lists are not empty */
        int val1 = list_first_entry(&players[0].head, struct card, list)->card;
        int val2 = list_first_entry(&players[1].head, struct card, list)->card;
        if (val2 > val1)
            winner = 1;
        winmove(players + winner, players + 1 - winner);
        round++;
    }

    return winner;
}

static long part2(player_t *players)
{
    int round = 1, winner = 0, game;
    static int maxgame = 0;
    DEFINE_HASHTABLE(hasht_deck, HBITS); /* htable for dup decks */

    log(2, "=== Game %d ===\n", game = ++maxgame);
    while (players[0].ncards > 0 && players[1].ncards > 0) {
        int val1, val2;
        winner = 0;

        log(2, "\n-- Round %d (Game %d) --\n", round, game);
        print_cards(players);

        if (find_deck(hasht_deck, players)) {
            log(3, "dup found\n");
            goto end; // return winner;
        } else {
            log(3, "dup not found\n");
        }

        /*  we can use list_first_entry() macro, as both lists are not empty */
        val1 = list_first_entry(&players[0].head, struct card, list)->card;
        val2 = list_first_entry(&players[1].head, struct card, list)->card;
        log(2, "Player 1 plays: %d\n", val1);
        log(2, "Player 2 plays: %d\n", val2);

        if (players[0].ncards > val1 && players[1].ncards > val2) {
            player_t sub[2];
            log(2, "Playing a sub-game to determine the winner...\n\n");
            winner = part2(create_subgame(players, sub));
            log(2, "\n...anyway, back to game %d\n", game);
        } else {
            if (val2 > val1)
                winner = 1;
        }
        winmove(players + winner, players + 1 - winner);
        log(2, "Player %d wins round %d of game %d!\n", winner + 1, round, game);

        round++;
    }
end:
    log(2, "The winner of game %d is player %d!\n", game, winner + 1);
    /*  cleanup decks */
    card_t *card, *tmp;
    if (game != 1) {
        for (int i = 0; i < 2; ++i) {
            list_for_each_entry_safe(card, tmp, &players[i].head, list) {
                list_del(&card->list);
                pool_add(pool_cards, card);
            }
        }
    }
    return winner;
}

int main(int ac, char **av)
{
    int opt, part = 1;

    while ((opt = getopt(ac, av, "d:p:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'p': /* 1 or 2 */
                part = atoi(optarg);
                if (part < 1 || part > 2)
                default:
                    return usage(*av);
        }
    }

    pool_cards = pool_create("cards", 128, sizeof(struct card));
    pool_hash = pool_create("hash", 128, sizeof(struct hash));
    zobrist_init();
    player_t players[2];
    parse(players);

    if (part == 1)
        part1(players);
    else
        part2(players);

    /* we don't need to check for winner, as one list is empty */
    log(2, "\n== Post-game results ==\n");
    print_cards(players);
    card_t *card;
    long res = 0, mult = 1;
    list_for_each_entry_reverse(card, &players[0].head, list)
        res += card->card * mult++;
    list_for_each_entry_reverse(card, &players[1].head, list)
        res += card->card * mult++;

    printf("%s : res=%ld\n", *av, res);
    exit(0);
}
