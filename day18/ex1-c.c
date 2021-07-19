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
static long get_tok() //, char **saveptr, long *res)
{
    char *p, c;
    long val=0;

    //printf("%s(%p, %p)\n", __func__, ptr, *saveptr);
    p=saveptr;
    //printf("%s: str=[%s]\n", __func__, p);
    while (!val) {
        c=*p;
        //printf("  %s: c=[%c] str=[%s]\n", __func__, c, p);
        switch (c) {
            case ' ':
                //case '\n':
                break;
            case '(':
                val=T_SUB;
                p++;
                saveptr=p;
                return eval_expr();
                break;
            case ')':
            case '\n':
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
                    //printf("   int parse: c=[%c] val=%d\n", c, val);
                    val=(val*10 + c - '0');
                    //printf("     -> val=%d\n", val);
                    p++;
                    c=*p;
                }
                p--;
                break;
        }
        p++;
    }
    saveptr=p;
    //printf("%s END: str=[%s] val=%ld p=[%s]\n", __func__, p, val, *saveptr);
    return val;
}
static char* debug_tok(long tok)
{
    static char res[32];

    switch (tok) {
        case T_END:
            sprintf (res, "T_END");
            break;
        case T_PLUS:
            sprintf (res, "T_PLUS");
            break;
         case T_MULT:
             sprintf (res, "T_MULT");
            break;
         case T_ERR:
             sprintf (res, "T_ERR");
            break;
        case T_SUB:
            sprintf (res, "T_SUB");
            break;
        default:
            sprintf (res, "TNUM[%ld]", tok);
            break;
    }
    return res;
}

static long eval_expr()
{
    long res=T_SUB, op=T_ERR;
    long left=0;

    res=get_tok();
    left=res;
    printf("--->EVAL TOK=: %ld tag=%s \n", res, debug_tok(res));
    while (res!=T_ERR && res!=T_END) {
        switch (res) {
            case T_END:
                //printf ("RETURNING: %ld\n", left);
                //return left;
                goto end;
                break;
            case T_PLUS:
            case T_MULT:
                op=res;
                break;
            case T_ERR:
                left=res;
                //return T_ERR;
                goto end;
                break;
             //case T_SUB:
             //sprintf (res, "T_SUB");
             //break;
            default:
                //printf ("DEFAULT(left=%ld, res=%ld)\n" , left, res);
                switch (op) {
                    case T_PLUS:
                        left+=res;
                        break;
                    case T_MULT:
                        left*=res;
                        break;
                }
                printf ("DEFAULT(left=%ld, res=%ld) - " , left, res);
                printf ("New left: %ld\n", left);
                break;
        }
        res=get_tok();
        printf("get_tok: %ld tag=%s \n", res, debug_tok(res));
        // printf("get_tok: %ld tag=%s \n", res, debug_tok(res));

    }
end:
    printf ("RETURNING: %ld\n", left);
    return left;
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[1024];
    long res=0, tmp;

    while (fgets(line, sizeof line, stdin)) {
        //*(line+strlen(line)-1)=0;
        saveptr=line;
        //printf("get_tok: %ld tag=%s \n", res, debug_tok(res));
        tmp=eval_expr();
        printf("EXPR=%s\n", debug_tok(tmp));
        res+=tmp;
        //} while (res!=T_ERR && res!=T_END);
            //res+=build_tree(line, &pnext);
    }
    printf("%s : res=%ld\n", *av, res);
    exit (0);
}
