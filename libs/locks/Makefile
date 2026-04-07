
# Picolibc support: if PICOLIBC=1, use --target=hexagon-h2-picolibc
ifeq ($(PICOLIBC),1)
TARGET_FLAG = --target=hexagon-h2-picolibc
else
TARGET_FLAG =
endif

H2_INCLUDES   := $(patsubst %,-I%,$(shell find ../h2/ -mindepth 1 -maxdepth 1 -type d))
POSIX_INCLUDES:= $(patsubst %,-I%,$(shell find ../posix/pthread/ -mindepth 1 -maxdepth 1 -type d))

REQUIRED_INCLUDES = $(H2_INCLUDES) $(POSIX_INCLUDES)

CFLAGS = $(TARGET_FLAG) -mv$(TOOLARCH) -DARCHV=$(ARCHV) $(REQUIRED_INCLUDES) -Wall -Wno-builtin-requires-header -Werror -g $(OPTIMIZE) -G0
ASFLAGS = $(TARGET_FLAG) -mv$(TOOLARCH) -DARCHV=$(ARCHV) $(REQUIRED_INCLUDES) -Wall

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)
ifeq ($(EN_LLVM_COV), 1)
CFLAGS += ${LLVM_COV_CFLAGS}
endif

CFLAGS += $(LIB_CFLAGS) $(H2_EXTRA_CFLAGS)

ASM_FILES = 
C_FILES = $(wildcard src/*.c)

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: liblocks.a 

clean: 
	rm -f liblocks.a $(O_FILES)

liblocks.a: $(O_FILES)
	$(AR) cru $@ $(sort $(O_FILES))

