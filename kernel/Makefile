
ALL: dummy

H2DIR ?= ..

include ../scripts/Makefile.inc.tools

export PKW_VERSIONS := hexagon-tools=$(TOOLS_FLAVOR_KERNEL) arch=$(ARCH_FLAVOR)

include ../scripts/Makefile.inc.config

include ../scripts/Makefile.inc.version

CFLAGS := @build/flags/abi -Iinclude -I../libs/h2/common -G0 $(OPTIMIZE) -mv$(KERNEL_TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g

CFLAGS += -DH2K_LINK_ADDR=$(H2K_LINK_ADDR) -DH2K_GUEST_END=$(H2K_GUEST_END)

ifdef NULL_ANGEL_TRAP
CFLAGS += -DNULL_ANGEL_TRAP
endif

ifdef DEBUG
CFLAGS += -DDEBUG
endif

ifdef HTHREADS
CFLAGS += -DNUM_HTHREADS=$(HTHREADS)
endif

ifdef HTHREADS_MASK
CFLAGS += -DHTHREADS_MASK=$(HTHREADS_MASK)
endif

ifdef MAX_BOOT_CONTEXTS
CFLAGS += -DMAX_BOOT_CONTEXTS=$(MAX_BOOT_CONTEXTS)
endif


ifeq ($(USE_TCM), 1)
CFLAGS += -DH2K_USE_TCM
endif

ifdef NO_DEVICES
CFLAGS += -DNO_DEVICES
endif

ifdef LOGBUF
CFLAGS += -DH2K_LOGBUF
endif

CFLAGS += -DH2K_KERNEL_PGSIZE=$(H2K_KERNEL_PGSIZE) -DH2K_KERNEL_ADDRBITS=$(H2K_KERNEL_ADDRBITS) -DH2K_GIT_COMMIT=$(H2K_GIT_COMMIT)

CFLAGS += $(KERNEL_CFLAGS) $(H2K_EXTRA_CFLAGS)

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

OPT_V60_CFILES := 
OPT_V60_SFILES := 
OPT_V60_OFILES := 

OPT_V65_CFILES := 
OPT_V65_SFILES := 
OPT_V65_OFILES := 

OPT_V68_CFILES := 
OPT_V68_SFILES := 
OPT_V68_OFILES := 

BUILD_DIR := .

MKINCLUDES := $(shell find . -name make.inc)

MAKE_INC_DEFAULT := build/make/make.inc.default


define DO_INCLUDE
DIR := $(dir $(MODPATH))
MOD := $(strip $(shell basename $(dir $(MODPATH))))
include $(MODPATH)
endef

$(foreach MODPATH,$(MKINCLUDES),$(eval $(DO_INCLUDE)))

ALL_OFILES := $(REF_OFILES) $(CHECK_OFILES) $(OPT_V2_OFILES) $(OPT_V3_OFILES) $(OPT_V4_OFILES) $(OPT_V5_OFILES) $(OPT_V60_OFILES) $(OPT_V65_OFILES) $(OPT_V68_OFILES)

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

ifeq ($(ARCHV),60)
OPT_OFILES := $(OPT_V60_OFILES)
endif

ifeq ($(ARCHV),65)
OPT_OFILES := $(OPT_V65_OFILES)
endif

ifeq ($(ARCHV),68)
OPT_OFILES := $(OPT_V68_OFILES)
endif

build/ref/libh2kernel.a: $(REF_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(sort $(REF_OFILES))

build/opt_v$(ARCHV)/libh2kernel.a: $(OPT_V$(ARCHV)_OFILES)
	@echo $(OPT_V$(ARCHV)_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(sort $(OPT_V$(ARCHV)_OFILES))

build/ref/libh2check.a: $(CHECK_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(sort $(CHECK_OFILES))


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(ALL_OFILES) $(DEST_HFILES) $(CLEAN_EXTRAS) include/* build/ref build/opt* \
	sched/check_sanity/check_sanity_debug.h util/intcontrol/intcontrol_debug.h \
	pmu_statsfile.txt

dummy:
	@echo $(REF_CFILES)
	@echo $(REF_SFILES)
	@echo $(REF_OFILES)
	@echo $(SRC_HFILES)
	@echo $(DEST_HFILES)
	@echo $(HELLO)

includes: $(DEST_HFILES)

ref: build/ref/libh2kernel.a

ref_install: info ref
	mkdir -p $(INSTALLPATH)/lib
	mkdir -p $(INSTALLPATH)/include
	cp build/ref/libh2kernel.a $(INSTALLPATH)/lib
	cp include/asm_offsets.h $(INSTALLPATH)/include # for state trap

opt: build/opt_v$(ARCHV)/libh2kernel.a

.PHONY: info
info :
	@echo Kernel PKW_VERSIONS $(PKW_VERSIONS)
	pkw --which $(CC)

opt_install: info opt
	mkdir -p $(INSTALLPATH)/lib
	mkdir -p $(INSTALLPATH)/include
	cp build/opt_v$(ARCHV)/libh2kernel.a $(INSTALLPATH)/lib
	cp include/asm_offsets.h $(INSTALLPATH)/include # for state trap

check: build/ref/libh2check.a

