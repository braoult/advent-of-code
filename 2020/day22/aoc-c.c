/* aoc-c.c: Advent2020, day 22, part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "list.h"
#include "pool.h"
#include "debug.h"

struct card {
    int card;
    struct list_head list;
};

struct player {
    int ncards;
    struct list_head head;
} players[2];

pool_t *pool_cards;

static void print_cards()
{
    struct card *card;
    for (int player = 0; player < 2; ++player) {
        log(2, "player %d (%d cards): ", player + 1, players[player].ncards);
        list_for_each_entry(card, &players[player].head, list) {
            log(2, "%d ", card->card);
        }
        log(2, "\n");
    }
}

static void parse()
{
    size_t alloc;
    ssize_t len;
    char *buf = NULL;
    int player = -1;
    struct card *card;

    INIT_LIST_HEAD(&players[0].head);
    INIT_LIST_HEAD(&players[1].head);
    players[0].ncards = players[1].ncards = 0;
    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        buf[--len] = 0;
        if (len == 0)
            continue;
        if (*buf == 'P') {
            player++;
            continue;
        } else if (isdigit(*buf)) {               /* card */
            card = pool_get(pool_cards);
            card->card = atoi(buf);
            players[player].ncards++;
            list_add_tail(&card->list, &players[player].head);
        }
    }
    free(buf);
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
}

static long part1()
{
    struct card *c1, *c2;
    int round = 0, mult = 1;
    long res = 0;

    while (players[0].ncards > 0 && players[1].ncards > 0) {
        c1 = list_first_entry_or_null(&players[0].head, struct card, list);
        c2 = list_first_entry_or_null(&players[1].head, struct card, list);
        if (c1->card > c2->card) {
            list_move_tail(&c1->list, &players[0].head);
            list_move_tail(&c2->list, &players[0].head);
            players[1].ncards--;
            players[0].ncards++;
        } else {
            list_move_tail(&c2->list, &players[1].head);
            list_move_tail(&c1->list, &players[1].head);
            players[0].ncards--;
            players[1].ncards++;
        }
        round++;
        log(2, "\nafter round %d\n", round);
        print_cards();
    }
    list_for_each_entry_reverse(c1,
                                players[0].ncards? &players[0].head: &players[1].head,
                                list) {
        res += c1->card * mult++;
    };
    return res;
}

static long part2()
{
    return 2;
}

int main(ac, av)
    int ac;
    char **av;
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
            default:
                    return usage(*av);
        }
    }

    pool_cards = pool_create("cards", 128, sizeof(struct card));

    parse();
    print_cards();
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
    exit (0);
}
