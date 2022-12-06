/* ex2-c: Advent2020 game, day 10/game 2
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

void merge(array, left, mid, right)
    unsigned long long *array;
    int left, mid, right;
{
    unsigned long long temp[right-left+1];
    int pos=0, lpos = left, rpos = mid + 1, iter;
    while(lpos <= mid && rpos <= right) {
        if(array[lpos] < array[rpos]) {
            temp[pos++] = array[lpos++];
        }
        else {
            temp[pos++] = array[rpos++];
        }
    }
    while(lpos <= mid)  temp[pos++] = array[lpos++];
    while(rpos <= right)temp[pos++] = array[rpos++];
    for(iter = 0;iter < pos; iter++) {
        array[iter+left] = temp[iter];
    }
    return;
}

void mergesort(array, left, right)
    unsigned long long *array;
    int left, right;
{
    int mid = (left+right)/2;
    if(left<right) {
        mergesort(array, left, mid);
        mergesort(array, mid+1,right);
        merge(array, left, mid, right);
    }
}

void print_list(list)
    struct list *list;
{
    unsigned i, psize=list->last;
    unsigned long long *ptr=list->list;

    fprintf(stderr, "LIST: address=%p pinstr=%p size=%d\n", list, ptr, psize);
    for (i=0; i<psize; ++i)
        printf("[%u] %llu\n", i, *(ptr+i));
}

struct list *add_val(list, val)
    struct list *list;
    unsigned long long val;
{
    //int val;
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

    //sscanf(line, "%d", &val);
    *ptr=val;
    list->last++;

    return list;
}

unsigned long long calc(list)
    struct list *list;
{
    unsigned long long *res, result;
    unsigned long long *ptr=list->list;
    unsigned last=list->last, ival, jval;
    unsigned diff, i, j;

    res=malloc(sizeof(unsigned long long)*list->last);

    for (i=1; i<last; ++i)
        res[i]=0;
    res[0]=1;

    for (i=0; i<last; ++i) {
        ival=ptr[i];

        for (j=i+1; j<i+4; ++j) {
            if (j<last) {
                jval=ptr[j];
                diff=jval-ival;
                if (diff > 0 && diff < 4) {
                    res[j]+=res[i];
                }
            }
        }
    }
    result=res[last-1];
    free(res);
    return result;
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[80];
    struct list *list=NULL;
    unsigned long long res, last;

    list=add_val(list, 0ll);
    while (fgets(line, sizeof line, stdin)) {
        sscanf(line, "%llu", &res);
        list=add_val(list, res);
    }
    mergesort(list->list, 0, list->last-1);
    last=list->list[list->last-1];
    list=add_val(list, last+3);
    //print_list(list);
    res=calc(list);
    printf("%s : size=%u res=%llu\n", *av, list->last, res);
    exit (0);
}
