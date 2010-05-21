/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_ERROR_H
#define BLAST_ERROR_H

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
/**
 * Power Collapse failed due to ISDB enabled
 */
#define EPC_ISDB                        21
/**
 * Power Collapse failed in Single threaded mode check
 */
#define EPC_NOSTM                       22

#define BLAST_EAGAIN                          23
#define BLAST_ENOENT                          24

/* Please don't change following error code definition. They are used by kernel */
#define EINT                            101
#define ESIG                            102

/* Return -1 for FAILURE, thread ID for SUCCESS */
unsigned int blast_reg_error_handler (unsigned int *ip, unsigned int *sp,
                                      unsigned int *badva, unsigned int *cause);
int blast_exit (int error);

#endif /* BLAST_ERROR_H */
