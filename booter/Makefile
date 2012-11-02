#If you are running outside hexframe.. sensible defaults
ifeq (,${BUILD_DIR})
ifneq (,${Q6VERSION})
ARCHV=$(subst v,,$(Q6VERSION))
else
ARCHV=4
endif
CC=qdsp6-gcc
RUN=qdsp6-sim
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

ifeq ($(H2K_LINK_ADDR),)
H2K_LINK_ADDR = 0xff000000
endif
export H2K_LINK_ADDR

ifeq ($(H2K_LOAD_ADDR),)
H2K_LOAD_ADDR = 0xff000000
endif
export H2K_LOAD_ADDR

ifeq ($(H2K_HEAP_SIZE),)
H2K_HEAP_SIZE = 0x100000
endif

CFLAGS = -Os $(OPTIMIZE) -mv$(ARCHV) -DARCHV=$(ARCHV) -Wall -Werror -g -moslib=h2 -I../install/include -I$(KERNELPATH)/include -L../install/lib

all: hello booter

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

vapa.x: vapa.x.in
	cat $< | perl -p -i -e 's/0xxxxxxxxx/$$ENV{H2K_LOAD_ADDR}/' > $@

hello: hello.c
	qdsp6-gcc $(CFLAGS)  -I../install/include -L../install/lib -Wl,--section-start=.start=0x00010000 -o hello hello.c 

booter: booter.o elf.o booter_event.o boot.o vapa.x
	${CC} $(CFLAGS) -lh2kernel -moslib=h2kernel -Wl,--section-start=.boot=$(H2K_LINK_ADDR) -Wl,--defsym=HEAP_SIZE=$(H2K_HEAP_SIZE) -Wl,--defsym=h2_v2p_offset=`perl -e '$$_ = (oct $$ENV{H2K_LINK_ADDR}); $$_ -= (oct $$ENV{H2K_LOAD_ADDR}); $$_ = sprintf("0x%0x", $$_); print'` -Wl,--script=vapa.x -o booter booter.o elf.o booter_event.o boot.o

	${CC} $(CFLAGS) -lh2kernel -moslib=h2kernel -Wl,--section-start=.boot=$(H2K_LINK_ADDR) -Wl,--defsym=HEAP_SIZE=$(H2K_HEAP_SIZE) -Wl,--defsym=h2_v2p_offset=`perl -e '$$_ = (oct $$ENV{H2K_LINK_ADDR}); $$_ -= (oct $$ENV{H2K_LOAD_ADDR}); $$_ = sprintf("0x%0x", $$_); print'` -Wl,--script=vapa.x -Wl,--entry=`hexagon-nm booter | egrep ' __boot$$' | perl -p -e 's/\s.*//; $$_ = oct "0x$$_"; $$_ -= ((oct $$ENV{H2K_LINK_ADDR}) - (oct $$ENV{H2K_LOAD_ADDR}));$$_ = sprintf("0x%0x", $$_)'` -o booter booter.o elf.o booter_event.o boot.o

test: hello booter
	qdsp6-sim -- booter hello

install: booter
	mkdir -p $(INSTALLPATH)/bin
	cp booter $(INSTALLPATH)/bin

clean:
	rm -f *.elf booter hello stats.* vapa.x *.o
