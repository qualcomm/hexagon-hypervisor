
# C library selection (the Picolibc target and cstdlib) comes from Makefile.inc.libc.
include ../../scripts/Makefile.inc.libc
TARGET_FLAG = $(OSLIB_CFLAGS)

H2_INCLUDES   := $(patsubst %,-I%,$(shell find ../h2/ -mindepth 1 -maxdepth 1 -type d))
POSIX_INCLUDES:= $(patsubst %,-I%,$(shell find ../posix/pthread/ -mindepth 1 -maxdepth 1 -type d))

REQUIRED_INCLUDES = $(H2_INCLUDES) $(POSIX_INCLUDES)

CFLAGS = $(TARGET_FLAG) -mv$(TOOLARCH) -DARCHV=$(ARCHV) $(REQUIRED_INCLUDES) -Wall -Wno-builtin-requires-header -Werror -g $(OPTIMIZE) -G0
ASFLAGS = $(TARGET_FLAG) -mv$(TOOLARCH) -DARCHV=$(ARCHV) $(REQUIRED_INCLUDES) -Wall

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)
ifeq ($(EN_LLVM_COV), 1)
CFLAGS += ${LLVM_COV_CFLAGS}
endif

BUILD_DIR := $(INSTALLPATH)/../build/libs/locks
FLAGS_STAMP := $(BUILD_DIR)/flags.stamp

ASM_FILES = 
C_FILES = $(wildcard src/*.c)

O_FILES = $(addprefix $(BUILD_DIR)/,$(ASM_FILES:.S=.o) $(C_FILES:.c=.o))

# Track header dependencies so edits to a .h rebuild the objects that use it.
CFLAGS += -MMD -MP
ASFLAGS += -MMD -MP

all: $(BUILD_DIR)/liblocks.a

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/liblocks.a: $(O_FILES)
	mkdir -p $(dir $@)
	rm -f $@
	$(AR) cru $@ $(sort $(O_FILES))

$(BUILD_DIR)/%.o: %.c $(FLAGS_STAMP)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.S $(FLAGS_STAMP)
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) $(DEPFLAGS) -c -o $@ $<

include $(H2DIR)/scripts/Makefile.inc.flagstamp

# Per-object header deps from $(DEPFLAGS); '-' tolerates a clean tree with no .d.
# Last, so a rule from a .d file never becomes the default goal.
-include $(O_FILES:.o=.d)
