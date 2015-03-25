/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_PRELIM_H
#define QURT_PRELIM_H 1

/* EJP: things that we'll use in header files */
#include <h2.h>

#define UNSUPPORTED while (1) { /* SPIN */ }
#define R_UNSUPPORTED do { while (1) { /* SPIN */ }; return 1; } while (0)

#endif
