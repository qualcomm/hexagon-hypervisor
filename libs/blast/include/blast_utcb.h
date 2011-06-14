/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_UTCB_H
#define BLAST_UTCB_H 1

#include <blast_anysignal.h>

typedef struct BLAST_utcb {
   /* SW Thread ID */
   unsigned int thread_id;

   /* Entry point of higher layer */
   void (*entrypoint)(void *);

   /* Argument to be to higher layer entry point */
   void *arg;

   /* Thread Name */
   long long int thread_name0;
   long long int thread_name1;

   /* Stack Pointer */
   unsigned int *stack_ptr;

   /* Stack Size */
   unsigned int stack_size;

   /* AnySignal */
   blast_anysignal_t anysignal;

} BLAST_utcb_t;

#endif /* BLAST_UTCB_H */
