#  Need to define how to get back to the main H2 dir
H2DIR=${UPDIR}../../../..

#  Everything else defined here
include ${H2DIR}/scripts/Makefile.inc.test

# FIXME: remove this and use devsim when devsim has qtimer
RUN := hexagon-sim
SIMF := $(subst, --magic_angel,, $(SIMF))

SIMF += --cosim_file timer_v${ARCHV}.cfg

