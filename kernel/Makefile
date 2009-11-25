
ALL: dummy

ifeq ($(ARCHV),)
ARCHV = 3
endif

CC = qdsp6-gcc 
AR = qdsp6-ar
SIM := qdsp6-sim

CFLAGS := -Iinclude -G0 -O2 -mv$(ARCHV) -DARCHV=$(ARCHV) -Wall

REF_CFILES := 
REF_SFILES := 
REF_OFILES := 
SRC_HFILES := 
DEST_HFILES := 
CLEAN_EXTRAS := 
HELLO := 

BUILD_DIR := .

MKINCLUDES := $(shell find . -name make.inc)

MAKE_INC_DEFAULT := build/make/make.inc.default


define DO_INCLUDE
DIR := $(dir $(MODPATH))
MOD := $(strip $(shell basename $(dir $(MODPATH))))
include $(MODPATH)
endef

$(foreach MODPATH,$(MKINCLUDES),$(eval $(DO_INCLUDE)))

$(REF_OFILES): $(DEST_HFILES)

build/ref/libblastkernel.a: $(REF_OFILES)
	mkdir -p $(dir $@)
	$(AR) cru $@ $(REF_OFILES)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(REF_OFILES) $(DEST_HFILES) $(CLEAN_EXTRAS) build/ref

dummy:
	@echo $(REF_CFILES)
	@echo $(REF_SFILES)
	@echo $(REF_OFILES)
	@echo $(SRC_HFILES)
	@echo $(DEST_HFILES)
	@echo $(HELLO)

includes: $(DEST_HFILES)

ref: build/ref/libblastkernel.a


