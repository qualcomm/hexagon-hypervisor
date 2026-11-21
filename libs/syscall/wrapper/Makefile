
# C library selection (the Picolibc target and cstdlib) comes from Makefile.inc.libc.
# The clang driver's --cstdlib=picolibc adds the Picolibc include paths.
include ../../../scripts/Makefile.inc.libc
TARGET_FLAG = $(OSLIB_CFLAGS)

CFLAGS = $(TARGET_FLAG) -H -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -I../angel/include -Wall -Wno-builtin-requires-header -Werror -I../../h2/include -I../../h2_compat/include -I../../posix/include -g $(OPTIMIZE) -G0
ASFLAGS = $(TARGET_FLAG) -H -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -I../angel/include -Wall -I../../h2/include -I../../posix/include

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)
ifeq ($(EN_LLVM_COV), 1)
CFLAGS += ${LLVM_COV_CFLAGS}
endif

BUILD_DIR := $(INSTALLPATH)/../build/libs/syscall/wrapper
FLAGS_STAMP := $(BUILD_DIR)/flags.stamp

ASM_FILES = 
C_FILES = $(wildcard src/*.c)

O_FILES = $(addprefix $(BUILD_DIR)/,$(ASM_FILES:.S=.o) $(C_FILES:.c=.o))

# Track header dependencies so edits to a .h rebuild the objects that use it.
CFLAGS += -MMD -MP
ASFLAGS += -MMD -MP

all: $(BUILD_DIR)/libsyscall_wrapper.a

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/libsyscall_wrapper.a: $(O_FILES)
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
