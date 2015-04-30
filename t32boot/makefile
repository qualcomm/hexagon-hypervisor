SHELL=/bin/bash
ULIMIT=ulimit -t 300

ifeq ($(TARGET),)
TARGET := 8974
endif

ifeq ($(TARGET), 8974)
ARCHV := 5
H2K_KERNEL_PGSIZE := 3
H2K_ALLOC_HEAP_SIZE := 0xb000
NO_LOAD=1
NO_PRINT=1
H2K_LOAD_ADDR=0x0
LINUX_LINK_ADDR=0x02000000
endif

include ../scripts/Makefile.inc.tools

H2DIR := ..

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif


OPTIMIZE=-O2

BOOTVM_PROG := t32boot

all: $(BOOTVM_PROG)

CFLAGS = -Wall -G0 -g $(OPTIMIZE) -mv${TOOLARCH} -DARCHV=${ARCHV} -I. -Iinc -Isrc -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -L. -L$(INSTALLPATH)/lib $(EXTRA_CFLAGS)
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2

include ../scripts/Makefile.inc.config
include ../scripts/Makefile.inc.bootvm


BOOT_OBJ = boot.o

.PHONY: all clean sim

$(BOOTVM_IMAGE): t32boot.o t32boot_event.o testload.o $(BOOTVM_IMAGE_DEPS) $(UCOS_BIN) $(UCOS_OBJ)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -o $@ t32boot.o t32boot_event.o testload.o $(UCOS_OBJ) $(BOOTVM_ENTRY_O)

clean:
	rm -f *.o $(BOOVM_PROG) $(BOOTVM_CLEAN)

sim test.log : $(BOOTVM_IMAGE)
	$(ULIMIT) && $(RUN) $(SIMF) -- $(BOOTVM_IMAGE) 2>&1 | tee test.log

