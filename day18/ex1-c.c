/* ex1-c: Advent2020 game, day 18/task 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T_SUB    (-1)
#define T_END    (-2)
#define T_PLUS   (-4)
#define T_MULT   (-8)
#define T_ERR    (-16)

#define DIGIT(c) (((c) >= '0') && ((c) <= '9'))

static long eval_expr();

static char *saveptr=NULL;
static long get_tok()
{
    char *p, c;
    long val=0;

    p=saveptr;
    while (!val) {
        c=*p;
        switch (c) {
            case ' ':
                break;
            case '(':
                val=T_SUB;
                p++;
                saveptr=p;
                return eval_expr();
                break;
            case ')':
            case '\n':
            case '\0':
                val=T_END;
                break;
            case '*':
                val=T_MULT;
                break;
            case '+':
                val=T_PLUS;
                break;
            default:
                if (! DIGIT(c)) {
                    val=T_ERR;
                    break;
                }
                while (DIGIT(c)) {
                    val=(val*10 + c - '0');
                    p++;
                    c=*p;
                }
                p--;
                break;
        }
        p++;
    }
    saveptr=p;
    return val;
}

static long eval_expr()
{
    long res=T_SUB, op=T_ERR;
    long left=0;

    res=get_tok();
    left=res;
    while (res!=T_ERR && res!=T_END) {
        switch (res) {
            case T_END:
                goto end;
                break;
            case T_PLUS:
            case T_MULT:
                op=res;
                break;
            case T_ERR:
                left=res;
                goto end;
                break;
            default:
                switch (op) {
                    case T_PLUS:
                        left+=res;
                        break;
                    case T_MULT:
                        left*=res;
                        break;
                }
                break;
        }
        res=get_tok();
    }
end:
    return left;
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[1024];
    long res=0, tmp;

    while (fgets(line, sizeof line, stdin)) {
        saveptr=line;
        tmp=eval_expr();
        res+=tmp;
    }
    printf("%s : res=%ld\n", *av, res);
    exit (0);
}
