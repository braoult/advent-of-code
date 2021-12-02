/* ex1-c: Advent2021 game, day 1/game 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"

#define	XMOVE 3

int my_strlen(str)
    char *str;
{
    int i;
    for (i=0; *str; ++i, ++str)
        ;
    return i;
}

static int usage(char *prg)
{
    fprintf(stderr, "Usage: %s [-ilw] [file...]\n", prg);
    return 1;
}

int main(int ac, char **av)
{
    int opt;
    char str[80];

    while ((opt = getopt(ac, av, "d:f:")) != -1) {
        switch (opt) {
            case 'd':
                debug_level_set(atoi(optarg));
                break;
            default:
                return usage(*av);
        }
    }
    printf("optind = %d ac = %d\n", optind, ac);

    if (optind < ac)
        return usage(*av);

    scanf("%s", str);          /* ignore 1st line */

    while (scanf("%s", str) != EOF) {
        line++;
        col+=XMOVE;
        linelen=my_strlen(str);
        mod=col%linelen;
        if (*(str+mod) == '#')
            count++;
    }
    printf ("%s : lines:%d pos:%d found:%d\n", *av, line, col, count);
    exit (0);
}
