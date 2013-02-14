


include/asm_offsets.h: $(BUILD_DIR)/offsets
	$(SIM) $< > $@

$(BUILD_DIR)/offsets: $(DIR)offsets.ref.c includes

# include/context.h include/c_std.h include/globals.h include/max.h include/trace.h include/trace_constants.h include/futex.h include/futex_common.h include/futex_pi.h include/futex_classic.h include/futex_misc.h
	$(CC) $(CFLAGS) -o $@ $<

DEST_HFILES += include/asm_offsets.h

CLEAN_EXTRAS += $(BUILD_DIR)/offsets

