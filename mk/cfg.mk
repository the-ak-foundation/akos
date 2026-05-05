PRJ_DIR ?= $(shell git rev-parse --show-toplevel)
$(info [CFG] PRJ_DIR=$(PRJ_DIR).)

PLATFORM ?= target
PLATFORM_LCASE = $(shell echo $(PLATFORM) | tr '[:upper:]' '[:lower:]')
$(info [CFG] PLATFORM must be either 'host' or 'target'.)
$(info [CFG] PLATFORM=$(PLATFORM).)

MODE ?= debug
$(info [CFG] MODE is either 'debug' or 'release'.)
$(info [CFG] MODE=$(MODE).)
MODE_LCASE = $(shell echo $(MODE) | tr '[:upper:]' '[:lower:]')

$(info [CFG] Specify include paths via INCS (no -I).)
$(info [CFG] Specify source files via SRCS.)
$(info [CFG] Specify definitions via DEFS (no -D).)
$(info [CFG] Specify linker script via LDSCRIPT (no -T).)

AS = $(TOOLCHAIN)-gcc -x assembler-with-cpp

ifeq ($(PLATFORM_LCASE),target)
    TOOLCHAIN = arm-none-eabi
    CC      = $(TOOLCHAIN)-gcc -std=c99 --sysroot=$(SYSROOT_DIR)
    CXX     = $(TOOLCHAIN)-g++ -std=c++17 --sysroot=$(SYSROOT_DIR)
    OBJCOPY = $(TOOLCHAIN)-objcopy
else
    CC      = gcc -std=c99
    CXX     = g++ -std=c++17
    OBJCOPY = objcopy
endif

COMMON_FLAGS =\
    -ffunction-sections -fdata-sections\
    -Wall -Werror\
    -Wshadow\
    -Wcast-qual\
    -Wpointer-arith\
    -Wwrite-strings\
    -Wno-stringop-overflow\
    -Wstrict-prototypes\
    -save-temps\
    -fverbose-asm

ifdef INCS
    INC_FLAGS = $(addprefix -I,$(INCS))
    $(info [CFG] INCS=$(INCS).)
endif

ifdef SRCS
    $(info [CFG] SRCS=$(SRCS).)
endif

ifdef DEFS
    DEF_FLAGS = $(addprefix -D,$(DEFS))
    $(info [CFG] DEFS=$(DEFS).)
endif

ifdef LDSCRIPT
    LDSCRIPT_FLAG = -T$(LDSCRIPT)
    $(info [CFG] LDSCRIPT=$(LDSCRIPT).)
endif

ifeq ($(MODE_LCASE),debug)
	OPT_FLAGS += -O0 -g3
else
	OPTI_FLAGS += -Os -flto
	DEF_FLAGS += -DNDEBUG
endif

ifeq ($(PLATFORM_LCASE),target)
	LDFLAGS += -static
endif

CPPFLAGS = -MMD -MP $(DEF_FLAGS) $(INC_FLAGS)
CFLAGS   = $(ARCH_FLAGS) $(OPTI_FLAGS) $(COMMON_FLAGS)
CXXFLAGS =\
    $(CFLAGS)\
    -fno-exceptions -fno-rtti -fno-unwind-tables -fomit-frame-pointer
LDFLAGS =\
    $(LDSCRIPT_FLAG) $(ARCH_FLAGS) $(SPECS_FLAGS)\
    -flto -Wl,--gc-sections

OBJS_C   = $(SRCS:.c=.o)
OBJS_CPP = $(OBJS_C:.cpp=.o)
OBJS_S   = $(OBJS_CPP:.s=.o)

OBJS = $(patsubst $(PRJ_DIR)/%.o,$(PRJ_DIR)/build/%.o,$(OBJS_S))
DEPS = $(OBJS:.o=.d)
