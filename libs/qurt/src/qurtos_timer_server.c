/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*****************************************************************************/
/*                           INCLUDE FILES                                   */
/*****************************************************************************/
#include <qurtos_internal.h>

#define QURT_TIMER_CLIENT_STACK_SIZE  4096

//#define bool int
//#define false (0)
//#define true (1)

extern int qurt_timer_sclk_id;

// qurt_mutex_t qurt_timer_lock;
//static qurt_sem_t qurt_timer_client_started;

/* struct qurtos_thread_info timerServer_thread_info; */
/* struct QURT_ugp_ptr timerServer_ugp_area; */

/*****************************************************************************/
/*                      FUNCTION DEFINITIONS                                 */
/*****************************************************************************/

/*****************************************************************************/
/*                   Library functions for Server Code                       */
/*****************************************************************************/

/**
 * This function Processes the expired timer
 * Note: May insert messages in message queue, which can cause task switches.
 * 
 * @param timer   Expiring timer to be processed
 * @return        None
 */
void qurt_timer_lib_expired ( qurt_timer_client_ptr timer )
{
    if (timer->magic == QURT_TIMER_INVALID)
			{
				/* FIXME: This is supposed to check if the pointer is actually in the
					 heap; can't use h2_free. But mostly these are not malloced
					 anyway. */
				// qurtos_free(timer);
        return;
    }

    timer->func (timer); 
} /* qurt_timer_lib_expired */

/**
 * Completes get attribute function in timer
 * Note: This function is executed from server side.
 *
 * @param client       Pointer to the client node
 * @param attr   [out] Pointer to the attribute object 
 * @return             QURT_EOK if message queue deletion is successful,
 *                     error code otherwise
 */
int qurt_timer_lib_get_attr ( qurt_timer_client_ptr client, qurt_timer_attr_t *attr )
{
    /* absolute time tick elapsed */
    qurt_timetick_type now;

    /* Duration value in timeticks */
    qurt_timetick_word_t duration;

    /* Fill the magic number */
    attr->magic        = QURT_TIMER_ATTR_MAGIC;

    /* get current absolute time tick */
    now = qurt_timer_lib_current_time(NULL);

    if (client->expiry > now)
        /* expiry is in future, set remaining time in us */
        attr->remaining = QURT_TIMER_TIMETICK_TO_US(client->expiry - now);
    else /*expiry has passed */
        attr->remaining = 0;

    /* If this is a PERIODIC timer, reload gives the duration */
    if ( 0 < client->reload )
    {
        /* Set duration in microsections */
        attr->duration = client->reload;

        /* Set the timer type */
        attr->type     = QURT_TIMER_PERIODIC;

    }
    else
    {
        /* Find duration from expiry time and start time */
        duration       = (qurt_timetick_word_t) (client->expiry - client->start);

        /* Convert timeticks into microsections */
        attr->duration = QURT_TIMER_TIMETICK_TO_US( duration );

        /* Convert timeticks into microsections */
        attr->expiry = QURT_TIMER_TIMETICK_TO_US( client->expiry );

        /* Set the timer type */
        attr->type     = QURT_TIMER_ONESHOT;

    }

    attr->group = client->group;

    return QURT_EOK;

} /* qurt_timer_lib_get_attr */

/**
 * Completes the create sleep timer operation.
 * Note: This function is executed from server side.
 *
 * This function will create a new timer node, insert it in the timer list 
 * and initialize callback for timer expiry.
 *
 * @param duration        Duration of the timer
 * @param to_tid          ID of client thread that is calling qsleep() function
 * @return                QURT_EOK if creation is successful, error code otherwise
 */

int qurt_timer_lib_sleep_activate (  qurt_timer_duration_t duration, qurt_anysignal_t *signal, unsigned int mask)
{
    /* Result, success or failure */
    int result; 

    /* Pointer to the new timer */
    qurt_timer_client_ptr new_timer;

    /* Initialize new timer */
    new_timer = NULL;

    /* Allocate space for new timer */
    new_timer = ( qurt_timer_client_ptr )h2_malloc( sizeof( qurt_timer_client ) ); 

    /* Check if the malloc successful */
    if ( NULL == new_timer )
    {
        return QURT_EMEM;
    }

    /* Define the Timer Handle */
    new_timer->signal = signal;
    new_timer->mask = mask;
    new_timer->func = qurt_timer_lib_process_sleep_callback;

    /* Perform the actual timer set */
    result = qurt_timer_lib_new( new_timer, duration, QURT_TIMER_DEFAULT_EXPIRY, 0 /*one-shot type*/, QURT_TIMER_DEFAULT_GROUP); 
    (void)result;         // Suppress warning
    
    /* Return success */
    return QURT_EOK;
}

/**
 * Completes the create timer operation.
 * Note: This function is executed from server side.
 *
 * This function will create a new timer node, insert it in the timer list 
 * and initialize callback for timer expiry.
 *
 * @param client   [out]  Pointer to the client node structure
 * @param duration        Duration of the timer
 * @param expiry          Expiry of the timer
 * @param type            Type of the new timer
 * @param callback        Pointer to Callback when the timer expires
 * @return                QURT_EOK if creation is successful, error code otherwise
 */
int qurt_timer_lib_timer_activate ( qurt_timer_t *client, qurt_timer_duration_t duration, qurt_timer_time_t expiry,  
                  qurt_timer_type_t type, const qurt_anysignal_t *signal, unsigned int mask , unsigned int group)
{
    /* Result, success or failure */
    int result;
    /* Reload value in timeticks */
    qurt_timer_duration_t reload;
    /* Pointer to the new timer */
    qurt_timer_client_ptr new_timer;
	
    /* Initialize new timer */
    new_timer = NULL;

    /* Allocate space for new timer */
    new_timer = ( qurt_timer_client_ptr )h2_malloc( sizeof( qurt_timer_client ) ); 

    /* Check if the malloc successful */
    if ( NULL == new_timer )
    {
        return QURT_EMEM;
    }

    new_timer->signal = (qurt_anysignal_t *)signal;
    new_timer->mask = mask;
    new_timer->func = qurt_timer_lib_process_signal_callback;
    
    /* If the timer is one-shot ... */
    if ( QURT_TIMER_ONESHOT == type ) 
    {
        /* ... reload value zero */
    	reload = 0;
    } 
    else 
    {
        /* Calculate the reload value for periodic timer */
        reload = duration;
    } 

    /* Perform the actual timer set */
    result = qurt_timer_lib_new( new_timer, duration, expiry, reload, group); 
    if(result != QURT_EOK)
    {
        h2_free( new_timer );
	return result;
    }

    /* Return Pointer to the user */
    *client = ( qurt_timer_t )new_timer;
   
    /* Return success */
    return QURT_EOK;
    
} /* qurt_timer_lib_timer_activate */

/**
 * Completes the operation to stop a timer.
 *
 * @param timer           Pointer to the timer object
 * @return                QURT_EOK if restart is successful, error code otherwise
 */
int qurt_timer_lib_timer_stop ( qurt_timer_client_ptr timer) 
{
    int result;

    /* Check if the timer is oneshot timer */
    if( timer->reload != 0)   
    {
        return QURT_ENOTALLOWED;
    }

    /* Check if the timer is valid */
    if( timer->magic == QURT_TIMER_EXPIRED)  // Expired Timer 
    {
        return QURT_EOK;
    }

    else if( timer->magic == QURT_TIMER_MAGIC)    // Running timer   
    {
        /* Stop an active timer */
        result = qurt_timer_lib_stop_active_timer( timer ); 
    }

    else
    {
        return QURT_ENOTALLOWED;
    }

    /* Return success */
    return result;
    
} /* qurt_timer_lib_timer_stop */

/**
 * Completes the restart-timer operation.
 *
 * @param timer           Pointer to the timer object
 * @param time            Duration or expiry (duration from startup)
 * @return                QURT_EOK if restart is successful, error code otherwise
 */
int qurt_timer_lib_timer_restart ( qurt_timer_client_ptr timer, qurt_timer_duration_t time) 
{
    int result;

    /* Check if the timer is oneshot timer */
    if( timer->reload != 0)   
    {
        return QURT_ENOTALLOWED;
    }

    /* Check if the timer is valid */
    if( timer->magic == QURT_TIMER_EXPIRED)  // Expired Timer 
    {
        /* Restart the expired timer */
        if(timer->type == QURT_TIMER_ABSOLUTE_EXPIRY)
	{
            result = qurt_timer_lib_new( timer, 0, time, 0, timer->group); 
	}
	else  /* QURT_TIMER_RELATIVE_DURATION */
	{
            /* Verify duration with min & max timer duration */
            if ( time < QURT_TIMER_MIN_DURATION || time > QURT_TIMER_MAX_DURATION )
            {
                 return QURT_EINVALID;
            }
            result = qurt_timer_lib_new( timer, time, QURT_TIMER_DEFAULT_EXPIRY, 0, timer->group); 
	}
    }

    else if( timer->magic == QURT_TIMER_MAGIC)    // Running timer   
    {
        /* Restart an active timer */
        result = qurt_timer_lib_restart_active_timer( timer, time ); 
    }

    else
    {
        return QURT_ENOTALLOWED;
    }

    /* Return success */
    return result;
    
} /* qurt_timer_lib_timer_restart */

/**
 * Completes deleting of a timer
 * Note: This function is executed from server side.
 *
 * @param client  Pointer to the client node
 * @return        QURT_EOK if message queue deletion is successful,
 *                error code otherwise
 */
int qurt_timer_lib_timer_cancel ( qurt_timer_client_ptr client )
{
    /* Store result */
    int result;
    bool remove = false;

     /* If this is a ONESHOT timer and expired, remove it */
     if ((0 >= client->reload) && (client->magic == QURT_TIMER_EXPIRED)) {
         remove = true;
     }
     else {
         /* if removing timer from active list successful */
         result = qurt_timer_lib_cancel (client);
         if (result == QURT_EOK) {
            remove = true;
         }
         else if (result == ETIMER_TOOCLOSE) {
            result = QURT_EOK; //in this case, we still return QURT_EOK, but don't free the memory.
         }
     }

     if (remove == true)
     {
         client->magic = QURT_TIMER_INVALID;

         /* Free memory allocated for the client */
				 /* FIXME: This is supposed to check if the pointer is actually in the
						heap; can't use h2_free. But mostly these are not malloced
						anyway. */
         // qurtos_free ( client ); 
         return QURT_EOK;
     }
    
     /* Return result */
     return result;
    
} /* qurt_timer_lib_timer_cancel */

/* void qurt_timer_driver_client (void *arg) */
/* { */
/*    int ret; */
/*    qurt_anysignal_t sclk_signal; */
/*    unsigned int sclk_mask = 1; */

/*    qurt_anysignal_init(&sclk_signal); */

/*    qurt_timer_sclk_id = qurt_sysclock_register (&sclk_signal, sclk_mask); */

/*    if (qurt_timer_sclk_id == QURT_EFATAL) { */
/*       qurtos_printf("Failed to register with QURT sclk driver\n"); */
/*       qurtos_assert(0); */
      
/*    } */

/*    qurt_sem_up(&qurt_timer_client_started); */

/*    do */
/*    { */
/*       ret = qurt_anysignal_wait(&sclk_signal, sclk_mask); */
/*       qurt_anysignal_clear(&sclk_signal, ret); */

/*       qurtos_timer_lock (&qurt_timer_lock); */
/*       qurt_timer_lib_process_active_timers(); */
/*       qurtos_timer_unlock (&qurt_timer_lock); */
      
/*    }while(1); */

/* } */

/* void qurt_timer_driver_client_init (void) */
/* { */
/*    void *stack; */
/*    int qurt_timer_server_prio = QURT_timerIST_priority + 1; */
/*    struct qurtos_thread_info *info = &timerServer_thread_info; */
/*    struct QURT_ugp_ptr *ugp = &timerServer_ugp_area;  */
/*    QURT_utcb_t *current_utcb; */
   
/*    stack = qurtos_malloc(QURT_TIMER_CLIENT_STACK_SIZE); */
/*    if (NULL == stack) */
/*    { */
/*       qurtos_printf("Memory allocation failed\n"); */
/*       qurtos_assert(0); */
/*    } */
/*    // Initialize the synchronizing semaphore between qurt_timerServer & IST */
/*    qurt_sem_init_val(&qurt_timer_client_started, 0); */

/*    qurt_thread_attr_init(&ugp->utcb.attr); */
/*    qurt_thread_attr_set_name(&ugp->utcb.attr, "qurt_timerCli"); */
/*    qurt_thread_attr_set_stack_size(&ugp->utcb.attr, QURT_TIMER_CLIENT_STACK_SIZE); */
/*    qurt_thread_attr_set_stack_addr(&ugp->utcb.attr, stack); */
/*    qurt_thread_attr_set_priority(&ugp->utcb.attr, (unsigned short)qurt_timer_server_prio); */
/*    qurt_thread_attr_set_timetest_id (&ugp->utcb.attr, (unsigned short)(-3)); */

/*    qurt_get_my_utcb(current_utcb); */

/*    ugp->utcb.qdi_info = current_utcb->qdi_info; */
/*    ugp->utcb.asid = 0; */
/*    ugp->utcb.entrypoint = qurt_timer_driver_client; */

/*    qurt_thread_osam_setup(&ugp->utcb.attr); */

/*    qurtos_thread_create(QDI_HANDLE_LOCAL_CLIENT, ugp, qurt_timer_driver_client, info); */

/*    //Wait for Timer IST to finish registering interrupt */
/*    qurt_sem_down(&qurt_timer_client_started); */

/*    qurtos_printf ("QURT Timer Driver client started\n"); */
/* } */

/* int qurtos_timer_create(const qurt_timer_attr_t *attr, */
/*                   const qurt_anysignal_t *signal, unsigned int mask)  */
/* { */
/*     qurt_timer_duration_t duration; */
/*     qurt_timer_time_t expiry; */
/*     qurt_timer_type_t type; */
/*     unsigned int group; */
/*     qurt_timer_t client; */
/*     int result; */
/*     //long long return_val; */

/*     /\* Get the timer duration *\/ */
/*     duration = attr->duration; */
/*     /\* Get the timer expiry *\/ */
/*     expiry = attr->expiry; */
/*     /\* Get the timer type *\/ */
/*     type = attr->type; */
                               
/*     /\* Get the timer group *\/ */
/*     group = attr->group; */

/*     qurtos_timer_lock (&qurt_timer_lock); */
/*     result = qurt_timer_lib_timer_activate ( &client, duration, expiry, type, (qurt_anysignal_t *)signal, mask, group); */
/*     qurtos_timer_unlock (&qurt_timer_lock); */

/*     if (result == QURT_EOK) */
/*         return client; */
/*     else */
/*         return 0; */
/*     //return_val = (long long) result << 32 | client; */
/*     //return return_val; // 17 */
/* } */

/* int qurtos_timer_stop(qurt_timer_t timer)  */
/* { */
/*     qurt_timer_client_ptr node; */
/*     int result; */
    
/*     /\* Get the client pointer *\/ */
/*     node = (qurt_timer_client_ptr)timer;  */
    
/*     /\* Perform the requested operation and return result *\/ */
/*     qurtos_timer_lock (&qurt_timer_lock); */
/*     result = qurt_timer_lib_timer_stop(node); */
/*     qurtos_timer_unlock (&qurt_timer_lock); */
/*     return result; */
/* } */

/* int qurtos_timer_delete(qurt_timer_t timer)  */
/* { */
/*     qurt_timer_client_ptr node; */
/*     int result; */
    
/*     /\* Get the client pointer *\/ */
/*     node = (qurt_timer_client_ptr)timer;  */

/*     /\* Validate Client pointer *\/ */
/*     if ( NULL == node) */
/*     { */
/*         qurtos_printf ("Client timer node is NULL\n" ); */
/*         result = QURT_EINVALID; */
/*     } */
/*     else if (QURT_TIMER_MAGIC != node->magic && QURT_TIMER_EXPIRED != node->magic) */
/*     { */
/*         qurtos_printf ("Client timer node is not valid, magic: 0x%x\n", node->magic); */
/*         result = QURT_EINVALID; */
/*     } */
/*     else */
/*     { */
/*         /\* Perform the requested operation and return result *\/ */
/*         qurtos_timer_lock (&qurt_timer_lock); */
/*         result = qurt_timer_lib_timer_cancel ( node ); */
/*         qurtos_timer_unlock (&qurt_timer_lock); */
/*     } */

/*     return result; // 19 */
/* } */

/* int qurtos_timer_restart(qurt_timer_t timer, qurt_timer_duration_t duration)  */
/* { */
/*     int result; */
    
/*     /\* Perform the requested operation and return result *\/ */
/*     qurtos_timer_lock (&qurt_timer_lock); */
/*     result = qurt_timer_lib_timer_restart( (qurt_timer_client_ptr)timer, duration ); */
/*     qurtos_timer_unlock (&qurt_timer_lock); */
/*     return result; */
/* } */

/* int qurtos_timer_sleep(qurt_timer_duration_t duration, qurt_anysignal_t *signal, unsigned int mask)  */
/* { */
/*     int result; */
    
/*     /\* Perform the requested operation and return result *\/ */
/*     qurtos_timer_lock (&qurt_timer_lock); */
/*     result = qurt_timer_lib_sleep_activate (duration, signal, mask); */
/*     qurtos_timer_unlock (&qurt_timer_lock); */
/*     return result; */
/* } */

/* int qurtos_timer_get_attr(qurt_timer_t timer, qurt_timer_attr_t *attr)  */
/* { */
/*     qurt_timer_client_ptr node; */
/*     int result = 0; */

/*     /\* Get the client pointer *\/ */
/*     node = (qurt_timer_client_ptr)timer; */

/*     /\* Validate Client pointer *\/ */
/*     if ( NULL == node || NULL == attr) */
/*     { */
/*         qurtos_printf ("Client timer node is NULL\n" ); */
/*         result = QURT_EINVALID; */
/*     } */
/*     else if (QURT_TIMER_MAGIC != node->magic && QURT_TIMER_EXPIRED != node->magic) */
/*     { */
/*         qurtos_printf ("Client timer node is not valid, magic: 0x%x\n", node->magic); */
/*         result = QURT_EINVALID; */
/*     } */
/*     else */
/*     { */
/*         /\* Perform the requested operation and return result *\/ */
/*         qurtos_timer_lock (&qurt_timer_lock); */
/*         result = qurt_timer_lib_get_attr ( node, attr ); */
/*         qurtos_timer_unlock (&qurt_timer_lock); */
/*     } */
/*     return result; */
/* } */

/* int qurtos_timer_get_ticks(qurt_timetick_type *ticks)  */
/* { */
/*     int result = 0; */
    
/*     /\* Validate Client pointer *\/ */
/*     if (NULL == ticks) */
/*     { */
/*         qurtos_printf ("Client ticks is NULL\n" ); */
/*         result = QURT_EINVALID; */
/*     } */
/*     else */
/*     { */
/*         qurtos_timer_lock (&qurt_timer_lock); */
/*         *ticks = qurt_timer_lib_current_time (NULL); */
/*         qurtos_timer_unlock (&qurt_timer_lock); */
/*         result = QURT_EOK; */
/*     } */
/*     return result; */
/* } */

/* int qurtos_timer_group_enable(unsigned int group)  */
/* { */
/*     int result; */
    
/*     qurtos_timer_lock (&qurt_timer_lock); */
/*     result = qurt_timer_lib_group_enable (group);     */
/*     qurtos_timer_unlock (&qurt_timer_lock); */
/*     return result; */
/* } */

/* int qurtos_timer_group_disable(unsigned int group)  */
/* { */
/*     int result; */
    
/*     qurtos_timer_lock (&qurt_timer_lock); */
/*     result = qurt_timer_lib_group_disable (group); */
/*     qurtos_timer_unlock (&qurt_timer_lock); */
/*     return result; */
/* } */

/* void qurtos_timer_recover_pc(void)  */
/* { */
/*     qurtos_timer_lock (&qurt_timer_lock); */
/*     qurt_timer_lib_process_active_timers(); */
/*     qurtos_timer_unlock (&qurt_timer_lock); */
/* } */

/* static int qurtos_timer_server_invocation(int client_handle, qurt_qdi_obj_t *obj, int method, */
/*                                           qurt_qdi_arg_t a1, qurt_qdi_arg_t a2, qurt_qdi_arg_t a3, */
/*                                           qurt_qdi_arg_t a4, qurt_qdi_arg_t a5, qurt_qdi_arg_t a6, */
/*                                           qurt_qdi_arg_t a7, qurt_qdi_arg_t a8, qurt_qdi_arg_t a9) */
/* { */
/*    //FIXME: need usermalloc signal and attributes used by timer service. */

/*    switch (method) { */
/*    case QDI_OS_TIMER_CREATE: */
/*       return qurtos_timer_create(a1.ptr, a2.ptr, a3.num); */
/*    case QDI_OS_TIMER_STOP: */
/*       return qurtos_timer_stop(a1.num); */
/*    case QDI_OS_TIMER_DELETE: */
/*       return qurtos_timer_delete(a1.num);  */
/*    case QDI_OS_TIMER_RESTART: */
/*       return qurtos_timer_restart(a1.num, COMBINE64(a2.num, a3.num)); */
/*    case QDI_OS_TIMER_SLEEP: */
/*       return qurtos_timer_sleep(COMBINE64(a1.num, a2.num), a3.ptr, a4.num); */
/*    case QDI_OS_TIMER_GET_ATTR: */
/*       return qurtos_timer_get_attr(a1.num, a2.ptr); */
/*    case QDI_OS_TIMER_GET_TICKS: */
/*       return qurtos_timer_get_ticks(a1.ptr); */
/*    case QDI_OS_TIMER_GROUP_ENABLE: */
/*       return qurtos_timer_group_enable(a1.num); */
/*    case QDI_OS_TIMER_GROUP_DISABLE: */
/*       return qurtos_timer_group_disable(a1.num); */
/*    case QDI_OS_TIMER_RECOVER_PC: */
/*       qurtos_timer_recover_pc(); */
/*       return QURT_EOK; */
/*    default: */
/*       return qurt_qdi_method_default(client_handle, obj, method, */
/*                                      a1, a2, a3, a4, a5, a6, a7, a8, a9); */
/*    } */
/* } */

/* void qurtos_timer_server_generic_init(void) */
/* { */
/*    static const struct qurtos_generic_method_handler list[] = { */
/*       { QDI_OS_TIMER_CREATE, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_STOP, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_DELETE, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_RESTART, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_SLEEP, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_GET_ATTR, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_GET_TICKS, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_GROUP_ENABLE, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_GROUP_DISABLE, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*       { QDI_OS_TIMER_RECOVER_PC, QURTOS_GENERIC_FN(qurtos_timer_server_invocation) }, */
/*    }; */

/*    qurtos_qdi_generic_register_methods(list, QURTOS_ASIZE(list)); */
/* } */
