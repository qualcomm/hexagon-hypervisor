H2DIR=${UPDIR}../../../..

include $(H2DIR)/scripts/Makefile.inc.tools

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif

BOOTVM_PROG := test.elf

CFLAGS = -nostartfiles $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include $(EXTRA_CFLAGS)
LDFLAGS = -L$(INSTALLPATH)/lib -moslib=h2

include $(H2DIR)/scripts/Makefile.inc.config
include $(H2DIR)/scripts/Makefile.inc.bootvm

all: $(BOOTVM_PROG)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

$(BOOTVM_IMAGE): test.o crt0.o $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -o $@ test.o crt0.o $(BOOTVM_ENTRY_O)

clean:
	rm -f *.elf $(BOOTVM_CLEAN) hello stats.* *.o pmu_statsfile.txt

