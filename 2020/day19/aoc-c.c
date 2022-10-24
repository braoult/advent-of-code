/* aoc-c.c: Advent2020, day 23, part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "list.h"
#include "debug.h"

#define MAX_RULES 256
#define MAX_MSG   512
/* to simplify code, we consider here that :
 * - a rule has no more than 3 sub-rules
 * - there are at most 2 possible sub-rules per rule
 */
struct subrule {
    int rule[2][3];
};
static struct rule {
    enum type {
        SUB,
        CHR
    } type;
    struct subrule sub;
    int str;
} rules[MAX_RULES] = {
    [0 ... (MAX_RULES-1)] = {
        .sub.rule  = {{-1, -1, -1}, {-1, -1, -1}}
    }
};

static char *mesg[MAX_MSG];
static int nrules, nmesg;

static void printall()
{
    for (int i = 0; i < nrules; ++i) {
        printf("rule %3d : ", i);
        if (rules[i].type == CHR) {
            printf("[%c]\n", rules[i].str);
            continue;
        }
        for (int j = 0; j < 3 && rules[i].sub.rule[0][j] != -1; ++j)
            printf("%d ", rules[i].sub.rule[0][j]);
        for (int j = 0; j < 3 && rules[i].sub.rule[1][j] != -1; ++j)
            printf("%s%d ", j? "": "| ", rules[i].sub.rule[1][j]);
        printf("\n");
    }
    for (int i = 0; i < nmesg; ++i) {
        printf("%3d: %s\n", i, mesg[i]);
    }
}

/**
 * parse - parse input.
 */
static void parse()
{
    size_t alloc;
    ssize_t len;
    char *buf = NULL, *tok;
    int rule;
    while ((len = getline(&buf, &alloc, stdin)) > 0) {
        int or = 0, sub = 0;

        buf[--len] = 0;
        if (len == 0)
            continue;
        if (isalpha(*buf)) {                      /* message */
            mesg[nmesg++] = strdup(buf);
        } else {                                  /* rule */
            if (!(tok = strtok(buf, ": ")))       /* rule number */
                continue;
            nrules++;
            rule = atoi(tok);
            while ((tok = strtok(NULL, ":\" "))) {
                switch (*tok) {
                    case 'a':                     /* final rule */
                    case 'b':
                        rules[rule].type = CHR;
                        rules[rule].str = *tok;
                        goto nextline;
                    case '|':                     /* second ruleset */
                        or++;
                        sub = 0;
                        break;
                    default:
                        rules[rule].type = SUB;
                        rules[rule].sub.rule[or][sub] = atoi(tok);
                        sub++;
                        break;
                }
            }
        }
    nextline: ;
    }
    free(buf);
}

static int match(char *str, int *pos, int rule, int depth)
{
    struct rule *r = rules+rule;
    int found = 0, postmp = *pos;
    char *space = "                                     ";
    log_f(3, "%.*sstr=%s pos=%d rule=%d\n", depth * 2, space, str, *pos, rule);
    /* check for no char left ? */
    //if (!str[*pos]) {
    //    printf("No char left !\n");
    //    found = 0;
    //    goto end;
    //}
    switch (r->type) {
        case SUB:
            found = 1;
            log_f(3, "%.*sLEFT\n", depth * 2, space);
            for (int sub = 0; sub < 3 && r->sub.rule[0][sub] >= 0; ++sub) {
                if (!match(str, pos, r->sub.rule[0][sub], depth + 1)) {
                    *pos = postmp;
                    found = 0;
                    break;
                }
            }
            if (found || r->sub.rule[1][0] == -1)
                goto end;
            log_f(3, "%.*sRIGHT\n", depth * 2, space);
            found = 1;
            for (int sub = 0; sub < 3 && r->sub.rule[1][sub] >= 0; ++sub) {
                if (!match(str, pos, r->sub.rule[1][sub], depth + 1)) {
                    *pos = postmp;
                    found = 0;
                    goto end;
                }
            }
            goto end;
        case CHR:
            if (rules[rule].str == str[*pos]) {
                (*pos)++;
                found = 1;
                goto end;
            }
            found = 0;
            goto end;
    }
end:
    /* check for exact length */
    if (depth == 0 && str[*pos]) {
        log_f(3, "chars remaining !\n");
        found = 0;
    }
    log_f(3, "%.*sstr=%s pos=%d rule=%d ret=%s\n", depth * 2, space, str+*pos, *pos, rule, found? "ok":
    "NOK");
    return found;                                     /* not reached */
}

static long part1()
{
    int ok = 0, pos;
    for (int msg = 0; msg < nmesg; ++msg) {
        pos = 0;
        if (match(mesg[msg], &pos, 0, 0)) {
            log(2, "%s: ok\n", mesg[msg]);
            ok++;
        } else {
            log(2, "%s: ok\n", mesg[msg]);
        }
    }
    return ok;
}

static long part2()
{
    static const struct subrule new[2] = {
        {{{42, -1, -1}, {42,  8, -1}}},
        {{{42, 31, -1}, {42, 11, 31}}}
    };
    rules[8].sub  = new[0];
    rules[11].sub  = new[1];

    int ok = 0, pos;
    for (int msg = 0; msg < nmesg; ++msg) {
        pos = 0;
        if (match(mesg[msg], &pos, 0, 0)) {
            printf("%s: ok\n", mesg[msg]);
            ok++;
        } else {
            printf("%s: NOK\n", mesg[msg]);
        }
    }
    return ok;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-d debug_level] [-p part]\n", prg);
    return 1;
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

    parse();
    printf("%s : res=%ld\n", *av, part == 1? part1(): part2());
//    printall();
    exit (0);
}