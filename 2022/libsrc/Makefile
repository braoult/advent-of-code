# AOC Makefile - GNU make only.
#
# Copyright (C) 2021-2022 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#
# You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
#

SHELL    := /bin/bash
CC       := gcc
BEAR     := bear
CCLSFILE:= compile_commands.json

#LIBS   = -lreadline -lncurses
CFLAGS   += -std=gnu11

CFLAGS += -O2
CFLAGS += -g
# for gprof
#CFLAGS += -pg
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -march=native

CFLAGS += -DDEBUG_DEBUG		# activate general debug (debug.c)
CFLAGS += -DDEBUG_POOL		# memory pools management
# Next one may be useful for valgrind (some invalid instructions)
# CFLAGS += -mno-tbm
CFLAGS  += -Wmissing-declarations
CFLAGS  += -Wno-unused-result

INCDIR    := ../include
LIBSRCDIR := .
LIBDIR    := ../lib
LIB       := libaoc_$(shell uname -m)
SLIB      := $(LIBDIR)/$(LIB).a
DLIB      := $(LIBDIR)/$(LIB).so
LIBSRC    := $(wildcard $(LIBSRCDIR)/*.c)
LIBOBJ    := $(patsubst %.c,%.o,$(LIBSRC))
LDFLAGS   := -L$(LIBDIR)
LDLIB     := -l$(LIB)

.PHONY: clean cleanlib cleanall all redo output lib $(SUBDIRS)

all: lib $(SUBDIRS)

compile: $(LIBOBJ)

clean:
	@$(RM) -f *.o *.s *.i $(LIBOBJ)

cleanall: clean
	@$(RM) -f $(SLIB) $(DLIB) $(LIBOBJ)

lib: $(DLIB) $(SLIB)

$(SLIB): $(LIBOBJ)
	@echo building $@ static library.
	@mkdir -p $(LIBDIR)
	@$(AR) $(ARFLAGS) -o $@ $^

$(DLIB): CFLAGS += -fPIC
$(DLIB): LDFLAGS += -shared
$(DLIB): $(LIBOBJ)
	@echo building $@ shared library.
	@mkdir -p $(LIBDIR)
	@$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	@echo compiling $<.
	@$(CC) -c $(CFLAGS) $(LDFLAGS) -I $(INCDIR) -o $@ $<

# generate pre-processed file (.i) and assembler (.s)
%.i: %.c
	@echo generating $@
	@$(CC) -E $(CFLAGS) -I $(INCDIR) $< -o $@

%.s: %.c
	@echo generating $@
	@$(CC) -S -fverbose-asm $(CFLAGS) -I $(INCDIR) $< -o $@

bear: clean
	@touch .ccls-root
	@$(BEAR) -- make compile
