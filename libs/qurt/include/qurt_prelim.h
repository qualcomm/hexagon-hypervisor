/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_PRELIM_H
#define QURT_PRELIM_H 1

/* EJP: things that we'll use in header files */
#include <h2.h>

#define UNSUP_PRINT h2_printf("UNSUPPORTED: %s: %s\n",__FILE__,__PRETTY_FUNCTION__)
#define UNSUPPORTED do { UNSUP_PRINT;  while (1) { /* SPIN */ } } while  (0)
#define R_UNSUPPORTED do { UNSUP_PRINT; while (1) { /* SPIN */ }; return 1; } while (0)

#endif
