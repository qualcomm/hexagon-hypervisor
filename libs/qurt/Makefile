

BUILD_DIR           := $(INSTALLPATH)/../build/libs/qurt
FLAGS_STAMP := $(BUILD_DIR)/flags.stamp
H2_BUILD_DIR        := $(INSTALLPATH)/../build/libs/h2
H2_COMPAT_BUILD_DIR := $(INSTALLPATH)/../build/libs/h2_compat
POSIX_BUILD_DIR     := $(INSTALLPATH)/../build/libs/posix

# C library selection (the Picolibc target and cstdlib) comes from Makefile.inc.libc.
include ../../scripts/Makefile.inc.libc
TARGET_FLAG = $(OSLIB_CFLAGS)

CC = hexagon-clang $(TARGET_FLAG) -mv$(TOOLARCH) -Iinclude -Wall -Wno-builtin-requires-header -I$(H2_BUILD_DIR)/include -I$(POSIX_BUILD_DIR)/include -I$(H2_COMPAT_BUILD_DIR)/include
AR = hexagon-ar
OPTIMIZE := -O2

CFLAGS = -g $(OPTIMIZE) $(OPT_ADD) -G0

export QURT_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = $(wildcard src/*.S)
C_FILES = $(wildcard src/*.c)

O_FILES = $(addprefix $(BUILD_DIR)/,$(ASM_FILES:.S=.o) $(C_FILES:.c=.o))

# Track header dependencies so edits to a .h rebuild the objects that use it.
CFLAGS += -MMD -MP

all: $(BUILD_DIR)/libqurt.a

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/libqurt.a: $(O_FILES)
	mkdir -p $(dir $@)
	rm -f $@
	$(AR) cru $@ $(O_FILES)

$(BUILD_DIR)/%.o: %.c $(FLAGS_STAMP)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.S $(FLAGS_STAMP)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

include $(H2DIR)/scripts/Makefile.inc.flagstamp

# Per-object header deps from $(DEPFLAGS); '-' tolerates a clean tree with no .d.
# Last, so a rule from a .d file never becomes the default goal.
-include $(O_FILES:.o=.d)
