/* aoc-c.c: Advent of Code 2022, day 7
 *
 * Copyright (C) 2022 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pool.h"
#include "debug.h"
#include "br.h"
#include "aoc.h"
#include "hashtable.h"
#include "pjwhash-inline.h"

/* we will totally ignore the tree structure here and only keep a hashtable
 * to keep directories sizes.
 */
#define HBITS 9                                   /* 9 bits: size is 512 */
#define DIRNAME_SIZE 128

DEFINE_HASHTABLE(hasht_dir, HBITS);

typedef struct dir {
    char name[DIRNAME_SIZE];
    uint namelen;
    uint hash;
    int size;
    struct hlist_node hlist;
} dir_t;

static pool_t *pool_dirs;

/**
 * find_dir - find entry in an hashtable bucket
 */
static dir_t *find_dir(struct hlist_head *head, uint hash, char *dir, uint len)
{
    dir_t *cur;

    hlist_for_each_entry(cur, head, hlist)
        if (cur->hash == hash && cur->namelen == len && !memcmp(cur->name, dir, len))
                return cur;
    return NULL;
}

static dir_t *find_dirname(char *name, uint len)
{
    uint hash = pjwhash(name, len);
    uint bucket = hash_32(hash, HBITS);

    return find_dir(&hasht_dir[bucket], hash, name, len);
}

/**
 * parent_dir - get new len of parent directory
 */
static int parent_dir(char *dir, uint len)
{
    for (--len; len && *(dir + len) != '/'; --len)
        ;
    return len? len: 1;
}

static void adjust_dirsize(dir_t *dir, uint size)
{
    dir_t *cur = dir;
    uint len = cur->namelen;

    while (len > 1) {
        cur = find_dirname(dir->name, len);
        if (!cur) {
            log(1, "FATAL hash issue");
            exit(1);
        }
        cur->size +=size;
        len = parent_dir(dir->name, len);
    };
    cur = find_dirname("/", 1);                   /* do not forget "/" */
    cur->size += size;
}

/**
 * add_dir - add an entry in hashtable
 */
static struct dir *add_dir(char *dir, uint len)
{
    uint hash = pjwhash(dir, len);
    uint bucket = hash_32(hash, HBITS);
    dir_t *new = pool_get(pool_dirs);

    memcpy(new->name, dir, len);
    new->namelen = len;
    new->hash = hash;
    new->size = 0;
    hlist_add_head(&new->hlist, &hasht_dir[bucket]);
    return new;
}

/**
 * add_dir_maybe - add an entry in hashtable if it does not exist
 */
static struct dir *add_dir_maybe(char *dir, int len)
{
    dir_t *new = find_dirname(dir, len);
    return new? new: add_dir(dir, len);
}

static struct dir *cd(struct dir *dir, char *to, uint tolen)
{
    char *newname = dir->name;
    int newlen, add_slash = 0;

    if (!dir)
        return add_dir_maybe(to, tolen);
    if (dir->namelen + tolen + 1 > DIRNAME_SIZE)  /* conservative (think / or ..) */
        return NULL;
    if (*to == '.') {                             /* .. = parent dir */
        newlen = parent_dir(dir->name, dir->namelen);
    } else if (*to == '/') {                      /* root */
        newname = to;
        newlen = tolen;
    } else {                                      /* subdir */
        newlen = dir->namelen + tolen;
        if (dir->namelen > 1) {                   /* add '/' separator */
            dir->name[dir->namelen] = '/';
            add_slash = 1;
            newlen++;
        }
        memcpy(dir->name + dir->namelen + add_slash, to, tolen);
    }
    return add_dir_maybe(newname, newlen);
}

#define CDLEN (sizeof("$ cd ") - 1)

static void parse()
{
    dir_t *curdir = NULL;
    size_t alloc = 0;
    char *buf = NULL, *tok;
    ssize_t buflen;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        tok = buf;
        if (*tok == '$' &&*(tok+2) == 'c')        /* ignore "ls"" */
            curdir = cd(curdir, tok + CDLEN, buflen - CDLEN);
        else if (isdigit(*tok))                   /* ignore "dir" keyword */
            adjust_dirsize(curdir, atoi(tok));
    }
    free(buf);
    return;
}

static int solve(int part)
{
    ulong bucket;
    dir_t *cur;
    int res = 0, needed;

    if (part == 1) {
        hash_for_each(hasht_dir, bucket, cur, hlist)
            if (cur->size <= 100000)
                res += cur->size;
    } else {
        res = find_dirname("/", 1)->size;
        needed = res - (70000000-30000000);
        hash_for_each(hasht_dir, bucket, cur, hlist)
            if (cur->size >= needed && cur->size < res)
                res = cur->size;
    }
    return res;
}

int main(int ac, char **av)
{
    int part = parseargs(ac, av);
    pool_dirs = pool_create("dirs", 128, sizeof(dir_t));

    parse();
    printf("%s: res=%d\n", *av, solve(part));
    pool_destroy(pool_dirs);
    exit(0);
}
