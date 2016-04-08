/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_LIBKERNEL_H
#define H2_LIBKERNEL_H 1

/** @addtogroup h2 
@{ */

/**
If the kernel should be linked in, this function causes the linker to pull in the 
H2 kernel library.  It does nothing else.
@returns None
@dependencies None
*/
void h2_init();

#define h2_init(...) h2_init()

/** @} */

#endif

