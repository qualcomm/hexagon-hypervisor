
H2DIR ?= ..

include $(H2DIR)/scripts/Makefile.inc.tools
include $(H2DIR)/scripts/Makefile.inc.install

ifeq ($(INSTALLPATH),)
ARTIFACTS_DIR ?= $(if $(filter 1,$(PICOLIBC)),artifacts_picolibc,artifacts)
export INSTALLPATH := $(H2DIR)/$(ARTIFACTS_DIR)/v$(ARCHV)/opt/install
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

include $(H2DIR)/scripts/Makefile.inc.libc

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
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=$(BOOTVM_IMAGE_HEAP_SIZE) -Wl,--defsym=STACK_SIZE=0x1000 -Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0 -o $@ $(BUILD_DIR)/booter.o $(BUILD_DIR)/booter_event.o $(BOOTVM_ENTRY_O) $(BOOTVM_LOCKS_LDFLAGS)


test: $(BUILD_DIR)/hello $(BOOTVM_PROG)
	$(SIM) -- $(BOOTVM_PROG) $(BUILD_DIR)/hello

$(call install_files,$(INSTALLPATH)/bin,$(BOOTVM_PROG))

install: all $(INSTALL_TARGETS)

clean:
	rm -f $(BOOTVM_CLEAN) pmu_statsfile.txt
	rm -rf $(BUILD_DIR)

