# Yo.  -*- Makefile -*-

# asm_offsets.h is generated at build time without running anything on the
# target: offsets.ref.c is compiled to assembly only, and its
# ".ascii \"->SYM value\"" markers are turned into #defines (the technique
# Linux uses for asm-offsets). Hexagon prints immediates as "#value", so
# strip a leading '#' (or '$').

define sed-offsets
	's:^[[:space:]]*\.ascii[[:space:]]*"->\([^ ]*\) [#$$]*\([^"]*\)".*:#define \1 \2:p'
endef

$(BUILD_DIR)/include/asm_offsets.h: $(BUILD_DIR)/offsets.s
	mkdir -p $(dir $@)
	{ echo '/* Generated from offsets.ref.c; do not edit. */'; \
	  sed -n $(sed-offsets) $<; } > $@.tmp
	grep -q '^#define CONTEXT_SIZE ' $@.tmp
	mv $@.tmp $@

# Also depends on every other installed header; see kernel/Makefile.
$(BUILD_DIR)/offsets.s: $(DIR)offsets.ref.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -S -o $@ $<

DEST_HFILES += $(BUILD_DIR)/include/asm_offsets.h

CLEAN_EXTRAS += $(BUILD_DIR)/offsets.s $(BUILD_DIR)/include/asm_offsets.h.tmp
