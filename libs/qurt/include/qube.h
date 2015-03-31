/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QUBE_H
#define QUBE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <qurt.h>
#include <qmutex.h>

/* Define Error codes as QuRT error codes preceed with QURT_ */
#ifndef EOK
#define EOK                             QURT_EOK
#endif /* EOK */
#ifndef EVAL
#define EVAL                            QURT_EVAL
#endif /* EVAL */
#ifndef EMEM
#define EMEM                            QURT_EMEM
#endif /* EMEM */
#ifndef EINVALID
#define EINVALID                        QURT_EINVALID
#endif /* EINVALID */

static inline void qube_init() { qurt_init(); }

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* QUBE_H */
