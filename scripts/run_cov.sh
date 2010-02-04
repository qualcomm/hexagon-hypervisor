#!/bin/sh
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


make -f scripts/Makefile.coverage libs $*

make -f scripts/Makefile.coverage all $*
touch test.cov_fns
make -f scripts/Makefile.coverage cov.txt $*
