
INCLUDES = -I../h2/include -I../blast/include -I../qube/include -Iinclude

CC = qdsp6-gcc -mv$(ARCHV) $(INCLUDES) -Wall
AR = qdsp6-ar

CFLAGS = -g -O2 -G0

ifeq (v4, $(Q6VERSION))
CFLAGS += -Wa,--gdwarf2
endif

ASM_FILES = asm/qTimerHw_asm.S
C_FILES = src/blast_sclk.c src/qTimerClient.c src/qTimerInit.c src/qTimerLibs.c src/qTimerServer.c


O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

########### Rules #####################

all: libtimer.a

libtimer.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)

clean:
	rm -Rf libtimer.a $(O_FILES)

