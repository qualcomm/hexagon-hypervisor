/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurtos_internal.h>

qurt_timers_type  timers = 
{
  /* Linked list of active timers */
  NULL,

  /* timer value programmed in hardware (set_value) */
  0x00FFFFFFFFFFFFFEULL,

  /* Initial HW count */
  0,

  /* Next expiry */
  0x00FFFFFFFFFFFFFFULL
};

qurt_timers_group_type timer_groups[QURT_TIMER_MAX_GROUPS];
int qurt_timer_sclk_id = 0;

/**
 * Initializes the Timer Subsytem
 * Note: This function is executed from server side.
 *
 * SIDE EFFECTS: May install an ISR for the 32-bit TCXO clock match register
 *
 * @param     None
 * @return    None
 */
void qurt_timer_lib_init ( void )
{
    int i;

    /* Initialize groups */
    for (i = 0; i < QURT_TIMER_MAX_GROUPS; i++) {
        timer_groups[i].list = NULL;
    }

    /* Initialize the list of active timers to contain no timers */
    timers.active_list = NULL;

    /* Initialize the timers */
    timers.set_expiry = QURT_TIMER_MAX_DURATION_TICKS;
    timers.match_value = QURT_TIMER_MAX_DURATION_TICKS;
    timers.count_value = 0;

    /* Create KxMutex object */
    // SLEEP_TIMER qmutex_create ( &alistMutex, QMUTEX_LOCAL );

} /* qurt_timer_lib_init */

/**
 * Return absolute current time since the hardware timer was enabled
 * Note: This function is executed from server side.
 *
 * @param     hw_count - pointer to save HW count
 * @return    Absolute current time
 */
qurt_timetick_type qurt_timer_lib_current_time (qurt_timetick_word_t *hw_count)
{
    qurt_timetick_word_t cur_count;
    /* Variable to Store the current time */
    qurt_timetick_type now;

    /* Calculate current time */
    cur_count = hw_timer_curr_timetick ();
    now = cur_count;

    /* Set the HW count value if the caller is interested */
    if (hw_count) {
        *hw_count = cur_count;
    }

    return now;
    
} /* qurt_timer_lib_current_time */

/**
 * Inserts a timer into the sorted list of timers.
 * Timers are sorted according to increasing expiration times
 * Note: Must be called from mutex lock.
 *
 * @param timer   Timer to be inserted into list of active timers
 * @return        None
 */
static void qurt_timer_lib_insert ( qurt_timer_client_ptr timer )
{
    /* Pointer to a pointer to a timer.  Used for walking list of timers */
    qurt_timer_client_ptr *ptr;

    /* Offset from "zero" time of list */
    qurt_timetick_type delta;

    /* Find position to insert timer at */
    delta = timer->expiry;

    /* pointer to the active list */
    ptr   = &timers.active_list;

    while ( NULL != *ptr && delta >= (*ptr)->expiry )
    {
        ptr = &(*ptr)->next;
    }

    /* Link timer into list at indicated position */
    timer->next = *ptr;
    *ptr = timer;

} /* qurt_timer_lib_insert */

/**
 * Removes a timer from the list of active timers.
 * Note: Must be called from mutex lock.
 *
 * @param timer   Timer to be removed from list of active timers
 * @return        None
 */
static int qurt_timer_lib_remove ( qurt_timer_client_ptr timer )
{
    /* Pointer to a pointer to a timer.  Used for walking list of timers */
    qurt_timer_client_ptr *ptr;

    /* Search list of timers for the given timer */
    ptr = &timers.active_list;

    while ( NULL != *ptr && timer != *ptr )
    {
        ptr = &(*ptr)->next;
    }

    /* If the timer was found in the active_list */
    if ( timer == *ptr )
    {
        /* link the previous pointer to the timer after this one */
        *ptr = timer->next;
    }
    else
    {
        int i;
        // Could't find the timer in the active list. Try in the group list now.
        for (i = 0; i < QURT_TIMER_MAX_GROUPS; i++)
        {
            if (!timer_groups[i].list)
                continue;
            ptr = &timer_groups[i].list;
            while ( NULL != *ptr && timer != *ptr )
            {
                ptr = &(*ptr)->next;
            }

            /* If the timer was found in the active_list */
            if ( timer == *ptr )
            {
                /* link the previous pointer to the timer after this one */
                *ptr = timer->next;
            }
            else
            {
                qurtos_printf ("Timer doesn't exist, 0x%x\n", (unsigned int)timer);
                return QURT_EINVALID;
            }
        }
    }

    timer->next = NULL;

    return QURT_EOK;

} /* qurt_timer_lib_remove */

void qurt_timer_lib_process_timer_expiry (qurt_timer_client_ptr timer,  qurt_timetick_type now)
{
        qurt_timetick_type       next_duration_in_ticks;
        qurt_timer_duration_t    now_in_us, start_time_in_us, n_round, next_duration_in_us;

        /* Reactivate timer, if required */
        if (timer->reload > 0) 
        {
            /* Now calculate next duration and adjust drift so far */
            now_in_us = QURT_TIMER_TIMETICK_TO_US(now);
            start_time_in_us = QURT_TIMER_TIMETICK_TO_US(timer->start);

            /* How many rounds so far in total*/
            n_round = (now_in_us - start_time_in_us) / timer->reload;

            /* Calculate Next duration, we should never reload expired timer*/
            do {
                next_duration_in_us = start_time_in_us + ((n_round + 1) * timer->reload) - now_in_us;
                next_duration_in_ticks = QURT_TIMER_TIMETICK_FROM_US(next_duration_in_us);
                n_round++;
            } while (timer->expiry == now + next_duration_in_ticks); 

            if ( next_duration_in_ticks <= (QURT_TIMETICK_ERROR_MARGIN + QURT_TIMETICK_CALCULATION_MARGIN) )  
            {
	        /* Too late to set up for the next timeout, so go to the one after. */
	    
                next_duration_in_ticks += QURT_TIMER_TIMETICK_FROM_US(timer->reload);

                /* Double check the duration with the error margin to see if the timer is too short */
                if (next_duration_in_ticks < QURT_TIMETICK_ERROR_MARGIN + QURT_TIMETICK_CALCULATION_MARGIN)
                {
                   /* Timer duration should be at least set to margin */
                   next_duration_in_ticks = QURT_TIMETICK_ERROR_MARGIN + QURT_TIMETICK_CALCULATION_MARGIN ; 
                }
            }
            timer->expiry = now + next_duration_in_ticks;

            /* Insert timer back in active list */
            qurt_timer_lib_insert (timer);
        }
        else {
            /* Timer is no longer running */
            timer->expiry = 0;

            /* Invalidate timer */
            if (timer->magic != QURT_TIMER_INVALID)
                timer->magic  = QURT_TIMER_EXPIRED;
        }

        /* Process expired timer */
        qurt_timer_lib_expired ( timer );
}

/**
 * Sets an active timer to expire after a given period of time.
 * Optionally, specifies the timer to repeatly expire with a given period.
 *
 * @param timer    Timer to set
 * @param duration timer duration
 * @param expiry   expiry time in micro-seconds
 * @param reload   Period in micro-seconds between repeated expiries (0 = not periodic)
 * @return         QURT_EOK
 */
int qurt_timer_lib_new ( qurt_timer_client_ptr timer, qurt_timer_duration_t duration, qurt_timer_time_t expiry, qurt_timer_duration_t reload , unsigned int group)
{
    /* Intialize the new timer */
    timer->magic = QURT_TIMER_MAGIC;

    /* Lock KxMutex */
    // SLEEP_TIMER qmutex_lock ( alistMutex );
  
    /* Get the current timer clock count value in clock ticks */
    timer->start  = qurt_timer_lib_current_time (NULL);

    /* Set the group ID */
    timer->group = group;

    /* Determine when timer should expire */
    if( expiry !=  QURT_TIMER_DEFAULT_EXPIRY)
    {
       timer->type = QURT_TIMER_ABSOLUTE_EXPIRY;
       timer->expiry = QURT_TIMER_TIMETICK_FROM_US(expiry); 
       if( timer->expiry <= timer->start + QURT_TIMETICK_ERROR_MARGIN )
       {
           return QURT_ENOTALLOWED;  // not allowed - too close.
       }
    }
    else
    {
       timer->expiry = timer->start + QURT_TIMER_TIMETICK_FROM_US(duration); 
       timer->type = QURT_TIMER_RELATIVE_DURATION;
    }

    timer->reload = reload;

    /* Insert timer in the active timer list */
    qurt_timer_lib_insert ( timer );
 
    /* Active timer list has changed - ensure next timer event is correct */
    qurt_timer_lib_process_active_timers();

    return QURT_EOK;

} /* qurt_timer_lib_new */

/**
 * Stop an active timer. 
 *
 * @param timer    Timer 
 */
int qurt_timer_lib_stop_active_timer ( qurt_timer_client_ptr timer )
{
    /* Remove the timer from the active timer list */
    qurt_timer_lib_remove( timer );

    /* Mark it as expired */
    timer->magic = QURT_TIMER_EXPIRED;

    /* Recheck active timer list and programe the timer HW */
    qurt_timer_lib_process_active_timers();

    return QURT_EOK;

} /* qurt_timer_lib_stop_active_timer */

/**
 * Restart an active timer. 
 *
 * @param timer    Timer 
 * @param time     duration or expiry (duration from startup)
 */
int qurt_timer_lib_restart_active_timer ( qurt_timer_client_ptr timer, qurt_timer_duration_t time)
{
    /* Remove the timer from the active timer list */
    qurt_timer_lib_remove( timer );

    /* Set the new values into the timer */
    timer->start  = qurt_timer_lib_current_time (NULL);;

    if(timer->type == QURT_TIMER_ABSOLUTE_EXPIRY)
    {
        timer->expiry = QURT_TIMER_TIMETICK_FROM_US(time); 
    }
    else  /* QURT_TIMER_RELATIVE_DURATION */
    {
       timer->expiry = timer->start + QURT_TIMER_TIMETICK_FROM_US(time); 
    }

    if( timer->expiry <= timer->start + QURT_TIMETICK_ERROR_MARGIN )
    {
        return QURT_ENOTALLOWED;  // not allowed - too close.
    }
    
    /* Insert timer in the active timer list */
    qurt_timer_lib_insert ( timer );
 
    /* Process the active timer list and programe the timer HW */
    qurt_timer_lib_process_active_timers();

    return QURT_EOK;

} /* qurt_timer_lib_restart_active_timer */

/**
 * Stops an active timer
 *
 * @param timer    Timer to stop
 * @return         QURT_EOK or error code
 */

int qurt_timer_lib_cancel ( qurt_timer_client_ptr timer )
{
    /* Current TCXO clock tick count */
    qurt_timetick_word_t now;

    /* Success or failure */
    int retVal = QURT_EOK;

    /* Verify Magic Number */
    if ( QURT_TIMER_MAGIC != timer->magic )
    {
        qurtos_printf ("if ( QURT_TIMER_MAGIC != timer->magic )\n");
        return QURT_EINVALID;
    }

    /* Lock KxMutex */
    // SLEEP_TIMER qmutex_lock ( alistMutex );

    /* Get the current uptime ticks */
    now = qurt_timer_lib_current_time (NULL);
    
    /* If the timer interrupt has already occurred or is about to occur... */
    if (timer->expiry > (now + QURT_TIMETICK_ERROR_MARGIN))
    {
        /* Timer is active - remove timer */
        retVal = qurt_timer_lib_remove( timer );

        /* Active timer list has changed - ensure next timer event is correct */
        qurt_timer_lib_process_active_timers();

        /* Invalidate the timer */
        timer->magic = QURT_TIMER_INVALID;
    }
    else
    {
        /* Timer has expired or is about to expire - set reload to zero */
        timer->reload = 0;
        timer->magic = QURT_TIMER_INVALID;

        retVal = ETIMER_TOOCLOSE;
            /* Timer removal failed, interrupt too close */
            //retVal =  ETIMER_TOOCLOSE;
    }

    /* Free KxMutex */
    // SLEEP_TIMER qmutex_unlock ( alistMutex );

    return retVal;

} /* qurt_timer_lib_cancel */

/**
 * This function processes expired timers in the active timer list, and then program the timer HW for next timer interrupt.
 * Considering that timerServer thread is not running at highest priority and context switch may happen during programming 
 * timer HW, this function detects context switch and then reprogram timer in a loopback mode. 
 *
 * @param    None
 * @return   None
 */

int recheck_timers;

void qurt_timer_lib_process_active_timers ( void )
{
    qurt_timetick_type now, first;
    qurt_timetick_word_t cur_count, diff_count, match_value;
    qurt_timer_client_ptr timer;

		do {
			/* Get the current uptime ticks */
			now = qurt_timer_lib_current_time (&cur_count);

			/* Process the active timer list */
			while ( timer = timers.active_list, timer != NULL && 
							( timer->expiry <= (now + QURT_TIMETICK_ERROR_MARGIN ) ) )
				{
					/* Process the expired timer */
					timers.active_list = timer->next;
					timer->next = NULL;
					qurt_timer_lib_process_timer_expiry (timer, now);
				} 

			/* Initialize first expiry to maximum value, and then update it */
			first = QURT_TIMER_MAX_DURATION_TICKS;
			if ( timers.active_list != NULL )
				{
					/* Get the time of the first expiring timer */
					first = timers.active_list->expiry;
					if (first == timers.set_expiry)  // No need to program timer HW 
						{
							return; 
						}
				}

			/* Program the timer for next interrupt */
			diff_count = first - now;
			match_value = cur_count + diff_count;

			/* qurt_sysclock_alarm_create may decide the timer is expired */
			recheck_timers = 0;
			qurt_sysclock_alarm_create (qurt_timer_sclk_id, cur_count, match_value);

			/* Update record of timer setup */
			timers.set_expiry = first;
			timers.match_value = match_value;
			timers.count_value = cur_count;
		} while (recheck_timers);

} /* qurt_timer_lib_process_active_timers */

/**
 * Complete IPC send operation using callback after sleep timer expires
 * Note: This function is executed from qurt_timer server pd.
 *
 * @param token     Sleeping client thread ID.
 * @return          QURT_EOK if send is successful, error code otherwise
 */
void qurt_timer_lib_process_signal_callback (qurt_timer_client_ptr timer)
{
    //FIXME: in multiPD, the signal shall be allocated from usermalloc
    qurt_anysignal_set (timer->signal, timer->mask);
} /* qurt_timer_lib_process_callback */

void qurt_timer_lib_process_sleep_callback (qurt_timer_client_ptr timer)
{
    //FIXME: in multiPD, the signal shall be allocated from usermalloc
    qurt_anysignal_set (timer->signal, timer->mask);

    /* SLEEP TIMER free sleep timer after expires */
    /* user is not aware of the sleep timer. Thus don't expect user to delete it */
    timer->magic = QURT_TIMER_INVALID;

		/* FIXME: This is supposed to check if the pointer is actually in the
			 heap; can't use h2_free. But mostly these are not malloced
			 anyway. */
		// qurtos_free (timer); 
} /* qurt_timer_lib_sleep_callback */
#if 0
void qurt_timer_lib_process_qmsgq_callback (qurt_timer_client_ptr timer)
{
    qmsgq_lib_process_callback ((unsigned int)timer->cb_data.qmsgq);
} /* qurt_timer_lib_process_callback */
#endif
/**
 * Process all the timers expired in a group list. Re-insert the timers in
 * active list, in case of periodic timers.
 *
 * @param    group
 * @return   none
 */
void qurt_timer_lib_process_group_expiry (unsigned int group)
{
    qurt_timetick_type now; 
    /* Pointer to a timer.  Used for walking list of timers */
    qurt_timer_client_ptr timer;

    /* Current time */ 
    now = qurt_timer_lib_current_time (NULL);

    while (timer = timer_groups[group].list, timer != NULL && 
           (timer->expiry <= (now + QURT_TIMETICK_ERROR_MARGIN))) {
        /* Remove expiring timer from group list */
        timer_groups[group].list = timer->next;
        timer->next = NULL;

        qurt_timer_lib_process_timer_expiry (timer, now);
    }
}

/**
 * Disables the timer group. Pulls the timers from active list and inserts
 * the timers at appropriate loctions in the group list. At the end updates
 * the HW register.
 *
 * @param    group
 * @return   QURT_EOK, QURT_EINVALID for wrong group
 */
int qurt_timer_lib_group_disable (unsigned int group)
{
    /* Pointer to a pointer to a timer.  Used for walking list of timers */
    qurt_timer_client_ptr *active_ptr, *group_ptr;

    /* Check for the valid range of group ID */
    if (group >= QURT_TIMER_MAX_GROUPS) {
        return QURT_EINVALID;
    }

    /* pointer to the active list and group list */
    active_ptr   = &timers.active_list;
    group_ptr = &timer_groups[group].list;

    while (*active_ptr != NULL) {
        if ((*active_ptr)->group == group) {
            /* Insert the timer to group list */
            *group_ptr = *active_ptr;
            group_ptr = &(*group_ptr)->next;

            /* Remove the the timer from active list */
            *active_ptr = *group_ptr;

            /* Mark the end of group list */
            *group_ptr = NULL;
        }
        else {
            /* Update the active list pointer */ 
            active_ptr = &(*active_ptr)->next;
        }
    }

    /* Recheck active timers and update the HW */
    qurt_timer_lib_process_active_timers();

    return QURT_EOK;
}

/**
 * Enables the timer group. Pulls the timers from group list and inserts
 * the timers at appropriate loctions in the active list. Processes the
 * timers already expired. At the end update the HW register.
 *
 * @param    group
 * @return   QURT_EOK, QURT_EINVALID for wrong group
 */
int qurt_timer_lib_group_enable (unsigned int group)
{
    qurt_timer_client_ptr *active_ptr, *group_ptr, temp;

    /* Check for the valid range of group ID */
    if (group >= QURT_TIMER_MAX_GROUPS) {
        return QURT_EINVALID;
    }

    /* Process expired group timers */
    qurt_timer_lib_process_group_expiry (group);

    /* pointer to the active list and group list */
    active_ptr   = &timers.active_list;
    group_ptr = &timer_groups[group].list;

    while (*group_ptr != NULL) {
        while ( (*active_ptr != NULL) && ((*group_ptr)->expiry >= (*active_ptr)->expiry)) {
            active_ptr = &(*active_ptr)->next;
        }
        /* Save the next element in group list */
        temp = (*group_ptr)->next;
        /* Insert the current group element in active list */
        (*group_ptr)->next = *active_ptr;
        *active_ptr = *group_ptr;

        /* Adjust the group list */
        *group_ptr = temp;
    }

    /* Process active timers and reprogram timer HW */
    qurt_timer_lib_process_active_timers();

    return QURT_EOK;
}
