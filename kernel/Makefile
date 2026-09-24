
ALL: dummy

H2DIR ?= ..

include ../scripts/Makefile.inc.tools
include ../scripts/Makefile.inc.install

ifeq ($(USE_PKW),1)
export PKW_VERSIONS := hexagon-tools=$(TOOLS_FLAVOR_KERNEL) arch=$(ARCH_FLAVOR)
endif

include ../scripts/Makefile.inc.config
include ../scripts/Makefile.inc.opensource
include ../scripts/Makefile.inc.version

CFLAGS := @build/flags/abi -I../libs/h2/common -G0 $(OPTIMIZE) $(OPT_ADD) -mv$(KERNEL_TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g

CFLAGS += -DH2K_LINK_ADDR=$(H2K_LINK_ADDR) -DH2K_LOAD_ADDR=$(H2K_LOAD_ADDR) -DH2K_GUEST_END=$(H2K_GUEST_END) -DH2K_BOOTMEM_SIZE=$(H2K_BOOTMEM_SIZE)

ifeq ($(HMX_HLX_SUPPORT),1)
	CFLAGS += -DHMX_HLX_SUPPORT
endif

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

ifdef MULTICORE
CFLAGS += -DMULTICORE
endif

# Force all assembly to go through ASM_BKL_LOCK/ASM_BKL_UNLOCK in asm_std.h.
CFLAGS += -Dk0lock=DO_NOT_USE_K0LOCK -Dk0unlock=DO_NOT_USE_K0UNLOCK

ifdef SOFT_BKL
CFLAGS += -DSOFT_BKL
endif

CFLAGS += -DH2K_KERNEL_PGSIZE=$(H2K_KERNEL_PGSIZE) -DH2K_KERNEL_ADDRBITS=$(H2K_KERNEL_ADDRBITS) -DH2K_GIT_COMMIT=$(H2K_GIT_COMMIT)

CFLAGS += $(KERNEL_CFLAGS) $(H2K_EXTRA_CFLAGS)
ifeq ($(EN_LLVM_COV), 1)
CFLAGS += ${LLVM_COV_CFLAGS}
endif

SRC_HFILES := 
DEST_HFILES := 
CLEAN_EXTRAS := 

REF_CFILES := 
REF_SFILES := 
REF_OFILES := 

CHECK_CFILES := 
CHECK_SFILES := 
CHECK_OFILES := 

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

OPT_V73_CFILES := 
OPT_V73_SFILES := 
OPT_V73_OFILES := 

OPT_V81_CFILES := 
OPT_V81_SFILES := 
OPT_V81_OFILES := 

# Derive build dir as a sibling of INSTALLPATH so it works regardless of
# whether INSTALLPATH ends in /install or is a user-supplied arbitrary path.
# e.g. artifacts/v81/opt/install → artifacts/v81/opt/build/kernel
#      install-my               → install-my/../build/kernel
BUILD_DIR := $(INSTALLPATH)/../build/kernel

# Add per-ARCHV/variant include dir so asm_offsets.h (generated into
# BUILD_DIR/include/) is found by all kernel compilation units.
CFLAGS += -I$(BUILD_DIR)/include

MKINCLUDES := $(shell find . -name make.inc)

MAKE_INC_DEFAULT := build/make/make.inc.default


define DO_INCLUDE
DIR := $(dir $(MODPATH))
MOD := $(strip $(shell basename $(dir $(MODPATH))))
include $(MODPATH)
endef

$(foreach MODPATH,$(MKINCLUDES),$(eval $(DO_INCLUDE)))

# The 'offsets' probe (build/offsets/make.inc) is compiled and then RUN to
# generate asm_offsets.h, so it needs the copied headers first.  Declared here,
# not in that make.inc, because prerequisites expand where written and
# DEST_HFILES is only complete after the includes above.  asm_offsets.h itself is
# filtered out: it is this chain's own output.
#
# Deliberately a blanket dependency on every kernel header, unlike the objects
# below: assembly hardcodes the offsets it computes, so a stale asm_offsets.h is
# not a slow build, it is silently wrong code.  Regenerating it is cheap.
$(BUILD_DIR)/offsets: $(filter-out $(BUILD_DIR)/include/asm_offsets.h,$(DEST_HFILES))

ALL_OFILES := $(REF_OFILES) $(CHECK_OFILES) $(OPT_V4_OFILES) $(OPT_V5_OFILES) $(OPT_V60_OFILES) $(OPT_V65_OFILES) $(OPT_V68_OFILES) $(OPT_V73_OFILES) $(OPT_V81_OFILES)

ifdef LOGBUF
_logbuf_one_mod   := $(word 1,$(subst [, ,$(LOGBUF)))
_logbuf_one_level := $(subst ],,$(word 2,$(subst [, ,$(LOGBUF))))
ifeq ($(_logbuf_one_mod),1)
CFLAGS += -DH2K_LOGBUF
ifneq ($(_logbuf_one_level),)
CFLAGS += -DH2K_LOG_LEVEL=$(_logbuf_one_level)
endif
else
_logbuf_comma := ,

$(foreach _tok,$(subst $(_logbuf_comma), ,$(LOGBUF)),\
  $(eval _ll_mod   := $(word 1,$(subst [, ,$(_tok))))\
  $(eval _ll_level := $(subst ],,$(word 2,$(subst [, ,$(_tok)))))\
  $(eval _logbuf_hits := $(filter %/$(basename $(_ll_mod)).o,$(ALL_OFILES)))\
  $(if $(_logbuf_hits),,\
    $(eval _logbuf_hits := $(foreach _o,$(ALL_OFILES),$(if $(findstring /$(_ll_mod)/,$(_o)),$(_o)))))\
  $(if $(_logbuf_hits),\
    $(eval $(_logbuf_hits) : CFLAGS += -DH2K_LOGBUF)\
    $(if $(_ll_level),$(eval $(_logbuf_hits) : CFLAGS += -DH2K_LOG_LEVEL=$(_ll_level))),\
    $(warning LOGBUF: token '$(_ll_mod)' matched no kernel sources)))

$(foreach _o,$(ALL_OFILES),\
  $(if $(findstring /util/log/,$(_o)),$(eval $(_o) : CFLAGS += -DH2K_LOGBUF)) \
  $(if $(findstring /init/setup/,$(_o)),$(eval $(_o) : CFLAGS += -DH2K_LOGBUF)))
endif
endif

# Order-only (|): headers must exist before compiling; the .d files carry the edges.
$(ALL_OFILES): | $(DEST_HFILES)

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

ifeq ($(ARCHV),73)
OPT_OFILES := $(OPT_V73_OFILES)
endif

ifeq ($(ARCHV),81)
OPT_OFILES := $(OPT_V81_OFILES)
endif

ifeq ($(BUILD_TYPE),ref)
$(BUILD_DIR)/libh2kernel.a: $(REF_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(sort $(REF_OFILES))
else ifeq ($(BUILD_TYPE),opt)
$(BUILD_DIR)/libh2kernel.a: $(OPT_OFILES)
	@echo $(OPT_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(sort $(OPT_OFILES))
else
	$(error BUILD_TYPE == $(value BUILD_TYPE))
endif

$(BUILD_DIR)/libh2check.a: $(CHECK_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(sort $(CHECK_OFILES))


$(BUILD_DIR)/%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o : %.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

clean:
	rm -rf $(DEST_HFILES) $(CLEAN_EXTRAS) $(BUILD_DIR) include \
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

# Must precede 'install': prerequisites expand at parse, so populate them first.
$(call install_files,$(INSTALLPATH)/lib,$(BUILD_DIR)/libh2kernel.a $(BUILD_DIR)/libh2check.a)
$(call install_files,$(INSTALLPATH)/include,$(BUILD_DIR)/include/asm_offsets.h)

install: info $(INSTALL_TARGETS)

.PHONY: info
info :
ifeq ($(USE_PKW),1)
	@echo Kernel PKW_VERSIONS $(PKW_VERSIONS)
	pkw --which $(CC)
endif

check: $(BUILD_DIR)/libh2check.a

# Per-object header deps from $(DEPFLAGS); '-' tolerates a clean tree with no .d.
-include $(ALL_OFILES:.o=.d)
