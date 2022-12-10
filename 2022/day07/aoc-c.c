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

static void print_hash()
{
    ulong bucket;
    dir_t *cur;
    log(3, "------------------- hashtable\n");
    hash_for_each(hasht_dir, bucket, cur, hlist) {
        log(3, "hash=%lu/%u [%.*s] size=%u\n", bucket, cur->hash,
            cur->namelen, cur->name, cur->size);
    }
    log(3, "-------------------\n");
}

/**
 * find_dir - find entry in an hashtable bucket
 */
static dir_t *find_dir(struct hlist_head *head, uint hash, char *dir, uint len)
{
    dir_t *cur;
    print_hash();

    hlist_for_each_entry(cur, head, hlist) {
        if (cur->hash == hash) {
            if (cur->namelen == len && !memcmp(cur->name, dir, len)) {
                log(4, "found hash [%.*s]\n", len, dir);
                return cur;
            }
        }
    }
    log(4, "hash [%.*s] not found\n", len, dir);
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
    log_f(4, "len=%u [%.*s] ", len, len, dir);
    if (len > 1) {                                /* not root dir */
        char *p = dir + len - 1;
        for (; *p != '/'; p--)                    /* assume there is always '/' */
            ;
        len = p - dir;
        if (!len)
            len = 1;
    }
    log(4, "newlen=%u [%.*s]\n", len, len, dir);
    return len;
}

static void adjust_dirsize(dir_t *dir, uint size)
{
    dir_t *cur = dir;
    uint len = cur->namelen;

    log_f(3, "dir=[%.*s]\n", len, cur->name);
    while (len > 1) {
        cur = find_dirname(dir->name, len);
        //bucket = hash_32(hash, HBITS);
        //cur = find_dir(&hasht_dir[bucket], hash, dir->name, len);
        if (!cur) {
            log(1, "FATAL hash issue");
            exit(1);
        }
        log_f(3, "adjusting %.*s size from %u to %u\n", len, cur->name,
              cur->size, cur->size + size);
        cur->size +=size;
        len = parent_dir(dir->name, len);
    };
    //hash = pjwhash("/", 1);
    //bucket = hash_32(hash, HBITS);
    cur = find_dirname("/", 1);
    cur->size += size;
}

/**
 * add_dir - add an entry in hashtable
 */
static struct dir *add_dir(char *dir, uint len, uint hash, uint bucket, int calc)
{
    dir_t *new = pool_get(pool_dirs);
    if (calc) {                                   /* not calculated yet */
        hash = pjwhash(dir, len);
        bucket = hash_32(hash, HBITS);
    }
    memcpy(new->name, dir, len);
    new->namelen = len;
    new->hash = hash;
    new->size = 0;
    hlist_add_head(&new->hlist, &hasht_dir[bucket]);
    log(3, "NEW DIR: ");
    log(3, "hash(%.*s)=%u/%u\n", len, dir, hash, bucket);
    return new;
}

/**
 * add_dir_maybe - add an entry in hashtable if it does not exist
 */
static struct dir *add_dir_maybe(char *dir, int len)
{
    uint hash = pjwhash(dir, len), bucket = hash_32(hash, HBITS);
    dir_t *new;
    if (!(new = find_dir(&hasht_dir[bucket], hash, dir, len))) {
        new = add_dir(dir, len, hash, bucket, 0);
        log(5, "add_maybe: added [%.*s]\n", len, dir);
    } else {
        log(5, "add_maybe: found existing [%.*s]\n", len, dir);
    }
    return new;
}

static struct dir *cd(struct dir *dir, char *to, uint tolen)
{
    //char *p = dir->name + dir->namelen - 1, *newname = dir->name;
    char *newname = dir->name;
    dir_t *newdir;
    int newlen;

    if (dir->namelen + tolen + 1 > DIRNAME_SIZE)  /* conservative (think / or ..) */
        return NULL;

    if (*to == '.') {                             /* .. = parent dir */
        newlen = parent_dir(dir->name, dir->namelen);
        if (!newlen)
            newlen = 1;

        log(3, "GOING UP from=%d[%.*s] ", dir->namelen, dir->namelen, newname);
        log(3, "to=%d[%.*s] ", tolen, tolen, to);
        log(3, "diff=%d\n", newlen);
        log(3, "new = %d[%.*s]\n", newlen, newlen, newname);
    } else if (*to == '/') {                      /* root */
        newname = to;
        newlen = tolen;
        log(3, "GOING ROOT from %d[%.*s]\n", dir->namelen, dir->namelen, dir->name);
    } else {                                      /* subdir */
        int notroot = 0;
        newlen = dir->namelen + tolen;
        if (dir->namelen > 1) {                   /* +1 for '/' */
            dir->name[dir->namelen] = '/';
            notroot = 1;
        }
        memcpy(dir->name + dir->namelen + notroot, to, tolen);
        newlen += notroot;
        log(3, "GOING DOWN from=%d[%.*s] ", dir->namelen, dir->namelen, dir->name);
        log(3, "to=%d[%.*s] ", tolen, tolen, to);
        log(3, "new = %d[%.*s]\n", newlen, newlen, dir->name);
    }
    newdir = add_dir_maybe(newname, newlen);
    return newdir;
}

#define CDLEN (sizeof("$ cd ") - 1)

static void parse(dir_t *curdir)
{
    size_t alloc = 0;
    char *buf = NULL, *tok;
    ssize_t buflen;
    uint size;

    while ((buflen = getline(&buf, &alloc, stdin)) > 0) {
        buf[--buflen] = 0;
        tok = buf;
        printf("PARSING buf=[%s] len=%zu\n", buf, buflen);
        if (*tok == '$') {
            if (*(tok+2) == 'l')                  /* ignore ls */
                continue;
            tok += CDLEN;
            printf("****** buf=[%s] len=%zu\n", tok, buflen - CDLEN);
            curdir = cd(curdir, tok, buflen - CDLEN);
        } else if (*tok != 'd') {                 /* ignore "dir" keyword */
            size = atoi(tok);
            printf("****** size=%u\n", size);
            adjust_dirsize(curdir, size);
        }
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
    puts("zobi");
    dir_t *root = add_dir("/", 1, 0, 0, 1);
    puts("zobi");
    parse(root);
    print_hash();
    printf("%s: res=%d\n", *av, solve(part));
    exit(0);
}
