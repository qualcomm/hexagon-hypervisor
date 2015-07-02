#  Need to define how to get back to the main H2 dir
H2DIR=${UPDIR}../../../../..

#  Everything else defined here

BOOTER_ARGS := --expect_status 0xaaafa7a1
include ${H2DIR}/scripts/Makefile.inc.test


