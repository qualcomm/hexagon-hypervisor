SHELL=/bin/bash
ULIMIT=ulimit -t 300

SIZE_TOOL = /prj/dsp/qdsp6/arch/scripts/section_size.py
LD = qdsp6-gcc

SIM=qdsp6-sim
RUN=$(SIM)

ifeq (,$(findstring -,$(RUN)))
COM_SIMF=--timing --quiet --timer 32000 --timerpin 2
else
COM_SIMF=--timing --quiet --cosim_file timer.cfg
endif

ifneq (,${Q6VERSION})
ARCHV=$(subst v,,$(Q6VERSION))
else
ARCHV=4
endif

COM_CFLAGS=
COM_LDFLAGS=

#If you are running outside hexframe.. sensible defaults
ifeq (,${BUILD_DIR})
CC=qdsp6-gcc
SIMF=$(COM_SIMF)
#hexframe likes duplication ATM Machine, PIN Number, Q6VERSION Version....
else
SIMF+=$(COM_SIMF)
endif

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(PWD)/../install
endif

ifeq ($(KERNELPATH),)
export KERNELPATH := $(PWD)/../kernel
endif

OPTIMIZE=-O2

CFLAGS = -Wall -g $(OPTIMIZE) -mv${ARCHV} -I. -Iinc -Isrc -I$(INSTALLPATH)/include -I$(KERNELPATH)/include -L. -L$(INSTALLPATH)/lib $(EXTRA_CFLAGS)

.PHONY: all clean sim

all: loadlinux

loadlinux: loadlinux.o
	${CC} -moslib=h2 -moslib=h2kernel ${CFLAGS} -Wl,--section-start=.start=0xff000000 -Wl,--defsym=linux_stext=0x00000000 -Wl,--defsym=HEAP_SIZE=0x100000 -o $@ loadlinux.o

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f loadlinux.o loadlinux


sim test.log : loadlinux
	$(ULIMIT) && $(RUN) $(SIMF) -- loadlinux $(LINUX) 2>&1 | tee test.log
