
CFLAGS = -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -Wno-builtin-requires-header -I../../h2/include -I../../h2_compat/include -I../../posix/include -g $(OPTIMIZE) -G0 -D_PROVIDE_POSIX_TIME_DECLS=1
ASFLAGS = -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -I../../h2/include -I../../posix/include  

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/angel.S
C_FILES = $(wildcard src/*.c)

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libangel.a 

clean: 
	rm -f libangel.a $(O_FILES)

libangel.a: $(O_FILES)
	$(AR) cru $@ $(sort $(O_FILES))

