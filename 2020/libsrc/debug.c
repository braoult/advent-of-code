/* debug.c - debug/log management
 *
 * Copyright (C) 2021-2022 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifndef DEBUG_DEBUG
#define DEBUG_DEBUG
#endif

#include "bits.h"
#include "debug.h"

#define NS_SEC   1000000000                       /* nano sec in sec */
#define MS_SEC   1000000                          /* microsec in sec */
#define NS_MS    1000                             /* nano in micro */

static long long timer_start;                     /* in nanosecond */
static u32 debug_level=0;

void debug_level_set(u32 level)
{
    debug_level = level;

    log(1, "debug level set to %u\n", level);
}

void debug_init(u32 level)
{
    struct timespec timer;

    debug_level_set(level);
    if (!clock_gettime(CLOCK_MONOTONIC, &timer)) {
        timer_start = timer.tv_sec * NS_SEC + timer.tv_nsec;
    }
    else {
        timer_start = 0;
    }
    log(0, "timer started.\n");
}

inline static long long timer_elapsed()
{
    struct timespec timer;

    clock_gettime(CLOCK_MONOTONIC, &timer);
    return (timer.tv_sec * NS_SEC + timer.tv_nsec) - timer_start;
}

/* void debug - log function
 * @timestamp : boolean
 * @indent    : indent level (2 spaces each)
 * @src       : source file/func name (or NULL)
 * @line      : line number
 */
void debug(u32 level, bool timestamp, u32 indent, const char *src,
           u32 line, const char *fmt, ...)
{
    if (level > debug_level)
        return;

    va_list ap;

    if (indent)
        printf("%*s", 2*(indent-1), "");

    if (timestamp) {
        long long diff = timer_elapsed();
        printf("%lld.%03lld ", diff / NS_SEC, (diff % NS_SEC) / NS_MS);
        printf("%010lldμs ", diff / NS_MS);
    }

    if (src) {
        if (line)
            printf("[%s:%u] ", src, line);
        else
            printf("[%s] ", src);
    }
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

#ifdef BIN_debug
#include <unistd.h>

int main()
{
    int foo=1;
    debug_init(5);

    log(0, "log0=%d\n", foo++);
    log(1, "log1=%d\n", foo++);
    log(2, "log2=%d\n", foo++);
    log_i(2, "log_i 2=%d\n", foo++);
    log_i(5, "log_i 5=%d\n", foo++);
    log_i(6, "log_i 6=%d\n", foo++);
    log_it(4, "log_it 4=%d\n", foo++);
    log_f(1, "log_f 5=%d\n", foo++);
}
#endif
