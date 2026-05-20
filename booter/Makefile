
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

CFLAGS = $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -Wno-builtin-requires-header -g -I. -I$(INSTALLPATH)/include -I$(KERNEL_BUILD_DIR)/include
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2 -Qunused-arguments

include $(H2DIR)/scripts/Makefile.inc.config
include $(H2DIR)/scripts/Makefile.inc.opensource
include $(H2DIR)/scripts/Makefile.inc.bootvm


$(BUILD_DIR)/hello: $(BUILD_DIR)/hello.o
	${CC} $(LDFLAGS) $(CFLAGS)  -Wl,--section-start=.start=$(H2K_GUEST_START) -o $@ $<

$(BUILD_DIR)/should_pagefault: OPTIMIZE = -O0
$(BUILD_DIR)/should_pagefault: $(BUILD_DIR)/should_pagefault.o
	${CC} $(LDFLAGS) $(CFLAGS)  -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o $@ $<

$(BUILD_DIR)/should_crash: OPTIMIZE = -O0
$(BUILD_DIR)/should_crash: $(BUILD_DIR)/should_crash.o
	${CC} $(LDFLAGS) $(CFLAGS)  -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o $@ $<

$(BUILD_DIR)/bigio: $(BUILD_DIR)/bigio.o
	${CC} $(LDFLAGS) $(CFLAGS)  -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o $@ $<

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

