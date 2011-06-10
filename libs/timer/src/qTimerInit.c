/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qube.h>
#include "qTimerDefines.h"
#include "qTimerLibs.h"
#include "qtimer.h"

#define BLAST_TIMER_IST_BUF_COUNT     10
#define BLAST_TIMER_SIT_MAX_MSG_SIZE  8

#define BLAST_TIMETEST_TID_TIMERIST   -3
#define BLAST_TIMETEST_TID_TIMERSERV  -4

#define MAX_QTIMER_MSGS               100

unsigned int BLAST_timer_intno = 2;
unsigned int BLAST_timerIST_priority = 254;
unsigned int BLAST_timerIST_bitmask = 0xff;
unsigned int BLAST_timer_priority = 253;
unsigned int BLAST_timer_bitmask = 0xff;

blast_pipe_t *qtimer_serv_pipe;
int qtimer_init_done = 0;

extern void qTimerServer (void*);

static blast_sem_t qtimer_IST_started;

void qtimer_IST (void *arg)
{
   int ret;
   unsigned int int_num;
   qtimer_cmd_t isr_cmd;

   isr_cmd.cmd_type = QTIMER_IPC_INTERRUPT;
   qinterrupt_register (BLAST_timer_intno);

   blast_sem_up(&qtimer_IST_started);

   while (1) {
      ret = qinterrupt_receive (&int_num);
      if (ret == EOK) {
         /* Send message to Qtimer Server */
         blast_pipe_send (qtimer_serv_pipe, isr_cmd.raw);
      }
   }
}

void blast_timer_IST_init (void)
{
   qthread_attr_t qta;
   qthread_t qtid;

   // Initialize the synchronizing semaphore between qTimerServer & IST
   blast_sem_init_val(&qtimer_IST_started, 0);

   // Start Timer IST
   qthread_attr_init (&qta);
   qthread_attr_setname (&qta, "qtimerIST");
   qthread_attr_setpriority (&qta, BLAST_timerIST_priority);
   qthread_attr_sethwbitmask(&qta, BLAST_timerIST_bitmask);
   /* Set TIMETEST TID */
   qthread_attr_settid (&qta, BLAST_TIMETEST_TID_TIMERIST);

   qthread_create (&qtid, &qta);
   qthread_start (qtid, qtimer_IST, NULL);

   //Wait for Timer IST to finish registering interrupt
   blast_sem_down(&qtimer_IST_started);

   printf ("BLAST Timer IST started\n");
}

void qtimer_init (void)
{
   qthread_attr_t qta;
   qthread_t qtid;
   unsigned int ret;

   if (qtimer_init_done) {
      return;
   }

   qtimer_init_done = 1;

   qthread_attr_init (&qta);
   qthread_attr_setname (&qta, "BlastTimerServ");
   qthread_attr_setpriority (&qta, BLAST_timer_priority);
   qthread_attr_sethwbitmask(&qta, BLAST_timer_bitmask);
   /* Set TIMETEST TID */
   qthread_attr_settid (&qta, BLAST_TIMETEST_TID_TIMERSERV);

   if ( (ret=qthread_create (&qtid, &qta)) != EOK) {
        printf("Failed to create BLAST Timer Server\n");
        assert(0);
   }

   qtimer_serv_pipe = blast_pipe_alloc ((MAX_QTIMER_MSGS * 8) + sizeof (blast_pipe_data_t) + sizeof (blast_pipe_t));

   if ( (ret=qthread_start (qtid, qTimerServer, NULL)) != EOK) {
        printf("Failed to start BLAST Timer Server\n");
        assert(0);
   }

   printf ("Started BLAST Timer Server\n");
}

int qtimer_is_init (void)
{
   return qtimer_init_done;
}
