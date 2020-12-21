/* ex1-c: Advent2020 game, day 13/game 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(ac, av)
    int ac;
    char **av;
{
    struct bus {
        unsigned bus;
        unsigned idx;
    } buses[1024];
    unsigned len=0, i=0, bus, idx;

    char line[1024], *ptr=line;
    unsigned long long lcm=1, result=0;

    fgets(line, sizeof line, stdin);
    fgets(line, sizeof line, stdin);
    ptr=strtok(line, ",");

    do {
        if (*ptr != 'x') {
            buses[len].bus=atol(ptr);
            buses[len].idx=i;
            len++;
        }
        i++;
        ptr=strtok(NULL, ",");
    } while (ptr);


    for (i=0; i<len-1; ++i) {
        bus=buses[i+1].bus;
        idx=buses[i+1].idx;
        lcm*=buses[i].bus;
        while ((result+idx)%bus != 0) {
            result+=lcm;
        }
    }

    printf("%s : res=%llu\n", *av, result);
    exit (0);
}
