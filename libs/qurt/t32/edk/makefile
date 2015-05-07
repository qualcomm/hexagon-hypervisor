# TRACE32 Extension Development Kit 
# Makefile to build TASK/EXTension modules
# Copyright Lauterbach GmbH

EDKDIR:=.
INCDIR:=$(EDKDIR)/inc
SRCDIR:=$(EDKDIR)/src
GNULIB:=$(EDKDIR)/lib
GNUDIR:=$(EDKDIR)/bin

XRM:=$(GNUDIR)/rm
XMD:=$(GNUDIR)/mkdir
XCC:=$(GNUDIR)/xgcc
XLD:=$(GNUDIR)/ld
XSR:=$(GNUDIR)/elf2t32

XCCOPT:=-g -O -I$(INCDIR) -B$(GNUDIR) -msoft-float 
XLDLIB:=-L$(GNULIB) -lc -lt32 -Telf32ppc.ld

ifeq "$(MAKE_VERSION)" "3.80"
  PATH:=$(GNUDIR) # avoid cygwin conflict
endif

.PHONY: default
default: usage

.PHONY: usage
usage:  # GNU make 3.80 does not have the 'info' command
	@echo '- make usage       -- prints this'
	@echo '- make all         -- builds generic.t32 ucos.t32 dalvik.t32)'
	@echo '- make generic.t32 -- builds "Generic"      extension sample'
	@echo '- make ucos.t32    -- builds "micro-C/OS"   extension sample'
	@echo '- make dalvik.t32  -- builds "Dalvik" VM Awareness extension'
	@echo '- make qurt_model.t32 -- needs YOUREXT.c and builds YOUREXT.t32'
	@echo '- make clean       -- deletes all intermediate files'
	@echo '- make distclean   -- deletes all generated files'

.PHONY: all
all:	generic.t32 ucos.t32 dalvik.t32

.PHONY: clean
clean:
	-$(XRM) -f *.o *.elf *.map
	
.PHONY: distclean
distclean: clean
	-$(XRM) -f *.t32

# do not delete intermediate ELF files (useful for debugging)
.PRECIOUS: %.elf

# core build rules

%.t32: %.elf
	$(XSR) $< $@

%.elf: %.o $(GNULIB)/crt0.o $(GNULIB)/libt32.a
	$(XLD) -Map $*.map -o $@ $(GNULIB)/crt0.o $< $(XLDLIB)
	
# two rules: find source in . and in ./src
%.o:  %.c $(INCDIR)/t32ext.h
	$(XCC) $(XCCOPT) -c -o $@ $<

# for gmake 3.80, we need the full command
%.o:  $(SRCDIR)/%.c $(INCDIR)/t32ext.h
	$(XCC) $(XCCOPT) -c -o $@ $<

# debugging: compile to assembly code
%.S:  %.c $(INCDIR)/t32ext.h
	$(XCC) $(XCCOPT) -S -o $@ $<

# avoid "No rule to make target [name]", name all missing EDK files
# [with $(error ..), each file would immediately stop the build] 
$(GNULIB)/libt32.a $(GNULIB)/crt0.o $(INCDIR)/t32ext.h:
	$(warning EDK installation not complete - missing file $@)

# eof

