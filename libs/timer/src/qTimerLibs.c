/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <qube.h>
#include "qTimerLibs.h"
#include "qTimerServer.h"
#include "qmsgq.h"
#include "blast_error.h"

qtimers_type  timers = 
{
  /* Linked list of active timers */
  NULL,

  /* timer value programmed in hardware (set_value) */
  QTIMETICK_MAX - 1,

  /* Initial HW count */
  0,

  /* Next expiry */
  QTIMETICK_MAX,

  /* Rollover ticks */
  0
};

qtimers_group_type timer_groups[QTIMER_MAX_GROUPS];

/**
 * Initializes the Timer Subsytem
 * Note: This function is executed from server side.
 *
 * SIDE EFFECTS: May install an ISR for the 32-bit TCXO clock match register
 *
 * @param     None
 * @return    None
 */
void qtimer_lib_init ( void )
{
    int i;
#ifdef QTIMER_ROLLOVER_TEST
    qtimetick_word_t diff_count;
#endif /* QTIMER_ROLLOVER_TEST */

    /* Initialize groups */
    for (i = 0; i < QTIMER_MAX_GROUPS; i++) {
        timer_groups[i].list = NULL;
    }

    /* Initialize the list of active timers to contain no timers */
    timers.active_list = NULL;

    /* Initialize the timers */
    timers.zero = 0;
#ifndef QTIMER_ROLLOVER_TEST
    timers.set_expiry = QTIMER_MAX_DURATION_TICKS;
    timers.match_value = QTIMER_MAX_DURATION_TICKS;
    timers.count_value = 0;
#else /* QTIMER_ROLLOVER_TEST */
    timers.set_expiry = timers.zero + QTIMER_TEST_COUNT + QTIMER_MAX_DURATION_TICKS;
    //timers.count_value = QTIMER_TEST_COUNT;

    /* The set value is not updated immediately */
    diff_count = timers.set_expiry - timers.zero - QTIMER_TEST_COUNT;

    /* Account for roll over */
    if (diff_count > (QTIMETICK_MAX - QTIMER_TEST_COUNT)) {
        /* Expire after roll over */
        timers.match_value = diff_count - (QTIMETICK_MAX - QTIMER_TEST_COUNT + 1);
    }
    else {
        /* Expire before roll over */
        timers.match_value = diff_count + QTIMER_TEST_COUNT;
    }
    /* Record the count value to take care of roll over upon expiry */
    timers.count_value = QTIMER_TEST_COUNT;
#endif /* 0 */

    /* Disable hardware timer, clear count & match value to prevent 
       an unexpected interrupt. */
    hw_timer_init (timers.match_value);

    /* Create KxMutex object */
    // SLEEP_TIMER qmutex_create ( &alistMutex, QMUTEX_LOCAL );

} /* qtimer_lib_init */

/**
 * Return absolute current time since the hardware timer was enabled
 * Note: This function is executed from server side.
 *
 * @param     hw_count - pointer to save HW count
 * @return    Absolute current time
 */
qtimetick_type qtimer_lib_current_time (qtimetick_word_t *hw_count)
{
    qtimetick_word_t cur_count;
    /* Variable to Store the current time */
    qtimetick_type now;

    /* Lock KxMutex */
    // SLEEP_TIMER qmutex_lock ( alistMutex );
 
    /* Calculate current time */
    cur_count = hw_timer_curr_timetick ();
    now = timers.zero + cur_count;
    /* Account the roll over */
    if (cur_count < timers.count_value) {
        now = now + QTIMETICK_MAX + 1;
    }

    /* Set the HW count value if the caller is interested */
    if (hw_count) {
        *hw_count = cur_count;
    }

    /* Free KxMutex */
    // SLEEP_TIIMER qmutex_unlock ( alistMutex );

    return now;
    
} /* qtimer_lib_current_time */

/**
 * Inserts a timer into the sorted list of timers.
 * Timers are sorted according to increasing expiration times
 * Note: Must be called from mutex lock.
 *
 * @param timer   Timer to be inserted into list of active timers
 * @return        None
 */
static void qtimer_lib_insert ( qtimer_client_ptr timer )
{
    /* Pointer to a pointer to a timer.  Used for walking list of timers */
    qtimer_client_ptr *ptr;

    /* Offset from "zero" time of list */
    qtimetick_type delta;

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

} /* qtimer_lib_insert */

/**
 * Removes a timer from the list of active timers.
 * Note: Must be called from mutex lock.
 *
 * @param timer   Timer to be removed from list of active timers
 * @return        None
 */
static int qtimer_lib_remove ( qtimer_client_ptr timer )
{
    /* Pointer to a pointer to a timer.  Used for walking list of timers */
    qtimer_client_ptr *ptr;

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
        for (i = 0; i < QTIMER_MAX_GROUPS; i++)
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
                printf ("Timer doesn't exist, 0x%x\n", (unsigned int)timer);
                return EINVALID;
            }
        }
    }

    timer->next = NULL;

    return EOK;

} /* qtimer_lib_remove */

void qtimer_lib_process_timer_expiry (qtimer_client_ptr timer,  qtimetick_type now)
{
        qtimetick_type       next_duration_in_ticks;
        qtimer_duration_t    now_in_us, start_time_in_us, n_round, next_duration_in_us;

        /* Reactivate timer, if required */
        if (timer->reload > 0) 
        {
            /* Now calculate next duration and adjust drift so far */
            now_in_us = QTIMER_TIMETICK_TO_US(now);
            start_time_in_us = QTIMER_TIMETICK_TO_US(timer->start);

            /* How many rounds so far in total*/
            n_round = (now_in_us - start_time_in_us) / timer->reload;

            /* Calculate Next duration*/
            next_duration_in_us = start_time_in_us + ((n_round + 1) * timer->reload) - now_in_us;
            next_duration_in_ticks = QTIMER_TIMETICK_FROM_US(next_duration_in_us);

            if ( next_duration_in_ticks <= (QTIMETICK_ERROR_MARGIN + QTIMETICK_CALCULATION_MARGIN) )  
            {
	        /* Too late to set up for the next timeout, so go to the one after. */
	    
                next_duration_in_ticks += QTIMER_TIMETICK_FROM_US(timer->reload);

                /* Double check the duration with the error margin to see if the timer is too short */
                if (next_duration_in_ticks < QTIMETICK_ERROR_MARGIN + QTIMETICK_CALCULATION_MARGIN)
                {
                   /* Timer duration should be at least set to margin */
                   next_duration_in_ticks = QTIMETICK_ERROR_MARGIN + QTIMETICK_CALCULATION_MARGIN ; 
                }
            }
            timer->expiry = now + next_duration_in_ticks;

            /* Insert timer back in active list */
            qtimer_lib_insert (timer);
        }
        else {
            /* Timer is no longer running */
            timer->expiry = 0;

            /* Invalidate timer */
            if (timer->magic != QTIMER_INVALID)
                timer->magic  = QTIMER_EXPIRED;
        }

        /* Process expired timer */
        qtimer_lib_expired ( timer );
}

/**
 * If the head of the timer list is modified, update the hardware timer to match
 * the expiry of the head of the timer list.
 *
 * Note: Must be called with Mutex Locked.
 * 
 * @param timer   None
 * @return        None
 */
static void qtimer_lib_program_hw ( void )
{
    /* Current TCXO clock count */
    qtimetick_word_t cur_count, diff_count, count_lapse, match_value;
    /* Time of first expiring timer */
    qtimetick_type first, now;

do_over:
    /* Get the current uptime ticks */
    now = qtimer_lib_current_time (&cur_count);

    /* Initialize first expiry to maximum value */
    first = now + QTIMER_MAX_DURATION_TICKS;

    /* If the timer interrupt has already occurred or is about to occur... */
    if ( timers.set_expiry <= (now + QTIMETICK_ERROR_MARGIN ) )
    {
        /* ... let the Timer ISR program the hardware timer */
        return;
    }

    /* The remaining tests deal with the first timer on the active timer list. */

    /* Are there timers on the timer list? */
    if ( timers.active_list != NULL )
    {
        /* Get the time of the first expiring timer */
        first = timers.active_list->expiry;

        /* If the first expiring timer matches the timers.set_value, ... */
        if (first == timers.set_expiry)
        {
            /* ... then it is already properly programmed in the hardware */
            return;
        }
    }

    /* The set value is not updated immediately */
    diff_count = first - now;
    match_value = cur_count + diff_count;

    /* XXX NOTE XXX: On L4 version we mask interrupts by clearing ssr[IE]. We
     * have problems doing that in BLAST due to Fast interrupts. However,
     * masking interrupts is not required in BLAST because the HW thread is
     * disabled to receive interrupt, if the task running is of higher
     * priority compared to rest of the tasks. And BlastTimerServer runs at
     * one of the highest priorities in the system */
    //qsystem_mask_loc_int (); 

    /* Set the next interrupt */
    count_lapse = hw_timer_prg_next_interrupt (match_value, cur_count);

    /* Unlock interrupts to avoid context switching */
    //qsystem_unmask_loc_int (); 

    /* XXX NOTE from Bala: There is a Legacy issue in Modem SW where EBI access may be hung for a while
     * This could make the match value historic. */
    if (count_lapse > 1) goto do_over;

    /* Update zero if roll over already occurred */
    if (cur_count < timers.count_value) {
        timers.zero = timers.zero + QTIMETICK_MAX + 1;
    }

    /* Program to expire at "first" */
    timers.set_expiry = first;
    timers.match_value = match_value;

    /* Record the count value to take care of roll over upon expiry */
    timers.count_value = cur_count;
} /* qtimer_lib_program_hw */

/**
 * Sets an active timer to expire after a given period of time.
 * Optionally, specifies the timer to repeatly expire with a given period.
 *
 * @param timer    Timer to set
 * @param duration timer duration
 * @param expiry   expiry time in micro-seconds
 * @param reload   Period in micro-seconds between repeated expiries (0 = not periodic)
 * @return         EOK
 */
int qtimer_lib_new ( qtimer_client_ptr timer, qtimer_duration_t duration, qtimer_time_t expiry, qtimer_duration_t reload , unsigned int group)
{
    /* Intialize the new timer */
    timer->magic = QTIMER_MAGIC;

    /* Lock KxMutex */
    // SLEEP_TIMER qmutex_lock ( alistMutex );
  
    /* Get the current timer clock count value in clock ticks */
    timer->start  = qtimer_lib_current_time (NULL);

    /* Set the group ID */
    timer->group = group;

    /* Determine when timer should expire */
    if( expiry !=  QTIMER_DEFAULT_EXPIRY)
    {
       timer->type = QTIMER_ABSOLUTE_EXPIRY;
       timer->expiry = QTIMER_TIMETICK_FROM_US(expiry); 
       if( timer->expiry <= timer->start + QTIMETICK_ERROR_MARGIN )
       {
           return ENOTALLOWED;  // not allowed - too close.
       }
    }
    else
    {
       timer->expiry = timer->start + QTIMER_TIMETICK_FROM_US(duration); 
       timer->type = QTIMER_RELATIVE_DURATION;
    }

    timer->reload = reload;

    /* Insert timer in the active timer list */
    qtimer_lib_insert ( timer );
 
    /* Active timer list has changed - ensure next timer event is correct */
    qtimer_lib_program_hw ( );

    /* Free KxMutex */
    // SLEEP_TIMER qmutex_unlock ( alistMutex );

    return EOK;

} /* qtimer_lib_new */

/**
 * Stop an active timer. 
 *
 * @param timer    Timer 
 */
int qtimer_lib_stop_active_timer ( qtimer_client_ptr timer )
{
    /* Remove the timer from the active timer list */
    qtimer_lib_remove( timer );

    /* Mark it as expired */
    timer->magic = QTIMER_EXPIRED;

    /* Programe the timer HW */
    qtimer_lib_program_hw ( );

    return EOK;

} /* qtimer_lib_stop_active_timer */

/**
 * Restart an active timer. 
 *
 * @param timer    Timer 
 * @param time     duration or expiry (duration from startup)
 */
int qtimer_lib_restart_active_timer ( qtimer_client_ptr timer, qtimer_duration_t time)
{
    /* Remove the timer from the active timer list */
    qtimer_lib_remove( timer );

    /* Set the new values into the timer */
    timer->start  = qtimer_lib_current_time (NULL);;

    if(timer->type == QTIMER_ABSOLUTE_EXPIRY)
    {
        timer->expiry = QTIMER_TIMETICK_FROM_US(time); 
    }
    else  /* QTIMER_RELATIVE_DURATION */
    {
       timer->expiry = timer->start + QTIMER_TIMETICK_FROM_US(time); 
    }

    if( timer->expiry <= timer->start + QTIMETICK_ERROR_MARGIN )
    {
        return ENOTALLOWED;  // not allowed - too close.
    }
    
    /* Insert timer in the active timer list */
    qtimer_lib_insert ( timer );
 
    /* Programe the timer HW */
    qtimer_lib_program_hw ( );

    return EOK;

} /* qtimer_lib_restart_active_timer */

/**
 * Stops an active timer
 *
 * @param timer    Timer to stop
 * @return         EOK or error code
 */

int qtimer_lib_cancel ( qtimer_client_ptr timer )
{
    /* Current TCXO clock tick count */
    qtimetick_word_t now;

    /* Success or failure */
    int retVal = EOK;

    /* Verify Magic Number */
    if ( QTIMER_MAGIC != timer->magic )
    {
        printf ("if ( QTIMER_MAGIC != timer->magic )\n");
        return EINVALID;
    }

    /* Lock KxMutex */
    // SLEEP_TIMER qmutex_lock ( alistMutex );

    /* Get the current uptime ticks */
    now = qtimer_lib_current_time (NULL);
    
    /* If the timer interrupt has already occurred or is about to occur... */
#ifdef QTIMER_TXCO
    if (timer->expiry > (now + QTIMETICK_INT_MARGIN))
#else
    if (timer->expiry > (now + QTIMETICK_ERROR_MARGIN))
#endif
    {
        /* Timer is active - remove timer */
        retVal = qtimer_lib_remove( timer );

        /* Active timer list has changed - ensure next timer event is correct */
        qtimer_lib_program_hw( );

        /* Invalidate the timer */
        timer->magic = QTIMER_INVALID;
    }
    else
    {
        /* Timer has expired or is about to expire - set reload to zero */
        timer->reload = 0;
        timer->magic = QTIMER_INVALID;

        retVal = ETIMER_TOOCLOSE;
            /* Timer removal failed, interrupt too close */
            //retVal =  ETIMER_TOOCLOSE;
    }

    /* Free KxMutex */
    // SLEEP_TIMER qmutex_unlock ( alistMutex );

    return retVal;

} /* qtimer_lib_cancel */

/**
 * This is a timer isr thread. It first registers itself as timer interrupt 
 * handler and waits for ipc. Upon receiving ipc, it processes the expired
 * timers by calling timer_isr function and finally sends a reply back to the
 * sender before going in wait again.
 *
 * @param    None
 * @return   None
 */

/**
 * This function locks mutex and processes those timers which have expired.
 * Note: May insert messages in message queue, which can cause task switches.
 *
 * @param    None
 * @return   None
 */
void qtimer_lib_isr ( void )
{
    /* Timer TCXO time-stamp values */
    qtimetick_type now, first;
    qtimetick_word_t cur_count, diff_count, new_count, count_lapse, match_value;
   
    /* Pointer to a timer.  Used for walking list of timers */
    qtimer_client_ptr timer;

    /* Lock KxMutex */
    // SLEEP_TIMER qmutex_lock ( alistMutex );
do_over:
        /* Get the current uptime ticks */
        now = qtimer_lib_current_time (&cur_count);

        /* Check to determine if the timer at head of the active timer list has
        past its expiry point.  If it has, remove it, process the timer, and
        repeat the steps with the new timer at the head of active timer list.  */

#ifdef QTIMER_TXCO
        while ( timer = timers.active_list, timer != NULL && 
                ( timer->expiry <= (now + QTIMETICK_INT_MARGIN ) ) )
#else
        while ( timer = timers.active_list, timer != NULL && 
                ( timer->expiry <= (now + QTIMETICK_ERROR_MARGIN ) ) )
#endif
        {
            /* Remove expiring timer from active timer list */
            timers.active_list = timer->next;
            timer->next = NULL;

            qtimer_lib_process_timer_expiry (timer, now);

        } /* while timers on timer.active_list are expiring */

    /* Timers that expire at and before "now" + "ERROR_MARGIN" have been processed. */

    /* Initialize first expiry to maximum value */
    first = now + QTIMER_MAX_DURATION_TICKS;
    /* Are there timers on the timer list? */
    if ( timers.active_list != NULL )
    {
        /* Get the time of the first expiring timer */
        first = timers.active_list->expiry;

        /* If the first expiring timer matches the timers.set_value, ...
         * We reached here due to spurious interrupt */
        if (first == timers.set_expiry)
        {
            /* ... then it is already properly programmed in the hardware */
            return;
        }
    }

    /* The set value is not updated immediately */
    diff_count = first - now;
    match_value = cur_count + diff_count;

    /* XXX NOTE XXX: On L4 version we mask interrupts by clearing ssr[IE]. We
     * have problems doing that in BLAST due to Fast interrupts. However,
     * masking interrupts is not required in BLAST because the HW thread is
     * disabled to receive interrupt, if the task running is of higher
     * priority compared to rest of the tasks. And BlastTimerServer runs at
     * one of the highest priorities in the system */
    //qsystem_mask_loc_int (); 

    /* Set the next interrupt */
    count_lapse = hw_timer_prg_next_interrupt (match_value, cur_count);

    /* Set the next interrupt */
    //qsystem_unmask_loc_int (); 

    /* XXX NOTE from Bala: There is a Legacy issue in Modem SW where EBI access may be hung for a while
     * This could make the match value historic. */
    if (count_lapse > 1) goto do_over;

    /* Update zero if roll over already occurred */
    if (cur_count < timers.count_value) {
        timers.zero = timers.zero + QTIMETICK_MAX + 1;
    }
    /* Program to expire at "first" */
    timers.set_expiry = first;
    timers.match_value = match_value;

    /* Record the count value to take care of roll over upon expiry */
    timers.count_value = cur_count;
} /* qtimer_lib_isr */

/**
 * Complete IPC send operation using callback after sleep timer expires
 * Note: This function is executed from qtimer server pd.
 *
 * @param token     Sleeping client thread ID.
 * @return          EOK if send is successful, error code otherwise
 */
void qtimer_lib_process_signal_callback (qtimer_client_ptr timer)
{
    blast_anysignal_set (timer->cb_data.signal, timer->cb_data.mask);
} /* qtimer_lib_process_callback */

void qtimer_lib_process_sleep_callback (qtimer_client_ptr timer)
{
    blast_anysignal_set (timer->cb_data.signal, timer->cb_data.mask);

    /* SLEEP TIMER free sleep timer after expires */
    /* user is not aware of the sleep timer. Thus don't expect user to delete it */
    timer->magic = QTIMER_INVALID;
    free (timer); 
} /* qtimer_lib_sleep_callback */

void qtimer_lib_process_qmsgq_callback (qtimer_client_ptr timer)
{
    qmsgq_lib_process_callback ((unsigned int)timer->cb_data.qmsgq);
} /* qtimer_lib_process_callback */

/**
 * Process all the timers expired in a group list. Re-insert the timers in
 * active list, in case of periodic timers.
 *
 * @param    group
 * @return   none
 */
void qtimer_lib_process_group_expiry (unsigned int group)
{
    qtimetick_type now; 
    /* Pointer to a timer.  Used for walking list of timers */
    qtimer_client_ptr timer;

    /* Current time */ 
    now = qtimer_lib_current_time (NULL);

    while (timer = timer_groups[group].list, timer != NULL && 
           (timer->expiry <= (now + QTIMETICK_ERROR_MARGIN))) {
        /* Remove expiring timer from group list */
        timer_groups[group].list = timer->next;
        timer->next = NULL;

        qtimer_lib_process_timer_expiry (timer, now);
    }
}

/**
 * Disables the timer group. Pulls the timers from active list and inserts
 * the timers at appropriate loctions in the group list. At the end updates
 * the HW register.
 *
 * @param    group
 * @return   EOK, EINVALID for wrong group
 */
int qtimer_lib_group_disable (unsigned int group)
{
    /* Pointer to a pointer to a timer.  Used for walking list of timers */
    qtimer_client_ptr *active_ptr, *group_ptr;

    /* Check for the valid range of group ID */
    if (group >= QTIMER_MAX_GROUPS) {
        return EINVALID;
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

    /* Update the HW */
    qtimer_lib_program_hw( );

    return EOK;
}

/**
 * Enables the timer group. Pulls the timers from group list and inserts
 * the timers at appropriate loctions in the active list. Processes the
 * timers already expired. At the end update the HW register.
 *
 * @param    group
 * @return   EOK, EINVALID for wrong group
 */
int qtimer_lib_group_enable (unsigned int group)
{
    qtimer_client_ptr *active_ptr, *group_ptr, temp;

    /* Check for the valid range of group ID */
    if (group >= QTIMER_MAX_GROUPS) {
        return EINVALID;
    }

    /* Process expired group timers */
    qtimer_lib_process_group_expiry (group);

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

    /* We can add a timer which is smaller than the timer which was programmed in HW.
     * qtimer_lib_program_hw() fails under this scenario. 
    qtimer_lib_program_hw( );
     */
    qtimer_lib_isr ( );

    return EOK;
}
