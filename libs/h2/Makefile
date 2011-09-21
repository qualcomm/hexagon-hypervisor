
ALL: dummy

ifeq ($(ARCHV),)
ARCHV = 3
endif

CC = qdsp6-gcc
AR = qdsp6-ar
SIM = qdsp6-sim

OPTIMIZE := -Os

CFLAGS := -Iinclude -G0 $(OPTIMIZE) -mv$(ARCHV) -DARCHV=$(ARCHV) -Wall -g

ifdef DEBUG
CFLAGS += -DDEBUG
endif

ifdef PARANOID
	CFLAGS += -DPARANOID
endif

#export H2_LIB_INCLUDE=$(shell pwd)/include
#
#ASM_FILES = src/h2_futex.S src/h2_intwait.S src/h2_mutex.S src/h2_thread.S src/h2_cycles.S src/h2_sem.S src/h2_fastint.S src/h2_prio.S src/#h2_allsignal.S src/h2_anysignal.S src/h2_prefetch.S src/h2_cond.S src/h2_barrier.S src/h2_config.S src/h2_vmtraps.S src/h2_hwconfig.S src/#h2_pmu.S src/h2_vm.S
#C_FILES = src/h2_alloc.c src/h2_pipe.c src/h2_printf.c src/h2_rmutex.c src/h2_map.c src/h2_error.c
#
#O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

SRC_HFILES := 
DEST_HFILES := 
CLEAN_EXTRAS := 

REF_CFILES := 
REF_SFILES := 
REF_OFILES := 

CHECK_CFILES := 
CHECK_SFILES := 
CHECK_OFILES := 

OPT_V2_CFILES := 
OPT_V2_SFILES := 
OPT_V2_OFILES := 

OPT_V3_CFILES := 
OPT_V3_SFILES := 
OPT_V3_OFILES := 

OPT_V4_CFILES := 
OPT_V4_SFILES := 
OPT_V4_OFILES := 

OPT_V5_CFILES := 
OPT_V5_SFILES := 
OPT_V5_OFILES := 

BUILD_DIR := .

MKINCLUDES := $(shell find . -name make.inc)

MAKE_INC_DEFAULT := build/make/make.inc.default

define DO_INCLUDE
LIB := h2
DIR := $(dir $(MODPATH))
MOD := $(strip $(shell basename $(dir $(MODPATH))))
include $(MODPATH)
endef

$(foreach MODPATH,$(MKINCLUDES),$(eval $(DO_INCLUDE)))

ALL_OFILES := $(REF_OFILES) $(CHECK_OFILES) $(OPT_V2_OFILES) $(OPT_V3_OFILES) $(OPT_V4_OFILES)

$(ALL_OFILES): $(DEST_HFILES)

ifeq ($(ARCHV),3)
OPT_OFILES := $(OPT_V3_OFILES)
endif

ifeq ($(ARCHV),4)
OPT_OFILES := $(OPT_V4_OFILES)
endif

ifeq ($(ARCHV),5)
OPT_OFILES := $(OPT_V5_OFILES)
endif

build/ref/libh2.a: $(REF_OFILES)
	mkdir -p $(dir $@); \
	$(AR) cru $@ $(REF_OFILES);
#	cp -R include build/ref;

build/opt_v$(ARCHV)/libh2.a: $(OPT_V$(ARCHV)_OFILES)
	echo $(OPT_V$(ARCHV)_OFILES); \
	mkdir -p $(dir $@); \
	$(AR) cru $@ $(OPT_V$(ARCHV)_OFILES);
#	cp -R include build/opt_v$(ARCHV)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(ALL_OFILES) $(DEST_HFILES) $(CLEAN_EXTRAS) build/ref build/opt*

dummy:
	@echo $(REF_CFILES)
	@echo $(REF_SFILES)
	@echo $(REF_OFILES)
	@echo $(SRC_HFILES)
	@echo $(DEST_HFILES)
	@echo $(HELLO)

includes: $(DEST_HFILES)

ref: build/ref/libh2.a

opt: build/opt_v$(ARCHV)/libh2.a


