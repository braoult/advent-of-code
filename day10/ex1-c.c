/* ex1-c: Advent2020 game, day 10/game 1
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

static struct list *add_val(list, val)
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

unsigned *calc(list)
    struct list *list;
{
    static unsigned res[4];
    unsigned long long *ptr=list->list;
    unsigned last=list->last, i;

    for (i=0; i<4; ++i)
        res[i]=0;
    for (i=1; i<last; ++i) {
        unsigned prev=ptr[i-1], cur=ptr[i];
        res[cur-prev]++;
    }
    return res;
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[80];
    struct list *list=NULL;
    unsigned long long res, last;
    unsigned *result;

    list=add_val(list, 0);
    while (fgets(line, sizeof line, stdin)) {
        sscanf(line, "%llu", &res);
        list=add_val(list, res);
    }
    mergesort(list->list, 0, list->last-1);
    last=list->list[list->last-1];
    list=add_val(list, last+3);
    //print_list(list);
    result=calc(list);
    printf("%s : diff1=%u diff2=%u res=%u\n", *av, result[1], result[3],
           result[1]*result[3]);
    exit (0);
}
