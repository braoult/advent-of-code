/* ex1-c: Advent2020 game, day 12/game 1
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
    unsigned val, direction=90, real;
    int x=0, y=0;

    while (fgets(line, sizeof line, stdin)) {
        command=*line;
        val=atoi(line+1);
        switch (command) {
            case 'N':
                real=0;
                break;
            case 'E':
                real=90;
                break;
            case 'S':
                real=180;
                break;
            case 'W':
                real=270;
                break;
            case 'F':
                real=direction;
                break;
            case 'L':
                val=360-val;
            case 'R':
                direction=(direction+val)%360;
        }
        if (command != 'R' && command != 'L') {
            switch (real) {
                case 0:
                    y+=val;
                    break;
                case 90:
                    x+=val;
                    break;
                case 180:
                    y-=val;
                    break;
                case 270:
                    x-=val;
                    break;
            }
        }
    }
    printf("%s : res=%d\n", *av, ABS(x)+ABS(y));
    exit (0);
}
