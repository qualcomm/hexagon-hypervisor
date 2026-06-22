# Yo.  -*- Makefile -*-

# offsets is a target-side generator program (run under simulator) and uses
# libc headers like <stdio.h>. When building with PICOLIBC, select the picolibc
# sysroot via --target and --cstdlib flags.
ifeq ($(PICOLIBC),1)
OFFSETS_TARGET_FLAG := --target=hexagon-none-elf --cstdlib=picolibc
else
OFFSETS_TARGET_FLAG :=
endif

ifeq ($(PICOLIBC),1)
# Provide heap symbols and default memory layout expected by picolibc's libc.
# Ask the compiler for the sysroot-relative path so this works across toolchains.
OFFSETS_LDSCRIPT := -Wl,-T,$(shell $(CC) $(OFFSETS_TARGET_FLAG) -print-file-name=picolibc.ld)
else
OFFSETS_LDSCRIPT :=
endif

$(BUILD_DIR)/include/asm_offsets.h: $(BUILD_DIR)/offsets
	mkdir -p $(dir $@)
ifeq ($(RUN), $(RUN_TOOLSIM))
	$(RUN) $< -- $@
else
	$(RUN) $< $@
endif

$(BUILD_DIR)/offsets: $(DIR)offsets.ref.c includes
	mkdir -p $(dir $@)
	$(CC) $(OFFSETS_TARGET_FLAG) $(OFFSETS_LDSCRIPT) $(CFLAGS) -o $@ $<

DEST_HFILES += $(BUILD_DIR)/include/asm_offsets.h

CLEAN_EXTRAS += $(BUILD_DIR)/offsets
