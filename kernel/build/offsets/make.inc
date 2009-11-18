
include/offsets.h: $(BUILD_DIR)/offsets
	$(SIM) $< > $@

$(BUILD_DIR)/offsets: $(DIR)offsets.ref.c include/context.h include/c_std.h
	$(CC) $(CFLAGS) -o $@ $<

DEST_HFILES += include/offsets.h

