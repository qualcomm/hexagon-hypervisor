/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_QUBE_H
#define H2_QUBE_H

#define EOK 0
#define EOK                             0
#define EVAL                            1
#define EMEM                            2
#define EINVALID                        4
#define ELEN                            5
#define EUNKNOWN                        6
#define ENO_MSGS                        7
#define EMAX_MSGS                       8
#define ENO_MSGQ                        9
#define EDUP_MSGQ                       10
#define EBLOCK                          11
#define EFAILED                         12
#define ENOTALLOWED                     13
#define EDUP_CLSID                      14
#define EBADPARM                        15
#define EINVALIDITEM                    16
#define EBADHANDLE                      17
#define ENO_IID                         18
#define EOBJ_ALIVE                      19
#define ENO_INTERRUPTS                  20

#include <qmemory.h>
#include <qmutex.h>
#include <qtypes.h>
#include <qmsgq.h>
//#include <qthread.h>

//  Not quite sure what this is, but smdl_target.c needs it
#define L2_INTERRUPT_NUM(l1_int_num, l2_int_num) (l1_int_num*32+l2_int_num)
#define QTHREAD_MAX_PRIORITY (252/8)
#define QTHREAD_NAME_LEN 16

static inline void qube_init(void)
{
}

#endif
