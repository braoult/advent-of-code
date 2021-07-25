/* ex1-c: Advent2020 game, day 2/game 1
 */

#include <stdio.h>
#include <stdlib.h>

int ccount(char *str, char c)
{
     int count=0;

     for (int i=0; str[i]; i++)
          count += (str[i] == c);
     return count;
}

int main(ac, av)
char **av;
{
     int beg, end, nc, found=0, nlines=0;
     char c, str[80];

     while (scanf("%d-%d %c: %s", &beg, &end, &c, str) != EOF) {
          nc=ccount(str, c);
          if (nc >= beg && nc <= end)
               found++;

          nlines++;
     }
     printf ("%s : lines: %d matched:%d\n", *av, nlines, found);
     exit (0);
}
