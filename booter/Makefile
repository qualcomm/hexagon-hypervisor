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

H2DIR ?= ..

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif

CFLAGS = -Os $(OPTIMIZE) -mv$(ARCHV) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2

include ../scripts/Makefile.inc.config
include ../scripts/Makefile.inc.bootvm


all: hello booter

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

hello: hello.o
	${CC} $(LDFLAGS) $(CFLAGS)  -Wl,--section-start=.start=$(H2K_GUEST_START) -o hello hello.o

bootvm_image: booter.o elf.o booter_event.o $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -o bootvm_image booter.o elf.o booter_event.o $(BOOTVM_ENTRY_O)

booter: $(BOOTVM_O) $(BOOTVM_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_CFLAGS) -o booter $(BOOTVM_O)


test: hello booter
	hexagon-sim -- booter hello

install: booter
	mkdir -p $(INSTALLPATH)/bin
	cp booter $(INSTALLPATH)/bin

clean:
	rm -f *.elf booter $(BOOTVM_CLEAN) hello stats.* *.o
