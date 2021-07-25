/* ex1-c: Advent2020 game, day 18/tasks 1 & 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define T_LPAR   (-'(')
#define T_RPAR   (-')')

#define T_PLUS   (-'+')
#define T_MULT   (-'*')

#define T_ERR    (-'E')
#define T_OK     (-'O')
#define T_END    (-'$')

#define LEN_MAX  1024

#define NPUSH(n) (push(&nstack, (n)))
#define OPUSH(o) (push(&ostack, (o)))

#define NPOP()   (pop(&nstack))
#define OPOP()   (pop(&ostack))

#define NTOP()   (top(&nstack))
#define OTOP()   (top(&ostack))

#define OEMPTY() (empty(&ostack))

#define DIGIT(c) (((c) >= '0') && ((c) <= '9'))

static struct stack {
    int last;
    long elt[LEN_MAX];
} nstack, ostack;

static char *saveptr=NULL;

static int prio_1(long op)
{
    return op==T_PLUS || op==T_MULT? 1: 0;
}
static int prio_2(long op)
{
    return op==T_PLUS? 2: op==T_MULT? 1: 0;
}
static int (*prio)()=&prio_1;

static long push(struct stack *s, long val)
{
    s->elt[s->last++]=val;
    return val;
}
static long pop(struct stack *s)
{
    return s->elt[--s->last];
}
static long top(struct stack *s)
{
    return s->elt[s->last-1];
}
static long empty(struct stack *s)
{
    return s->last==0;
}

static void print() {
    int i;
    printf("NSTACK: ");
    for (i=0; i<nstack.last; ++i) {
        printf("%ld ", nstack.elt[i]);
    }
    printf("\nOSTACK: ");
    for (i=0; i<ostack.last; ++i) {
        printf("%c ", (char) -ostack.elt[i]);
    }
    printf("\n");
}
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
                   case ')':
            case '*':
            case '+':
                val=-c;
                break;
            case '\n':
            case '\0':
                val=T_END;
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

static long eval_top()
{
    long val2 = NPOP();
    long val1 = NPOP();
    char op = OPOP();
    NPUSH(op==T_PLUS? val1+val2: val1*val2);
    return NTOP();
}

static long eval_expr()
{
    long res=T_LPAR;

    res=get_tok();
    while (res!=T_ERR && res!=T_END) {
        switch (res) {
            case T_LPAR:
                OPUSH(res);
                break;
            case T_RPAR:
                while(!OEMPTY() && OTOP() != T_LPAR)
                    eval_top();
                if(!OEMPTY())                     // remove '('
                    OPOP();
                break;
            case T_PLUS:
            case T_MULT:

                while (!OEMPTY() && (*prio)(OTOP()) >= (*prio)(res))
                    eval_top();
                OPUSH(res);
                break;

            default:
                NPUSH(res);
                break;
        }
        res=get_tok();
        //print();
    }
    while(!OEMPTY())
        eval_top();

    // return NSTACK's top
    //printf("Returning %ld\n", NTOP());
    return NPOP();
//end:
//    return left;
}

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
