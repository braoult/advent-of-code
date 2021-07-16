/* ex1-c: Advent2020 game, day 17/game 2
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

static CELL cube[SIZE][SIZE][SIZE][SIZE];

#define CUBEZERO           cube[0][0][0][0]
#define CUBE2X(cell)       (((cell)-&CUBEZERO)/SIZE/SIZE/SIZE)
#define CUBE2Y(cell)       (((cell)-&CUBEZERO)/SIZE/SIZE%SIZE)
#define CUBE2Z(cell)       (((cell)-&CUBEZERO)/SIZE%SIZE)
#define CUBE2W(cell)       (((cell)-&CUBEZERO)%SIZE)

#define HEADRESET(elt)     {                                            \
        (elt)->next=POISON_POINTER1;(elt)->prev=POISON_POINTER1; }

static void reset_cell(CELL *pcell)
{
    pcell->state=INACTIVE;
    pcell->oldstate=INACTIVE;
    pcell->visited=false;
    pcell->count=0;
}

static void reset_cube()
{
    int i, j, k, l;

    for (i=0; i<SIZE; ++i)
        for (j=0; j<SIZE; ++j)
            for (k=0; k<SIZE; ++k)
                for (l=0; l<SIZE; ++l)
                    reset_cell(&cube[i][j][k][l]);
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

static void init_row(char *line, int row)
{
    int i, x, y, z, w;
    static int ncols=0;
    CELL *pcell;

    if (ncols == 0)
        ncols=strlen(line)-1;
    y=ZERO-ncols/2+row;
    z=ZERO;
    w=ZERO;
    for (i=0; *line; ++i, ++line) {
        x=ZERO+i-ncols/2;
        pcell=&cube[x][y][z][w];
        if (*line == '#') {
            list_add(&pcell->set, &qset);
            list_add(&pcell->viewed, &qviewed);
            pcell->state=ACTIVE;
            pcell->visited=1;
        }
    }
}

void run_life()
{
    struct cell *pcell, *ptmp;
    int x, y, z, w, x1, y1, z1, w1;

    /* 1) count +1 for neighbors */
    list_for_each_entry(pcell, &qset, set) {
        x=CUBE2X(pcell);
        y=CUBE2Y(pcell);
        z=CUBE2Z(pcell);
        w=CUBE2W(pcell);
        for (x1=x-1; x1<=x+1; x1++) {
            for (y1=y-1; y1<=y+1; y1++) {
                for (z1=z-1; z1<=z+1; z1++) {
                    for (w1=w-1; w1<=w+1; w1++) {
                        if ((x1!=x || y1!=y || z1!=z || w1!=w)) {
                            ptmp=&cube[x1][y1][z1][w1];
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
    }
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

    while (fgets(line, sizeof line, stdin)) {
        init_row(line, nline);
        nline++;
    }
    for (int i=0; i<LOOPS; ++i)
        run_life();

    printf("%s : res=%d\n", *av, count_active());

    exit (0);
}
