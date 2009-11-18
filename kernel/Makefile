
ALL: dummy

REF_CFILES := 
REF_SFILES := 
REF_OFILES := 
SRC_HFILES := 
DEST_HFILES := 
HELLO := 

MKINCLUDES := $(shell find . -name make.inc)

MAKE_INC_DEFAULT := build/make/make.inc.default

define DO_INCLUDE
DIR := $(dir $(MODPATH))
MOD := $(strip $(shell basename $(dir $(MODPATH))))
include $(MODPATH)
endef

$(foreach MODPATH,$(MKINCLUDES),$(eval $(DO_INCLUDE)))


dummy:
	@echo $(REF_CFILES)
	@echo $(REF_SFILES)
	@echo $(REF_OFILES)
	@echo $(SRC_HFILES)
	@echo $(DEST_HFILES)
	@echo $(HELLO)

includes: $(DEST_HFILES)

