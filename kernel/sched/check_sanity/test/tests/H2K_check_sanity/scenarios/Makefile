STANDALONE=1

OBJS+=test.o
EXEC=test.elf
PYTHON_GENERATOR_SCRIPT=scenarios.py
PYTHON_GENERATED_FILES=scenarios.h

ULIMIT := 120

# Include the include makefile.
include Makefile.inc
CC += -I../../../include -ffixed-r28

# Empty out the OSLIB variable.
OSLIB=
