/* ex2-c: Advent2020 game, day 12/game 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABS(i)	((i)>=0?(i):(-i))

int main(ac, av)
    int ac;
    char **av;
{
    char line[512], command;
    unsigned val, direction, i;
    int x=0, y=0, wx=10, wy=1, tmp;

    while (fgets(line, sizeof line, stdin)) {
        command=*line;
        val=atoi(line+1);
        direction=0;
        switch (command) {
            case 'L':
                direction=360-val;
                break;
            case 'R':
                direction=val;
                break;
            case 'F':
                for (i=0; i<val; ++i) {
                    x+=wx;
                    y+=wy;
                }
                break;
            case 'N':
                wy+=val;
                break;
            case 'E':
                wx+=val;
                break;
            case 'S':
                wy-=val;
                break;
            case 'W':
                wx-=val;
                break;
        }
        if (direction) {
            for (i=90; i<=direction; i+=90) {
                tmp=wy;
                wy=-wx;
                wx=tmp;
            }
        }
    }
    printf("%s : res=%d\n", *av, ABS(x)+ABS(y));
    exit (0);
}
