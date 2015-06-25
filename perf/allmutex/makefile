###############################################################################

EXEC   := allmutex
LD      = hexagon-gcc
AS      = hexagon-gcc
RUN     = hexagon-sim
RM      = rm -f
SIZE_TOOL = /prj/dsp/qdsp6/arch/scripts/section_size.py
ULIMIT = ulimit -t 1300

CLDFLAGS   := -moslib=h2 -lqdsp -lh2kernel -lh2


Q6VERSION := v4
CC     = hexagon-gcc
ifneq (,${DEBUG})
CFLAGS += -O0 -g -DDEBUG
else
CFLAGS += -O3
endif
CFLAGS += $(CCFLAGS) -m$(Q6VERSION)
CFLAGS += -I$(Q6_RTOS_INSTALL)/include
LDFLAGS += -m$(Q6VERSION) $(CLDFLAGS) -L$(Q6_RTOS_INSTALL)/lib
SIMF = --timing --quiet

PKW_H2:=$(subst Q6_RTOS_INSTALL=,,$(subst %,,$(shell PKW_h2=$(Q6VERSION)_latest pkw --env h2 --lang make|grep Q6_RTOS_INSTALL)))

ifeq (,$(Q6_RTOS_INSTALL))
ifeq (,$(PKW_H2))
Q6_RTOS_INSTALL = ./h2/$(Q6VERSION)
else
Q6_RTOS_INSTALL = $(PKW_H2)
endif
endif

.PHONY : all objs clean cleanall rebuild

all : $(EXEC)

SRC = allmutex.c
OBJ = $(SRC:.c=.o)


# Rules for producing the objects.
#---------------------------------------------------
%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o : %.S
	$(AS) -c $(ASMFLAGS) $< -o $@

# Rules for producing the executable.
#----------------------------------------------
$(EXEC) : $(OBJ)
	$(LD) -o $(EXEC) $(OBJ) $(LDFLAGS)

sim : $(EXEC)
	$(ULIMIT) && time $(RUN) $(SIMF) -- $(EXEC)

size: $(EXEC)
	hexagon-objdump -h $(OBJ) | $(SIZE_TOOL) text > size && \
	cat size;

clean :
	@$(RM) $(OBJ) $(EXEC) stats.txt result.yuv size


cleanall: clean
	@$(RM) $(EXEC) $(EXEC).exe



### End of the Makefile ##  Suggestions are welcome  ## All rights reserved ###

###############################################################################
