
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

ASM_FILES = 
C_FILES = $(wildcard src/*.c)

O_FILES = $(addprefix $(BUILD_DIR)/,$(ASM_FILES:.S=.o) $(C_FILES:.c=.o))

all: $(BUILD_DIR)/libsyscall_wrapper.a

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/libsyscall_wrapper.a: $(O_FILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(sort $(O_FILES))

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) $(DEPFLAGS) -c -o $@ $<


# Per-object header deps from $(DEPFLAGS); '-' tolerates a clean tree with no .d.
-include $(O_FILES:.o=.d)
