

CFLAGS =  -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include -I../blast/include -I../timer/include -g $(OPTIMIZE) -G0
ASFLAGS =  -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include -I../blast/include -I../timer/include 

export QUBE_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/qmemory.S
C_FILES = src/qmsgq.c src/qthread.c src/qmsgq_callback.c src/qmemory_c.c

#src/qthread.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libqube.a 

clean: 
	rm -f libqube.a $(O_FILES)

libqube.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)

