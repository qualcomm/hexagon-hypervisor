STANDALONE=1

OBJS+=test.o
EXEC=test.elf
PYTHON_GENERATOR_SCRIPT=gentests.py
PYTHON_GENERATED_FILES=generated_tests.dat

# Use 'override' so Makefile.inc.test's default (H2K_HEAP_SIZE = 0x00010000)
# cannot overwrite this value.  The tree test needs 1 MB to hold 700-node
# trees across 32 test cases without exhausting the heap.
override H2K_HEAP_SIZE := 0x00100000

include Makefile.inc

OSLIB=
