#  Need to define how to get back to the main H2 dir
H2DIR=${UPDIR}../../../../..

#  Everything else defined here

BOOTER_ARGS := --expect_status 0xaaafa7a1

POST_RUN := grep 'DEBUG DUMP' test.log && perl -pi -e 's/VM 2 status 0xaaafa7a1/TEST PASSED/' test.log

include ${H2DIR}/scripts/Makefile.inc.test
