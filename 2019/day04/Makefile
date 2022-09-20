# AOC daily Makefile - GNU make only.
#
# Copyright (C) 2021 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#
# You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
#

INPUT   := INPUT.txt
SHELL   := /bin/bash

CC      := gcc

LIB     := aoc_$(shell uname -m)
INCDIR  := ../include
LIBDIR  := ../lib
LDFLAGS := -L$(LIBDIR)
#LDLIB   := -l$(LIB) -lm
LDLIB   := -l$(LIB)

export LD_LIBRARY_PATH = $(LIBDIR)

CFLAGS  += -std=gnu99
CFLAGS  += -O2
CFLAGS  += -g
# for gprof
#CFLAGS += -pg
CFLAGS  += -Wall
CFLAGS  += -Wextra
CFLAGS  += -march=native
# Next one may be useful for valgrind (some invalid instructions)
# CFLAGS += -mno-tbm
CFLAGS  += -Wmissing-declarations
CFLAGS  += -Wno-unused-result

CFLAGS  += -DDEBUG_DEBUG 	# activate general debug (debug.c)
CFLAGS  += -DDEBUG_POOL		# memory pools management

VALGRIND := valgrind
VALGRINDFLAGS := -q -s --leak-check=full --show-leak-kinds=all --track-origins=yes


TIME    := \time -f "\ttime: %E real, %U user, %S sys\n\tcontext-switch:\t%c+%w, page-faults: %F+%R\n"
export PATH := .:$(PATH)

.PHONY: clean all compile assembly memcheck memcheck1 memcheck2 ex1 ex2

all: README.org ex1 ex2

memcheck: memcheck1 memcheck2

memcheck1: aoc-c
	@$(VALGRIND) $(VALGRINDFLAGS) aoc-c -p 1 < $(INPUT)

memcheck2: aoc-c
	@$(VALGRIND) $(VALGRINDFLAGS) aoc-c -p 2 < $(INPUT)
	@#@valgrind -s --track-origins=yes aoc-c -p 2 < $(INPUT)

compile: aoc-c

assembly: aoc-c.s

ex1: aoc-c
	@$(TIME) aoc-c -p 1 < $(INPUT)

ex2: aoc-c
	@$(TIME) aoc-c -p 2 < $(INPUT)

clean:
	@rm -f aoc-c core* vgcore* gmon.out aoc-c.s aoc-c.i README.html

.c:
	@echo compiling $<
	@$(CC) $(CFLAGS) $(LDFLAGS) -I $(INCDIR) $< $(LDLIB) -o $@

# generate pre-processed file (.i) and assembler (.s)
%.i: %.c
	@echo generating $@
	@$(CC) -E $(CFLAGS) -I $(INCDIR) $< -o $@

%.s: %.c
	@echo generating $@
	@$(CC) -S -fverbose-asm $(CFLAGS) -I $(INCDIR) $< -o $@

# generate README.org from README.html (must cleanup !)
%.org: %.html
	@echo generating $@. Cleanup before commit !
	@pandoc $< -o $@
