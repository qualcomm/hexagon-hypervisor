#!/bin/sh
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


TAG=$1
shift
TYPE=$1
shift

make -f scripts/Makefile.coverage libs $*

make -f scripts/Makefile.coverage all $*
touch test.cov_fns
make -f scripts/Makefile.coverage cov.txt $*
make -f scripts/Makefile.coverage report.html $*

mkdir -p ~rkuo/public_html/h2/$TAG 
cp report.html ~rkuo/public_html/h2/$TAG/$TYPE.html

make -f scripts/Makefile.coverage check
if [ $? ne 0 ]; then
   touch ~rkuo/public_html/h2/$TAG/$TYPE.html.failed
fi

