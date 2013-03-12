SHELL=/bin/bash
ULIMIT=ulimit -t 300

SIZE_TOOL = /prj/dsp/qdsp6/arch/scripts/section_size.py
LD = hexagon-gcc

SIM=hexagon-sim
RUN=$(SIM)

ifeq (,$(findstring -,$(RUN)))
COM_SIMF=--timing --quiet --timer 32000 --timerpin 2
else
COM_SIMF= --quiet --cosim_file cosim.cfg
endif

ifneq (,${Q6VERSION})
ARCHV=$(subst v,,$(Q6VERSION))
else
ARCHV=4
endif

COM_CFLAGS=
COM_LDFLAGS=

#If you are running outside hexframe.. sensible defaults
ifeq (,${BUILD_DIR})
CC=hexagon-gcc
SIMF=$(COM_SIMF)
#hexframe likes duplication ATM Machine, PIN Number, Q6VERSION Version....
else
SIMF+=$(COM_SIMF)
endif

H2DIR := ..

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif


OPTIMIZE=-O2

CFLAGS = -Wall -G0 -g $(OPTIMIZE) -mv${ARCHV} -DARCHV=${ARCHV} -I. -Iinc -Isrc -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -L. -L$(INSTALLPATH)/lib $(EXTRA_CFLAGS)
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2

include ../scripts/Makefile.inc.config
include ../scripts/Makefile.inc.bootvm


ifeq ($(LINUX_LINK_ADDR),)
LINUX_LINK_ADDR = $(H2K_GUEST_START)
endif

ifeq ($(LINUX_OFFSET_ADDR),)
LINUX_OFFSET_ADDR = 0x00000000
endif

LINUX_STEXT = $(LINUX_LINK_ADDR)

ifeq ($(LINUX_LOAD_ADDR),)
LINUX_LOAD_ADDR = $(LINUX_LINK_ADDR)
endif

ifeq ($(FRAME_BUFFER),)
FRAME_BUFFER = 0x89000000
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

LINUX = 1
DO_CLOCK_SETUP = 1

ifneq ($(LINUX),)
CFLAGS += -DLINUX
endif

ifneq ($(DO_CLOCK_SETUP),)
CFLAGS += -DDO_CLOCK_SETUP
endif

CFLAGS += -DLINUX_LINK_ADDR=$(LINUX_LINK_ADDR) -DLINUX_LOAD_ADDR=$(LINUX_LOAD_ADDR) -DLINUX_OFFSET_ADDR=$(LINUX_OFFSET_ADDR)

ifneq ($(UCOS),)

# must be at least 64K aligned because of gp fixup in ucos_vm_boot
ifeq ($(UCOS_LINK_ADDR),)
UCOS_LINK_ADDR = 0x81400000
endif

ifeq ($(UCOS_LOAD_ADDR),)
UCOS_LOAD_ADDR = 0x81400000
endif

CFLAGS += -DUCOS -DUCOS_LINK_ADDR=$(UCOS_LINK_ADDR) -DUCOS_LOAD_ADDR=$(UCOS_LOAD_ADDR) -DFRAME_BUFFER=$(FRAME_BUFFER)

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
