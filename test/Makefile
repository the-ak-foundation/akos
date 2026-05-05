$(info [TEST] Specify test to build via TEST.)

ifdef TEST
    $(info [TEST] TEST=$(TEST).)
    RULE = $(shell echo $(TEST) | tr '[:upper:]' '[:lower:]')
else
    RULE = all
endif

TESTS = list prio rdy_list

.PHONY: prebuild all $(TESTS)

ifeq ($(RULE),all)
all: $(TESTS)
else
$(RULE): prebuild
	make -C $@
endif

prebuild:
	. ./setup.sh
