
CFLAGS = -H -I$(PICOLIBPATH)/target/hexagon-unknown-none-picolibc/include -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -I../angel/include -Wall -Wno-builtin-requires-header -Werror -I../../h2/include -I../../h2_compat/include -I../../posix/include -g $(OPTIMIZE) -G0
ASFLAGS = -H -I$(PICOLIBPATH)/target/hexagon-unknown-none-picolibc/include -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -I../angel/include -Wall -I../../h2/include -I../../posix/include

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)
ifeq ($(EN_LLVM_COV), 1)
CFLAGS += ${LLVM_COV_CFLAGS}
endif

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = 
C_FILES = $(wildcard src/*.c)

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libsyscall_wrapper.a 

clean: 
	rm -f libsyscall_wrapper.a $(O_FILES)

libsyscall_wrapper.a: $(O_FILES)
	$(AR) cru $@ $(sort $(O_FILES))

