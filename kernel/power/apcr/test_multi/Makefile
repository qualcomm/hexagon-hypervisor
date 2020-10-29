EXEC = test.elf
OBJS += test.o
#OBJS += $(BOOTVM_ENTRY_O)
include Makefile.inc


ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(H2DIR)/install
endif

# ifeq ($(KERNELPATH),)
# export KERNELPATH := $(H2DIR)/kernel
# endif

H2K_KERNEL_PGSIZE := 2

BOOTVM_PROG := .test.elf

ifeq ($(TARGET), rtl)
CRT0_S := $(INSTALLPATH)/src/min_crt0.S
CRT0_O := min_crt0.o
NOSTARTFILES := -nostartfiles
EXTRA_CFLAGS += -DRTL
else
EXTRA_CFLAGS += -DDEBUG
endif

CFLAGS := $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -Wno-builtin-requires-header $(EXTRA_CFLAGS)
LDFLAGS = $(NOSTARTFILES) -L$(INSTALLPATH)/lib -moslib=h2 -Qunused-arguments


all: $(BOOTVM_PROG)

include $(H2DIR)/scripts/Makefile.inc.bootvm

CLEANUP += *.elf $(BOOTVM_CLEAN) hello stats.* *.o pmu_statsfile.txt


$(BOOTVM_IMAGE): test.o $(CRT0_O) $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0 -o $@ test.o $(CRT0_O) $(BOOTVM_ENTRY_O) 2>&1 | perl -pe 's/.*Symbol.*__h2_.*_hook__.*is defined in both linker script and input file.*//';

# This bit of stupidity is needed to suppress warnings about changing the
# recipe for test.elf
$(EXEC) : private CC = true
test.log: | symlink
test.objdump: | symlink

.PHONY: symlink
symlink: $(BOOTVM_PROG)
	if [ ! -f $(EXEC) ]; then ln -s $(BOOTVM_PROG) $(EXEC); fi;
