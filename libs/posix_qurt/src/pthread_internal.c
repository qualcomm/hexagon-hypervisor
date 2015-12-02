/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <string.h> //memset
#include <pthread.h>
#include "pthread_internal.h"

pthread_id_table_t pthread_id_table;
unsigned long long pthread_objs[PTHREAD_MAX_THREADS]; //Each id has two words

/* the tls key used for store pthread tcb pointer */
int pthread_tcb_key;

typedef struct ltl   ltl;
struct ltl
{
    ltl       *next;
    pthread_i *ltcb;
};

static ltl             *ltl_head = NULL;
static pthread_mutex_t ltl_mutex  = PTHREAD_MUTEX_INITIALIZER;

int ltl_insert(pthread_i *ltcb)
{
    ltl * ltl_node;

    if (!ltcb)
        goto fail;

    if (0 != pthread_mutex_lock(&ltl_mutex))
        goto fail;

    ltl_node = (ltl *) malloc(sizeof(ltl));
    if (!ltl_node)
        goto fail;

    ltl_node->next = ltl_head;
    ltl_node->ltcb = ltcb;
    ltl_head       = ltl_node;

    if (0 != pthread_mutex_unlock(&ltl_mutex))
        goto fail;

    return 0;

 fail:
    return -1;
}

pthread_i * ltl_search(int qurtid)
{
    ltl * ltl_tmp_node;

    if (!qurtid)
        qurtid = qurt_thread_get_id();

    if (0 != pthread_mutex_lock(&ltl_mutex))
        goto fail;

    ltl_tmp_node = ltl_head;
    while (ltl_tmp_node)
    {
        if (ltl_tmp_node->ltcb)
        {
            if (ltl_tmp_node->ltcb->pthread == qurtid)
            {
                if (0 != pthread_mutex_unlock(&ltl_mutex))
                    goto fail;
                return ltl_tmp_node->ltcb;
            }
        }
        ltl_tmp_node = ltl_tmp_node->next;
    }

    if (0 != pthread_mutex_unlock(&ltl_mutex))
        goto fail;

 fail:
    return NULL;
}

int ltl_delete(int qurtid)
{
    ltl * ltl_del_node = ltl_head;
    ltl * ltl_tmp_node = 0;

    if (!qurtid)
        qurtid = qurt_thread_get_id();

    if (0 != pthread_mutex_lock(&ltl_mutex))
        goto fail;

    while (ltl_del_node)
    {
        if (ltl_del_node->ltcb->pthread == qurtid)
        {
            if (!ltl_tmp_node) // deleting the first node
            {
                ltl_head = ltl_del_node->next;
            }
            else
            {
                ltl_tmp_node->next = ltl_del_node->next;
            }
            free(ltl_del_node);

            if (0 != pthread_mutex_unlock(&ltl_mutex))
                goto fail;

            return 0;
        }
        ltl_tmp_node = ltl_del_node;
        ltl_del_node = ltl_del_node->next;
    }

    if (0 != pthread_mutex_unlock(&ltl_mutex))
        goto fail;

 fail:
    return -1;
}

int _getltcb(pthread_i **ltcb, pthread_t pthreadid)
{
    qurt_obj_t obj;

    /* do not check NULL pointer since it is internal function */

    *ltcb = NULL;
    
    obj.raw = pthreadid;
    if (pthread_id_is_valid(obj))
    {
        *ltcb = pthread_id_get_handle(obj);        
    }
    if (!*ltcb)
        return -1;

    return 0;
}

int _getltcb_self(pthread_i **ltcb)
{   
    /* do not check NULL pointer since it is internal function */

    *ltcb = (pthread_i*)qurt_tls_get_specific(pthread_tcb_key);
    return 0;
}

void _deinit_ltcb(pthread_t pthreadid)
{
    pthread_i * ltcb;

    if (0 != _getltcb(&ltcb, pthreadid))
        return;

    pthread_id_delete(pthreadid);

    ltcb->magic = 0;
    
    if (ltcb->select_mask)
    {
        free(ltcb->select_mask);
        ltcb->select_mask = 0;
    }

    free(ltcb);
}

int * _geterrnoaddr(void)
{
    pthread_i *ltcb;
    
    ltcb = (pthread_i*)qurt_tls_get_specific(pthread_tcb_key);
    if (!ltcb)
        return NULL;

    return &ltcb->last_err;
}

int _posix_init(void)
{
    qurt_tls_create_key (&pthread_tcb_key, NULL);
    pthread_id_table_init(&pthread_id_table, pthread_objs, PTHREAD_MAX_THREADS * 4 * 2);
    return 0;
}

/* this is the weak version of the function in env that the actual funciton is not available */
void * __attribute__((weak)) rex_create_fake_tcb(void* sp, size_t siz, char* tname)
{
    /* malloc something to just make the rest of code happy */
    return (void*)malloc(1);
}

/* this is the weak version of the function in env that the actual funciton is not available */
void __attribute__((weak)) rex_destroy_fake_tcb(void* rex_tcb)
{
    free(rex_tcb);
}

