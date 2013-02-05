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

PARAMS := $(H2K_KERNEL_ADDRBITS) $(H2K_KERNEL_PGSIZE) $(H2K_ALLOC_HEAP_SIZE) $(H2K_LINK_ADDR) $(H2K_LOAD_ADDR)

CONSTS := ./consts.pl

CFLAGS = -Os $(OPTIMIZE) -mv$(ARCHV) -DARCHV=$(ARCHV) -Wall -Werror -g -moslib=h2 -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -L$(INSTALLPATH)/lib

CFLAGS += -DH2K_KERNEL_PGSIZE=$(H2K_KERNEL_PGSIZE) -DH2K_KERNEL_ADDRBITS=$(H2K_KERNEL_ADDRBITS)

SCRIPTS = Makefile.inc.constants vapa_kernel.x vapa_bootvm.x 
SRCFILES = bootvm_entry.S bootvm.S
FILES := $(SCRIPTS) $(SRCFILES)

.PHONY: constants

all: $(FILES)

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

constants: kernel_tmp
	$(eval BOOTVM_OFFSET := $(shell $(CONSTS) bootvm_offset kernel_tmp $(PARAMS)))
	$(eval BOOTVM_START := $(shell $(CONSTS) bootvm_start kernel_tmp $(PARAMS)))
	$(eval V2P_OFFSET := $(shell $(CONSTS) v2p_offset kernel_tmp $(PARAMS)))
	$(eval KERNEL_NPAGES := $(shell $(CONSTS) kernel_npages kernel_tmp $(PARAMS)))

Makefile.inc.constants: constants
	echo 'export H2K_BOOTVM_OFFSET := $(BOOTVM_OFFSET)' > $@
	echo 'export H2K_BOOTVM_START := $(BOOTVM_START)' >> $@
	echo 'export H2K_V2P_OFFSET := $(V2P_OFFSET)' >> $@
	echo 'export H2K_KERNEL_NPAGES := $(KERNEL_NPAGES)' >> $@

vapa_kernel.x: vapa_kernel.x.in kernel_tmp
	cat $< | perl -p -i -e 's/0xxxxxxxxx/$(H2K_LOAD_ADDR)/;' -e 's/0xyyyyyyyy/$(BOOTVM_OFFSET)/;' > $@

vapa_kernel_tmp.x: vapa_kernel.x.in
	cat $< | perl -p -i -e 's/0xxxxxxxxx/$(H2K_LOAD_ADDR)/;' -e 's/0xyyyyyyyy/0x0/;' > $@

# so we can calculate the number of kernel pages
kernel_tmp: vapa_kernel_tmp.x $(INSTALLPATH)/lib/libh2kernel.a
	${CC} $(CFLAGS) -nostdinc -nostdlib -Wl,-mv$(ARCHV) -Wl,--whole-archive $(INSTALLPATH)/lib/libh2kernel.a -Wl,-no-whole-archive -Wl,--section-start=.entry=$(H2K_LINK_ADDR) -Wl,--defsym=__bootvm_entry=0x0 -Wl,--defsym=HEAP_SIZE=$(H2K_HEAP_SIZE) -Wl,--defsym=STACK_SIZE=$(H2K_STACK_SIZE) -Wl,--defsym=H2K_ALLOC_HEAP_SIZE=$(H2K_ALLOC_HEAP_SIZE) -Wl,--defsym=H2K_KERNEL_NPAGES=0x0 -Wl,--defsym=H2K_LOAD_ADDR=$(H2K_LOAD_ADDR) -Wl,--script=vapa_kernel_tmp.x -o kernel_tmp

install: $(FILES)
	mkdir -p $(INSTALLPATH)/scripts
	cp $(SCRIPTS) $(INSTALLPATH)/scripts
	mkdir -p $(INSTALLPATH)/src
	cp $(SRCFILES) $(INSTALLPATH)/src

clean:
	rm -f Makefile.inc.constants vapa_kernel.x vapa_kernel_tmp.x kernel_tmp