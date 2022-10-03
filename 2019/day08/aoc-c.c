/* aoc-c.c: Advent of Code 2019, day 8 parts 1 & 2
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

#include "br.h"
#include "bits.h"
#include "debug.h"
#include "list.h"
#include "pool.h"

struct input {
    int len;
    char *buf;

};

static int part1(struct input *input, int width, int height)
{
    int depth = input->len / width / height;
    int minzero = input->len, n1n2;

    for (int i = 0; i < depth; ++i) {
        char *layer = input->buf + i * (width * height);
        int tmp[10] = {0};
        for (int j = 0; j < width*height; ++j) {
            tmp[layer[j] - '0'] ++;
        }
        if (tmp[0] < minzero) {
            minzero = tmp[0];
            n1n2 = tmp[1] * tmp[2];
        }
    }
    return n1n2;
}

static int part2(struct input *input, int width, int height)
{
    for (int line = 0; line < height; line++) {
        for (int pixel = 0; pixel < width; ++pixel) {
            char *pos = input->buf + line * width + pixel;
            while (pos < input->buf + input->len && *pos == '2')
                pos += width * height;
            putchar(*pos == '0'? ' ': '#');
        }
        putchar('\n');
    }
    return 0;
}

static int parse(struct input *input)
{
    size_t alloc = 0;
    ssize_t buflen;
    char *buf = NULL;

    if ((buflen = getline(&buf, &alloc, stdin)) <= 0) {
        fprintf(stderr, "error reading file.\n");
        return 0;
    }
    buf[buflen--] = 0;
    input->buf = buf;
    input->len = buflen;
    return buflen;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part] [-i input]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt, part = 1, width = 25, height = 6;
    struct input input = { 0 };

    while ((opt = getopt(ac, av, "d:p:w:h:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            case 'w':
                width = atoi(optarg);
                break;
            case 'h':
                height = atoi(optarg);
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

    parse(&input);
    printf("%s : res=%d\n", *av,
           part == 1?
           part1(&input, width, height) :
           part2(&input, width, height));
    free(input.buf);
    exit(0);
}
