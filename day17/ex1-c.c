/* ex1-c: Advent2020 game, day 17/game 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include "list.h"

#define LOOPS    6
#define MAXINIT  8
#define SIZE     ((2*LOOPS)+MAXINIT)
#define ZERO     (SIZE/2)

#define ACTIVE   '#'
#define INACTIVE '.'

typedef struct cell {
    char state;
    char oldstate;
    bool visited;                                 /* redundant ? */
    unsigned count;                               /* active neighbors */
    // for runs, we don't care unused cells
    struct list_head set;                         /* current actives */
    struct list_head viewed;                      /* current visited */
} CELL;

LIST_HEAD(qset);
LIST_HEAD(qviewed);

static CELL cube[SIZE][SIZE][SIZE];

#define R(x)               ((x)+ROOT)
#define SET(pcell,x,y,z)   (pcell[R(x)]=
#define CUBE2X(cell)       (((cell)-&cube[0][0][0])/SIZE/SIZE)
#define CUBE2Y(cell)       (((cell)-&cube[0][0][0])/SIZE%SIZE)
#define CUBE2Z(cell)       (((cell)-&cube[0][0][0])/(SIZE/SIZE)%SIZE)

#define HEADRESET(elt)     { \
        (elt)->next=POISON_POINTER1;(elt)->prev=POISON_POINTER1; }


static void reset_cell(CELL *pcell)
{
    pcell->state=INACTIVE;
    pcell->oldstate=INACTIVE;
    pcell->visited=false;
    pcell->count=0;
    //HEADRESET(&pcell->set);
    //HEADRESET(&pcell->viewed);
}
static void reset_cube()
{
    int i, j, k;

    for (i=0; i<SIZE; ++i)
        for (j=0; j<SIZE; ++j)
            for (k=0; k<SIZE; ++k)
                reset_cell(&cube[i][j][k]);
}
static void check_cube_sanity()
{
    CELL *pos;
    int x, y, z, count1=0, count2=0, count=0;

    /* check state of active cells */
    list_for_each_entry(pos, &qset, set) {
        count1++;
        if (pos->state != ACTIVE)
            printf("cell (%ld,%ld,%ld): state is not ACTIVE.\n",
               CUBE2X(pos), CUBE2Y(pos), CUBE2Z(pos));
        if (pos->visited != 1)
            printf("cell (%ld,%ld,%ld): state is not VISITED.\n",
               CUBE2X(pos), CUBE2Y(pos), CUBE2Z(pos));
    }
    for (x=0; x<SIZE; ++x) {
        for (y=0; y<SIZE; ++y) {
            for (z=0; z<SIZE; ++z) {
                if (cube[x][y][z].state == ACTIVE) {
                    count2++;
                    if (cube[x][y][z].count != 0)
                        count++;
                    if (cube[x][y][z].visited != 1)
                        printf("cell (%d,%d,%d): ACTIVE & not VISITED.\n",
                               x, y, z);
                } else {
                    if (cube[x][y][z].visited != 0)
                        printf("cell (%d,%d,%d): INACTIVE & VISITED.\n",
                               x, y, z);
                }
            }
        }
    }
    printf("sanity check: count1=%d, count2=%d count=%d\n",
           count1, count2, count);
}
static void print_cell(CELL *p)
{
    printf("(%ld,%ld,%ld): st=%c vi=%d co=%d\n",
           CUBE2X(p), CUBE2Y(p), CUBE2Z(p),
           p->state, p->visited, p->count);

}
static void set_print_cell(struct list_head *p)
{
    print_cell(list_entry(p, CELL, set));
}
static void viewed_print_cell(struct list_head *p)
{
    print_cell(list_entry(p, CELL, viewed));
}
static void print_cube()
{
    int x, y, z;

    for (z=0; z<SIZE; ++z) {                      /* # plan */
        printf("z=%d\n", z-ZERO);
        for (y=0; y<SIZE; ++y) {
            for (x=0; x<SIZE; ++x) {
                putchar(cube[x][y][z].state);
            }
            putchar('\n');
        }
        putchar('\n');
    }
}

void print_set()
{
    struct cell *pos;
    printf("set: %p n=%p p=%p\n", &qset, qset.next, qset.prev);
    list_for_each_entry(pos, &qset, set) {
        printf("  %p: %p %p - %ld %ld %ld\n",
               &pos->set, pos->set.prev, pos->set.next,
               CUBE2X(pos), CUBE2Y(pos), CUBE2Z(pos));
    }
}
void print_viewed()
{
    struct cell *pos;
    printf("viewed: %p n=%p p=%p\n", &qviewed, qviewed.next, qviewed.prev);
    list_for_each_entry(pos, &qviewed, viewed) {
        printf("  %p: %p %p\n", &pos->viewed, pos->viewed.prev, pos->viewed.next);
    }
}

static int count_active()
{
    struct list_head *p;
    int i=0;

    list_for_each(p, &qset) {
        ++i;
    }
    return i;
}
/* fill initial plane */
static void add_row(char *line, int row)
{
    int i, x, y, z;
    static int ncols=0;
    CELL *pcell;

    printf("LINE %d: %lu %s\n", row, strlen(line)-1, line);
    if (ncols == 0)
        ncols=strlen(line)-1;
    y=ZERO-ncols/2+row;
    z=ZERO;

    for (i=0; *line; ++i, ++line) {
        x=ZERO+i-ncols/2;
        pcell=&cube[x][y][z];
        printf("pos=(%d,%d,%d): %#x ", x, y, z, *line);
        printf(" --> (%ld, %ld, %ld)\n", CUBE2X(pcell),
               CUBE2Y(pcell), CUBE2Z(pcell));
        if (*line == '#') {
            list_add(&pcell->set, &qset);
            list_add(&pcell->viewed, &qviewed);
            pcell->state=ACTIVE;
            pcell->visited=1;
            //print_set();
        }
    }
}

void run_life()
{
    struct cell *pcell, *ptmp;
    int x, y, z, x1, y1, z1;

    /* 1) count +1 for neighbors */
    list_for_each_entry(pcell, &qset, set) {
        x=CUBE2X(pcell);
        y=CUBE2Y(pcell);
        z=CUBE2Z(pcell);
        for (x1=x-1; x1<=x+1; x1++) {
            for (y1=y-1; y1<=y+1; y1++) {
                for (z1=z-1; z1<=z+1; z1++) {
                    if ((x1!=x || y1!=y || z1!=z)) {
                        ptmp=&cube[x1][y1][z1];
                        ++ptmp->count;
                        if (!ptmp->visited) {
                            list_add(&ptmp->viewed, &qviewed);
                            ptmp->visited=1;
                        }
                    }
                }
            }
        }
    }
    /*print_set();
      print_viewed();*/
    /* 2) apply rules for all visited cells */
    list_for_each_entry_safe(pcell, ptmp, &qviewed, viewed) {
        switch (pcell->state) {
            case ACTIVE:
                if (pcell->count != 2 && pcell->count != 3) {
                    list_del(&pcell->set);
                    pcell->state=INACTIVE;
                    pcell->visited=0;
                }
                break;
            case INACTIVE:
                if (pcell->count == 3) {
                    list_add(&pcell->set, &qset);
                    pcell->state=ACTIVE;
                    pcell->visited=1;
                }
                break;
        }
        pcell->count=0;
        if (pcell->state == INACTIVE) {
            pcell->visited=0;
            list_del(&pcell->viewed);
        }
    }
    printf("active cells:\n");
    list_for_each_entry(pcell, &qset, set) {
        print_cell(pcell);
    }
    printf("viewed cells:\n");
    list_for_each_entry(pcell, &qviewed, viewed) {
        print_cell(pcell);
    }
    check_cube_sanity();
}

int main(ac, av)
    int ac;
    char **av;
{
    char line[16];
    int nline=0;

    reset_cube();
    INIT_LIST_HEAD(&qset);
    INIT_LIST_HEAD(&qviewed);
    printf("set: %p n=%p p=%p\n", &qset, qset.next, qset.prev);
    printf("vis: %p n=%p p=%p\n", &qviewed, qviewed.next, qviewed.prev);
    while (fgets(line, sizeof line, stdin)) {
        add_row(line, nline);
        nline++;
    }
    for (int i=0; i<LOOPS; ++i)
        run_life();
    /*print_cube();
    run_life();
    print_cube();
    run_life();
    print_cube();
    run_life();
    print_cube();*/

    printf("%s : res=%d\n", *av, count_active());

    exit (0);
}



/*
void print_count(plane)
    struct plane *plane;
{
    unsigned i, psize=plane->last;
    unsigned ncols=plane->ncol;
    struct seats *ptr=plane->seats;

    for (i=0; i<psize; ++i) {
        if (i>0 && !(i%ncols)) {
            putchar('\n');
        }
        printf("%2d ", (ptr+i)->neighbours);
    }
    putchar('\n');
}

void print_seats(plane)
    struct plane *plane;
{
    unsigned i, psize=plane->last;
    unsigned ncols=plane->ncol, nrow=plane->nrow;
    struct seats *ptr=plane->seats;

    fprintf(stderr, "PLANE: address=%p seat=%p rows=%d cols=%d size=%d\n",
            plane, ptr, nrow, ncols, psize);
    for (i=0; i<psize; ++i) {
        if (i>0 && !(i%ncols)) {
            putchar('\n');
        }
        printf("%c ", (ptr+i)->status);
    }
    putchar('\n');
}

void reset_seats(plane)
    struct plane *plane;
{
    unsigned i, last=plane->last;
    struct seats *seat=plane->seats;

    for (i=0; i<last; ++i, ++seat)
        seat->neighbours=0;
}

struct plane *add_row(plane, c)
    struct plane *plane;
    char *c;
{
    unsigned size;

    if (!plane) {
        plane=malloc(sizeof(struct plane));
        plane->seats=malloc(sizeof(struct seats)*BLOCKSIZE);
        plane->size=BLOCKSIZE;
        plane->ncol=strlen(c)-1;
        plane->last=0;
    }
    size=plane->size;

    while (*c) {
        if (*c != '\n') {
            if (plane->last == size) {
                size+=BLOCKSIZE;
                plane->size=size;
                plane->seats=realloc(plane->seats, sizeof(struct seats)*size);
            }
            plane->seats[plane->last].status=*c;
            plane->last++;
            plane->nrow=plane->last/plane->ncol;
        }
        c++;
    }
    //sscanf(line, "%d", &val);

    return plane;
}

int sit(plane)
    struct plane *plane;
{
    unsigned changed=0, cur, seated=0;
    unsigned last=plane->last;
    struct seats *seats=plane->seats;

    for (cur=0; cur<last; ++cur) {
        switch (seats[cur].status) {
            case '.':
                break;
            case '#':
                if (seats[cur].neighbours >= 4) {
                    seats[cur].status='L';
                    changed++;
                } else {
                    seated++;
                }
                break;
            case 'L':
                if (seats[cur].neighbours == 0) {
                    seats[cur].status='#';
                    changed++;
                    seated++;
                }
                break;
        }
    }
    plane->seated=seated;
    return changed;
}

int calc(plane)
    struct plane *plane;
{
    unsigned row, col, cur;
    unsigned last=plane->last;
    unsigned cols=plane->ncol;
    unsigned rows=plane->nrow;
    struct seats *seats=plane->seats;

    for (cur=0; cur<last; ++cur) {
        row=cur/cols;
        col=cur%cols;
        if (seats[cur].status == '#') {
            if (row > 0) {
                seats[cur - cols].neighbours++;
                if (col > 0)
                    seats[cur - cols - 1].neighbours++;
                if (col < (cols-1))
                    seats[cur - cols + 1].neighbours++;
            }
            if (col > 0) {
                seats[cur - 1].neighbours++;
                if (row < (rows-1))
                    seats[cur + cols - 1].neighbours++;
            }
            if (col < (cols-1)) {
                seats[cur + 1].neighbours++;
                if (row < (rows-1))
                    seats[cur + cols + 1].neighbours++;
            }
            if (row < (rows-1))
                seats[cur + cols].neighbours++;
        }
    }
    return 1;
}
*/
