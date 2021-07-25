/* ex1-c: Advent2020 game, day 16/game 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* tickets ranges */
struct srange {
    char *name;
    int s1, e1;
    int s2, e2;
};

struct list {
    int size;
    int last;
    struct srange *ranges;
} list = {
    0, 0, NULL
};

#define BLOCKSIZE	1024    /* number of elements for realloc() */

int *myticket=NULL;
int *curticket=NULL;            /* current ticket */
int ticketsize=0;

void print_ranges()
{
    unsigned i, n=list.last;
    struct srange *p=list.ranges;

    fprintf(stderr, "LIST: address=%p last=%u size=%u\n", list.ranges, n, list.size);
    for (i=0; i<n; ++i, ++p)
        printf("\t[%03u] %u-%u %u-%u [%s]\n", i, p->s1, p->e1, p->s2, p->e2, p->name);
}

struct srange *add_range(name, s1, e1, s2, e2)
    char *name;
    int s1, e1, s2, e2;
{
    struct srange *p;

    if (list.last == list.size) {
        list.ranges=realloc(list.ranges, sizeof(struct srange)*BLOCKSIZE);
        list.size+=BLOCKSIZE;
    }
    p=list.ranges+list.last;
    p->name=strdup(name);
    p->s1=s1;
    p->e1=e1;
    p->s2=s2;
    p->e2=e2;
    list.last++;
    return p;
}

void print_ticket(ticket)
    int *ticket;
{
    int i;
    for (i=0; i<ticketsize; ++i)
        printf("\t%2d: %d\n", i, *(ticket+i));
}

int *parse_ticket(str)
    char *str;
{
    int i;
    char *p;

    if (!curticket)
        curticket=malloc(ticketsize * sizeof *myticket);
    for (i=0, p=str; i<ticketsize; ++i, ++p)
        curticket[i]=(int)strtol(p, &p, 10);
    return curticket;
}

int *parse_myticket(str)
    char *str;
{
    int i=0;
    char *p;

    ticketsize=1;
    for (p=str; *p; p++)
        if (*p==',')
            ticketsize++;

    myticket=malloc(ticketsize * sizeof *myticket);
    for (p=str, i=0; i<ticketsize; ++i, ++p)
        myticket[i]=(int)strtol(p, &p, 10);
    return myticket;
}

int is_in_range(i, range)
    int i;
    struct srange *range;
{
    if ((i>=range->s1 && i<=range->e1) || (i>=range->s2 && i<=range->e2))
        return 1;
    else
        return 0;
}

int ex1(ticket)
    int *ticket;
{
    int i, r, res=0, found;

    for (i=0; i<ticketsize; ++i) {
        found=0;
        for (r=0; r<list.last; ++r) {
            if (is_in_range(*(ticket+i), list.ranges+r)) {
                found=1;
                break;
            }
        }
        if (!found)
            res+=ticket[i];
    }
    return res;
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[1024], pname[80], *elabel;
    int s1, e1, s2, e2, end=0, status=0, res=0;
    int *ticket;

    while (fgets(line, sizeof line, stdin)) {
        if (*line=='\n')
            continue;
        if (*line >= '0' && *line <= '9') {
            switch (status) {
                case 1:
                    ticket=parse_myticket(line);
                    break;
                case 2:
                    ticket=parse_ticket(line);
                    res += ex1(ticket);
                    break;
            }
            continue;
        }
        if (elabel=strchr(line, ':')) {
            if (*(elabel+1)=='\n') {
                status++;
                continue;
            }
            sscanf(line, "%[^:]: %d-%d or %d-%d %n",
                   pname, &s1, &e1, &s2, &e2, &end);
            add_range(pname, s1, e1, s2, e2);

        }
    }
    printf("%s : res=%d\n", *av, res);
    exit (0);
}
