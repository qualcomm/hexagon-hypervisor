EXEC = test.elf

H2DIR=${UPDIR}../../../..

include ${H2DIR}/scripts/Makefile.inc.test
include $(H2DIR)/scripts/Makefile.inc.tools

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(H2DIR)/kernel
endif

H2K_KERNEL_PGSIZE := 2
H2K_ALLOC_HEAP_SIZE := 0x1000

BOOTVM_PROG := .test.elf

CRT0_S := $(INSTALLPATH)/src/min_crt0.S
CRT0_O := min_crt0.o


CFLAGS = $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include $(EXTRA_CFLAGS)
LDFLAGS = -nostartfiles -L$(INSTALLPATH)/lib -moslib=h2 -Qunused-arguments

all: $(BOOTVM_PROG)

include $(H2DIR)/scripts/Makefile.inc.config
include $(H2DIR)/scripts/Makefile.inc.bootvm

CLEANUP += *.elf $(BOOTVM_CLEAN) hello stats.* *.o pmu_statsfile.txt

$(BOOTVM_IMAGE): test.o $(CRT0_O) $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -o $@ test.o $(CRT0_O) $(BOOTVM_ENTRY_O)

# This bit of stupidity is needed to suppress warnings about changing the
# recipe for test.elf
$(EXEC) : private CC = true
test.log: | symlink
test.objdump: | symlink

.PHONY: symlink
symlink: $(BOOTVM_PROG)
	if [ ! -f $(EXEC) ]; then ln -s $(BOOTVM_PROG) $(EXEC); fi;
