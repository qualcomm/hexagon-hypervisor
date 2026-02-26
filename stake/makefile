include ../scripts/Makefile.inc.tools

H2DIR ?= ..

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif


all: kernel_tmp

include ../scripts/Makefile.inc.config
include ../scripts/Makefile.inc.opensource
include ../scripts/Makefile.inc.bootvm

CONSTS := ./consts.pl
SYMS := ./syms.pl

CFLAGS = $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g -moslib=h2 -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -L$(INSTALLPATH)/lib

CFLAGS += -DH2K_KERNEL_PGSIZE=$(H2K_KERNEL_PGSIZE) -DH2K_KERNEL_ADDRBITS=$(H2K_KERNEL_ADDRBITS) -Qunused-arguments

SCRIPTS = $(CONSTS) $(SYMS) vapa_bootvm.x vapa_kernel.x.in
SRCFILES = bootvm_entry.S bootvm.S min_crt0.S
BINFILES = kernel_tmp
FILES := $(SCRIPTS) $(SRCFILES) $(BINFILES)

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

vapa_kernel_tmp.x: vapa_kernel.x.in
	cat $< | perl -p -i -e 's/0xxxxxxxxx/$(H2K_LOAD_ADDR)/;' -e 's/0xyyyyyyyy/0x0/;' > $@

# so we can calculate the number of kernel pages
kernel_tmp: vapa_kernel_tmp.x $(INSTALLPATH)/lib/libh2kernel.a
	${CC} $(CFLAGS) -nostartfiles -Wl,--whole-archive $(INSTALLPATH)/lib/libh2kernel.a -Wl,-no-whole-archive -Wl,--section-start=.entry=$(H2K_LINK_ADDR) -Wl,--defsym=__bootvm_entry=0x0, -Wl,--defsym=__bootvm_entry_point=0x0  -Wl,--defsym=HEAP_SIZE=$(H2K_HEAP_SIZE) -Wl,--defsym=STACK_SIZE=$(H2K_STACK_SIZE) -Wl,--defsym=H2K_ALLOC_HEAP_SIZE=$(H2K_ALLOC_HEAP_SIZE) -Wl,--defsym=H2K_KERNEL_NPAGES=0x0 -Wl,--defsym=H2K_LOAD_ADDR=$(H2K_LOAD_ADDR) -Wl,--script=vapa_kernel_tmp.x -Wl,--entry=$(H2K_LOAD_ADDR) -Wl,-mv$(TOOLARCH) -o kernel_tmp

# $(INSTALLPATH)/stake/kernel_tmp: kernel_tmp
# 	mkdir -p $(INSTALLPATH)/stake
# 	cp kernel_tmp $(INSTALLPATH)/stake

# $(INSTALLPATH)/scripts/vapa_kernel.x.in: vapa_kernel.x.in
# 	mkdir -p $(INSTALLPATH)/scripts
# 	cp vapa.kernel.x.in $(INSTALLPATH)/scripts


install: $(FILES)
	mkdir -p $(INSTALLPATH)/scripts
	cp $(SCRIPTS) $(INSTALLPATH)/scripts
	mkdir -p $(INSTALLPATH)/src
	cp $(SRCFILES) $(INSTALLPATH)/src
	mkdir -p $(INSTALLPATH)/stake
	cp $(BINFILES) $(INSTALLPATH)/stake

clean:
	rm -f vapa_kernel.x vapa_kernel_tmp.x kernel_tmp
