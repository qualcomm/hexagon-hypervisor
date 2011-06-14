/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <blast_tls.h>
#include <blast_anysignal.h>

/* FIXME: hackage */
unsigned int blast_get_my_anysignal()
{
   struct BLAST_ugp_ptr *pUgp;

   blast_get_my_utcb(pUgp);
   return (unsigned int)(&(pUgp->utcb.anysignal));
}
