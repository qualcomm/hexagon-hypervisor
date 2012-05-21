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
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qube.h"
#include "blast_error.h"
#include "qtimer.h"
#include "qTimerLibs.h"
#include "qTimerServer.h"

#define QTIMER_SERV_BUF_COUNT     64
#define QTIMER_SERV_MSG_SIZE      32

extern blast_pipe_t *qtimer_serv_pipe;

blast_mutex_t blast_timer_lock;

/*****************************************************************************/
/*                                                                           */
/*                      FUNCTION DEFINITIONS                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                   Library functions for Server Code                       */
/*                                                                           */
/*****************************************************************************/

/**
 * This function Processes the expired timer
 * Note: May insert messages in message queue, which can cause task switches.
 * 
 * @param timer   Expiring timer to be processed
 * @return        None
 */
void qtimer_lib_expired ( qtimer_client_ptr timer )
{
    if (timer->magic == QTIMER_INVALID)
    {
        /* Free payload if it is a message callback */
        if (timer->cb_data.type == QTIMER_CALLBACK_QMSGQ_TYPE) {
              qmsgq_lib_free_callback ((unsigned int)timer->cb_data.qmsgq);
        }
        free(timer);
        return;
    }

    timer->func (timer); 
} /* qtimer_lib_expired */

/**
 * Completes get attribute function in timer
 * Note: This function is executed from server side.
 *
 * @param client       Pointer to the client node
 * @param attr   [out] Pointer to the attribute object 
 * @return             EOK if message queue deletion is successful,
 *                     error code otherwise
 */
static int qtimer_lib_get_attr ( qtimer_client_ptr client, qtimer_attr_t *attr )
{
    /* absolute time tick elapsed */
    qtimetick_type now;

    /* Duration value in timeticks */
    qtimetick_word_t duration;

    /* Fill the magic number */
    attr->magic        = QTIMER_ATTR_MAGIC;

    /* get current absolute time tick */
    now = qtimer_lib_current_time(NULL);

    if (client->expiry > now)
        /* expiry is in future, set remaining time in us */
        attr->remaining = QTIMER_TIMETICK_TO_US(client->expiry - now);
    else /*expiry has passed */
        attr->remaining = 0;

    /* If this is a PERIODIC timer, reload gives the duration */
    if ( 0 < client->reload )
    {
        /* Set duration in microsections */
        attr->duration = client->reload;

        /* Set the timer type */
        attr->type     = QTIMER_PERIODIC;

    }
    else
    {
        /* Find duration from expiry time and start time */
        duration       = (qtimetick_word_t) (client->expiry - client->start);

        /* Convert timeticks into microsections */
        attr->duration = QTIMER_TIMETICK_TO_US( duration );

        /* Convert timeticks into microsections */
        attr->expiry = QTIMER_TIMETICK_TO_US( client->expiry );

        /* Set the timer type */
        attr->type     = QTIMER_ONESHOT;

    }

    attr->group = client->group;

    return EOK;

} /* qtimer_lib_get_attr */

/**
 * Completes the create sleep timer operation.
 * Note: This function is executed from server side.
 *
 * This function will create a new timer node, insert it in the timer list 
 * and initialize callback for timer expiry.
 *
 * @param duration        Duration of the timer
 * @param to_tid          ID of client thread that is calling qsleep() function
 * @return                EOK if creation is successful, error code otherwise
 */

static int qtimer_lib_sleep_activate (  qtimer_duration_t duration, qtimer_callback_t *callback)
{
    /* Result, success or failure */
    int result; 

    /* Pointer to the new timer */
    qtimer_client_ptr new_timer;

    /* Initialize new timer */
    new_timer = NULL;

    /* Allocate space for new timer */
    new_timer = ( qtimer_client_ptr )malloc( sizeof( qtimer_client ) ); 

    /* Check if the malloc successful */
    if ( NULL == new_timer )
    {
        return EMEM;
    }

    /* Define the Timer Handle */
    new_timer->cb_data.type = callback->type;
    new_timer->cb_data.signal = callback->signal;
    new_timer->cb_data.mask = callback->mask;
    new_timer->func = qtimer_lib_process_sleep_callback;

    /* Perform the actual timer set */
    result = qtimer_lib_new( new_timer, duration, QTIMER_DEFAULT_EXPIRY, 0 /*one-shot type*/, QTIMER_DEFAULT_GROUP); 

    /* Return success */
    return EOK;
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
 * @return                EOK if creation is successful, error code otherwise
 */
static int qtimer_lib_timer_activate ( qtimer_t *client, qtimer_duration_t duration, qtimer_time_t expiry,  
                  qtimer_type_t type, qtimer_callback_t* callback, unsigned int group)
{
    /* Result, success or failure */
    int result;
    /* Reload value in timeticks */
    qtimer_duration_t reload;
    /* Pointer to the new timer */
    qtimer_client_ptr new_timer;
	
    /* Initialize new timer */
    new_timer = NULL;

    /* Allocate space for new timer */
    new_timer = ( qtimer_client_ptr )malloc( sizeof( qtimer_client ) ); 

    /* Check if the malloc successful */
    if ( NULL == new_timer )
    {
        return EMEM;
    }

    new_timer->cb_data.type = callback->type;
    if (callback->type == QTIMER_CALLBACK_QMSGQ_TYPE)
    {
        qmsgq_callback_t *qmsgq_cb;

        /* Initialize user callback */
        if ( EOK != ( result = qmsgq_lib_allocate_callback (callback->qmsgq, (unsigned int *)&qmsgq_cb)))
        {
            return result;
        }
        new_timer->cb_data.qmsgq = qmsgq_cb;
        new_timer->func = qtimer_lib_process_qmsgq_callback;
    }
    /* Default callback is signal based */
    else
    {
        new_timer->cb_data.signal = callback->signal;
        new_timer->cb_data.mask = callback->mask;
        new_timer->func = qtimer_lib_process_signal_callback;
    }

    /* If the timer is one-shot ... */
    if ( QTIMER_ONESHOT == type ) 
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
    result = qtimer_lib_new( new_timer, duration, expiry, reload, group); 
    if(result != EOK)
    {
        /* Free payload if it is a message callback */
        if (new_timer->cb_data.type == QTIMER_CALLBACK_QMSGQ_TYPE) {
            qmsgq_lib_free_callback ((unsigned int)new_timer->cb_data.qmsgq);
        }
        free( new_timer );
	return result;
    }

    /* Return Pointer to the user */
    *client = ( qtimer_t )new_timer;
   
    /* Return success */
    return EOK;
    
} /* qtimer_lib_timer_activate */

/**
 * Completes the operation to stop a timer.
 *
 * @param timer           Pointer to the timer object
 * @return                EOK if restart is successful, error code otherwise
 */
static int qtimer_lib_timer_stop ( qtimer_client_ptr timer) 
{
    int result;

    /* Check if the timer is oneshot timer */
    if( timer->reload != 0)   
    {
        return ENOTALLOWED;
    }

    /* Check if the timer is valid */
    if( timer->magic == QTIMER_EXPIRED)  // Expired Timer 
    {
        return EOK;
    }

    else if( timer->magic == QTIMER_MAGIC)    // Running timer   
    {
        /* Stop an active timer */
        result = qtimer_lib_stop_active_timer( timer ); 
    }

    else
    {
        return ENOTALLOWED;
    }

    /* Return success */
    return result;
    
} /* qtimer_lib_timer_stop */

/**
 * Completes the restart-timer operation.
 *
 * @param timer           Pointer to the timer object
 * @param time            Duration or expiry (duration from startup)
 * @return                EOK if restart is successful, error code otherwise
 */
static int qtimer_lib_timer_restart ( qtimer_client_ptr timer, qtimer_duration_t time) 
{
    int result;

    /* Check if the timer is oneshot timer */
    if( timer->reload != 0)   
    {
        return ENOTALLOWED;
    }

    /* Check if the timer is valid */
    if( timer->magic == QTIMER_EXPIRED)  // Expired Timer 
    {
        /* Restart the expired timer */
        if(timer->type == QTIMER_ABSOLUTE_EXPIRY)
	{
            result = qtimer_lib_new( timer, 0, time, 0, timer->group); 
	}
	else  /* QTIMER_RELATIVE_DURATION */
	{
            /* Verify duration with min & max timer duration */
            if ( time < QTIMER_MIN_DURATION || time > QTIMER_MAX_DURATION )
            {
                 return EINVALID;
            }
            result = qtimer_lib_new( timer, time, QTIMER_DEFAULT_EXPIRY, 0, timer->group); 
	}
    }

    else if( timer->magic == QTIMER_MAGIC)    // Running timer   
    {
        /* Restart an active timer */
        result = qtimer_lib_restart_active_timer( timer, time ); 
    }

    else
    {
        return ENOTALLOWED;
    }

    /* Return success */
    return result;
    
} /* qtimer_lib_timer_restart */

/**
 * Completes deleting of a timer
 * Note: This function is executed from server side.
 *
 * @param client  Pointer to the client node
 * @return        EOK if message queue deletion is successful,
 *                error code otherwise
 */
static int qtimer_lib_timer_cancel ( qtimer_client_ptr client )
{
    /* Store result */
    int result;
    bool remove = false;

     /* If this is a ONESHOT timer and expired, remove it */
     if ((0 >= client->reload) && (client->magic == QTIMER_EXPIRED)) {
         remove = true;
     }
     else {
         /* if removing timer from active list successful */
         result = qtimer_lib_cancel (client);
         if (result == EOK) {
            remove = true;
         }
         else if (result == ETIMER_TOOCLOSE) {
            result = EOK; //in this case, we still return EOK, but don't free the memory.
         }
     }

     if (remove == true)
     {
         /* Free payload if it is a message callback */
         if (client->cb_data.type == QTIMER_CALLBACK_QMSGQ_TYPE) {
             qmsgq_lib_free_callback ((unsigned int)client->cb_data.qmsgq);
         }

         client->magic = QTIMER_INVALID;

         /* Free memory allocated for the client */
         free ( client ); 
         return EOK;
     }
    
     /* Return result */
     return result;
    
} /* qtimer_lib_timer_cancel */

/**
 * Start qTimerServer and wait for ipc messages from the clients
 * Register the thread with iguana naming and wait for l4 ipc from clients
 *
 * @return        Nothing
 */
void qTimerServer ( void *arg)
{
    bool respond;
    /* Pointer to the client */
    qtimer_t *client;
    qtimer_attr_t *attr;
    qtimer_client_ptr node;
    qtimer_duration_t duration;
    qtimer_time_t expiry;
    qtimer_type_t type;
    qtimer_callback_t *callback;
    blast_anysignal_t *ack_signal;
    int group;
    qtimer_cmd_t cmd;

    printf ("BLAST timer server started\n");
    /* Initialize timer lib */
    qtimer_lib_init ( );

    blast_rmutex_init (&blast_timer_lock);

    /* Start the IST thread */
    blast_timer_IST_init ();

    /* Define this as zero to remove warning */
    ack_signal=0;

    while (1) {
        respond = true;
        cmd.raw = blast_pipe_recv (qtimer_serv_pipe);

        blast_rmutex_lock (&blast_timer_lock);

        switch (cmd.cmd_type)
        {
            case QTIMER_IPC_CREATE:
                /* Get the client pointer */
                client = &cmd.create->timer;
                /* Get the ack signal */
                ack_signal = cmd.create->ack;
                /* Get the timer duration */
                duration = cmd.create->duration;
                /* Get the timer expiry */
                expiry = cmd.create->expiry;
                /* Get the timer type */
                type = cmd.create->type;
                /* Get the timer callback */
                callback = &cmd.create->cb;
                /* Get the timer group */
                group =  cmd.create->group;

                /* Validate Client pointer */
                if ( NULL == client )
                {
                    printf (" if ( null == client )\n" );
                    cmd.create->result = EINVALID;
                }
                else
                {
                    /* Perform the requested operation and return result */
                    cmd.create->result = qtimer_lib_timer_activate ( client, duration, expiry, type, callback, group);
                }

                break;

            case QTIMER_IPC_STOP:
                /* Perform the requested operation and return result */
                 cmd.create->result = qtimer_lib_timer_stop( (qtimer_client_ptr)cmd.create->timer);

                /* Get the ack signal */
                ack_signal = cmd.create->ack;

                break;

            case QTIMER_IPC_RESTART:
                /* Perform the requested operation and return result */
                 cmd.create->result = qtimer_lib_timer_restart( (qtimer_client_ptr)cmd.create->timer, cmd.create->duration );

                /* Get the ack signal */
                ack_signal = cmd.create->ack;

                break;
		
            case QTIMER_IPC_DELETE:
                /* Get the client pointer */
                node = (qtimer_client_ptr)cmd.free->timer; 
                /* Get the ack signal */
                ack_signal = cmd.free->ack;

                /* Validate Client pointer */
                if ( NULL == node)
                {
                    printf ("Client timer node is NULL\n" );
                    cmd.free->result = EINVALID;
                }
                else if (QTIMER_MAGIC != node->magic && QTIMER_EXPIRED != node->magic)
                {
                    printf ("Client timer node is not valid, magic: 0x%x\n", node->magic);
                    cmd.free->result = EINVALID;
                }
                else
                {
                    /* Perform the requested operation and return result */
                    cmd.free->result = qtimer_lib_timer_cancel ( node );
                }

                break;
            case QTIMER_IPC_GET_ATTR:
                /* Get the client pointer */
                node = (qtimer_client_ptr)cmd.attr->timer;
                /* Get the ack signal */
                ack_signal= cmd.attr->ack;
                /* Get the attribute pointer */
                attr = cmd.attr->attr;
                /* Validate Client pointer */
                if ( NULL == node)
                {
                     printf ("Client timer node is NULL\n" );
                     cmd.attr->result = EINVALID;
                }
                else if (QTIMER_MAGIC != node->magic && QTIMER_EXPIRED != node->magic)
                {
                    printf ("Client timer node is not valid, magic: 0x%x\n", node->magic);
                    cmd.attr->result = EINVALID;
                }
                else
                {
                    /* Perform the requested operation and return result */
                    cmd.attr->result = qtimer_lib_get_attr ( node, attr );
                }

                break;

            /* SLEEP_TIMER */
            case QTIMER_IPC_SLEEP:
                /* Get the timer callback */
                callback = &cmd.create->cb;
                /* Get the ack signal */
                ack_signal = cmd.create->ack;
                /* Get the timer duration */
                duration = cmd.create->duration;
                
                /* Perform the requested operation and return result */
                cmd.create->result = qtimer_lib_sleep_activate (duration, callback);
                
                break;
            case QTIMER_IPC_GET_TICKS:
                /* Get the ack signal */
                ack_signal = cmd.ticks->ack;

                /* Perform the requested operation and return result */
                cmd.ticks->ticks = qtimer_lib_current_time (NULL);
                cmd.ticks->result = EOK;
                break;
            case QTIMER_IPC_GROUP_EN:
                group = cmd.group->group;
                /* Get the ack signal */
                ack_signal = cmd.group->ack;

                /* Perform the requested operation and return result */
                cmd.group->result =  qtimer_lib_group_enable (group);
                break;
            case QTIMER_IPC_GROUP_DIS:
                group = cmd.group->group;
                /* Get the ack signal */
                ack_signal = cmd.group->ack;

                /* Perform the requested operation and return result */
                cmd.group->result =  qtimer_lib_group_disable (group);
                break;
            case QTIMER_IPC_INTERRUPT:
            case QTIMER_IPC_PC:
                qtimer_lib_isr();
                respond = false;
                break;
            default:
                printf ("Timer server received unknown request\n");
                respond = false;
                /* Release the global lock */
                blast_rmutex_unlock (&blast_timer_lock);
                continue;
        }
        /* Release the global lock */
        blast_rmutex_unlock (&blast_timer_lock);

        if (respond == true) {
           /* The sender of the command is waiting for is the ack. */
           blast_anysignal_set (ack_signal, QTIMER_CLIENT_ACK_SIGMASK);
        }
    }
} /* qTimerServer */
