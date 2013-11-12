# Yo.  -*- Makefile -*-

ifeq (0, $(shell [ $(ARCHV) -gt 5 ]; echo $$?))
OFFSETS_SIM := PKW_VERSIONS=hexagon-tools=v60_stable $(RUN) --quiet
else
OFFSETS_SIM := PKW_VERSIONS=hexagon-tools=default $(RUN) --quiet
endif

include/asm_offsets.h: $(BUILD_DIR)/offsets
	$(OFFSETS_SIM) $< > $@

$(BUILD_DIR)/offsets: $(DIR)offsets.ref.c includes
	$(CC) $(CFLAGS) -o $@ $<

DEST_HFILES += include/asm_offsets.h

CLEAN_EXTRAS += $(BUILD_DIR)/offsets

