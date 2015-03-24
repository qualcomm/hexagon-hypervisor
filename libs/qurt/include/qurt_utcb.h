/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_UTCB_H
#define QURT_UTCB_H

#include "qurt_anysignal.h"
#include "qurt_thread.h"

/* 
 * EJP: Not sure if we need all this
 * We need threadid to be the zeroth thing.
 * Other than that, it's just whatever information we need to make qurt work.
 * I see that (as far as I can tell) the per-thread signals for interrupt delivery are gone?
 */

/**
 *  Structure definition for QURT User TCB (UTCB) 
 *  
 *  The content in UTCB can be accessed in user mode
 * 
 */

typedef struct QURT_utcb 
{
   /* SW Thread ID */
   unsigned int thread_id;

   /* Entry point of higher layer */
   void (*entrypoint)(void *);

   /* Argument to be to higher layer entry point */
   void *arg;

   /* process startup flags passed in by the user when creating the process */
   int startup_flags;

   /* qdi_info has to be 8 bytes aligned */

   struct {
      void *qurt_qdi_table0_ptr;           /* Offset 80, points at handle table #0 */
      int   qurt_qdi_table0_max;           /* Offset 84, max index in handle table #0 */
      void *qurt_qdi_outertable_ptr;       /* Offset 88, points at table of handle tables */
      int   qurt_qdi_outertable_max;       /* Offset 92, max index in table of handle tables */
      int   qurt_qdi_localhandle_max;      /* Offset 96, max handle which dispatches locally */
   } qdi_info __attribute__((aligned(8)));

   short asid;
   short pid;

      /* Thread attributes */
   qurt_thread_attr_t attr;

} QURT_utcb_t;

/**
 * Get Qurt UTCB pointer
 *
 */
#define qurt_get_my_utcb(pUgp)        __asm__ __volatile__ ( " %0 = ugp " :"=r"(pUgp) ) ; 

void qurt_thread_set_ugp(void *);

#endif /* QURT_UTCB_H */

