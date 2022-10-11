/* ex1-c: Advent2020 game, day 19/tasks 1 & 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct rule {
    struct rule *left;
    struct rule *right;

};
int main(ac, av)
    int ac;
    char **av;
{
    char line[1024];
    long res=0, tmp;

    if (ac != 2) {
        fprintf(stderr, "usage: %s [1|2]\n", *av);
        exit (1);
    }
    if (**(av+1) == '2')
        prio=&prio_2;

    while (fgets(line, sizeof line, stdin)) {
        //gets(line, sizeof line, stdin);
        //NPUSH(10);
        //NPUSH(100);
        //NPUSH(1000);
        //print();
        //printf("TOP=%ld\n", NTOP());
        //NPOP();
        //print();

        saveptr=line;
        //printf("%s", line);
        tmp=eval_expr();
        //printf("%s : res=%ld\n", line, tmp);
        res+=tmp;
    }
    printf("%s : res=%ld\n", *av, res);
    exit (0);
}
