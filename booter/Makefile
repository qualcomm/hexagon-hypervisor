
H2DIR ?= ..

include $(H2DIR)/scripts/Makefile.inc.tools

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif

BOOTVM_PROG := booter

BOOTVM_IMAGE_HEAP_SIZE := 0x1000000

all: hello should_crash should_pagefault bigio $(BOOTVM_PROG)

ifeq ($(PICOLIBC),1)
OSLIB_FLAG = --target=hexagon-h2-picolibc
# Derive template path from the resolved compiler location: bin/../templates/staticExecutable/
PICOLIBC_LCS := $(abspath $(dir $(shell which $(CC)))/../templates/staticExecutable/static-executable-h2-picolibc.lcs.template)
GUEST_LDSCRIPT = -Wl,--script=$(PICOLIBC_LCS)
else
OSLIB_FLAG = -moslib=h2
GUEST_LDSCRIPT =
endif

CFLAGS = $(OSLIB_FLAG) $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -Wno-builtin-requires-header -g -I. -I$(INSTALLPATH)/include -I$(KERNELPATH)/include
LDFLAGS = -L$(INSTALLPATH)/lib $(OSLIB_FLAG) -Qunused-arguments

include $(H2DIR)/scripts/Makefile.inc.config
include $(H2DIR)/scripts/Makefile.inc.opensource
include $(H2DIR)/scripts/Makefile.inc.bootvm


hello: hello.o
	${CC} $(LDFLAGS) $(CFLAGS) $(GUEST_LDSCRIPT) -Wl,--section-start=.start=$(H2K_GUEST_START) -o hello hello.o

should_pagefault: OPTIMIZE = -O0
should_pagefault: should_pagefault.o
	${CC} $(LDFLAGS) $(CFLAGS) $(GUEST_LDSCRIPT) -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o should_pagefault should_pagefault.o

should_crash: OPTIMIZE = -O0
should_crash: should_crash.o
	${CC} $(LDFLAGS) $(CFLAGS) $(GUEST_LDSCRIPT) -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o should_crash should_crash.o

bigio: bigio.o
	${CC} $(LDFLAGS) $(CFLAGS) $(GUEST_LDSCRIPT) -O0 -Wl,--section-start=.start=$(H2K_GUEST_START) -o bigio bigio.o

# %.o: %.c
# 	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

$(BOOTVM_IMAGE): booter.o booter_event.o $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=$(BOOTVM_IMAGE_HEAP_SIZE) -Wl,--defsym=STACK_SIZE=0x1000 -Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0 -o $@ booter.o booter_event.o $(BOOTVM_ENTRY_O)


test: hello $(BOOTVM_PROG)
	$(SIM) -- $(BOOTVM_PROG) hello

install: all
	mkdir -p $(INSTALLPATH)/bin
	cp $(BOOTVM_PROG) $(INSTALLPATH)/bin

clean:
	rm -f *.elf $(BOOTVM_CLEAN) hello should_crash should_pagefault stats.* *.o pmu_statsfile.txt

