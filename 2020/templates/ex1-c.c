/* ex1-c: Advent2020 game, day 3/game 1
 */

#include <stdio.h>
#include <stdlib.h>

#define	XMOVE 3

int my_strlen(str)
     char *str;
{
     int i;
     for (i=0; *str; ++i, ++str)
          ;
     return i;
}


int main(ac, av)
     char **av;
{

     int col=0, line=0, linelen=0, mod=0, count=0;
     char str[80];

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
