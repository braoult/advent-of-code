/* ex2-c: Advent2020 game, day 9/game 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct list {
    unsigned size;
    unsigned last;
    unsigned long long *list;
};

#define DEFNSUM		25
#define BLOCKSIZE	1024    /* number of elements for realloc() */

void print_list(list)
    struct list *list;
{
    unsigned i, psize=list->last;
    unsigned long long *ptr=list->list;

    fprintf(stderr, "PROGRAM: address=%p pinstr=%p size=%d\n", list, ptr, psize);
    for (i=0; i<psize; ++i)
        printf("[%u] %llu\n", i, *(ptr+i));
}

static struct list *add_line(list, line)
    struct list *list;
    char *line;
{
    int val;
    unsigned cur, size;
    unsigned long long *ptr;

    if (!list) {
        list=malloc(sizeof(struct list));
        list->list=malloc(sizeof(unsigned long long)*BLOCKSIZE);
        list->size=BLOCKSIZE;
        list->last=0;
    }
    cur=list->last;
    size=list->size;

    if (cur == size) {
        size+=BLOCKSIZE;
        list->size=size;
        list->list=realloc(list->list, sizeof(unsigned long long)*size);
        fprintf(stderr, "realloc buf: cur=%d size=%d ptr=%p\n", cur, size, list->list);
    }
    ptr=list->list+cur;

    sscanf(line, "%d", &val);
    *ptr=val;
    list->last++;

    return list;
}

unsigned long long badnum(list, cur, nsum)
    struct list *list;
    int cur;
    unsigned nsum;
{
    unsigned long long *ptr=list->list;
    unsigned long long res=*(ptr+cur);
    int start=cur-nsum, i, j;

    for (i=start; i<cur; ++i) {
        for (j=i+1; j<cur; ++j) {
            if (ptr[i]+ptr[j] == res)
                return 0;
        }
    }
    return res;
}

unsigned long long findsum(list, target, start)
    struct list *list;
    unsigned long long target;
    int start;
{
    unsigned long long sum=0;
    unsigned int i;
    unsigned size=list->last;
    unsigned long long *ptr=list->list, cur;
    unsigned long long min=*(ptr+start), max=min;

    for (i=start; i<size && sum<target; ++i) {
        cur=*(ptr+i);
        sum+=cur;
        if (cur < min)
            min=cur;
        if (cur > max)
            max=cur;
        if (sum == target) {
            return min+max;
        }
    }
    return 0;
}

int main(ac, av)
    int ac;
    char **av;
{
    unsigned nsum=DEFNSUM;
    char line[80];
    struct list *list=NULL;
    unsigned i;
    unsigned long long res1, res2;

    if (ac==2) {
        nsum=atoi(*(av+1));
    }

    while (fgets(line, sizeof line, stdin)) {
        list=add_line(list, line);
    }
    //print_list(list);
    for (i=nsum; i<list->last; ++i) {
        if (res1=badnum(list, i, nsum)) {
            //printf("%s : res=%llu\n", *av, res1);
            break;
        }
    }
    for (i=0; i<list->last; ++i) {
        if (res2=findsum(list, res1, i)) {
            printf("%s : res=%llu sum=%llu\n", *av, res1, res2);
            break;
        }
    }
    exit (0);
}
