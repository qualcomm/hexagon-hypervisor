/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

//#include "qurt_mutex.h"
#include "h2.h"
#include "qurt.h"
#include <q6protos.h>

/* QURT TLS reservation */
struct QURT_tls_reserve QURT_tls_reserve = {{0}, {0}};

/* Mutex to protect QURT_tls_reserve data structure */
h2_mutex_t qurt_tls_lock = H2_MUTEX_T_INIT;
 
/**
 * Description: Allocates a TLS key
 *
 * @param   *key  - the allocated key value
 *          *destructor - callback function when a thread exits.
 * @return  0      - SUCCESS
 *          EAGAIN - No free TLS key available
 */
int qurt_tls_key_create (int *key, void (*destructor)(void *))
{
   int index, bit_pos;

   h2_mutex_lock (&qurt_tls_lock);

   for (index = 0; index < QURT_MAX_TLS_INDEX; index++) {
      bit_pos = Q6_R_ct1_R (QURT_tls_reserve.tls_bitmask[index]);
      if (bit_pos < 32)
         break;
   }
   if (index == QURT_MAX_TLS_INDEX) {
      h2_mutex_unlock (&qurt_tls_lock);
      return QURT_EVAL;
   }
   /* If MAX_QURT_TLS is not a multiple of 32, we can have the following
    * situation. */
   if (((index * 32) + bit_pos) > QURT_MAX_TLS) {
      h2_mutex_unlock (&qurt_tls_lock);
      return QURT_EVAL;
   }
   *key = (index * 32) + bit_pos;
   /* Mark the reservation */
   QURT_tls_reserve.tls_bitmask[index] =
         Q6_R_setbit_RR (QURT_tls_reserve.tls_bitmask[index], bit_pos);
   QURT_tls_reserve.destructor[*key] = destructor;
   h2_mutex_unlock (&qurt_tls_lock);

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
int qurt_tls_setspecific (int key, const void *value)
{
   struct QURT_ugp_ptr *pUgp;

   /* Is the key in range? */
   if (key >= QURT_MAX_TLS) {
      return QURT_EVAL;
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
void *qurt_tls_getspecific (int key)
{
   struct QURT_ugp_ptr *pUgp;

   /* Is the key in range? */
   if (key >= QURT_MAX_TLS) {
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
int qurt_tls_key_delete (int key)
{
   int index, bit_pos;

   /* Is the key in range? */
   if (key >= QURT_MAX_TLS) {
      return QURT_EINVALID;
   }

   index = key/32;
   bit_pos = key % 32;
   h2_mutex_lock (&qurt_tls_lock);
   if (!Q6_p_tstbit_RR (QURT_tls_reserve.tls_bitmask[index], bit_pos)) {
      h2_mutex_unlock (&qurt_tls_lock);
      return QURT_EVAL;
   }
   /* Clear the reservation */
   QURT_tls_reserve.tls_bitmask[index] =
         Q6_R_clrbit_RR (QURT_tls_reserve.tls_bitmask[index], bit_pos);
   QURT_tls_reserve.destructor[key] = 0;

   h2_mutex_unlock (&qurt_tls_lock);
   return 0;
}
