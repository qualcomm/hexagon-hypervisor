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

CFLAGS = -Os $(OPTIMIZE) -mv$(ARCHV) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2

include ../scripts/Makefile.inc.config
include ../scripts/Makefile.inc.bootvm


all: hello booter

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

hello: hello.c
	${CC} $(CFLAGS)  -I../install/include -L../install/lib -Wl,--section-start=.start=0x01000000 -o hello hello.c

bootvm_image: booter.o elf.o booter_event.o bootvm_entry.o $(BOOTVM_IMAGE_DEPS)
	${CC} $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -o bootvm_image booter.o elf.o booter_event.o bootvm_entry.o

bootvm_image.bin: bootvm_image
	hexagon-objcopy -O binary bootvm_image bootvm_image.bin

bootvm.o: $(BOOTVM_S) bootvm_image.bin
	${CC} ${CFLAGS} -c -o $@ $<

bootvm_entry.o: $(BOOTVM_ENTRY_S)
	${CC} ${CFLAGS} -c -o $@ $<

booter: bootvm.o $(BOOTVM_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_CFLAGS) -o booter bootvm.o


test: hello booter
	hexagon-sim -- booter hello

install: booter
	mkdir -p $(INSTALLPATH)/bin
	cp booter $(INSTALLPATH)/bin

clean:
	rm -f *.elf booter bootvm_image bootvm_image.bin hello stats.* *.o
