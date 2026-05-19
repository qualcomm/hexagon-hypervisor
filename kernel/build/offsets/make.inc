# Yo.  -*- Makefile -*-

$(BUILD_DIR)/include/asm_offsets.h: $(BUILD_DIR)/offsets
	mkdir -p $(dir $@)
ifeq ($(RUN), $(RUN_TOOLSIM))
	$(RUN) $< -- $@
else
	$(RUN) $< $@
endif

$(BUILD_DIR)/offsets: $(DIR)offsets.ref.c includes
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

DEST_HFILES += $(BUILD_DIR)/include/asm_offsets.h

CLEAN_EXTRAS += $(BUILD_DIR)/offsets

