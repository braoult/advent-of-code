# AOC Makefile - GNU make only.
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


SUBDIRS := $(shell echo day??)

CC       = gcc

#LIBS   = -lreadline -lncurses
CFLAGS   += -std=gnu11

CFLAGS += -O2
CFLAGS += -g
#CFLAGS += -pg
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -march=native

CFLAGS += -DDEBUG_DEBUG		# activate general debug (debug.c)
CFLAGS += -DDEBUG_POOL		# memory pools management

INCDIR    := ./include
LIBSRCDIR := ./libsrc
LIBDIR    := ./lib
LIB       := libaoc_$(shell uname -m)
SLIB      := $(LIBDIR)/$(LIB).a
DLIB      := $(LIBDIR)/$(LIB).so
LIBSRC    := $(wildcard $(LIBSRCDIR)/*.c)
LIBOBJ    := $(patsubst %.c,%.o,$(LIBSRC))
LDFLAGS   := -L$(LIBDIR)
LDLIB     := -l$(LIB)

.PHONY: clean cleanlib cleanall all redo output lib $(SUBDIRS)

all: lib $(SUBDIRS)

clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) --no-print-directory -C $$dir clean ; \
	done

cleanlib: clean
	@$(RM) -f $(SLIB) $(DLIB) $(LIBOBJ)

cleanall: clean cleanlib

redo: cleanall all

$(SUBDIRS):
	@echo "========================================="
	@echo "================= $@ ================="
	@echo "========================================="
	@echo
	@echo "+++++++++++++++++ part 1"
	+@$(MAKE) --no-print-directory -C $@ ex1 2>&1
	@echo "+++++++++++++++++ part 2"
	+@$(MAKE) --no-print-directory -C $@ ex2 2>&1

output:
	@$(MAKE) --no-print-directory all >OUTPUT 2>&1

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
