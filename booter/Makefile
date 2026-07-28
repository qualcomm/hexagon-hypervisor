
H2DIR ?= ..

include $(H2DIR)/scripts/Makefile.inc.tools

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/artifacts/v$(ARCHV)/opt/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif

# All intermediate build artifacts go here.
BUILD_DIR        := $(INSTALLPATH)/../build/booter
KERNEL_BUILD_DIR := $(INSTALLPATH)/../build/kernel

BOOTVM_PROG := $(BUILD_DIR)/booter

BOOTVM_IMAGE_HEAP_SIZE := 0x1000000

all: $(BUILD_DIR)/hello $(BUILD_DIR)/should_crash $(BUILD_DIR)/should_pagefault $(BUILD_DIR)/bigio $(BOOTVM_PROG)

ifeq ($(PICOLIBC),1)
OSLIB_CFLAGS  = --target=hexagon-h2-elf --cstdlib=picolibc
OSLIB_LDFLAGS = --target=hexagon-h2-elf --cstdlib=picolibc
# Derive template path from the resolved compiler location: bin/../templates/staticExecutable/
PICOLIBC_LCS := $(abspath $(dir $(shell which $(CC)))/../templates/staticExecutable/static-executable-h2-picolibc.lcs.template)
GUEST_LDSCRIPT = -Wl,--script=$(PICOLIBC_LCS)
else
OSLIB_CFLAGS  =
OSLIB_LDFLAGS = -moslib=h2
GUEST_LDSCRIPT =
endif

CFLAGS = $(OSLIB_CFLAGS) $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Wno-builtin-requires-header -g -I. -I$(INSTALLPATH)/include -I$(KERNEL_BUILD_DIR)/include
LDFLAGS = -L$(INSTALLPATH)/lib $(OSLIB_LDFLAGS) -Qunused-arguments

include $(H2DIR)/scripts/Makefile.inc.config
include $(H2DIR)/scripts/Makefile.inc.opensource
include $(H2DIR)/scripts/Makefile.inc.bootvm


$(BUILD_DIR)/hello: $(BUILD_DIR)/hello.o
	${CC} $(LDFLAGS) $(CFLAGS) $(GUEST_LDSCRIPT) -Wl,--section-start=.start=$(H2K_GUEST_START) -o $@ $<

$(BUILD_DIR)/should_pagefault: OPTIMIZE = -O0
$(BUILD_DIR)/should_pagefault: $(BUILD_DIR)/should_pagefault.o
	${CC} $(LDFLAGS) $(CFLAGS) $(GUEST_LDSCRIPT) -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o $@ $<

$(BUILD_DIR)/should_crash: OPTIMIZE = -O0
$(BUILD_DIR)/should_crash: $(BUILD_DIR)/should_crash.o
	${CC} $(LDFLAGS) $(CFLAGS) $(GUEST_LDSCRIPT) -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o $@ $<

$(BUILD_DIR)/bigio: $(BUILD_DIR)/bigio.o
	${CC} $(LDFLAGS) $(CFLAGS) $(GUEST_LDSCRIPT) -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o $@ $<

$(BOOTVM_IMAGE): $(BUILD_DIR)/booter.o $(BUILD_DIR)/booter_event.o $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=$(BOOTVM_IMAGE_HEAP_SIZE) -Wl,--defsym=STACK_SIZE=0x1000 -Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0 -o $@ $(BUILD_DIR)/booter.o $(BUILD_DIR)/booter_event.o $(BOOTVM_ENTRY_O)


test: $(BUILD_DIR)/hello $(BOOTVM_PROG)
	$(SIM) -- $(BOOTVM_PROG) $(BUILD_DIR)/hello

install: all
	mkdir -p $(INSTALLPATH)/bin
	cp $(BOOTVM_PROG) $(INSTALLPATH)/bin

clean:
	rm -f $(BOOTVM_CLEAN) pmu_statsfile.txt
	rm -rf $(BUILD_DIR)

