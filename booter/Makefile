#If you are running outside hexframe.. sensible defaults
ifeq (,${BUILD_DIR})
ifneq (,${Q6VERSION})
ARCHV=$(subst v,,$(Q6VERSION))
else
ARCHV=4
endif
CC=hexagon-gcc
RUN=hexagon-sim
SIMF=--timing
#hexframe likes duplication ATM Machine, PIN Number, Q6VERSION Version....
else
ARCHV=$(subst v,,$(Q6VERSION))
endif

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(CURDIR)/../install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(CURDIR)/../kernel
endif

include ../scripts/Makefile.inc.config

CONSTANTS := $(INSTALLPATH)/scripts/Makefile.inc.constants
VAPA_KERNEL := $(INSTALLPATH)/scripts/vapa_kernel.x
VAPA_BOOTVM := $(INSTALLPATH)/scripts/vapa_bootvm.x
BOOTVM_S := $(INSTALLPATH)/src/bootvm.S
BOOTVM_ENTRY_S := $(INSTALLPATH)/src/bootvm_entry.S

-include $(CONSTANTS)

CFLAGS = -Os $(OPTIMIZE) -mv$(ARCHV) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2
LDLIBS = -lh2kernel

CFLAGS += -DH2K_KERNEL_PGSIZE=$(H2K_KERNEL_PGSIZE) -DH2K_KERNEL_ADDRBITS=$(H2K_KERNEL_ADDRBITS)


all: hello booter

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

hello: hello.c
	${CC} $(CFLAGS)  -I../install/include -L../install/lib -Wl,--section-start=.start=0x01000000 -o hello hello.c

bootvm_image: booter.o elf.o booter_event.o bootvm_entry.o $(VAPA_BOOTVM) $(CONSTANTS)
	${CC} $(CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -Wl,--section-start=.entry=$(H2K_BOOTVM_OFFSET) -Wl,--script=$(VAPA_BOOTVM) -o bootvm_image booter.o elf.o booter_event.o bootvm_entry.o

bootvm_image.bin: bootvm_image
	hexagon-objcopy -O binary bootvm_image bootvm_image.bin

bootvm.o: $(BOOTVM_S) bootvm_image.bin
	${CC} ${CFLAGS} -c -o $@ $<

bootvm_entry.o: $(BOOTVM_ENTRY_S)
	${CC} ${CFLAGS} -c -o $@ $<

booter: bootvm.o $(VAPA_KERNEL) $(CONSTANTS) $(INSTALLPATH)/lib/libh2kernel.a
	${CC} -nostdinc -nostdlib $(LDFLAGS) $(CFLAGS) -Wl,-mv$(ARCHV) -Wl,--whole-archive $(INSTALLPATH)/lib/libh2kernel.a -Wl,-no-whole-archive -Wl,--section-start=.entry=$(H2K_LINK_ADDR) -Wl,--section-start=.bootvm=$(H2K_BOOTVM_START) -Wl,--defsym=HEAP_SIZE=$(H2K_HEAP_SIZE) -Wl,--defsym=STACK_SIZE=$(H2K_STACK_SIZE) -Wl,--defsym=H2K_ALLOC_HEAP_SIZE=$(H2K_ALLOC_HEAP_SIZE) -Wl,--defsym=h2_v2p_offset=$(H2K_V2P_OFFSET) -Wl,--defsym=H2K_KERNEL_NPAGES=$(H2K_KERNEL_NPAGES) -Wl,--defsym=H2K_LOAD_ADDR=$(H2K_LOAD_ADDR) -Wl,--script=$(VAPA_KERNEL) -Wl,--entry=$(H2K_LOAD_ADDR) -o booter bootvm.o


test: hello booter
	hexagon-sim -- booter hello

install: booter
	mkdir -p $(INSTALLPATH)/bin
	cp booter $(INSTALLPATH)/bin

clean:
	rm -f *.elf booter bootvm_image bootvm_image.bin hello stats.* *.o
