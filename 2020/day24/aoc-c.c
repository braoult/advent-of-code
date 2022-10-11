/* ex1-c: Advent2020, day 24/part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(ac, av)
    int ac;
    char **av;
{
    size_t alloc;
    ssize_t len;
    char *buf = NULL;

    while ((len = getline(&buf, &alloc, stdin))) {
        buf[len - 1] = 0;
        int x=0, y=0;
        char *c = buf;
        while (*c) {
            switch (*c) {
                case 'e':
                    ++x;
                    break;
                case 'w':
                    ++y;
                    break;
                case 's':
                    --y, ++c;
                    break;
                case 'n':
                    ++y, ++c;
            }
            if (*c == 'e')
                ++x;
            else if (*c == 'w')
                ++y;
        }
    }
    printf("%s : res=%lu\n", *av, res);
    exit (0);
}
