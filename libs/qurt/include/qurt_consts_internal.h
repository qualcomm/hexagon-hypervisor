/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_CONSTS_INTERNAL_H
#define QURT_CONSTS_INTERNAL_H 

/** 
 Maximum no. of QURT TLSs 
*/
#define QURT_MAX_TLS              64 
#define QURT_MAX_TLS_INDEX (((QURT_MAX_TLS - 1)/32) + 1)
#define QURT_MAX_TLS_DESTRUCTOR_ITERATIONS 1

#define QURT_FLAG_FALSE  0
#define QURT_FLAG_TRUE   1

#endif /* QURT_CONSTS_INTERNAL_H */

