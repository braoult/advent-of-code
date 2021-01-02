/* ex1-c: Advent2020 game, day 15/games 1 and 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(ac, av)
    int ac;
    char **av;
{
    char line[1024], *pval;
    int target, cur=0, *array;
    register int diff, previous=-1;

    if (ac != 2) {
        fprintf(stderr, "usage: %s number\n", *av);
    }
    target=atol(*(av+1));
    array=malloc(sizeof(unsigned long) * target);
    for (int i=0; i<target; ++i) {
        array[i]=-1;
    }

    fgets(line, sizeof line, stdin);
    pval=strtok(line, ",");
    do {
        diff=atoi(pval);
        array[diff]=cur++;
    } while (pval=strtok(NULL, ","));

    for (; cur<target; ++cur) {
        diff = (previous>=0)? cur-previous-1: 0;
        previous=array[diff];
        array[diff]=cur;
    }

    printf("%s : res[%d]=%d\n", *av, target, diff);
    exit (0);
}
