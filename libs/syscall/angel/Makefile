
# Picolibc support: if PICOLIBC=1, use --target=hexagon-h2-picolibc
ifeq ($(PICOLIBC),1)
TARGET_FLAG = --target=hexagon-h2-picolibc
# _STD_END/_STD_BEGIN are Dinkumware-specific macros; define them as no-ops
TARGET_FLAG += -D_STD_END= -D_STD_BEGIN=
else
TARGET_FLAG =
endif

CFLAGS = $(TARGET_FLAG) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -Wno-builtin-requires-header -Werror -I../../h2/include -I../../h2_compat/include -I../../posix/include -g $(OPTIMIZE) $(OPT_ADD) -G0
ASFLAGS = $(TARGET_FLAG) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -I../../h2/include -I../../posix/include

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)

ifdef NULL_SYS_WRITE_FD1
CFLAGS += -DNULL_SYS_WRITE_FD1
endif

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/angel.S
C_FILES = $(wildcard src/*.c)

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libangel.a 

clean: 
	rm -f libangel.a $(O_FILES)

libangel.a: $(O_FILES)
	$(AR) cru $@ $(sort $(O_FILES))

