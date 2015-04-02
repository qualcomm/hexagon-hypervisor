/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurtos_internal.h>

 /* NOTE:
 * -----
 * The qurt_system_sclk_* APIs are implemented as stubs using Qurt timers. This
 * approach is chosen for quick turn around of the feature.
 *
 * These APIs should be low level APIs and the the Qurt timer should use these
 * APIs instead of using HW directly.
 */

/* SCLK driver is not used by applications other than QuRT Island. PI mutex
 * is not supported by QuRT Island mode */
#undef CONFIG_PRIORITY_INHERITANCE

#define QURT_TIMER_INTERRUPT 35

/* extern struct qurtos_thread_info timerIST_thread_info; */
/* extern struct QURT_ugp_ptr timerIST_ugp_area; */

// extern qurt_mutex_t qurt_sclk_mutex;

/* Status of the clients actively using Timers */
// extern unsigned int qurt_sclk_valid;

/* Timer HW status */
//extern struct qurt_sclk_hw_status qurt_sclk_hw_status;

/* Client information */
//extern qurt_sclk_client_t qurt_sclk_client[QURT_SCLK_CLIENTS_MAX];

/* extern void qurt_sclk_int_handler (qurt_anysignal_t *int_signal, unsigned int int_mask, unsigned int int_num); */

/* extern int qurtos_sclk_invocation(int client_handle, qurt_qdi_obj_t *obj, int method, */
/*                                   qurt_qdi_arg_t a1, qurt_qdi_arg_t a2, qurt_qdi_arg_t a3, */
/*                                   qurt_qdi_arg_t a4, qurt_qdi_arg_t a5, qurt_qdi_arg_t a6, */
/*                                   qurt_qdi_arg_t a7, qurt_qdi_arg_t a8, qurt_qdi_arg_t a9); */

/* extern void qurt_timer_IST (void *arg); */

//#ifdef INCLUDE_ISLAND_CONTENTS

/* struct qurtos_thread_info timerIST_thread_info; */
/* struct QURT_ugp_ptr timerIST_ugp_area; */

qurt_mutex_t qurt_sclk_mutex;

/* Status of the clients actively using Timers */
unsigned int qurt_sclk_valid;

/* Timer HW status */
struct qurt_sclk_hw_status qurt_sclk_hw_status;

/* Client information */
qurt_sclk_client_t qurt_sclk_client[QURT_SCLK_CLIENTS_MAX] = {{0}};

int qurtos_sysclock_get_hw_ticks (unsigned long long *ticks)
{
    int result = 0;
    if (NULL == ticks)
    {
//        qurtos_printf ("Client ticks is NULL\n" );
        result = QURT_EINVALID;
    }
    else
    {
        *ticks = hw_timer_curr_timetick ();
        result = QURT_EOK;
    }
    return result;
}

static void qurt_sclk_reprogram_hw (void)
{
   int id;
   unsigned int mask;
   unsigned int order;
   unsigned long long hw_ticks, match_value, min_match_value, count_lapse;

do_over:
   qurtos_sysclock_get_hw_ticks (&hw_ticks);

   /* The default match value is the one that can produce longest timer */
   min_match_value = QURT_SYSCLOCK_MAX_DURATION_TICKS;

   mask = qurt_sclk_valid;
   while (mask) {
      id = Q6_R_ct0_R (mask);

      match_value = qurt_sclk_client[id].match_value;

      if (((hw_ticks + (unsigned int)QURT_SYSCLOCK_ERROR_MARGIN ))
           >= (match_value)) {
         /* Timer expired */
				// qurt_anysignal_set (qurt_sclk_client[id].signal, qurt_sclk_client[id].signal_mask);
				recheck_timers = 1;

         qurt_sclk_valid = qurt_sclk_valid & ~(1 << id);
      }
      else {
         if ((min_match_value - hw_ticks) > (qurt_sclk_client[id].match_value - hw_ticks)) {

            min_match_value = qurt_sclk_client[id].match_value;
         }
      }

      mask = mask & ~(1<< id);
   }

   if (min_match_value != qurt_sclk_hw_status.match_value) 
   {
      /* Set writing order */
      if( min_match_value > qurt_sclk_hw_status.match_value ) {
          order = 0 ; 
      }
      else{
          order = 1 ; 
      }

      /* We need to write new match value to HW */
      qurt_sclk_hw_status.match_value = min_match_value;
      qurt_sclk_hw_status.count_value = hw_ticks;
      count_lapse = hw_timer_prg_next_interrupt (min_match_value, hw_ticks, order);
      if ((count_lapse + (unsigned int)QURT_SYSCLOCK_ERROR_MARGIN)
          >= min_match_value - hw_ticks) {
         /* Timer programmed to HW is historic */
         goto do_over;
      }
   }
}

/* void qurt_sclk_int_handler (qurt_anysignal_t *int_signal, unsigned int int_mask, unsigned int int_num) */
/* { */
/*    int ret; */

/*    do */
/*    { */
/*       ret = qurt_anysignal_wait(int_signal, int_mask); */
/*       qurt_anysignal_clear(int_signal, ret); */

/*        //Processing timer interrupt in IST context */
/* #ifdef CONFIG_PRIORITY_INHERITANCE */
/*       qurt_pimutex_lock(&qurt_sclk_mutex); */
/* #else */
/*       qurt_rmutex_lock(&qurt_sclk_mutex); */
/* #endif */

/*       qurt_sclk_reprogram_hw(); */

/* #ifdef CONFIG_PRIORITY_INHERITANCE */
/*       qurt_pimutex_unlock(&qurt_sclk_mutex); */
/* #else */
/*       qurt_rmutex_unlock(&qurt_sclk_mutex); */
/* #endif */

/*       (void)qurt_interrupt_acknowledge(int_num); */

/*    }while(1); */
/* } */

/* void qurt_sclk_interrupt (void) */
/* { */
/* #ifdef CONFIG_PRIORITY_INHERITANCE */
/*    qurt_pimutex_lock(&qurt_sclk_mutex); */
/* #else */
/*    qurt_rmutex_lock(&qurt_sclk_mutex); */
/* #endif */

/*    /\* Expire timers and re-program HW *\/ */
/*    qurt_sclk_reprogram_hw (); */

/* #ifdef CONFIG_PRIORITY_INHERITANCE */
/*    qurt_pimutex_unlock(&qurt_sclk_mutex); */
/* #else */
/*    qurt_rmutex_unlock(&qurt_sclk_mutex); */
/* #endif */
/* } */

/* int qurt_sysclock_register (qurt_anysignal_t *signal, unsigned int signal_mask) */
/* { */
/*    int i; */
/*    int rc = QURT_EFATAL; */
   
/* #ifdef CONFIG_PRIORITY_INHERITANCE */
/*    qurt_pimutex_lock(&qurt_sclk_mutex); */
/* #else */
/*    qurt_rmutex_lock(&qurt_sclk_mutex); */
/* #endif */

/*    for (i = 0; i < QURT_SCLK_CLIENTS_MAX; i++) { */
/*       if (!qurt_sclk_client[i].allocated) { */
/*          qurt_sclk_client[i].allocated = 1; */
/*          qurt_sclk_client[i].signal = signal; */
/*          qurt_sclk_client[i].signal_mask = signal_mask; */
/*          rc = i; */
/*          break; */
/*       } */
/*    } */

/* #ifdef CONFIG_PRIORITY_INHERITANCE */
/*    qurt_pimutex_unlock(&qurt_sclk_mutex); */
/* #else */
/*    qurt_rmutex_unlock(&qurt_sclk_mutex); */
/* #endif */

/*    return rc; */
/* } */

unsigned long long qurt_sysclock_alarm_create (int id, unsigned long long ref_count, unsigned long long match_value)
{
   unsigned long long hw_ticks;
   bool hw_change = true;
   
#ifdef CONFIG_PRIORITY_INHERITANCE
   qurt_pimutex_lock(&qurt_sclk_mutex);
#else
   qurt_rmutex_lock(&qurt_sclk_mutex);
#endif

   qurtos_sysclock_get_hw_ticks (&hw_ticks);

   if (((hw_ticks + QURT_SYSCLOCK_ERROR_MARGIN))
       >= (match_value))  
    {
      /* Historic timer */
			//      qurt_anysignal_set (qurt_sclk_client[id].signal, qurt_sclk_client[id].signal_mask);
			recheck_timers = 1;

      if (!(qurt_sclk_valid & (1 << id))) {
          /* No need to reprogram HW, because we were not valid when HW was
             last programmed */
          hw_change = false;
   }
      qurt_sclk_valid = qurt_sclk_valid & ~(1 << id);
   }
   else {
      qurt_sclk_valid = qurt_sclk_valid | (1 << id);
      qurt_sclk_client[id].match_value = match_value;
   }

   if (hw_change == true)
      qurt_sclk_reprogram_hw ();

#ifdef CONFIG_PRIORITY_INHERITANCE
   qurt_pimutex_unlock(&qurt_sclk_mutex);
#else
   qurt_rmutex_unlock(&qurt_sclk_mutex);
#endif

        return match_value;
}

int qurt_sysclock_timer_create (int id, unsigned long long duration)
{
   int result;
   unsigned long long hw_ticks, match_value;

   result = qurtos_sysclock_get_hw_ticks (&hw_ticks);
   if (result != QURT_EOK) {
     return QURT_EINVALID;
   }

   match_value = QURT_SYSCLOCK_TIMETICK_FROM_US(duration) + hw_ticks;

   (void)qurt_sysclock_alarm_create (id, hw_ticks, match_value);

   return QURT_EOK;
}

unsigned long long qurt_sysclock_get_expiry (void)
{
   unsigned long long expiry;

   expiry = hw_timer_match_val () - hw_timer_curr_timetick ();

   return expiry;
}

/* int qurtos_sclk_invocation(int client_handle, qurt_qdi_obj_t *obj, int method, */
/*                                   qurt_qdi_arg_t a1, qurt_qdi_arg_t a2, qurt_qdi_arg_t a3, */
/*                                   qurt_qdi_arg_t a4, qurt_qdi_arg_t a5, qurt_qdi_arg_t a6, */
/*                                   qurt_qdi_arg_t a7, qurt_qdi_arg_t a8, qurt_qdi_arg_t a9) */
/* { */
/*    switch (method) { */
/*    case QDI_OS_SCLK_GET_TICKS: */
/*       return qurtos_sysclock_get_hw_ticks(a1.ptr); */
/*    default: */
/*       return qurt_qdi_method_default(client_handle, obj, method, */
/*                                      a1, a2, a3, a4, a5, a6, a7, a8, a9); */
/*    } */
/* } */

//#endif /* INCLUDE_ISLAND_CONTENTS */

// #ifdef INCLUDE_MAIN_CONTENTS

extern qurt_mutex_t qurt_timer_lock;

void qurt_timer_IST (void *arg)
{

	qurtos_printf("qurt_timer_IST started\n");

	while (1) {
		h2_intwait(QURT_TIMER_INTERRUPT);
qurtos_printf("Timer interrupt\n");

#ifdef CONFIG_PRIORITY_INHERITANCE
   qurt_pimutex_lock(&qurt_sclk_mutex);
#else
   qurt_rmutex_lock(&qurt_sclk_mutex);
#endif
		qurt_sclk_reprogram_hw();

#ifdef CONFIG_PRIORITY_INHERITANCE
   qurt_pimutex_unlock(&qurt_sclk_mutex);
#else
   qurt_rmutex_unlock(&qurt_sclk_mutex);
#endif

		qurtos_timer_lock (&qurt_timer_lock);
    qurt_timer_lib_process_active_timers();
    qurtos_timer_unlock (&qurt_timer_lock);
	}

/*    qurt_anysignal_t int_signal; */
/*    unsigned int int_mask = 1 ; */

/*    qurt_anysignal_init(&int_signal); */

/*    if( qurt_interrupt_register (QURT_timer_intno, &int_signal, int_mask) != QURT_EOK) */
/*    { */
/*       qurtos_printf("Failed to register QURT timer interrupt\n"); */
/*       qurtos_assert(0); */
/*    } */

/*    qurt_sclk_int_handler (&int_signal, int_mask, QURT_timer_intno); */

/*    /\* Should never reach here *\/ */
/*    qurtos_assert(0); */
}

#define IST_STACK_SIZE 1024
unsigned long long qurt_timer_IST_stack[IST_STACK_SIZE];

void qurt_timer_IST_init (void)
{
	h2_thread_create(qurt_timer_IST, &qurt_timer_IST_stack[IST_STACK_SIZE - 1], 0, 0);

/*     void *stack; */
/*     struct qurtos_thread_info *info = &timerIST_thread_info; */
/*     struct QURT_ugp_ptr *ugp = &timerIST_ugp_area;  */
/*     QURT_utcb_t *current_utcb; */
   
/*     stack = qurtos_island_malloc(QURT_SCLK_STACK_SIZE); */
/*     if (NULL == stack) */
/*     { */
/*         qurtos_printf("Memory allocation failed\n"); */
/*         qurtos_assert(0); */
/*     } */

/*     qurt_thread_attr_init(&ugp->utcb.attr); */
/*     qurt_thread_attr_set_name(&ugp->utcb.attr, "qtimerIST"); */
/*     qurt_thread_attr_set_stack_size(&ugp->utcb.attr, QURT_SCLK_STACK_SIZE); */
/*     qurt_thread_attr_set_stack_addr(&ugp->utcb.attr, stack); */
/*     qurt_thread_attr_set_priority(&ugp->utcb.attr, (unsigned short)QURT_timerIST_priority); */
/*     qurt_thread_attr_set_timetest_id (&ugp->utcb.attr, (unsigned short)(-3)); */
/*     qurt_thread_attr_set_tcb_partition(&ugp->utcb.attr, QURT_timerIST_tcb_partition); */

/*     qurt_get_my_utcb(current_utcb); */

/*     ugp->utcb.qdi_info = current_utcb->qdi_info; */
/*     ugp->utcb.asid = 0; */
/*     ugp->utcb.entrypoint = qurt_timer_IST; */

/*     qurt_thread_osam_setup(&ugp->utcb.attr); */

/*     qurtos_thread_create(QDI_HANDLE_LOCAL_CLIENT, ugp, qurt_timer_IST, info); */

/*    qurtos_printf ("QURT Timer IST started\n"); */
}

void qurt_sysclock_init (void)
{
   int i;

   qurt_sclk_valid = 0;
#ifdef CONFIG_PRIORITY_INHERITANCE
   qurt_pimutex_init(&qurt_sclk_mutex);
#else
   qurt_rmutex_init(&qurt_sclk_mutex);
#endif
   for (i = 0; i < QURT_SCLK_CLIENTS_MAX; i++) {
      qurt_sclk_client[i].allocated = 0;
      qurt_sclk_client[i].signal = 0;
      qurt_sclk_client[i].signal_mask = 0;
      qurt_sclk_client[i].match_value = 0;
      qurt_sclk_client[i].reference = 0;
   } 

   /* // initialize qurt timer variables */
   /* if (qurt_timer_base == -1) { */
   /*    qurt_timer_base = QDSP6_QURT_TIMER_BASE; */
   /*    qurt_timer_int_num = QURT_timer_intno; */
   /* } */

   qurt_timer_IST_init ();

#ifndef QURT_TIMER_ROLLOVER_TEST
   qurt_sclk_hw_status.count_value = 0;
   qurt_sclk_hw_status.match_value = 0xffffffff;
#else /* QURT_TIMER_ROLLOVER_TEST */
    /* Record the count value to take care of roll over upon expiry */
   qurt_sclk_hw_status.count_value = QURT_TIMER_TEST_COUNT;
   qurt_sclk_hw_status.match_value = QURT_TIMER_TEST_COUNT + 0xffffffff;
#endif /* 0 */

    /* Disable hardware timer, clear count & match value to prevent 
       an unexpected interrupt. */
	 hw_timer_init (qurt_sclk_hw_status.match_value);
}

/* void qurtos_sclk_generic_init(void) */
/* { */
/*    static const struct qurtos_generic_method_handler list[] = { */
/*       { QDI_OS_SCLK_GET_TICKS, QURTOS_GENERIC_FN(qurtos_sclk_invocation)}, */
/*    }; */

/*    qurtos_qdi_generic_register_methods(list, QURTOS_ASIZE(list)); */
/* } */

// #endif /* INCLUDE_MAIN_CONTENTS */
