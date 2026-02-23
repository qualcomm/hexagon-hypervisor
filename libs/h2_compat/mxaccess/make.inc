include $(MAKE_INC_DEFAULT)

include $(H2DIR)/scripts/Makefile.inc.opensource
ifeq ($(HMX_HLX_SUPPORT),1)
CFLAGS += -DHMX_HLX_SUPPORT
endif
