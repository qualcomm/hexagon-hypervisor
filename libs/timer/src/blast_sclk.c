/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qube.h"
#include "qtimer.h"
#include "qTimerDefines.h"
#include "blast_sclk.h"

 /* NOTE:
 * -----
 * The blast_system_sclk_* APIs are implemented as stubs using Qtimers. This
 * approach is chosen for quick turn around of the feature.
 *
 * These APIs should be low level APIs and the the Qtimer should use these
 * APIs instead of using HW directly.
 */

#define MIN_REARM_TICKS_FOR_QTIMER (QTIMETICK_ERROR_MARGIN + 1)

struct blast_sclk_client {
   char valid;
   blast_anysignal_t *signal;
   unsigned int signal_mask;
   qtimer_t qtimer;
};

struct blast_sclk_client blast_sclk_client[MAX_BLAST_SCLK_CLIENTS] = {0};

/* Mutex to protect the blast_sclk_client */
blast_mutex_t blast_system_lock;

void blast_system_sclk_init (void)
{
   blast_mutex_init (&blast_system_lock);
}

int blast_system_sclk_register (blast_anysignal_t *signal, unsigned int signal_mask)
{
   int i;

   blast_mutex_lock (&blast_system_lock);

   for (i = 0; i < MAX_BLAST_SCLK_CLIENTS; i++) {
      if (blast_sclk_client[i].valid == NULL)
         break;
   }
   if (i == MAX_BLAST_SCLK_CLIENTS) {
      blast_mutex_unlock (&blast_system_lock);
      return -1;
   }

   blast_sclk_client[i].valid = 1;
   blast_sclk_client[i].signal = signal;
   blast_sclk_client[i].signal_mask = signal_mask;

   blast_mutex_unlock (&blast_system_lock);

   return i;
}

int blast_system_sclk_alarm (int id, unsigned int ref_count, unsigned int match_value)
{
   qtimer_attr_t attr;
   unsigned int hw_ticks, duration;
   int rc;

   blast_mutex_lock (&blast_system_lock);

   if (blast_sclk_client[id].qtimer) {
      qtimer_delete (blast_sclk_client[id].qtimer);
   }
   qtimer_attr_init (&attr);
   hw_ticks = blast_system_sclk_attr_gethwticks ();
   if (((hw_ticks + (unsigned int)MIN_REARM_TICKS_FOR_QTIMER) - ref_count) > (match_value -ref_count))  
   {
      /* Historic timer */
      blast_anysignal_set (blast_sclk_client[id].signal, blast_sclk_client[id].signal_mask);
      blast_mutex_unlock (&blast_system_lock);
      //MSG_HIGH("Timer_Debug: Immediately setting the timer signal hw_ticks=%d, ref_count=%d,match_value=%d",hw_ticks,ref_count,match_value); 
      return match_value;
   }

   duration = match_value - (hw_ticks - ref_count) - ref_count;
   qtimer_attr_setduration (&attr, QTIMER_TIMETICK_TO_US(duration));
   rc = sclk_timer_create (&blast_sclk_client[id].qtimer, &attr, blast_sclk_client[id].signal, blast_sclk_client[id].signal_mask);
   if (rc != EOK) {
      printf ("Qtimer create failed, rc =  \n", rc);
   }

   blast_mutex_unlock (&blast_system_lock);
   return match_value;
}

int blast_system_sclk_timer (int id, unsigned int duration)
{
   qtimer_attr_t attr;
   int rc;

   blast_mutex_lock (&blast_system_lock);

   if (blast_sclk_client[id].qtimer) {
      qtimer_delete (blast_sclk_client[id].qtimer);
   }
   qtimer_attr_init (&attr);

   qtimer_attr_setduration (&attr, duration);
   rc = sclk_timer_create (&blast_sclk_client[id].qtimer, &attr, blast_sclk_client[id].signal, blast_sclk_client[id].signal_mask);
   if (rc != EOK) {
      printf ("Qtimer create failed, rc =  \n", rc);
   }

   blast_mutex_unlock (&blast_system_lock);

   return 0;
}
