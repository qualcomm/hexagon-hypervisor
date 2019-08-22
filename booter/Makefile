
H2DIR ?= ..

include $(H2DIR)/scripts/Makefile.inc.tools

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif

BOOTVM_PROG := booter

BOOTVM_IMAGE_HEAP_SIZE := 0x10000

CFLAGS = $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -D_PROVIDE_POSIX_TIME_DECLS=1
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2 -Qunused-arguments

include $(H2DIR)/scripts/Makefile.inc.config
include $(H2DIR)/scripts/Makefile.inc.bootvm


all: hello $(BOOTVM_PROG)

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

hello: hello.o
	${CC} $(LDFLAGS) $(CFLAGS)  -Wl,--section-start=.start=$(H2K_GUEST_START) -o hello hello.o

$(BOOTVM_IMAGE): booter.o elf.o booter_event.o $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=$(BOOTVM_IMAGE_HEAP_SIZE) -Wl,--defsym=STACK_SIZE=0x1000 -o $@ booter.o elf.o booter_event.o $(BOOTVM_ENTRY_O)


test: hello $(BOOTVM_PROG)
	$(SIM) -- $(BOOTVM_PROG) hello

install: all
	mkdir -p $(INSTALLPATH)/bin
	cp $(BOOTVM_PROG) $(INSTALLPATH)/bin

clean:
	rm -f *.elf $(BOOTVM_CLEAN) hello stats.* *.o pmu_statsfile.txt

