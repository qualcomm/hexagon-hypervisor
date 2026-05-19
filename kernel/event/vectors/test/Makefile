STANDALONE=1

OBJS+=test.o vectors.ref.o
EXEC=test.elf

include Makefile.inc

# vectors.ref.S lives in the parent directory; reference it by absolute path
# so the build works both in-source and in an out-of-tree build directory.
vectors.ref.o: $(H2DIR)/kernel/event/vectors/vectors.ref.S
	$(CC) $(ASFLAGS) -c -o $@ $<; \
	if [ $$? -ne 0 ]; then echo -e "FAIL ${TEST}\nbuild fail"|tee test.log; fi

