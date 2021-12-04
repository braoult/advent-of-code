# AOC daily Makefile - GNU make only.
#
# Copyright (C) 2021 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#  * You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
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
LDLIB   := -l$(LIB)

export LD_LIBRARY_PATH = $(LIBDIR)

CFLAGS += -std=gnu99
CFLAGS += -O2
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -march=native
CFLAGS += -Wmissing-declarations
CFLAGS += -Wno-unused-result

TIME   := \time -f "\ttime: %E real, %U user, %S sys\n\tcontext-switch:\t%c+%w, page-faults: %F+%R\n"
export PATH := .:$(PATH)

.PHONY: clean all compile ex1 ex2

all: ex1 ex2

compile: aoc-c

ex1: aoc-c
	@$(TIME) aoc-c -p 1 < $(INPUT)

ex2: aoc-c
	@$(TIME) aoc-c -p 2 < $(INPUT)

clean:
	@rm -f aoc-c core*

.c:
	@echo compiling $<
	@$(CC) $(CFLAGS) $(LDFLAGS) -I $(INCDIR) $< $(LDLIB) -o $@
