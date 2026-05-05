PRJ_DIR   ?= ../..
BUILD_DIR = $(PRJ_DIR)/build/test/prio
CURR_DIR  = $(PRJ_DIR)/test/prio

PLATFORM = host

include $(PRJ_DIR)/ak/Makefile.mk
include $(PRJ_DIR)/ext/Makefile.mk
include $(PRJ_DIR)/port/Makefile.mk

INCS = $(AK_INCS) $(UNITY_INCS) $(CMSIS_INCS) $(PORT_INCS)
SRCS =\
	$(PRJ_DIR)/ak/src/ak_prio.c\
	$(UNITY_SRCS)\
	$(CURR_DIR)/tests.c\
	$(CURR_DIR)/main.c

include $(PRJ_DIR)/mk/cfg.mk

.PHONY: all prebuild clean

all: prebuild $(BUILD_DIR)/target

prebuild:
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/target: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	@rm -rf $(BUILD_DIR)

include $(PRJ_DIR)/mk/rules.mk
