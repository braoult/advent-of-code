/* ex2-c: Advent2020 game, day 5/game 2
 */

#include <stdio.h>
#include <stdlib.h>

static int seats[128][8];

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

void print_seats()
{
    int i, j;
    printf("seats:\n");
    for (i=0; i<128; i++) {
        printf("%03d : ", i);
        for (j=0; j<8; ++j)
            printf("%d ", seats[i][j]);
        printf("\n");
    }
}

int main(ac, av)
    char **av;
{
    int cur, count=0;
    char rowstr[80], colstr[80], line[80];
    int *pseats=(int *)seats;

    while (fgets(line, sizeof line, stdin)) {
        count++;
        sscanf(line, "%[FB]%[RL]", rowstr, colstr);
        cur=(str2int(rowstr)<<3) ^ str2int(colstr);
        *(pseats+cur)=1;
    }
    for (; *pseats == 0; ++pseats)
        ;
    for (; *pseats == 1; ++pseats)
        ;
    printf("%s : lines=%d seat=%ld\n", *av, count, pseats-(int *)seats);
    exit (0);
}
