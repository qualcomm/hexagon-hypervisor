

BUILD_DIR           := $(patsubst %/install,%/build/libs/qurt,$(INSTALLPATH))
H2_BUILD_DIR        := $(patsubst %/install,%/build/libs/h2,$(INSTALLPATH))
H2_COMPAT_BUILD_DIR := $(patsubst %/install,%/build/libs/h2_compat,$(INSTALLPATH))
POSIX_BUILD_DIR     := $(patsubst %/install,%/build/libs/posix,$(INSTALLPATH))

CC = hexagon-clang -mv$(TOOLARCH) -Iinclude -Wall -Wno-builtin-requires-header -I$(H2_BUILD_DIR)/include -I$(POSIX_BUILD_DIR)/include -I$(H2_COMPAT_BUILD_DIR)/include
AR = hexagon-ar

OPTIMIZE := -O2

CFLAGS = -g $(OPTIMIZE) $(OPT_ADD) -G0

export QURT_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = $(wildcard src/*.S)
C_FILES = $(wildcard src/*.c)

O_FILES = $(addprefix $(BUILD_DIR)/,$(ASM_FILES:.S=.o) $(C_FILES:.c=.o))

all: $(BUILD_DIR)/libqurt.a

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/libqurt.a: $(O_FILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(O_FILES)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

