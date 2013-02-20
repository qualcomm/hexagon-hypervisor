


include/asm_offsets.h: $(BUILD_DIR)/offsets
	$(SIM) $< > $@

$(BUILD_DIR)/offsets: $(DIR)offsets.ref.c includes
	$(CC) $(CFLAGS) -o $@ $<

DEST_HFILES += include/asm_offsets.h

CLEAN_EXTRAS += $(BUILD_DIR)/offsets

