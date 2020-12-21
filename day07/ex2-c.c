/* ex2-c: Advent2020 game, day 7/game 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    struct chain *chain;
    struct node *next;
    int num;
    //int taken;
};

struct chain {
    char key[80];               /* arbitrary, to avoid multiple mallocs.
                                   char* would be better */
    struct chain *next;         /* if collision */
    struct node *contains;      /* bags contained */
    struct node *isin;          /* is part of */
    int taken;
};

#define HASH_SIZE 1024
static struct chain *thehash[HASH_SIZE];
static nkeys=0;

unsigned long get_hash(str)
    char *str;
{
    register unsigned long hash = 5381;
    register int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

struct chain *find_key(key)
    char *key;
{
    unsigned long hash;
    struct chain *entry;

    hash=get_hash(key)%HASH_SIZE;
    entry=thehash[hash];
    while (entry) {
        if (!strcmp(key, entry->key))
            break;
        entry=entry->next;
    }
    return entry;
}

struct chain *insert_key(key)
    char *key;
{
    unsigned long hash;
    struct chain *entry;

    if (!(entry=find_key(key))) {
        hash=get_hash(key)%HASH_SIZE;
        entry=malloc(sizeof (struct chain));

        strcpy(entry->key, key);
        entry->next=thehash[hash];
        thehash[hash]=entry;

        entry->contains=NULL;
        entry->isin=NULL;
        entry->taken=0;
        nkeys++;
    }
    return entry;
}

void insert_node(container, contained, num)
    struct chain *container;
    struct chain *contained;
    int num;
{
    struct node *node;

    node=malloc(sizeof node);
    node->chain=contained;
    node->num=num;
    //node->taken=0;
    node->next=container->contains;
    container->contains=node;

    node=malloc(sizeof node);
    node->chain=container;
    node->num=0;
    //node->taken=0;
    node->next=contained->isin;
    contained->isin=node;
}

void print_node_list(node)
    struct node*node;
{
    while (node) {
        printf("\t\t%d %s", node->num, node->chain->key);
        node=node->next;
    }
    printf("\n");

}

int count_containers(chain)
    struct chain *chain;
{
    struct chain *p;
    struct node *node;
    int i=0;

    node = chain->isin;
    while (node) {
        p=node->chain;
        if (p->taken == 0) {
            i++;
            p->taken=1;
            i+=count_containers(p);
        }
        node=node->next;
    }
    chain->taken=1;
    return i;
}
int count_contained(chain)
    struct chain *chain;
{
    struct chain *p;
    struct node *node;
    int i=1, num;

    node = chain->contains;
    while (node) {
        p=node->chain;
        num=node->num;
        i+=num*count_contained(p);
        node=node->next;
    }
    return i;
}

void print_keys()
{
    int i, j;
    struct chain *chain;
    struct node *node;

    for (i=0; i<HASH_SIZE; ++i) {
        chain=thehash[i];
        j=0;
        while (chain) {
            printf("%d: %d=[%s]\n", i, j, chain->key);
            node=chain->contains;
            if (node) {
                printf("\tCONTAINS:");
                print_node_list(node);
            }

            node=chain->isin;
            if (node) {
                printf("\tIS IN:");
                print_node_list(node);
            }

            chain=chain->next;
            j++;
        }
    }
}

char*sep=" \t\n";               /* separator */

char *getcolor(line)
    char *line;
{
    static char buf[80], *tok, *ret=NULL;

    if (tok=strtok(line, sep)) {
        strcpy(buf, tok);
        if (tok=strtok(NULL, sep)) {
            ret=strcat(buf, tok);
        }
    }
    return ret;
}

int main(ac, av)
    int ac;
    char **av;
{
    int num=0, count=0;
    char line[512], *color, *numstr;
    struct chain *entry, *isin;

    if (ac != 2) {
        fprintf(stderr, "usage: %s key\n", *av);
        exit (1);
    }
    while (fgets(line, sizeof line, stdin)) {
        count++;
        if (!(color=getcolor(line))) {
            continue;
        }
        entry=insert_key(color);

        // skip "bag" and "contain"
        strtok(NULL, sep);
        strtok(NULL, sep);

        while (numstr=strtok(NULL, sep)) {
            if (num=atoi(numstr)) {
                color=getcolor(NULL);
                isin=insert_key(color);
                insert_node(entry, isin, num);
                strtok(NULL, sep);
            }
        }

        continue;
    }
    //print_keys();
    entry=find_key(*(av+1));
    num=count_contained(entry)-1; /* ex 2 */
    //num=count_containers(entry); /* ex 1 */

    printf("%s : target=%s nkeys=%d res=%d\n", *av, *(av+1), nkeys, num);
    exit (0);
}
