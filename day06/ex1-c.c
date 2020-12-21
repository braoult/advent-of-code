/* ex1-c: Advent2020 game, day 4/game 1
 */

#include <stdio.h>
#include <stdlib.h>

#define QSIZE	('z'-'a'+1)
static int Q[QSIZE];

void reset_array(){
    int i;
    for (i=0; i<QSIZE; ++i)
        Q[i]=0;
}

void print_array()
{
    int i;
    for (i=0; i<QSIZE; i++) {
        printf("%d ", Q[i]);
    }
    printf("\n");
}

void set_array(str)
    char *str;
{
    for (; *str; str++) {
	if (*str >= 'a' && *str <= 'z')
            Q[*str-'a']++;
    }
}

int count_array ()
{
    int count=0, i;
    for (i=0; i<QSIZE; ++i) {
        if (Q[i] > 0)
            count++;
    }
    return count;
}

void calcgroup(group, count, people)
    int *group, *count, *people;
{
    (*group)++;
    *count += count_array();
    *people=0;
    reset_array();
}

int main(ac, av)
    char **av;
{
    int people=0, group=0, count=0;
    char line[80];

    while (fgets(line, sizeof line, stdin)) {
        if (*line == '\n') {
            calcgroup(&group, &count, &people);
            continue;
        }
        people++;
        set_array(line);
        //sscanf(line, "%[FB]%[RL]", rowstr, colstr);
        //cur=(str2int(rowstr)<<3) ^ str2int(colstr);
        //    *(pseats+cur)=1;
    }
    //for (; *pseats == 0; ++pseats)
    //    ;
    //for (; *pseats == 1; ++pseats)
    //    ;
    //printf("%s : lines=%d seat=%ld\n", *av, count, pseats-(int *)seats);
    calcgroup(&group, &count, &people);
    printf("%s : groups=%d count=%d\n", *av, group, count);
    exit (0);
}
