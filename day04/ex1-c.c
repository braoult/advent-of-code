/* ex1-c: Advent2020 game, day 4/game 1
 */

#include <stdio.h>
#include <stdlib.h>

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

int valid_passport()
{
     int valid=1;
     struct key *key=&keys[0];

     while (key->mandatory != -1) {
          if (key->mandatory && !*key->val) {
               valid=0;
               break;
          }
          key++;
     }
     return valid;
}

char *my_getkeyZOBI(str)            /* search string before ':' */
     char *str;
{
     char *p=str;

     while (*str++ == ' ')      /* skip spaces */
          ;
     p=str;
     while (*p && *p++ != ':')  /* skip spaces */
          ;
     *p='\0';
     return my_strlen(str) ? str: NULL;
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
               nvalid += valid_passport();
               count++;
               reset_keys();
          }
     }
     nvalid += valid_passport();
     count++;
     printf("%s : valid=%d/%d\n", *av, nvalid, count);
     exit (0);
}
