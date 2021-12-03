# AOC Makefile - GNU make only.
#
# Copyright (C) 2021 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#  * You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
#


SUBDIRS := $(shell echo day??)

CC       = gcc

#LIBS   = -lreadline -lncurses
CFLAGS   += -std=gnu99

#CFLAGS += -O2
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -march=native

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

.PHONY: clean cleanall all redo output lib $(SUBDIRS)

all: lib $(SUBDIRS)

clean:
	for dir in $(SUBDIRS) ; do \
		$(MAKE) -C $$dir clean ; \
	done

cleanall: clean
	echo AAAAAAAAAAAAAAAAAAAAA
	$(RM) -f $(SLIB) $(DLIB)

redo: cleanall all

$(SUBDIRS):
	@echo "========================================="
	@echo "================= $@ ================="
	@echo "========================================="
	@echo
	@echo "+++++++++++++++++ ex1"
	+$(MAKE) --no-print-directory -C $@ ex1 2>&1
	@echo "+++++++++++++++++ ex2"
	+$(MAKE) --no-print-directory -C $@ ex2 2>&1

output:
	@$(MAKE) --no-print-directory all >OUTPUT 2>&1

lib: $(DLIB) $(SLIB)

$(SLIB): $(LIBOBJ)
	@#echo ZZZZZZZZZZZZZZZZZZZZ  $(SLIB) $(DLIB)
	mkdir -p $(LIBDIR)
	$(AR) $(ARFLAGS) -o $@ $^

$(DLIB): CFLAGS += -fPIC
$(DLIB): LDFLAGS += -shared
$(DLIB): $(LIBOBJ)
	@#echo YYYYYYYYYYYYYYYYYYYY $(SLIB) $(DLIB)
	mkdir -p $(LIBDIR)
	$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	echo TOTO
	$(CC) -c $(CFLAGS) $(LDFLAGS) -I $(INCDIR) -o $@ $<
