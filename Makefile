SUBDIRS := $(shell echo day??)
INSTALLDIR := ~/dev/www/tk.bodiccea/advent-2020
DEST := arwen:www/tk.bodiccea/advent-2020

EXCLUDE := --exclude 'cob-01/'

.PHONY: clean deploy $(SUBDIRS)

clean:
	for dir in $(SUBDIRS) ; do \
		make -C $$dir clean ; \
	done

deploy: clean
	rsync -aHixv $(EXCLUDE) --delete --delete-excluded ./ $(INSTALLDIR)
	rsync -aHixv $(EXCLUDE) ./ $(DEST) --delete --delete-excluded

all: $(SUBDIRS)

$(SUBDIRS):
	@echo "========================================="
	@echo "================= $@ ================="
	@echo "========================================="
	@echo
	@echo "+++++++++++++++++ ex1"
	@$(MAKE) --no-print-directory -C $@ ex1 2>&1
	@echo "+++++++++++++++++ ex2"
	@$(MAKE) --no-print-directory -C $@ ex2 2>&1

output:
	@$(MAKE) --no-print-directory all >OUTPUT 2>&1
