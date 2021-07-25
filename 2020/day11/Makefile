INPUT := INPUT.txt
SHELL := /bin/bash
#CFLAGS := -w -pg
CFLAGS := -w -O3
TIME := \time -f "\ttime: %E real, %U user, %S sys\n\tcontext-switch:\t%c+%w, page-faults: %F+%R\n"
export PATH := .:$(PATH)

.PHONY: clean all compile deploy ex1 ex2

all: ex1 ex2

output:
	@$(MAKE) --no-print-directory all 2>&1 > OUTPUT

compile: ex1-c ex2-c

ex1: ex1-c
	@#$(TIME) ex1-slow.bash < $(INPUT) 2>&1
	@$(TIME) ex1.bash < $(INPUT) 2>&1
	@$(TIME) ex1-c < $(INPUT) 2>&1

ex2: ex2-c
	@$(TIME) ex2.bash < $(INPUT) 2>&1
	@$(TIME) ex2-c < $(INPUT) 2>&1

clean:
	@rm -f ex1-c ex2-c core

deploy:
	@$(MAKE) -C .. deploy
