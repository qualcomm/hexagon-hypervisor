/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_DEFS_INTERNAL_H
#define QURT_DEFS_INTERNAL_H 

//#include "qurt_utcb.h"
#include "qurt_consts_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* /\** */
/*  QuRT TLS reservation  */
/*  *\/ */
/* struct QURT_tls_reserve { */
/*    unsigned int tls_bitmask[QURT_MAX_TLS_INDEX]; */
/*    void (*destructor [QURT_MAX_TLS]) (void *); */
/* }; */

/* struct QURT_ugp_ptr { */
/*    /\* Define QURT_UTCB here *\/ */
/*    QURT_utcb_t utcb; */
/*    void *tls[QURT_MAX_TLS]; */
/* }; */

/* /\** */
/*  QURT system process status change event */
/*  *\/ */
/* typedef struct _qurt_sysevent_process_t */
/* { */
/*     unsigned int asid; */
/*     unsigned int status; */
/*     unsigned int padding[4]; */
    
/* } qurt_sysevent_process_t; */

/* #define QURT_MAX_ASIDS 32 */

/* typedef struct { */
/*      char name[32]; */
/*      unsigned int pool_handle; */
/* } qurt_mem_pool_internal_t; */

/* /\*============================================================================= */
/*                         FUNCTIONS */
/* =============================================================================*\/ */
/* int qurt_power_control (int power, int tcxo_control); */
/* int qurt_apcr_control (int cmd); */
/* int qurt_tcm_control(int cmd, unsigned int in_arg); */

/* unsigned qurt_thread_get_gp(void); */
/* unsigned qurt_get_thread_id_in_tcb(void); */

/* void qurt_thread_osam_setup(struct _qurt_thread_attr *); */

/* int qurt_interrupt_set_island(unsigned int intno, unsigned int reset_flag ); */

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
#endif /* QURT_DEFS_INTERNAL_H */

