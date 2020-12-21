INPUT := INPUT.txt
SHELL := /bin/bash
CFLAGS := -w
TIME := \time -f "\ttime: %E real, %U user, %S sys\n\tcontext-switch:\t%c+%w, page-faults: %F+%R\n"
export PATH := .:$(PATH)

.PHONY: clean all compile deploy ex1 ex2

all: ex1 ex2

compile: ex1-c ex2-c

ex1: compile
	@$(TIME) ex1.bash < $(INPUT) 2>&1
	@$(TIME) ex1-bis.bash < $(INPUT) 2>&1
	@$(TIME) ex1-c < $(INPUT) 2>&1

ex2: compile
	@$(TIME) ex2.bash < $(INPUT) 2>&1
	@$(TIME) ex2-bis.bash < $(INPUT) 2>&1
	@$(TIME) ex2-c < $(INPUT) 2>&1

bash:
	@$(TIME) $(MAKE) --no-print-directory bash-run

c: compile
	@$(TIME) $(MAKE) --no-print-directory c-run

bash-run:
	@ex1.bash < $(INPUT) 2>&1
	@ex2.bash < $(INPUT) 2>&1

c-run:
	@ex1-c < $(INPUT) 2>&1
	@ex2-c < $(INPUT) 2>&1

clean:
	@rm -f ex1-c ex2-c core

deploy:
	@$(MAKE) -C .. deploy
