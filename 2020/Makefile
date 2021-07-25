SUBDIRS := $(shell echo day??)

EXCLUDE := --exclude 'cob-01/'

.PHONY: clean $(SUBDIRS)

clean:
	for dir in $(SUBDIRS) ; do \
		make -C $$dir clean ; \
	done

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
