/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_FATAL_H
#define H2_FATAL_H 1

/*
 * QuRT compatibility requires a multiphase 
 * fatal error handling mechanism.
 */

#define H2_FATAL_STMODE     0x1
#define H2_FATAL_CLEAN      0x2
#define H2_FATAL_OFF        0x4
#define H2_FATAL_ALL	    (~0)

void h2_fatal_crash(unsigned int flags);

#endif

/** @} */
