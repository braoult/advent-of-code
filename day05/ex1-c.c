/* ex1-c: Advent2020 game, day 4/game 1
 */

#include <stdio.h>
#include <stdlib.h>

int str2int(str)
    char *str;
{
    int res=0;
    while (*str) {
        res <<= 1;
        if (*str == 'B' || *str == 'R')
            res ^= 1;
        str++;
    }
    return res;
}

int main(ac, av)
    char **av;
{
    int max=0, cur, count=0;
    char rowstr[80], colstr[80], line[80];

    while (fgets(line, sizeof line, stdin)) {
        count++;
        sscanf(line, "%[FB]%[RL]", rowstr, colstr);
        cur=(str2int(rowstr)<<3) ^ str2int(colstr);
        if (cur > max)
            max=cur;
    }
    printf("%s : lines=%d max=%d\n", *av, count, max);
    exit (0);
}
