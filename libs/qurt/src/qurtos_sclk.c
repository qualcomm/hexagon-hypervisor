/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurtos_internal.h>
#include <qurt.h>

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

h2_sem_t ist_sem;

int qurtos_sysclock_get_hw_ticks (unsigned long long *ticks)
{
    int result = 0;
    if (NULL == ticks)
    {
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

	if (min_match_value != qurt_sclk_hw_status.match_value) {

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

   if (hw_change == true) {
		 /* Wake up IST to reprogram the timer */
		 h2_sem_up(&ist_sem);
	 }

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

extern qurt_mutex_t qurt_timer_lock;

void qurt_timer_interrupt (void) {

	h2_sem_up(&ist_sem);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT, 0);
}

extern void qurt_timer_vectors (void);

void qurt_timer_IST (void *arg)
{

	h2_vmtrap_setvec(qurt_timer_vectors);
	h2_vmtrap_setie(1);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT, 0);

	while (1) {
		h2_sem_down(&ist_sem);

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
		qurt_timer_lib_process_active_timers();  // Always need to do this? I think so
		qurtos_timer_unlock (&qurt_timer_lock);
	}
}

#define IST_STACK_SIZE 1024
unsigned long long qurt_timer_IST_stack[IST_STACK_SIZE];

void qurt_timer_IST_init (void)
{
	
	h2_sem_init_val(&ist_sem, 0);

	qurt_thread_attr_t attr;
	qurt_thread_t tid;
	int ret;
	void *stack = qurt_timer_IST_stack;
	qurt_thread_attr_init (&attr);
	qurt_thread_attr_set_name (&attr, "QTimerIST");
	qurt_thread_attr_set_stack_size (&attr, IST_STACK_SIZE * sizeof(unsigned long long));
	qurt_thread_attr_set_stack_addr (&attr, stack);
	qurt_thread_attr_set_priority (&attr, 64);
	ret = qurt_thread_create (&tid, &attr, qurt_timer_IST, (void *)0);
	if (ret == -1) {
		qurt_printf(" failed to create thread \n");
		qurt_assert(0);
	}
	qurt_printf("QuRT Timer IST started\n");
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
	 hw_timer_init ();
}
