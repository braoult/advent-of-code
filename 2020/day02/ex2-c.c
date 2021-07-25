/* ex2-c: Advent2020 game, day 2/game 2
 */

#include <stdio.h>
#include <stdlib.h>

int ccount(char *str, char c, int pos1, int pos2)
{
     return ((str[pos1-1] == c) + (str[pos2-1] == c)) == 1;
}

int main(ac, av)
char **av;
{
     int beg, end, nc, found=0, nlines=0;
     char c, str[80];

     while (scanf("%d-%d %c: %s", &beg, &end, &c, str) != EOF) {
          found += ccount(str, c, beg, end);
          nlines++;
     }
     printf ("%s : lines: %d matched:%d\n", *av, nlines, found);
     exit (0);
}
