INPUT := INPUT.txt
SHELL := /bin/bash
#CFLAGS := -w -g
#CFLAGS := -w -g -pg
CFLAGS := -w -O3
TIME := \time -f "\ttime: %E real, %U user, %S sys\n\tcontext-switch:\t%c+%w, page-faults: %F+%R\n"
export PATH := .:$(PATH)

.PHONY: clean all compile deploy ex1 ex2

all: ex1 ex2

output:
	@$(MAKE) --no-print-directory all 2>&1 > OUTPUT

compile: ex1-c ex2-c

ex1:
	@$(TIME) ex1-v1.bash < $(INPUT) 2>&1
	@$(TIME) ex1.bash < $(INPUT) 2>&1
	@$(TIME) ex1-c < $(INPUT) 2>&1

ex2:
	@$(TIME) ex2.bash < $(INPUT) 2>&1
	@$(TIME) ex2-c < $(INPUT) 2>&1

clean:
	@rm -f ex1-c ex2-c core

deploy:
	@$(MAKE) -C .. deploy
