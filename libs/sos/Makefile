
ALL: dummy

OPTIMIZE := -Os

CFLAGS := -Iinclude -G0 $(OPTIMIZE) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Wall -g

ifdef DEBUG
CFLAGS += -DDEBUG
endif

ifdef PARANOID
	CFLAGS += -DPARANOID
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

MAKE_INC_DEFAULT := ../h2/build/make/make.inc.default

define DO_INCLUDE
LIB := sos
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

ifeq ($(ARCHV),60)
OPT_OFILES := $(OPT_V60_OFILES)
endif

build/ref/libsos.a: $(REF_OFILES)
	mkdir -p $(dir $@); \
	$(AR) cru $@ $(sort $(REF_OFILES);)

build/opt_v$(ARCHV)/libsos.a: $(OPT_V$(ARCHV)_OFILES)
	echo $(OPT_V$(ARCHV)_OFILES); \
	mkdir -p $(dir $@); \
	$(AR) cru $@ $(sort $(OPT_V$(ARCHV)_OFILES);)

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

ref: build/ref/libsos.a

opt: build/opt_v$(ARCHV)/libsos.a


