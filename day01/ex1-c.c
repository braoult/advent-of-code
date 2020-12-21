/* ex1-c:c: Advent2020 game, day 1/game 1
 */

#include <stdio.h>
#include <stdlib.h>

int main(ac, av)
char **av;
{
     int numbers[1000];         /* should do differently :
                                 * either read twice, either create list
                                 */
     int n=0, i, j, n1, n2;

     while (scanf("%d", numbers+n) != EOF)
          n++;

     for (i=0; i<n; ++i) {
          n1=numbers[i];
          for (j=i+1; j<n; ++j) {
               n2=numbers[j];
               if (n1+n2 == 2020) {
                    printf("%s : %d:%d %d:%d sum=%d mul=%d\n", *av,
                           i, n1, j, n2, n1+n2, n1*n2);
                    exit (0);
               }
          }
     }
}
