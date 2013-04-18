
ALL: dummy

include ../scripts/Makefile.inc.tools

CFLAGS := -Iinclude -G0 $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -Werror -g

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
TEST_INC_DEFAULT := build/make/test.inc.default


define DO_INCLUDE
DIR := $(dir $(MODPATH))
MOD := $(strip $(shell basename $(dir $(MODPATH))))
include $(MODPATH)
endef

$(foreach MODPATH,$(MKINCLUDES),$(eval $(DO_INCLUDE)))

ALL_OFILES := $(REF_OFILES) $(CHECK_OFILES) $(OPT_V2_OFILES) $(OPT_V3_OFILES) $(OPT_V4_OFILES) $(OPT_V5_OFILES) $(OPT_V60_OFILES)

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

build/ref/libh2os.a: $(REF_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(REF_OFILES)

build/opt_v$(ARCHV)/libh2os.a: $(OPT_V$(ARCHV)_OFILES)
	@echo $(OPT_V$(ARCHV)_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(OPT_V$(ARCHV)_OFILES)

build/ref/libh2check.a: $(CHECK_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(CHECK_OFILES)


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

ref: build/ref/libh2os.a

ref_install: ref
	mkdir -p $(INSTALLPATH)/lib
	cp build/ref/libh2os.a $(INSTALLPATH)/lib

opt: build/opt_v$(ARCHV)/libh2os.a

opt_install: opt
	mkdir -p $(INSTALLPATH)/lib
	cp build/opt_v$(ARCHV)/libh2os.a $(INSTALLPATH)/lib

check: build/ref/libh2check.a

