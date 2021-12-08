/* aoc-c: Advent2021 game, day 6 parts 1 & 2
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
#include <string.h>
#include <malloc.h>

#include "debug.h"
#include "bits.h"
#include "list.h"
#include "pool.h"

typedef struct fish {
    s16 value;
    struct list_head list;
} fish_t;

static pool_t *pool;

LIST_HEAD(fish_head);
static int nfish=0;

//#ifdef DEBUG
static void print_fish()
{
    fish_t *fish;
    int i = 0;

    printf("fish # = %d\n", nfish);
    list_for_each_entry(fish, &fish_head, list) {
        printf("%s%d", i? ",":"", fish->value);
        i++;
    }
    printf("\n");
}
//#endif

static struct list_head *read_fish()
{
    char *buf, *token;
    size_t alloc = 0;
    fish_t *fish;

    if (getline(&buf, &alloc, stdin) < 0)
        return NULL;

    if (!(pool = pool_init("fish", 1024, sizeof (struct fish))))
        return NULL;

    token = strtok(buf, ",\n");
    while (token) {
        //printf("token=[%s]\n", token);
        if (!(fish = pool_get(pool)))
            return NULL;
        fish->value = atoi(token);
        list_add_tail(&fish->list, &fish_head);
        nfish ++;
        token = strtok(NULL, ",\n");
        //print_fish();
    }
    free(buf);
    return &fish_head;
}

static int doit(int part)
{
    fish_t *fish, *new;
    int toadd;
    int iter = part == 1? 80: 256;

    /* initial algorithm surely does not work for part 2:
     * Too many memory allocation, nneed to rethink the whole...
     *
     */
    for (; iter; --iter) {
        //printf("iter = %2d: ", iter);
        toadd = 0;
        list_for_each_entry(fish, &fish_head, list) {
            fish->value--;
            if (fish->value < 0) {            /* was zero: create new fish */
                fish->value = 6;
                toadd++;
                nfish++;
            }
        }
        while (toadd--) {
            if (!(new = pool_get(pool)))
                return -1;
            new->value = 8;
            list_add_tail(&new->list, &fish_head);
        }
    }
    return nfish;
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
                if (exercise < 1 || exercise > 2)
                    return usage(*av);
                break;
            default:
                return usage(*av);
        }
    }
    if (optind < ac)
        return usage(*av);

    read_fish();
    print_fish();
    res = doit(exercise);
    //print_fish();
    printf ("%s : res=%d\n", *av, res);
    /* TODO: free board/mem pool */
    exit (0);
}
