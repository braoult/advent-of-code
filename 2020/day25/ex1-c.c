/* ex1-c: Advent2020, day 25/part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(ac, av)
    int ac;
    char **av;
{
    ulong pub[] = {1, 1}, enc[] = {1, 1}, key[2], res;

    scanf("%lu", key);
    scanf("%lu", key+1);
    while (1) {
        pub[0] = pub[0] * 7 % 20201227;
        pub[1] = pub[1] * 7 % 20201227;
        enc[0] = enc[0] * key[0] %20201227;
        enc[1] = enc[1] * key[1] %20201227;
        if (pub[0] == key[0]) {
            res=enc[1];
            break;
        } else if (pub[1] == key[1]) {
            res=enc[0];
            break;
        }
    }
    printf("%s : res=%lu\n", *av, res);
    exit (0);
}
