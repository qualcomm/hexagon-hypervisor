/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

//#include "blast_mutex.h"
#include "h2.h"
#include "blast_tls.h"
#include <q6protos.h>
#include "blast_error.h"

/* BLAST TLS reservation */
struct BLAST_tls_reserve BLAST_tls_reserve = {{0}, {0}};

/* Mutex to protect BLAST_tls_reserve data structure */
h2_mutex_t blast_tls_lock = H2_MUTEX_T_INIT;
 
/**
 * Description: Allocates a TLS key
 *
 * @param   *key  - the allocated key value
 *          *destructor - callback function when a thread exits.
 * @return  0      - SUCCESS
 *          EAGAIN - No free TLS key available
 */
int blast_tls_key_create (int *key, void (*destructor)(void *))
{
   int index, bit_pos;

   h2_mutex_lock (&blast_tls_lock);

   for (index = 0; index < MAX_BLAST_TLS_INDEX; index++) {
      bit_pos = Q6_R_ct1_R (BLAST_tls_reserve.tls_bitmask[index]);
      if (bit_pos < 32)
         break;
   }
   if (index == MAX_BLAST_TLS_INDEX) {
      h2_mutex_unlock (&blast_tls_lock);
      return BLAST_EAGAIN;
   }
   /* If MAX_BLAST_TLS is not a multiple of 32, we can have the following
    * situation. */
   if (((index * 32) + bit_pos) > MAX_BLAST_TLS) {
      h2_mutex_unlock (&blast_tls_lock);
      return BLAST_EAGAIN;
   }
   *key = (index * 32) + bit_pos;
   /* Mark the reservation */
   BLAST_tls_reserve.tls_bitmask[index] =
         Q6_R_setbit_RR (BLAST_tls_reserve.tls_bitmask[index], bit_pos);
   BLAST_tls_reserve.destructor[*key] = destructor;
   h2_mutex_unlock (&blast_tls_lock);

   return 0;
}

/**
 * Description: Sets a value in TLS indexed by key
 *
 * @param   key   - Operated TLS key
 *          value - data to save at the specified key location.
 * @return  0        - SUCCESS
 *          EINVALID - Not a valid key
 */
int blast_tls_setspecific (int key, const void *value)
{
   struct BLAST_ugp_ptr *pUgp;

   /* Is the key in range? */
   if (key >= MAX_BLAST_TLS) {
      return EINVALID;
   }
   asm ("	%0 = ugp \n"
         : "=r"(pUgp));

   pUgp->tls[key] = (void *)value;

   return 0;
}

/**
 * Description: Gets a value in TLS indexed by key
 *
 * @param   key   - Operated TLS key
 * @return  value saved in TLS indexed by key
 */
void *blast_tls_getspecific (int key)
{
   struct BLAST_ugp_ptr *pUgp;

   /* Is the key in range? */
   if (key >= MAX_BLAST_TLS) {
      return 0;
   }
   asm ("	%0 = ugp \n"
         : "=r"(pUgp));

   return (pUgp->tls[key]);
}

/**
 * Description: Delete a TLS key
 *
 * @param   key  - Free the allocated key
 * @return  0      - SUCCESS
 *          ENOENT - The key is not already free
 */
int blast_tls_key_delete (int key)
{
   int index, bit_pos;

   /* Is the key in range? */
   if (key >= MAX_BLAST_TLS) {
      return EINVALID;
   }

   index = key/32;
   bit_pos = key % 32;
   h2_mutex_lock (&blast_tls_lock);
   if (!Q6_p_tstbit_RR (BLAST_tls_reserve.tls_bitmask[index], bit_pos)) {
      h2_mutex_unlock (&blast_tls_lock);
      return BLAST_ENOENT;
   }
   /* Clear the reservation */
   BLAST_tls_reserve.tls_bitmask[index] =
         Q6_R_clrbit_RR (BLAST_tls_reserve.tls_bitmask[index], bit_pos);
   BLAST_tls_reserve.destructor[key] = 0;

   h2_mutex_unlock (&blast_tls_lock);
   return 0;
}
