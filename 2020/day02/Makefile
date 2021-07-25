INPUT := INPUT.txt
SHELL := /bin/bash
CFLAGS := -w
TIME := \time -f "\ttime: %E real, %U user, %S sys\n\tcontext-switch:\t%c+%w, page-faults: %F+%R\n"
export PATH := .:$(PATH)

.PHONY: clean all compile deploy ex1 ex2

all: ex1 ex2

compile: ex1-c ex2-c

ex1: ex1-c
	@$(TIME) ex1.bash < $(INPUT)
	@$(TIME) ex1-c < $(INPUT)

ex2: ex2-c
	@$(TIME) ex2.bash < $(INPUT)
	@#$(TIME) ex2-sort.bash < $(INPUT)
	@$(TIME) ex2-c < $(INPUT)

clean:
	@rm -f ex1-c ex2-c core

deploy:
	@$(MAKE) -C .. deploy
