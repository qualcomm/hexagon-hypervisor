# Yo.  -*- Makefile -*-

# offsets is a target-side generator program (run under simulator) and uses
# libc headers like <stdio.h>. When building with PICOLIBC, select the picolibc
# sysroot via the target triple.
ifeq ($(PICOLIBC),1)
OFFSETS_TARGET_FLAG := --target=hexagon-none-picolibc
else
OFFSETS_TARGET_FLAG :=
endif

ifeq ($(PICOLIBC),1)
# Provide heap symbols and default memory layout expected by picolibc's libc.
# Use the linker script from the selected toolchain sysroot.
ifneq ($(TOOLCHAIN_BIN),)
OFFSETS_LDSCRIPT := -Wl,-T,$(abspath $(TOOLCHAIN_BIN)/../target/hexagon-unknown-none-picolibc/lib/v$(ARCHV)/G0/picolibc.ld)
else
OFFSETS_LDSCRIPT := -Wl,-T,$(shell $(CC) $(OFFSETS_TARGET_FLAG) -print-file-name=picolibc.ld)
endif
else
OFFSETS_LDSCRIPT :=
endif

include/asm_offsets.h: $(BUILD_DIR)/offsets
ifeq ($(RUN), $(RUN_TOOLSIM))
	$(RUN) $< -- $@
else
	$(RUN) $< $@
endif

$(BUILD_DIR)/offsets: $(DIR)offsets.ref.c includes
	$(CC) $(OFFSETS_TARGET_FLAG) $(OFFSETS_LDSCRIPT) $(CFLAGS) -o $@ $<

DEST_HFILES += include/asm_offsets.h

CLEAN_EXTRAS += $(BUILD_DIR)/offsets
