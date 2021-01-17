/* ex2-c: Advent2020 game, day 16/game 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* tickets ranges */
struct srange {
    char *name;
    int s1, e1;
    int s2, e2;
    int col;
    int possiblecount;
    int *possible;              /* possible[i]=-1 => column i cannot be */
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
    int j;
    struct srange *p=list.ranges;

    printf("RANGES: address=%p last=%u size=%u\n", list.ranges, n, list.size);
    for (i=0; i<n; ++i, ++p) {
        printf("  [%03u] %u-%u %u-%u [%s]\n", i, p->s1, p->e1, p->s2, p->e2, p->name);
        printf("    Possible cols (%d remaining) : ", p->possiblecount);
        for (j=0; j<ticketsize; ++j)
            if (p->possible[j] >= 0)
                printf(" %d", p->possible[j]);
        putchar('\n');
    }
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
    p->col=-1;
    list.last++;
    return p;
}

void print_ticket(ticket)
    int *ticket;
{
    int i;
    printf("Ticket: ");
    for (i=0; i<ticketsize; ++i)
        printf("%d ", *(ticket+i));
    putchar('\n');
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
    int i, j, *possible;
    char *p;

    ticketsize=1;
    for (p=str; *p; p++)
        if (*p==',')
            ticketsize++;

    myticket=malloc(ticketsize * sizeof *myticket);
    for (p=str, i=0; i<ticketsize; ++i, ++p)
        myticket[i]=(int)strtol(p, &p, 10);
    // set possible columns for all ranges
    for (i=0; i<list.last; ++i) {
        possible=malloc(sizeof (int) * ticketsize);
        list.ranges[i].possible=possible;
        list.ranges[i].possiblecount=ticketsize;
        for (j=0; j<ticketsize; ++j)
            possible[j]=j;
    }
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

int check_valid(ticket)
    int *ticket;
{
    int i, r, found;

    for (i=0; i<ticketsize; ++i) {
        found=0;
        for (r=0; r<list.last; ++r) {
            if (is_in_range(*(ticket+i), list.ranges+r)) {
                found=1;
                break;
            }
        }
        if (!found)
            break;
    }
    return found;
}

void ex2_add(ticket)
    int *ticket;
{
    int i, r;

    if (check_valid(ticket)) {
        for (i=0; i<ticketsize; ++i) {
            for (r=0; r<list.last; ++r) {
                if (!is_in_range(*(ticket+i), list.ranges+r)) {
                    // this value cannot be in this range, we unset this column
                    list.ranges[r].possible[i]=-1;
                    list.ranges[r].possiblecount--;
                }
            }
        }
    }
}

unsigned long ex2()
{
    int i, r, r1, unique=0;
    unsigned long res=1;
    struct srange *range, *range1;

    while (!unique) {
        unique=1;
        for (r=0; r<list.last; ++r) {
            range=list.ranges+r;
            if (range->col >= 0)
                continue;
            if (range->possiblecount == 1) {
                for (i=0; range->possible[i]==-1 && i<ticketsize; ++i)
                    ;
                range->col=i;
                for (r1=0; r1<list.last; ++r1) {
                    range1=list.ranges+r1;
                    if (range1 != range && range1->possible[i] != -1) {
                        range1->possible[i]=-1;
                        range1->possiblecount--;
                        unique=0;
                    }
                }
                break;
            }
        }
    }
    for (r=0; r<list.last; ++r) {
        range=list.ranges+r;
        if (!strncmp(range->name, "departure", 9)) {
            res*=myticket[range->col];
        }
    }
    return res;
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[1024], pname[80], *elabel;
    int s1, e1, s2, e2, end=0, status=0;
    unsigned long res=0;
    int *ticket;

    while (fgets(line, sizeof line, stdin)) {
        if (*line=='\n')
            continue;
        if (*line >= '0' && *line <= '9') {
            switch (status) {
                case 1:
                    ticket=parse_myticket(line);
                    // print_ranges();
                    break;
                case 2:
                    ticket=parse_ticket(line);
                    // valid ticket
                    // print_ticket(ticket);
                    ex2_add(ticket);
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
    res=ex2();
    printf("%s : res=%lu\n", *av, res);
    exit (0);
}
