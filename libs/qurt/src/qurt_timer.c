/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2.h"
#include "qurt.h"
#include <qurtos_internal.h>

qurt_mutex_t qurt_timer_lock;

/**
 * Verify attributes passed to qurt_timer_verify_args
 *
 * The qurt_timer_verify_args is used to verfiy all the input arguments to 
 * qurt_timer_create function. Error checks related to minimum and maximum duration 
 * and type of timer are done in this function
 * function.
 *
 * This function can only be used from "qTimerClient.c" 
 *
 * @param attr        Specifies timer duration and timer type
 * @return            QURT_EOK if creation is successful, error code otherwise
 */
static inline int qurt_timer_verify_args ( const qurt_timer_attr_t *attr )
{
    int rc = QURT_EOK;

    /* If attribute pointer non-null, verify all attributes */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
        {
            rc = QURT_EINVALID;
            qurt_printf ("if ( QURT_TIMER_ATTR_MAGIC != attr->magic )\n");
        }

        /* Verify duration with min & max timer duration */
        if ( QURT_TIMER_MIN_DURATION > attr->duration ||
                QURT_TIMER_MAX_DURATION < (attr->duration ))
        {
            rc = QURT_EINVALID;
            qurt_printf ("if ( QURT_TIMER_MIN_DURATION > attr->duration || QURT_TIMER_MAX_DURATION < (attr->duration ))\n");
        }

        /* Verify timer type */
        if ( QURT_TIMER_ONESHOT != attr->type &&
                QURT_TIMER_PERIODIC < attr->type )
        {
            rc = QURT_EINVALID;
            qurt_printf ("if ( QURT_TIMER_ONESHOT != attr->type && QURT_TIMER_PERIODIC < attr->type )\n");
        }

        /* Verify timer expiry */
        if (attr->type == QURT_TIMER_PERIODIC && attr->expiry != QURT_TIMER_DEFAULT_EXPIRY )
        {
            rc = QURT_EINVALID;
            qurt_printf ("if (attr->type == QURT_TIMER_PERIODIC && attr->expiry != QURT_TIMER_DEFAULT_EXPIRY)\n");
        }

        if (attr->group >= QURT_TIMER_MAX_GROUPS)
        {
            rc = QURT_EINVALID;
            qurt_printf ("if ( attr->group >= QURT_TIMER_MAX_GROUPS)\n");
        }
    } /* if ( null != attr ) */
    else /* Null attributes are not allowed while creating a message queue */
    {
        rc = QURT_EINVALID;
        qurt_printf ("Null attributes are not allowed while creating a qurt_timer\n");
    }

    /* All input arguments are verified */
    return rc;
    
} /* qurt_timer_verify_args */

/*****************************************************************************/
/*                                                                           */
/*                   Functions exposed to the user                           */
/*                                                                           */
/*****************************************************************************/

void qurt_timer_init() {

#ifdef CONFIG_PRIORITY_INHERITANCE
	qurt_pimutex_init(&qurt_timer_lock);
#else
	qurt_rmutex_init(&qurt_timer_lock);
#endif

	qurt_timer_lib_init();
	qurt_sysclock_init();
}

/**
 * Stop a one-shot timer. 
 *
 * @param timer        Timer ID. 
 * @return             QURT_EOK:        Successful stop
 * @return             QURT_EINVALID:   Invalid timer ID
 * @return             QURT_ENOTALLOWED: Timer is not a oneshot timer.
 * @return             QURT_EMEM:        Out of memory error,
 */

int qurt_timer_stop (qurt_timer_t timer )
{

    qurt_timer_client_ptr node;
    int result;
    
    /* Get the client pointer */
    node = (qurt_timer_client_ptr)timer; 
    
    /* Perform the requested operation and return result */
    qurtos_timer_lock (&qurt_timer_lock);
    result = qurt_timer_lib_timer_stop(node);
    qurtos_timer_unlock (&qurt_timer_lock);
    return result;
}

/**
 * Restart a one-shot timer with a duration. 
 *
 * @param timer        Timer ID. 
 * @param time         duration or expiry (dration from startup).
 * @return             QURT_EOK:        Successful restart
 * @return             QURT_EINVALID:   Invalid timer ID or duration value
 * @return             QURT_ENOTALLOWED: Timer is not a oneshot timer.
 * @return             QURT_EMEM:        Out of memory error,
 */

int qurt_timer_restart (qurt_timer_t timer, qurt_timer_duration_t time )
{
    /* Check the timer ID */
	if ( timer == (qurt_timer_t)NULL )
    {
        return QURT_EINVALID;
    }

    int result;
    
    /* Perform the requested operation and return result */
    qurtos_timer_lock (&qurt_timer_lock);
    result = qurt_timer_lib_timer_restart( (qurt_timer_client_ptr)timer, time );
    qurtos_timer_unlock (&qurt_timer_lock);
    return result;
		

}

int qurt_timer_create (qurt_timer_t *timer, const qurt_timer_attr_t *attr,
                  const qurt_anysignal_t *signal, unsigned int mask)
{
    int result;
    qurt_timer_t created_timer;

    /* Verify all the input arguments */
    if(QURT_EOK != (result = qurt_timer_verify_args ( attr ) ) )
    {
        return result;
    }

    qurt_timer_duration_t duration;
    qurt_timer_time_t expiry;
    qurt_timer_type_t type;
    unsigned int group;
    qurt_timer_t client;
    //long long return_val;

    /* Get the timer duration */
    duration = attr->duration;
    /* Get the timer expiry */
    expiry = attr->expiry;
    /* Get the timer type */
    type = attr->type;
                               
    /* Get the timer group */
    group = attr->group;

    qurtos_timer_lock (&qurt_timer_lock);
    result = qurt_timer_lib_timer_activate ( &client, duration, expiry, type, signal, mask, group);
    qurtos_timer_unlock (&qurt_timer_lock);

    if (result == QURT_EOK)
			created_timer = client;
    else
			created_timer = 0;
    //return_val = (long long) result << 32 | client;
    //return return_val; // 17

    
    if (created_timer != 0)
    {
        *timer = created_timer;
        return QURT_EOK;
    }
    else
        return QURT_EUNKNOWN;
}

/**
 * Initialize attribute object with default values.
 *
 * The default values are QURT_TIMER_ONESHOT for timer type and
 * 1ms for timer duration
 *
 * @param attr  Attributes object
 */
void qurt_timer_attr_init ( qurt_timer_attr_t *attr )
{
    /* Initialize attribute data structure */
    attr->magic        = QURT_TIMER_ATTR_MAGIC;
    attr->type         = QURT_TIMER_DEFAULT_TYPE;
    attr->duration     = QURT_TIMER_DEFAULT_DURATION;
    attr->expiry       = QURT_TIMER_DEFAULT_EXPIRY;
    attr->remaining    = QURT_TIMER_DEFAULT_DURATION;
    attr->group        = QURT_TIMER_DEFAULT_GROUP;

    /* Return success */
    return;

} /* qurt_timer_attr_init */

/**
 * Get attributes of a message queue
 *
 * @param msgq         Message queue object
 * @param attr  [OUT]  Message queue attributes
 * @return             QURT_EOK:       get_attr successful,
 *                     EFAILED:   IPC related failures,
 *                     QURT_EINVALID:  Wrong parameters,
 */
int qurt_timer_get_attr(qurt_timer_t timer, qurt_timer_attr_t *attr)
{
    qurt_timer_client_ptr node;
    int result = 0;

    /* Get the client pointer */
    node = (qurt_timer_client_ptr)timer;

    /* Validate Client pointer */
    if ( NULL == node || NULL == attr)
    {
        qurtos_printf ("Client timer node is NULL\n" );
        result = QURT_EINVALID;
    }
    else if (QURT_TIMER_MAGIC != node->magic && QURT_TIMER_EXPIRED != node->magic)
    {
        qurtos_printf ("Client timer node is not valid, magic: 0x%x\n", node->magic);
        result = QURT_EINVALID;
    }
    else
    {
        /* Perform the requested operation and return result */
        qurtos_timer_lock (&qurt_timer_lock);
        result = qurt_timer_lib_get_attr ( node, attr );
        qurtos_timer_unlock (&qurt_timer_lock);
    }
    return result;

}

/**
 * Set duration
 *
 * @param attr      Timer attributes object
 * @param duration  Duration for the timer
 */
void qurt_timer_attr_set_duration(qurt_timer_attr_t *attr, qurt_timer_duration_t duration)
{
    /* Verify Magic number */
    if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
    {
       return;
    }

    /* Check whether Expiry Field has been changed */
    /* qurt_timer_attr_set_duration() and qurt_timer_attr_setexpiry() are mutual exlusive */ 
    if ( QURT_TIMER_DEFAULT_EXPIRY != attr->expiry )
    {
       return;
    }

    /* Set duration in the attribute */
    attr->duration = duration;
    attr->remaining = duration;

    /* Return success */
    return;

} /* qurt_timer_attr_set_duration */

/**
 * Set expiry
 *
 * @param attr      Timer attributes object
 * @param time      Absolute expiry time for the timer in micro-seconds
 */
void qurt_timer_attr_set_expiry(qurt_timer_attr_t *attr, qurt_timer_time_t time)
{
    /* Verify Magic number */
    if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
    {
       return;
    }

    /* Check whether Duration Field has been changed */
    /* qurt_timer_attr_set_duration() and qurt_timer_attr_setexpiry() are mutual exlusive */ 
    if ( QURT_TIMER_DEFAULT_DURATION != attr->duration )
    {
       return;
    }

    /* Set Expiry in the attribute */
    attr->expiry = time;

    /* Return success */
    return;

} /* qurt_timer_attr_set_duration */

/**
 * Get duration
 *
 * @param attr      Timer attributes object
 * @param duration  Duration for the timer
 */
void qurt_timer_attr_get_duration(qurt_timer_attr_t *attr, qurt_timer_duration_t *duration)
{
    /* If attribute pointer non-null, verify attribute pointer */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
        {
            return;
        }

        /* Duration should be non-null */
        if ( NULL == duration )
        {
            return;
        }

        /* Return duration */
        *duration = attr->duration;

        /* Return success */
        return;

    } /* if ( null != attr ) */

    /* Attribute pointer is not valid */
    return;

} /* qurt_timer_attr_get_duration */

/**
 * Get remaining time
 *
 * @param attr          Timer attributes object
 * @param remaining     Remaining time for the timer
 */
void qurt_timer_attr_get_remaining(qurt_timer_attr_t *attr, qurt_timer_duration_t *remaining)
{
    /* If attribute pointer non-null, verify attribute pointer */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
        {
            return;
        }

        /* Duration should be non-null */
        if ( NULL == remaining )
        {
            return;
        }

        /* Return duration */
        *remaining = attr->remaining;

        /* Return success */
        return;

    } /* if ( null != attr ) */

    /* Attribute pointer is not valid */
    return;

} /* qurt_timer_attr_get_remaining */

/**
 * Set timer type - one shot or periodic
 *
 * @param attr   Timer attributes object
 * @param type   Timer type
 */
void qurt_timer_attr_set_type(qurt_timer_attr_t *attr, qurt_timer_type_t type)
{
    /* Verify Magic number */
    if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
    {
       return;
    }
    
    /* Set timer type in the attribute */
    attr->type = type;

    /* Return success */
    return;

} /* qurt_timer_attr_set_type */

/**
 * Set duration
 *
 * @param attr  Timer attributes object
 * @param type  Timer type
 */
void qurt_timer_attr_get_type(qurt_timer_attr_t *attr, qurt_timer_type_t *type)
{
    /* If attribute pointer non-null, verify attribute pointer */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
        {
            return;
        }

        /* type should be non-null */
        if ( NULL == type )
        {
            return;
        }

        /* Return duration */
        *type = attr->type;

        /* Return success */
        return;

    } /* if ( null != attr ) */

    /* Attribute pointer is not valid */
    return;

} /* qurt_timer_attr_get_type */

/**
 * Set timer group, ranging 0 to QURT_TIMER_MAX_GROUPS - 1
 *
 * @param attr   Timer attributes object
 * @param group  Group ID
 */
void qurt_timer_attr_set_group (qurt_timer_attr_t *attr, unsigned int group)
{
    /* Verify Magic number */
    if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
    {
       return;
    }
    
    /* Set timer type in the attribute */
    attr->group = group;

    /* Return success */
    return;

} /* qurt_timer_attr_set_group */

/**
 * Get the group ID from the attribute
 *
 * @param attr  Timer attributes object
 * @param type  Timer type
 */
void qurt_timer_attr_get_group (qurt_timer_attr_t *attr, unsigned int *group)
{
    /* If attribute pointer non-null, verify attribute pointer */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QURT_TIMER_ATTR_MAGIC != attr->magic )
        {
            return;
        }

        /* Read group ID */
        *group = attr->group;

        /* Return success */
        return;

    } /* if ( null != attr ) */

    /* Attribute pointer is not valid */
    return;

} /* qurt_timer_attr_get_group */

/**
 * Deletes timer
 *
 * @param timer  Pointer to timer object
 * @return       QURT_EOK:       Successful create,
 *               EFAILED:   IPC related failures,
 *               QURT_EINVALID:  Wrong timer 
 */
int qurt_timer_delete(qurt_timer_t timer)
{
    qurt_timer_client_ptr node;
    int result;
    
    /* Get the client pointer */
    node = (qurt_timer_client_ptr)timer; 

    /* Validate Client pointer */
    if ( NULL == node)
    {
        qurtos_printf ("Client timer node is NULL\n" );
        result = QURT_EINVALID;
    }
    else if (QURT_TIMER_MAGIC != node->magic && QURT_TIMER_EXPIRED != node->magic)
    {
        qurtos_printf ("Client timer node is not valid, magic: 0x%x\n", node->magic);
        result = QURT_EINVALID;
    }
    else
    {
        /* Perform the requested operation and return result */
        qurtos_timer_lock (&qurt_timer_lock);
        result = qurt_timer_lib_timer_cancel ( node );
        qurtos_timer_unlock (&qurt_timer_lock);
    }

    return result; // 19

} /* qurt_timer_delete */

/* SLEEP_TIMER */
/**
 * Sleep function.
 *  
 * Cause the calling thread to be suspended from execution
 * until the specified duration has elapsed.
 *
 * @param duration  number of us to sleep for 
 * @return             QURT_EOK:       Successful create,
 *                     QURT_EMEM:      Out of memory error,
 *                     EFAILED:   IPC related failures,
 *                     EVAL:      Wrong duration value.
 */
int qurt_timer_sleep ( qurt_timer_duration_t duration )
{
    qurt_anysignal_t cb_signal;
    int result;
    
    /* Verify duration with min & max timer duration */
    if ( QURT_TIMER_MIN_DURATION > duration ||
            QURT_TIMER_MAX_DURATION < duration )
    {
        qurt_printf ("if ( QURT_TIMER_MIN_DURATION > duration || QURT_TIMER_MAX_DURATION < duration )\n");
        return QURT_EINVALID;
    }

    /* Initialize the qurt signal */
    qurt_anysignal_init (&cb_signal);
    
    /* Perform the requested operation and return result */
    qurtos_timer_lock (&qurt_timer_lock);
    result = qurt_timer_lib_sleep_activate (duration, &cb_signal, QURT_TIMER_CLIENT_ACK_SIGMASK);
    qurtos_timer_unlock (&qurt_timer_lock);
    
    if (result != QURT_EOK)
        return result;        
    
    qurt_anysignal_wait (&cb_signal, QURT_TIMER_CLIENT_ACK_SIGMASK);
    /* Release the resources allocated by the signal */
    qurt_anysignal_destroy (&cb_signal);
    return QURT_EOK;
    
} /* qurt_timer_attr_init */

unsigned long long qurt_timer_get_ticks (void)
{
    qurt_timetick_type ticks;
    
		qurtos_timer_lock (&qurt_timer_lock);
		ticks = qurt_timer_lib_current_time (NULL);
		qurtos_timer_unlock (&qurt_timer_lock);

    return ticks;
}

int qurt_timer_group_enable (unsigned int group)
{
    int result;
    
    qurtos_timer_lock (&qurt_timer_lock);
    result = qurt_timer_lib_group_enable (group);    
    qurtos_timer_unlock (&qurt_timer_lock);
    return result;

}

int qurt_timer_group_disable (unsigned int group)
{
    int result;
    
    qurtos_timer_lock (&qurt_timer_lock);
    result = qurt_timer_lib_group_disable (group);
    qurtos_timer_unlock (&qurt_timer_lock);
    return result;
}

void qurt_timer_recover_pc (void) {

    qurtos_timer_lock (&qurt_timer_lock);
    qurt_timer_lib_process_active_timers();
    qurtos_timer_unlock (&qurt_timer_lock);

}
