# TRACE32 Extension Development Kit 
# Makefile to build tool 'elf2t32' tool
# Copyright Lauterbach GmbH

# make variable EDKHOST:={win32|lnx32|sol_s} selects host
# Example:
# ../bin/make -f elf2t32.mk EDKHOST:=win32

GNUDIR := ../bin
SRC    := elf2t32.c

ifeq ($(EDKHOST)A,win32A)
  TARGET := $(GNUDIR)/elf2t32.exe
  RM     := del
  HOSTCC := cl
else
  TARGET := $(GNUDIR)/elf2t32
  RM     := rm
  HOSTCC := gcc
endif

.PHONY: default
default: $(TARGET)

.PHONY: clean
clean:
	-$(RM) $(TARGET)

$(GNUDIR)/elf2t32: $(SRC)
	$(HOSTCC) -DUNIX -s -o$@ $<

$(GNUDIR)/elf2t32.exe: $(SRC)
	$(HOSTCC) -DWINDOWS -nologo -Ox -Fe$@ $< -MT -link -subsystem:console
	-$(RM) elf2t32.obj

#eof

