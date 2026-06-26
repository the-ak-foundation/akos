################################################################################
# @file    config.mk
# @brief   Default AKOS build-time configuration values.
#
# @author  HUYNNG | BUUPQ
# @date    Created: 2026-06-11
# @date    Updated: 2026-06-26
#
# @module  AKOS
################################################################################

# ============================================================
# 1. Kernel Config
# ============================================================

OS_CFG_SYSTICK_CLOCK_HZ       ?= 48000000u
OS_CFG_HEAP_SIZE              ?= 2144u
OS_CFG_PRIO_MAX               ?= 10u
OS_CFG_DELAY_MAX              ?= 0xffffffffUL

# ============================================================
# 2. Task Config
# ============================================================

OS_CFG_TASK_STK_SIZE_MIN      ?= 17u
OS_CFG_TASK_STACK_FILL_BYTE   ?= 0x5Au
OS_CFG_TASK_MSG_Q_SIZE_NORMAL ?= 8u

# ============================================================
# 3. Pool Config
# ============================================================

OS_CFG_MSG_POOL_SIZE          ?= 32u
OS_CFG_TIMER_POOL_SIZE        ?= 8u
OS_CFG_TIMER_TASK_PRI         ?= 0u

# ============================================================
# 4. Feature Config
# ============================================================

OS_CFG_USE_LOG                ?= 1u
OS_CFG_USE_CLI                ?= 0u

# ============================================================
# 5. Build Log Config
# ============================================================

AKOS_BUILD_FULL_PATH ?= 0

AKOS_COLOR_RESET  := \033[0m
AKOS_COLOR_STEP   := \033[1;36m
AKOS_COLOR_TARGET := \033[1;32m
AKOS_COLOR_PATH   := \033[0;37m

ifeq ($(AKOS_BUILD_FULL_PATH),1)
AKOS_Q :=
else
AKOS_Q := @
endif

define AKOS_LOG
	@printf "$(AKOS_COLOR_STEP)%s.$(AKOS_COLOR_RESET) %s $(AKOS_COLOR_TARGET)%s$(AKOS_COLOR_RESET)\n" "$(1)" "$(2)" "$(3)"
endef

define AKOS_LOG_PATH
	@printf "$(AKOS_COLOR_STEP)%s.$(AKOS_COLOR_RESET) %s $(AKOS_COLOR_PATH)%s$(AKOS_COLOR_RESET)\n" "$(1)" "$(2)" "$(3)"
endef

AKOS_DISPLAY = $(if $(filter 1,$(AKOS_BUILD_FULL_PATH)),$(1),$(notdir $(1)))

ifeq ($(AKOS_BUILD_FULL_PATH),1)
define AKOS_SIZE
	$(AKOS_Q)$(SIZE) $(1)
endef
else
define AKOS_SIZE
	$(AKOS_Q)$(SIZE) $(1) | sed 's#$(abspath $(1))#$(notdir $(1))#g'
endef
endif

# ============================================================
# 6. Exported Defines
# ============================================================

KERNEL_CONFIG_DEFINES := \
	-DOS_CFG_SYSTICK_CLOCK_HZ=$(OS_CFG_SYSTICK_CLOCK_HZ) \
	-DOS_CFG_HEAP_SIZE=$(OS_CFG_HEAP_SIZE) \
	-DOS_CFG_PRIO_MAX=$(OS_CFG_PRIO_MAX) \
	-DOS_CFG_DELAY_MAX=$(OS_CFG_DELAY_MAX) \
	-DOS_CFG_TASK_STK_SIZE_MIN=$(OS_CFG_TASK_STK_SIZE_MIN) \
	-DOS_CFG_TASK_STACK_FILL_BYTE=$(OS_CFG_TASK_STACK_FILL_BYTE) \
	-DOS_CFG_TASK_MSG_Q_SIZE_NORMAL=$(OS_CFG_TASK_MSG_Q_SIZE_NORMAL) \
	-DOS_CFG_MSG_POOL_SIZE=$(OS_CFG_MSG_POOL_SIZE) \
	-DOS_CFG_TIMER_POOL_SIZE=$(OS_CFG_TIMER_POOL_SIZE) \
	-DOS_CFG_TIMER_TASK_PRI=$(OS_CFG_TIMER_TASK_PRI) \
	-DOS_CFG_USE_LOG=$(OS_CFG_USE_LOG) \
	-DOS_CFG_USE_CLI=$(OS_CFG_USE_CLI)
