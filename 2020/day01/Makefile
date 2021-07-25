INPUT := INPUT.txt
SHELL := /bin/bash
CFLAGS := -w
COBFLAGS=-fmfcomment -free -x
TIME := \time -f "\ttime: %E real, %U user, %S sys\n\tcontext-switch:\t%c+%w, page-faults: %F+%R\n"
export PATH := .:$(PATH)

.PHONY: clean all compile deploy ex1 ex2 output

all: ex1 ex2

compile: ex1-c ex2-c ex2-pure-sort-cob ex2-sort-cob

ex1: ex1-c ex1-cob
	@$(TIME) ex1.bash < $(INPUT)
	@echo -n "ex1-cob : res="
	@$(TIME) ex1-cob < $(INPUT)
	@$(TIME) ex1-c < $(INPUT)

ex2: ex2-c ex2-sort-cob ex2-pure-sort-cob
	@$(TIME) ex2.bash < $(INPUT)
	@$(TIME) ex2-sort.bash < $(INPUT)
	@$(TIME) ex2-pure-sort.bash < $(INPUT)
	@echo -n "ex2-sort-cob : res="
	@$(TIME) ex2-sort-cob < $(INPUT)
	@echo -n "ex2-pure-sort-cob : res="
	@$(TIME) ex2-pure-sort-cob < $(INPUT)
	@$(TIME) ex2-c < $(INPUT)

clean:
	@rm -f ex1-c ex2-c core ex2-galoisgirl-cob ex2-cob

output:
	$(MAKE) --no-print-directory all >OUTPUT 2>&1

deploy:
	$(MAKE) -C .. deploy

%: %.cob
	cobc $(COBFLAGS) $^
