/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_TLS
#define BLAST_TLS
#include <blast_utcb.h>

/* Maximum no. of BLAST TLSs */
#define MAX_BLAST_TLS 64 
#define MAX_BLAST_TLS_INDEX (((MAX_BLAST_TLS - 1)/32) + 1)
#define MAX_BLAST_TLS_DESTRUCTOR_ITERATIONS 1

struct BLAST_tls_reserve {
   unsigned int tls_bitmask[MAX_BLAST_TLS_INDEX];
   void (*destructor [MAX_BLAST_TLS]) (void *);
};

struct BLAST_ugp_ptr {
   /* Define BLAST_UTCB here */
   BLAST_utcb_t utcb;
   struct BLAST_ugp_ptr *next;
   h2_mutex_t join_lock;
   h2_cond_t join_cond;
   int join_refcount; 
   enum { BLAST_JOIN_STATE_RUNNING, BLAST_JOIN_STATE_DONE } join_state;
   void *tls[MAX_BLAST_TLS];
};

int blast_tls_key_create (int *key, void (*destructor)(void *));
int blast_tls_setspecific (int key, const void *value);
void *blast_tls_getspecific (int key);
int blast_tls_key_delete (int key);

#endif /* BLAST_TLS */
