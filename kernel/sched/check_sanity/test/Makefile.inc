#  Need to define how to get back to the main H2 dir
H2DIR=${CURDIR}/${UPDIR}../../../..

#  All tests for this unit actually use the oslib thing
OSLIB=-moslib=h2

#  Everything else defined here
include ${H2DIR}/scripts/Makefile.inc.test

