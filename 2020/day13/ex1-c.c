/* ex1-c: Advent2020 game, day 13/game 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABS(i)	((i)>=0?(i):(-i))

int main(ac, av)
    int ac;
    char **av;
{
    struct bus {
        unsigned bus;
        unsigned idx;
    } buses[1024];
    int len=0, i=0, bus, val, found=0, thebus=0;

    char line[1024], *ptr=line;
    unsigned long long target;

    target=atol(fgets(line, sizeof line, stdin));
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

    for (i=0; i<len; ++i) {
        bus=buses[i].bus;
        val=((target/bus)+1)*bus;
        if (found == 0 || val < found) {
            found=val;
            thebus=bus;
        }
    }

    printf("%s : res=%llu\n", *av, (found-target)*thebus);
    exit (0);
}
