include $(H2DIR)/scripts/Makefile.inc.config
include $(H2DIR)/scripts/Makefile.inc.tools

DOX_HEADERS ?= $(H2DIR)/artifacts/v$(ARCHV)/$(TARGET)/install/include

all:
	mkdir -p $(H2DIR)/artifacts/docs/html/h2
	rm -f ../inc/others
	ln -s $(DOX_HEADERS) ../inc/others
	$(DOXYGEN) Doxy_Config.ldd

clean:
	rm -Rf $(H2DIR)/docs/html/h2 warn.log ../inc/others
