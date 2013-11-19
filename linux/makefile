SHELL=/bin/bash
ULIMIT=ulimit -t 300

ifeq ($(TARGET), 8960)
ARCHV := 4
H2K_KERNEL_PGSIZE := 3
H2K_ALLOC_HEAP_SIZE := 0xb000
NO_LOAD=1
NO_PRINT=1
H2K_LOAD_ADDR=0xbf800000
LINUX_LINK_ADDR=0x80400000
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

CFLAGS = -Wall -G0 -g $(OPTIMIZE) -mv${TOOLARCH} -DARCHV=${ARCHV} -I. -Iinc -Isrc -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -L. -L$(INSTALLPATH)/lib $(EXTRA_CFLAGS)
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2

include ../scripts/Makefile.inc.config
include ../scripts/Makefile.inc.bootvm


ifeq ($(LINUX_LINK_ADDR),)
#LINUX_LINK_ADDR = $(H2K_GUEST_START)
LINUX_LINK_ADDR = 0x80400000
endif

ifeq ($(LINUX_OFFSET_ADDR),)
LINUX_OFFSET_ADDR = 0x00000000
endif

LINUX_STEXT = $(LINUX_LINK_ADDR)

ifeq ($(LINUX_LOAD_ADDR),)
LINUX_LOAD_ADDR = $(LINUX_LINK_ADDR)
endif

ifeq ($(FRAME_BUFFER),)
FRAME_BUFFER = 0xbf400000
endif

ifeq ($(MILLISECONDS),)
MILLISECONDS = 5
endif

ifneq ($(NO_PRINT),)
CFLAGS += -DNO_PRINT
endif

ifneq ($(NO_LOAD),)
CFLAGS += -DNO_LOAD
endif

ifneq ($(NO_STLB),)
CFLAGS += -DNO_STLB
endif

ifneq ($(INT_SHIFT),)
CFLAGS += -DINT_SHIFT
endif

LINUX = 1

ifneq ($(LINUX),)
CFLAGS += -DLINUX
endif

CFLAGS += -DLINUX_LINK_ADDR=$(LINUX_LINK_ADDR) -DLINUX_LOAD_ADDR=$(LINUX_LOAD_ADDR) -DLINUX_OFFSET_ADDR=$(LINUX_OFFSET_ADDR) -DFRAME_BUFFER=$(FRAME_BUFFER)

ifneq ($(UCOS),)

# must be at least 64K aligned because of gp fixup in ucos_vm_boot
ifeq ($(UCOS_LINK_ADDR),)
UCOS_LINK_ADDR = 0xbfc00000
endif

ifeq ($(UCOS_LOAD_ADDR),)
UCOS_LOAD_ADDR = $(UCOS_LINK_ADDR)
endif

CFLAGS += -DUCOS -DUCOS_LINK_ADDR=$(UCOS_LINK_ADDR) -DUCOS_LOAD_ADDR=$(UCOS_LOAD_ADDR)

UCOS_BIN = ucos.bin

UCOS_LINK = -Wl,--defsym=ucos_entry=$(UCOS_LINK_ADDR) -Wl,--defsym=ucos_loadaddr=$(UCOS_LOAD_ADDR)
UCOS_OBJ = ucos_image.o
endif

BOOT_OBJ = boot.o

.PHONY: all clean sim

all: loadlinux

$(UCOS_BIN):
	$(MAKE) -C ../ucos clean UCOS_FOREVER=1 UCOS_LINK_ADDR=$(UCOS_LINK_ADDR) UCOS_LOAD_ADDR=$(UCOS_LOAD_ADDR) MILLISECONDS=$(MILLISECONDS) FRAME_BUFFER=$(FRAME_BUFFER) NO_PRINT=$(NO_PRINT) DEMO_DISPLAY=$(DEMO_DISPLAY) ucos.bin
	ln -s ../ucos/ucos.bin .

$(UCOS_OBJ): ucos_image.S $(UCOS_BIN)
	${CC} ${CFLAGS} -c -o $@ ucos_image.S

bootvm_image: loadlinux.o loadlinux_event.o $(BOOTVM_IMAGE_DEPS) $(UCOS_BIN) $(UCOS_OBJ)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -Wl,--defsym=linux_stext=$(LINUX_STEXT) $(UCOS_LINK) -o bootvm_image loadlinux.o loadlinux_event.o $(UCOS_OBJ) $(BOOTVM_ENTRY_O)

loadlinux: $(BOOTVM_O) $(BOOTVM_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_CFLAGS) -o loadlinux $(BOOTVM_O)

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	$(MAKE) -C ../ucos clean
	rm -f *.o loadlinux ucos.bin $(BOOTVM_CLEAN)


sim test.log : loadlinux
	$(ULIMIT) && $(RUN) $(SIMF) -- loadlinux 2>&1 | tee test.log
