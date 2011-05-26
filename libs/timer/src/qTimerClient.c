/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*****************************************************************************/
/*                                                                           */
/*               Q D S P 6   T I M E R  S U B S Y S T E M                    */
/*                                                                           */
/* GENERAL DESCRIPTION                                                       */
/* Implements timer type functionality. This header files will be part of    */
/* the qTimerClient.c code.                                                  */
/*                                                                           */
/* FUNCTIONS                                                                 */
/*                                                                           */
/* INITIALIZATION AND SEQUENCING REQUIREMENTS                                */
/*                                                                           */

/*                                                                           */
/*****************************************************************************/

/************************ COPYRIGHT NOTICE ***********************************/

/* All data and information contained in or disclosed by this                */
/* document is confidential and proprietary information of                   */
/* QUALCOMM, Inc and all rights therein are expressly reserved.              */
/* By accepting this material the recipient agrees that this                 */
/* material and the information contained therein is held in                 */
/* confidence and in trust and will not be used, copied,                     */
/* reproduced in whole or in part, nor its contents revealed in              */
/* any manner to others without the express written permission               */
/* of QUALCOMM, Inc.                                                         */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                           INCLUDE FILES                                   */
/*                                                                           */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <qube.h>
#include <qtimer.h>
#include "qTimerDefines.h"

/*****************************************************************************/
/*                                                                           */
/*                          DATA DECLARATIONS                                */
/*                                                                           */
/*****************************************************************************/
extern blast_pipe_t *qtimer_serv_pipe;
extern blast_mutex_t blast_timer_lock;

/*****************************************************************************/
/*                                                                           */
/*                      FUNCTION DEFINITIONS                                 */
/*                                                                           */
/*****************************************************************************/
/**
 * Verify attributes passed to qtimer_verify_args
 *
 * The qtimer_verify_args is used to verfiy all the input arguments to 
 * qtimer_create function. Error checks related to minimum and maximum duration 
 * and type of timer are done in this function
 * function.
 *
 * This function can only be used from "qTimerClient.c" 
 *
 * @param attr        Specifies timer duration and timer type
 * @return            EOK if creation is successful, error code otherwise
 */
static inline int qtimer_verify_args ( const qtimer_attr_t *attr )
{
    int rc = EOK;

    /* If attribute pointer non-null, verify all attributes */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QTIMER_ATTR_MAGIC != attr->magic )
        {
            rc = EINVALID;
            printf ("if ( QTIMER_ATTR_MAGIC != attr->magic )\n");
        }

        /* Verify duration with min & max timer duration */
        if ( QTIMER_MIN_DURATION > attr->duration ||
                QTIMER_MAX_DURATION < (attr->duration ))
        {
            rc = EINVALID;
            printf ("if ( QTIMER_MIN_DURATION > attr->duration || QTIMER_MAX_DURATION < (attr->duration ))\n");
        }

        /* Verify timer type */
        if ( QTIMER_ONESHOT != attr->type &&
                QTIMER_PERIODIC < attr->type )
        {
            rc = EINVALID;
            printf ("if ( QTIMER_ONESHOT != attr->type && QTIMER_PERIODIC < attr->type )\n");
        }

        /* Verify timer expiry */
        if (attr->type == QTIMER_PERIODIC && attr->expiry != QTIMER_DEFAULT_EXPIRY )
        {
            rc = EINVALID;
            printf ("if (attr->type == QTIMER_PERIODIC && attr->expiry != QTIMER_DEFAULT_EXPIRY)\n");
        }

        if (attr->group >= QTIMER_MAX_GROUPS)
        {
            rc = EINVALID;
            printf ("if ( attr->group >= QTIMER_MAX_GROUPS)\n");
        }
    } /* if ( null != attr ) */
    else /* Null attributes are not allowed while creating a message queue */
    {
        rc = EINVALID;
        printf ("Null attributes are not allowed while creating a qtimer\n");
    }

    /* All input arguments are verified */
    return rc;
    
} /* qtimer_verify_args */

/*****************************************************************************/
/*                                                                           */
/*                   Library functions for Client Code                       */
/*                                                                           */
/*****************************************************************************/
/**
 * Completes the timer create and activate operation.
 * Note: This function is executed from client side.
 *
 * This function will send an IPC to the server to complete the create request.
 *
 * @param timer       Address where the timer information is stored in client code 
 * @param duration    Duration of the new timer
 * @param expiry      Expiry of the new timer
 * @param type        Type of the new timer
 * @param cb          Pointer to the message queue callback.
 * @return            EOK if creation is successful, error code otherwise
 */
static inline int qtimer_lib_timer_create ( qtimer_t *timer, qtimer_duration_t duration, qtimer_time_t expiry,
                         qtimer_type_t type, const qmsgq_callback_t *cb, unsigned int group)
{
    qtimer_cmd_t cmd;
    qtimer_cmd_create_t create;

    /* Get the blast signal */
    create.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize IPC parameters */
    create.duration = duration;
    create.expiry = expiry;
    create.cb.type = QTIMER_CALLBACK_QMSGQ_TYPE;
    create.cb.qmsgq = (qmsgq_callback_t *)cb;
    create.type = type;
    create.group = group;

    cmd.cmd_type = QTIMER_IPC_CREATE;
    cmd.create = &create;
    /* Send IPC and return response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (create.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (create.ack, QTIMER_CLIENT_ACK_SIGMASK);
    *timer = create.timer;
    return create.result;
} /* qtimer_lib_timer_create */

static inline int sclk_timer_lib_timer_create ( qtimer_t *timer, qtimer_duration_t duration, qtimer_time_t expiry,  
                         qtimer_type_t type, const blast_anysignal_t *signal, unsigned int mask, unsigned int group)
{
    qtimer_cmd_t cmd;
    qtimer_cmd_create_t create;

    /* Get the blast signal */
    create.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize IPC parameters */
    create.duration = duration;
    create.expiry = expiry;
    create.cb.type = QTIMER_CALLBACK_SIG_TYPE;
    create.cb.signal = (blast_anysignal_t *)signal;
    create.cb.mask = mask;
    create.type = type;
    create.group = group;

    cmd.cmd_type = QTIMER_IPC_CREATE;
    cmd.create = &create;
    /* Send IPC and return response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (create.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (create.ack, QTIMER_CLIENT_ACK_SIGMASK);
    *timer = create.timer;
    return create.result;
} /* qtimer_lib_timer_create */

/**
 * Completes deleting an active timer
 * Note: This function is executed from client side.
 *
 * @param timer   Timer to be freed
 * @return        EOK if timer exists and free is successful,
 *                error code otherwise
 */
static inline int qtimer_lib_timer_free ( qtimer_t timer )
{
    qtimer_cmd_t cmd;
    qtimer_cmd_free_t free;

    /* Get the blast signal */
    free.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize IPC parameters */
    free.timer = timer;
    
    cmd.cmd_type = QTIMER_IPC_DELETE;
    cmd.free = &free;
    /* Delete timer by sending IPC to timer_server and
     * waiting for response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (free.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (free.ack, QTIMER_CLIENT_ACK_SIGMASK);
    return free.result;
   
} /* qtimer_lib_timer_free */

/*****************************************************************************/
/*                                                                           */
/*                   Functions exposed to the user                           */
/*                                                                           */
/*****************************************************************************/
/**
 * Creates a timer with specific attributes
 *
 * @param timer  [OUT] Timer object
 * @param attr         Specifies timer duration and timer type (
 *                     one-shot or periodic)
 * @param cb           Message queue and message sent when timer expires
 * @return             EOK:       Successful create,
 *                     EMEM:      Out of memory error,
 *                     EFAILED:   IPC related failures,
 *                     EINVALID:  Wrong attribute pointer, null client pointer,
 *                                wrong callback
 *                     EVAL:      Wrong values for parameters like 
 *                                duration outside range, timer type not valid.
 */
int qtimer_create(qtimer_t *timer, const qtimer_attr_t *attr,
                  const qmsgq_callback_t *cb)
{
    int result;

    /* Verify all the input arguments */
    if(EOK != (result = qtimer_verify_args ( attr ) ) )
    {
        return result;
    }

    /* Send IPC to timer_server and wait for response */
    return qtimer_lib_timer_create ( timer, attr->duration, attr->expiry, attr->type, cb, attr->group);

}

/**
 * Stop a one-shot timer. 
 *
 * @param timer        Timer ID. 
 * @return             EOK:        Successful stop
 * @return             EINVALID:   Invalid timer ID
 * @return             ENOTALLOWED: Timer is not a oneshot timer.
 * @return             EMEM:        Out of memory error,
 */

int qtimer_stop (qtimer_t timer )
{
    qtimer_cmd_t cmd;
    qtimer_cmd_create_t create;

    /* Check the timer ID */
    if ( timer == NULL )
    {
        return EINVALID;
    }

    /* Get the blast signal */
    create.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize IPC parameters */
    create.timer = timer;

    cmd.cmd_type = QTIMER_IPC_STOP;
    cmd.create = &create;
    /* Send IPC and return response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (create.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (create.ack, QTIMER_CLIENT_ACK_SIGMASK);

    return create.result;
}

/**
 * Restart a one-shot timer with a duration. 
 *
 * @param timer        Timer ID. 
 * @param time         duration or expiry (dration from startup).
 * @return             EOK:        Successful restart
 * @return             EINVALID:   Invalid timer ID or duration value
 * @return             ENOTALLOWED: Timer is not a oneshot timer.
 * @return             EMEM:        Out of memory error,
 */

int qtimer_restart (qtimer_t timer, qtimer_duration_t time )
{
    qtimer_cmd_t cmd;
    qtimer_cmd_create_t create;

    /* Check the timer ID */
    if ( timer == NULL )
    {
        return EINVALID;
    }

    /* Get the blast signal */
    create.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize IPC parameters */
    create.timer = timer;
    create.duration = time;

    cmd.cmd_type = QTIMER_IPC_RESTART;
    cmd.create = &create;
    /* Send IPC and return response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (create.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (create.ack, QTIMER_CLIENT_ACK_SIGMASK);

    return create.result;
}

int sclk_timer_create (qtimer_t *timer, const qtimer_attr_t *attr,
                  const blast_anysignal_t *signal, unsigned int mask)
{
    int result;

    /* Verify all the input arguments */
    if(EOK != (result = qtimer_verify_args ( attr ) ) )
    {
        return result;
    }

    /* Send IPC to timer_server and wait for response */
    return sclk_timer_lib_timer_create (timer, attr->duration, attr->expiry, attr->type, signal, mask, attr->group);

}

int sclk_timer_stop (qtimer_t timer )
{
     /* It should be okay just calling the qtimer_stop() */
     return qtimer_stop (timer);
}

int sclk_timer_restart (qtimer_t timer, qtimer_duration_t time )
{
     /* It should be okay just calling the qtimer_restart() */
     return qtimer_restart (timer, time );
}

/**
 * Initialize attribute object with default values.
 *
 * The default values are QTIMER_ONESHOT for timer type and
 * 1ms for timer duration
 *
 * @param attr  Attributes object
 */
void qtimer_attr_init ( qtimer_attr_t *attr )
{
    /* Initialize attribute data structure */
    attr->magic        = QTIMER_ATTR_MAGIC;
    attr->type         = QTIMER_DEFAULT_TYPE;
    attr->duration     = QTIMER_DEFAULT_DURATION;
    attr->expiry       = QTIMER_DEFAULT_EXPIRY;
    attr->remaining    = QTIMER_DEFAULT_DURATION;
    attr->group        = QTIMER_DEFAULT_GROUP;

    /* Return success */
    return;

} /* qtimer_attr_init */

/**
 * Get attributes of a message queue
 *
 * @param msgq         Message queue object
 * @param attr  [OUT]  Message queue attributes
 * @return             EOK:       get_attr successful,
 *                     EFAILED:   IPC related failures,
 *                     EINVALID:  Wrong parameters,
 */
int qtimer_get_attr(qtimer_t timer, qtimer_attr_t *attr)
{
    qtimer_cmd_t cmd;
    qtimer_cmd_getattr_t get_attr;

    /* Get the blast signal */
    get_attr.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize IPC parameters */
    get_attr.timer = timer;
    get_attr.attr = attr;
    
    cmd.cmd_type = QTIMER_IPC_GET_ATTR;
    cmd.attr = &get_attr;
    /* Delete timer by sending IPC to timer_server and
     * waiting for response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (get_attr.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (get_attr.ack, QTIMER_CLIENT_ACK_SIGMASK);
    return get_attr.result;
}

/**
 * Set duration
 *
 * @param attr      Timer attributes object
 * @param duration  Duration for the timer
 */
void qtimer_attr_setduration(qtimer_attr_t *attr, qtimer_duration_t duration)
{
    /* Verify Magic number */
    if ( QTIMER_ATTR_MAGIC != attr->magic )
    {
       return;
    }

    /* Check whether Expiry Field has been changed */
    /* qtimer_attr_setduration() and qtimer_attr_setexpiry() are mutual exlusive */ 
    if ( QTIMER_DEFAULT_EXPIRY != attr->expiry )
    {
       return;
    }

    /* Set duration in the attribute */
    attr->duration = duration;
    attr->remaining = duration;

    /* Return success */
    return;

} /* qtimer_attr_setduration */

/**
 * Set expiry
 *
 * @param attr      Timer attributes object
 * @param time      Absolute expiry time for the timer in micro-seconds
 */
void qtimer_attr_setexpiry(qtimer_attr_t *attr, qtimer_time_t time)
{
    /* Verify Magic number */
    if ( QTIMER_ATTR_MAGIC != attr->magic )
    {
       return;
    }

    /* Check whether Duration Field has been changed */
    /* qtimer_attr_setduration() and qtimer_attr_setexpiry() are mutual exlusive */ 
    if ( QTIMER_DEFAULT_DURATION != attr->duration )
    {
       return;
    }

    /* Set Expiry in the attribute */
    attr->expiry = time;

    /* Return success */
    return;

} /* qtimer_attr_setduration */

/**
 * Get duration
 *
 * @param attr      Timer attributes object
 * @param duration  Duration for the timer
 */
void qtimer_attr_getduration(qtimer_attr_t *attr, qtimer_duration_t *duration)
{
    /* If attribute pointer non-null, verify attribute pointer */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QTIMER_ATTR_MAGIC != attr->magic )
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

} /* qtimer_attr_getduration */

/**
 * Get remaining time
 *
 * @param attr          Timer attributes object
 * @param remaining     Remaining time for the timer
 */
void qtimer_attr_getremaining(qtimer_attr_t *attr, qtimer_duration_t *remaining)
{
    /* If attribute pointer non-null, verify attribute pointer */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QTIMER_ATTR_MAGIC != attr->magic )
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

} /* qtimer_attr_getremaining */

/**
 * Set timer type - one shot or periodic
 *
 * @param attr   Timer attributes object
 * @param type   Timer type
 */
void qtimer_attr_settype(qtimer_attr_t *attr, qtimer_type_t type)
{
    /* Verify Magic number */
    if ( QTIMER_ATTR_MAGIC != attr->magic )
    {
       return;
    }
    
    /* Set timer type in the attribute */
    attr->type = type;

    /* Return success */
    return;

} /* qtimer_attr_settype */

/**
 * Set duration
 *
 * @param attr  Timer attributes object
 * @param type  Timer type
 */
void qtimer_attr_gettype(qtimer_attr_t *attr, qtimer_type_t *type)
{
    /* If attribute pointer non-null, verify attribute pointer */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QTIMER_ATTR_MAGIC != attr->magic )
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

} /* qtimer_attr_gettype */

/**
 * Set timer group, ranging 0 to QTIMER_MAX_GROUPS - 1
 *
 * @param attr   Timer attributes object
 * @param group  Group ID
 */
void qtimer_attr_setgroup (qtimer_attr_t *attr, unsigned int group)
{
    /* Verify Magic number */
    if ( QTIMER_ATTR_MAGIC != attr->magic )
    {
       return;
    }
    
    /* Set timer type in the attribute */
    attr->group = group;

    /* Return success */
    return;

} /* qtimer_attr_settype */

/**
 * Get the group ID from the attribute
 *
 * @param attr  Timer attributes object
 * @param type  Timer type
 */
void qtimer_attr_getgroup (qtimer_attr_t *attr, unsigned int *group)
{
    /* If attribute pointer non-null, verify attribute pointer */
    if ( NULL != attr ) 
    {
        /* Verify Magic number */
        if ( QTIMER_ATTR_MAGIC != attr->magic )
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

} /* qtimer_attr_getgroup */

/**
 * Deletes timer
 *
 * @param timer  Pointer to timer object
 * @return       EOK:       Successful create,
 *               EFAILED:   IPC related failures,
 *               EINVALID:  Wrong timer 
 */
int qtimer_delete(qtimer_t timer)
{
    /* Send IPC to timer_server and wait for response */
    return qtimer_lib_timer_free ( timer );
    
} /* qtimer_delete */

/* SLEEP_TIMER */
/**
 * Sleep function.
 *  
 * Cause the calling thread to be suspended from execution
 * until the specified duration has elapsed.
 *
 * @param duration  number of us to sleep for 
 * @return             EOK:       Successful create,
 *                     EMEM:      Out of memory error,
 *                     EFAILED:   IPC related failures,
 *                     EVAL:      Wrong duration value.
 */
int qtimer_sleep ( qtimer_duration_t duration )
{
    /* Parameters to send to message queue server */
    long long unsigned int data[4]; /* Cache line. Aligned for fast send */
    unsigned int *data_ptr = (unsigned int *)data;
    blast_anysignal_t cb_signal;
    //qtimer_callback_t cb;
    int rc, result;
    qtimer_cmd_t cmd;
    qtimer_cmd_create_t create;

    /* Verify duration with min & max timer duration */
    if ( QTIMER_MIN_DURATION > duration ||
            QTIMER_MAX_DURATION < duration )
    {
        printf ("if ( QTIMER_MIN_DURATION > duration || QTIMER_MAX_DURATION < duration )\n");
        return EINVALID;
    }

    /* Get the blast signal */
    create.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize the blast signal */
    blast_anysignal_init (&cb_signal);

    /* Initialize IPC parameters */
    create.cb.type = QTIMER_CALLBACK_SIG_TYPE;
    create.cb.signal = &cb_signal;
    create.cb.mask = QTIMER_CLIENT_ACK_SIGMASK;
    create.duration = duration;

    cmd.cmd_type = QTIMER_IPC_SLEEP;
    cmd.create = &create;
    /* suspending calling thread by sending IPC to timer_server and
     * waiting for response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (create.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (create.ack, QTIMER_CLIENT_ACK_SIGMASK);
    if (create.result != EOK)
        return create.result;

    blast_anysignal_wait (&cb_signal, QTIMER_CLIENT_ACK_SIGMASK);
    /* Release the resources allocated by the signal */
    blast_anysignal_destroy (&cb_signal);
    return EOK;
} /* qtimer_attr_init */

unsigned long long qsystem_sclk_attr_getticks (void)
{
    qtimer_cmd_t cmd;
    qtimer_cmd_getticks_t ticks;

    /* Get the blast signal */
    ticks.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize IPC parameters */
    cmd.cmd_type = QTIMER_IPC_GET_TICKS;
    cmd.ticks = &ticks;
    /* suspending calling thread by sending IPC to timer_server and
     * waiting for response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (ticks.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (ticks.ack, QTIMER_CLIENT_ACK_SIGMASK);
    if (ticks.result == EOK) {
        return ticks.ticks;
    }

    return 0;
}

int qtimer_group_enable (unsigned int group)
{
    qtimer_cmd_t cmd;
    qtimer_cmd_group_t group_cmd;

    /* Get the blast signal */
    group_cmd.ack = (blast_anysignal_t *) blast_get_my_anysignal();

    /* Initialize IPC parameters */
    group_cmd.group = group;

    cmd.cmd_type = QTIMER_IPC_GROUP_EN;
    cmd.group = &group_cmd;
    /* suspending calling thread by sending IPC to timer_server and
     * waiting for response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (group_cmd.ack, QTIMER_CLIENT_ACK_SIGMASK);
    blast_anysignal_clear (group_cmd.ack, QTIMER_CLIENT_ACK_SIGMASK);
    return group_cmd.result;
}

int qtimer_group_disable (unsigned int group)
{
    /* This function needs to be called in single threaded mode.
     * Hence it can't cause context switch */
#if 0
    qtimer_cmd_t cmd;
    qtimer_cmd_group_t group_cmd;

    /* Initialize the blast signal */
    blast_anysignal_init (&group_cmd.ack);

    cmd.cmd_type = QTIMER_IPC_GROUP_DIS;
    cmd.group = &group_cmd;
    /* Initialize IPC parameters */
    group_cmd.group = group;

    /* suspending calling thread by sending IPC to timer_server and
     * waiting for response */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
    blast_anysignal_wait (&group_cmd.ack, QTIMER_CLIENT_ACK_SIGMASK);
    /* Release the resources allocated by the signal */
    blast_anysignal_destroy (&group_cmd.ack);
    return group_cmd.result;
#else /* 0 */
    extern int qtimer_lib_group_disable (unsigned int group);

    blast_rmutex_lock (&blast_timer_lock);
    qtimer_lib_group_disable (group);
    blast_rmutex_unlock (&blast_timer_lock);
#endif /* 0 */
}

void qtimer_recover_pc (void)
{
    qtimer_cmd_t cmd;

    cmd.cmd_type = QTIMER_IPC_PC;
    cmd.group = NULL;

    /* Send the message to Timer Server */
    blast_pipe_send (qtimer_serv_pipe, cmd.raw);
}

unsigned int qsystem_sclk_attr_getexpiry (void)
{
   unsigned int expiry;
   extern unsigned int hw_timer_match_val (void);

   expiry = hw_timer_match_val () - hw_timer_curr_timetick ();

   return expiry;
}

unsigned int blast_system_sclk_attr_gethwticks (void)
{
    unsigned int count;

    count = hw_timer_curr_timetick ();

    return count;
}
