/* ex1-c: Advent2020 game, day 14/game 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct memcell {
    unsigned long long num;
    unsigned long long val;
    struct memcell *next;       /* if collision */
};
struct hashtable {
    struct memcell *first;
};

#define HASHSIZE	1024
#define HASH(i)		((i)%HASHSIZE)

static struct hashtable mem[HASHSIZE];
static char maskstr[128];
static unsigned long long exp[128];

struct memcell *get_cell(cell)
    unsigned long long cell;
{
    struct memcell *p=mem[HASH(cell)].first;

    while (p && p->num != cell)
        p=p->next;
    return p;
}

struct memcell *set_cell(cell, val)
    unsigned long long cell;
    unsigned long long val;
{
    struct memcell *p=get_cell(cell);
    unsigned hash=HASH(cell);

    if (!p) {
        p=malloc(sizeof (struct memcell));
        p->next=mem[hash].first;
        mem[hash].first=p;
    }
    p->num=cell;
    p->val=val;
    return p;
}

unsigned long long print_cells()
{
    unsigned i;
    unsigned long long res=0;
    struct memcell *p;

    puts("CELLS :");
    for (i=0; i<HASHSIZE; ++i) {
        p=mem[i].first;
        while (p) {
            printf("[%llu]=%llu\n", p->num, p->val);
            res+=p->val;
            p=p->next;
        }
    }
    return res;
}

unsigned long long calc_cells()
{
    unsigned i;
    unsigned long long res=0;
    struct memcell *p;

    for (i=0; i<HASHSIZE; ++i) {
        p=mem[i].first;
        while (p) {
            res+=p->val;
            p=p->next;
        }
    }
    return res;
}

unsigned long long ormask(str)
    char *str;
{
    char *p;
    unsigned long long mask=0, exp=1;

    for (p=str+strlen(str)-1; p>=str; p--, exp<<=1)
        if (*p == '1')
            mask+=exp;
    return mask;
}

unsigned long long andmask(str)
    char *str;
{
    char *p;
    unsigned long long mask=0, exp=1;

    for (p=str+strlen(str)-1; p>=str; p--, exp<<=1)
        if (*p != 'X')
            mask+=exp;
    return mask;
}

void print_exp()
{
    int i;
    for (i=0; exp[i]; ++i) {
        printf("%2d: %llu", i, exp[i]);
    }

}

void calc_masks(mempos, val, cur)
    unsigned long mempos, val;
    int cur;
{
    unsigned long long mask=exp[cur];

    if (mask == 0) {
        set_cell(mempos, val);
        return;
    }
    calc_masks(mempos, val, cur+1);
    calc_masks(mempos+mask, val, cur+1);
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[1024];
    unsigned long long val, mempos, or, and;
    int i=0, j;

    while (fgets(line, sizeof line, stdin)) {
        if (!strncmp(line, "mask", 4)) {
            sscanf(line, "%*[^01X]%s", maskstr);
            or=ormask(maskstr);
            and=andmask(maskstr);
            val=1; j=0;
            for (i=strlen(maskstr)-1; i>=0; --i) {
                if (maskstr[i]=='X') {
                    exp[j]=val;
                    j++;
                }

                val<<=1;
            }
            exp[j]=0;
        } else {
            sscanf(line, "%*[^0-9]%llu] = %llu", &mempos, &val);
            mempos=(mempos|or)&and;
            calc_masks(mempos, val, 0);
        }
    }
    printf("%s : res=%llu\n", *av, calc_cells());
    exit (0);
}
