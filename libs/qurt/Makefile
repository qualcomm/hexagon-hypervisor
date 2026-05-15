

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
C_FILES = src/qurt_fatal.c src/qurt_memory.c src/qurtos_sclk.c src/qurtos_timer_libs.c src/qurtos_timer_server.c src/qurt_pgalloc.c src/qurt_thread.c src/qurt_timer.c src/qurt_timer_hw.c src/qurt_tls.c src/qurt_int.c src/qurt_fd.c src/qurt_qdi.c src/qurt_qdi_local_client.c src/qurt_qdi_signal.c src/qurt_mmap.c src/qurt_misc.c src/qurt_trace.c src/qmutex.c src/qurt_cache.c src/qurt_tlb.c src/qurt_s5alloc.c

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

