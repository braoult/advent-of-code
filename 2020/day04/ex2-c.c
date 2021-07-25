/* ex2-c: Advent2020 game, day 4/game 2
 */

#include <stdio.h>
#include <stdlib.h>

#define	BYR	0
#define	IYR	1
#define	EYR	2
#define	HGT	3
#define	HCL	4
#define	ECL	5
#define	PID	6
#define	CID	7

struct key {
    int mandatory;
    char *key;
    char val[80];
} keys[] = {
    { 1, "byr", "" },
    { 1, "iyr", "" },
    { 1, "eyr", "" },
    { 1, "hgt", "" },
    { 1, "hcl", "" },
    { 1, "ecl", "" },
    { 1, "pid", "" },
    { 0, "cid", "" },
    { -1, NULL,  "" }
};

char *eyes[] = {
    "amb", "blu", "brn", "gry", "grn", "hzl", "oth", NULL
};

int my_strlen(str)
    char *str;
{
    int i;
    for (i=0; *str; ++i, ++str)
        ;
    return i;
}

int my_strcmp(str1, str2)
    char *str1, *str2;
{
    for(; *str1 && (*str1==*str2); str1++, str2++)
        ;
    return *str1 - *str2;
}

char *my_strcpy(dst, src)
    char *dst, *src;
{
    char *p=dst;
    while (*dst++ = *src++)
        ;
    return p;
}
int my_digitstr(str)
    char *str;
{
    for (; *str >= '0' && *str <= '9'; ++str)
        ;
    return *str? 0: 1;
}

struct key *get_key(key)
    char *key;
{
    int i=0, found=0;

    for (i=0; keys[i].mandatory != -1; ++i) {
        if (!my_strcmp(keys[i].key, key)) {
            found=1;
            break;
        }
    }
    return found? &keys[i]: NULL;
}
int get_key_num(key)
    char *key;
{
    struct key *k=get_key(key);

    return k? k-&keys[0]: -1;
}

struct key *set_key(key, val)
    char *key, *val;
{
    struct key *thekey;

    if (thekey=get_key(key))
        my_strcpy(thekey->val, val);
    return thekey;
}

void print_keys()
{
    int i=0;

    printf("keys: \n");
    for (i=0; keys[i].mandatory != -1; ++i)
        printf("\t[%s]=[%s]\n", keys[i].key, keys[i].val);
    return;
}

void reset_keys()
{
    int i=0;
    for (i=0; keys[i].mandatory != -1; ++i)
        *keys[i].val='\0';
    return;
}
int check_eyes(val)
    char *val;
{
    int ret=0;
    char **col=eyes;

    while (*col) {
        if (!my_strcmp(val, *col)) {
            ret=1;
            break;
        }
        col++;
    }
    return ret;
}
int valid_key(keypos)
    int keypos;
{
    int good=0, i, len;
    char *val=keys[keypos].val, buf[80];

    len=my_strlen(val);
    switch (keypos) {
        case BYR:
            if (len == 4 && my_digitstr(val)) {
                i=atoi(val);
                if (i >= 1920 && i <= 2002)
                    good=1;
            }
            break;
        case IYR:
            if (len == 4 && my_digitstr(val)) {
                i=atoi(val);
                if (i >= 2010 && i <= 2020)
                    good=1;
            }
            break;
        case EYR:
            if (len == 4 && my_digitstr(val)) {
                i=atoi(val);
                if (i >= 2020 && i <= 2030)
                    good=1;
            }
            break;
        case HGT:
            if (sscanf(val, "%d%s", &i, buf) == 2) {
                if ((!my_strcmp(buf, "cm") && i >= 150 && i <= 193 ) ||
                    (!my_strcmp(buf, "in") && i >= 59 && i <= 76 ))
                    good=1;
            }
            break;
        case HCL:
            if (len == 7 && *val == '#') {
                i = sscanf(val+1, "%[a-f0-9]", buf);
                good = 1;
            }
            break;
        case ECL:
            good=check_eyes(val);
            break;
        case PID:
            if (my_strlen(val) == 9 &&
                my_digitstr(val)) {
                good=1;
            }
            break;
        case CID:
                good=1;
                break;
        default:
            break;
    }
    return good;
}

int valid_passport()
{
    int valid=1;
    struct key *key=&keys[0];

    while (key->mandatory != -1) {
        if (!valid_key(key-&keys[0])) {
            valid=0;
            break;
        }
        key++;
    }
    return valid;
}

int main(ac, av)
    char **av;
{
    int consumed, nkeys, nvalid=0, count=0;
    char line[160], key[80], val[80];
    char *p;

    while (fgets(line, sizeof line, stdin)) {
        nkeys=0;
        p=line;
        consumed=1;
        while (sscanf(p, "%[^:]: %s %n", key, val, &consumed)==2) {
            set_key(key, val);
            p+=consumed;
            nkeys++;
        }
        if (!nkeys) {         /* empty line: reset */
            nvalid+=valid_passport();
            count++;
            reset_keys();
        }
    }
    nvalid += valid_passport();
    count++;
    printf("%s : valid=%d/%d\n", *av, nvalid, count);
    exit (0);
}
