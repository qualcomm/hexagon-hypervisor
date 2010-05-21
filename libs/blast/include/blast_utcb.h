/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_UTCB_H
#define BLAST_UTCB_H

/* BLAST user TCB */
typedef struct BLAST_utcb {
   /* Entry point of higher layer */
   void (*entrypoint)(void *);
   /* Argument to be to higher layer entry point */
   void *arg;
} BLAST_utcb_t;

#endif /* BLAST_UTCB_H */
