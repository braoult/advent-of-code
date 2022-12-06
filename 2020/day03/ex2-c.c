/* ex1-c: Advent2020 game, day 3/game 2
 */

#include <stdio.h>
#include <stdlib.h>

struct {
    int dx;
    int dy;
    int pos;
    int count;
} set[] = {
    { 1,  1, 0, 0},
    { 3,  1, 0, 0},
    { 5,  1, 0, 0},
    { 7,  1, 0, 0},
    { 1,  2, 0, 0},
    {-1, -1, 0, 0}              /* end marker - not necessary */
};

int my_strlen(str)
    char *str;
{
    int i;
    for (i=0; *str; ++i, ++str)
        ;
    return i;
}

int main(ac, av)
    int ac;
    char **av;
{
    int line=1, linelen=0, mod=0, i;
    unsigned long long res=1;
    char str[80];

    scanf("%s", str);          /* ignore 1st line */

    while (scanf("%s", str) != EOF) {
        line++;
        linelen=my_strlen(str);
        for (i=0; set[i].dx >= 0; ++i) {
            if (! (line % set[i].dy )) { /* line ok for this set */
                set[i].pos += set[i].dx; /* increment set column */
                mod = set[i].pos % linelen;
                if ( str[mod] == '#')
                    set[i].count++;
            }
        }
    }
    for (i=0; set[i].dx != -1; ++i)
        res*=set[i].count;
    printf ("%s : lines=%d res=%llu\n", *av, line, res);
    exit (0);
}
