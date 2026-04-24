
BUILD_DIR := $(patsubst %/install,%/build/libs/syscall/angel,$(INSTALLPATH))

CFLAGS = -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -Wno-builtin-requires-header -Werror -I../../h2/include -I../../h2_compat/include -I../../posix/include -g $(OPTIMIZE) $(OPT_ADD) -G0
ASFLAGS = -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -I../../h2/include -I../../posix/include

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)

ifdef NULL_SYS_WRITE_FD1
CFLAGS += -DNULL_SYS_WRITE_FD1
endif

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/angel.S
C_FILES = $(wildcard src/*.c)

O_FILES = $(addprefix $(BUILD_DIR)/,$(ASM_FILES:.S=.o) $(C_FILES:.c=.o))

all: $(BUILD_DIR)/libangel.a

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/libangel.a: $(O_FILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(sort $(O_FILES))

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ $<

