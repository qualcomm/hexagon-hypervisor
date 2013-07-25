
include ../scripts/Makefile.inc.tools

H2DIR ?= ..

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif

BOOTVM_PROG := booter

CFLAGS = $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2

include ../scripts/Makefile.inc.config
include ../scripts/Makefile.inc.bootvm


all: hello $(BOOTVM_PROG)

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

hello: hello.o
	${CC} $(LDFLAGS) $(CFLAGS)  -Wl,--section-start=.start=$(H2K_GUEST_START) -o hello hello.o

$(BOOTVM_IMAGE): booter.o elf.o booter_event.o $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -o $@ booter.o elf.o booter_event.o $(BOOTVM_ENTRY_O)


test: hello $(BOOTVM_PROG)
	$(SIM) -- $(BOOTVM_PROG) hello

install: $(BOOTVM_PROG)
	mkdir -p $(INSTALLPATH)/bin
	cp $(BOOTVM_PROG) $(INSTALLPATH)/bin

clean:
	rm -f *.elf $(BOOTVM_CLEAN) hello stats.* *.o pmu_statsfile.txt

