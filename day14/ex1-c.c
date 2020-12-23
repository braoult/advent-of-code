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
        if (*p != '0')
            mask+=exp;
    return mask;
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[1024];
    unsigned long long val, mempos, or, and;

    while (fgets(line, sizeof line, stdin)) {
        if (!strncmp(line, "mask", 4)) {
            sscanf(line, "%*[^01X]%s", maskstr);
            or=ormask(maskstr);
            and=andmask(maskstr);
        } else {
            sscanf(line, "%*[^0-9]%llu] = %llu", &mempos, &val);
            //printf("mem[%llu]=%llu\n", mempos, val);
            set_cell(mempos, (val&and)|or);
        }
    }
    //print_cells();

    printf("%s : res=%llu\n", *av, calc_cells());
    exit (0);
}
