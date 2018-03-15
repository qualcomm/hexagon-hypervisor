EXEC = test.elf
OBJS += test.o
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

CFLAGS := $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g -I$(INSTALLPATH)/include -I$(KERNELPATH)/include $(EXTRA_CFLAGS)
LDFLAGS = $(NOSTARTFILES) -L$(INSTALLPATH)/lib -moslib=h2

all: $(BOOTVM_PROG)

include $(H2DIR)/scripts/Makefile.inc.bootvm


$(BOOTVM_IMAGE): test.o $(CRT0_O) $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=0x10000 -Wl,--defsym=STACK_SIZE=0x1000 -o $@ test.o $(CRT0_O) $(BOOTVM_ENTRY_O)

# This bit of stupidity is needed to suppress warnings about changing the
# recipe for test.elf
$(EXEC) : private CC = true
test.log: move
test.objdump: move

.PHONY: move
move: $(BOOTVM_PROG)
	rm -f $(EXEC)
	mv $(BOOTVM_PROG) $(EXEC)

.PHONY: .clean
.clean:
	rm -f *.elf $(BOOTVM_CLEAN) hello stats.* *.o pmu_statsfile.txt

clean: .clean
