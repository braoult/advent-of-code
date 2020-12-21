/* ex1-c: Advent2020 game, day 10/game 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct seats {
    char status;
    unsigned neighbours;
};

struct plane {
    unsigned nrow;
    unsigned ncol;
    unsigned size;
    unsigned last;
    unsigned seated;
    struct seats *seats;
};

#define BLOCKSIZE	(10*1024) /* number of elements for realloc() */

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

int main(ac, av)
    int ac;
    char **av;
{
    char line[512];
    struct plane *plane=NULL;

    while (fgets(line, sizeof line, stdin)) {
        plane=add_row(plane, line);
    }
    do {
        reset_seats(plane);
        calc(plane);
    } while (sit(plane) > 0);

    printf("%s : res=%d\n", *av, plane->seated);
    exit (0);
}
