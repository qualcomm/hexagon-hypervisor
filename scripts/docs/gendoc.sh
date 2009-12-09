#!/bin/sh
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


# We want the local python (2.5.2) instead of the QCT /pkg one.

export PATH=/usr/local/bin:/usr2/rkuo/local/all/python2.5:$PATH
export PYTHONPATH=/usr2/rkuo/local/all/python2.5

make -f scripts/docs/Makefile.sphinx prepare
make -f scripts/docs/Makefile.sphinx clean html
