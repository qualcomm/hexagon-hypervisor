SHELL=/bin/bash
ULIMIT=ulimit -t 300

SIZE_TOOL = /prj/dsp/qdsp6/arch/scripts/section_size.py
LD = qdsp6-gcc

SIM=qdsp6-sim
RUN=$(SIM)

ifeq (,$(findstring -,$(RUN)))
COM_SIMF=--timing --quiet --timer 32000 --timerpin 2
else
COM_SIMF=--timing --quiet --cosim_file timer.cfg
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
CC=qdsp6-gcc
SIMF=$(COM_SIMF)
#hexframe likes duplication ATM Machine, PIN Number, Q6VERSION Version....
else
SIMF+=$(COM_SIMF)
endif

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(PWD)/../install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(PWD)/../kernel
endif

OPTIMIZE=-O2

ifeq ($(H2K_LINK_ADDR),)
H2K_LINK_ADDR = 0xff000000
endif
export H2K_LINK_ADDR

ifeq ($(H2K_LOAD_ADDR),)
H2K_LOAD_ADDR = 0xff000000
endif
export H2K_LOAD_ADDR

# remove this temporary hack
ifeq ($(H2K_PHYS_OFFSET),)
H2K_PHYS_OFFSET = 0x7ec00000
endif
export H2K_PHYS_OFFSET

ifeq ($(H2K_HEAP_SIZE),)
H2K_HEAP_SIZE = 0x100000
endif

ifeq ($(LINUX_LINK_ADDR),)
LINUX_LINK_ADDR = 0x00000000
endif

ifeq ($(FRAME_BUFFER),)
FRAME_BUFFER = 0x89000000
endif

ifeq ($(MILLISECONDS),)
MILLISECONDS = 20
endif

LINUX_STEXT = $(LINUX_LINK_ADDR)
LINUX_LOAD_ADDR = $(LINUX_LINK_ADDR)

CFLAGS = -Wall -G0 -g $(OPTIMIZE) -mv${ARCHV} -I. -Iinc -Isrc -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -L. -L$(INSTALLPATH)/lib -DH2K_PHYS_OFFSET=$(H2K_PHYS_OFFSET) $(EXTRA_CFLAGS)

ifneq ($(NO_PRINT),)
CFLAGS += -DNO_PRINT
endif

ifneq ($(NO_LOAD),)
CFLAGS += -DNO_LOAD
endif

LINUX = 1

ifneq ($(LINUX),)
CFLAGS += -DLINUX
endif

CFLAGS += -DLINUX_LINK_ADDR=$(LINUX_LINK_ADDR) -DLINUX_LOAD_ADDR=$(LINUX_LOAD_ADDR)

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

.PHONY: all clean sim

all: loadlinux

$(UCOS_BIN):
	$(MAKE) -C ../ucos clean UCOS_LINK_ADDR=$(UCOS_LINK_ADDR) UCOS_LOAD_ADDR=$(UCOS_LOAD_ADDR) MILLISECONDS=$(MILLISECONDS) FRAME_BUFFER=$(FRAME_BUFFER) NO_PRINT=$(NO_PRINT) DEMO_DISPLAY=$(DEMO_DISPLAY) ucos.bin
	ln -s ../ucos/ucos.bin .

$(UCOS_OBJ): ucos_image.S $(UCOS_BIN)
	${CC} ${CFLAGS} -c -o $@ ucos_image.S

vapa.x: vapa.x.in
	cat $< | perl -p -i -e 's/0xxxxxxxxx/$$ENV{H2K_LOAD_ADDR}/' > $@

# second link changes entry point to physical address
loadlinux: loadlinux.o vapa.x $(UCOS_BIN) $(UCOS_OBJ)
	${CC} -moslib=h2 -moslib=h2kernel ${CFLAGS} -Wl,--section-start=.start=$(H2K_LINK_ADDR) -Wl,--defsym=linux_stext=$(LINUX_STEXT) -Wl,--defsym=HEAP_SIZE=$(H2K_HEAP_SIZE) $(UCOS_LINK) -Wl,--script=vapa.x -o $@ loadlinux.o $(UCOS_OBJ)

	${CC} -moslib=h2 -moslib=h2kernel ${CFLAGS} -Wl,--section-start=.start=$(H2K_LINK_ADDR) -Wl,--defsym=linux_stext=$(LINUX_STEXT) -Wl,--defsym=HEAP_SIZE=$(H2K_HEAP_SIZE) $(UCOS_LINK) -Wl,--script=vapa.x -Wl,--entry=`qdsp6-nm loadlinux | egrep ' start$$' | perl -p -e 's/\s.*//; $$_ = oct "0x$$_"; $$_ -= ((oct $$ENV{H2K_LINK_ADDR}) - (oct $$ENV{H2K_LOAD_ADDR}));$$_ = sprintf("0x%0x", $$_)'` -o $@ loadlinux.o $(UCOS_OBJ)

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f loadlinux.o loadlinux vapa.x ucos.bin


sim test.log : loadlinux
	$(ULIMIT) && $(RUN) $(SIMF) -- loadlinux $(LINUX) 2>&1 | tee test.log
