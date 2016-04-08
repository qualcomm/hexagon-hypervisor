/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <string.h> //memset
#include <qurt_assert.h>
#include <qurt_atomic_ops.h>
#include <pthread.h>
#include "pthread_internal.h"

/* EJP: This is unacceptable.  Particularly things like the pthread_mutex implementations. */

extern void* __attribute__((weak)) rex_create_fake_tcb(void* sp, size_t siz, char* tname);
extern void  __attribute__((weak)) rex_destroy_fake_tcb(void* tcb_ptr);

#define QURT_INIT_ID                       0xFEFEFEFE
#define EXTERNAL_CONTEXT_BITMASK           0x0
#define CONFIG_PRIORITY_SET 1

static const char pthread_default_name[] = PTHREAD_DEFAULT_NAME;
pthread_mutex_t pthread_mtx = PTHREAD_MUTEX_INITIALIZER;

static void _pthread_stub (void* arg)
{
    void               *exit_status;
    pthread_i *ltcb = (pthread_i *)arg;

    /* set pthread tcb into qurt tls */
    qurt_tls_set_specific(pthread_tcb_key, (const void *)ltcb);
    
    if (EXTERNAL_CONTEXT_BITMASK == ltcb->attr.ext_context)
    {
        ltcb->rex_tcb = (void*)rex_create_fake_tcb(
                (void*)ltcb->attr.stackaddr, 
                ltcb->attr.stacksize,
                ltcb->attr.name);
        if(ltcb->rex_tcb == NULL)
        {
            qurt_assert(0);
        }
    }

    /* wait for parent to let go */
    qurt_sem_down(&ltcb->start_lock);

    exit_status = ltcb->start_routine(ltcb->args);
    pthread_exit(exit_status);

    qurt_assert(0);  //should never reach here
}

int pthread_create_internal(pthread_t * restrict thread, const pthread_attr_t * restrict attr,
                   void *(*start)(void *), void * restrict arg, int create_thread)
{
    pthread_t id;
    size_t    stacksize  = PTHREAD_DEFAULT_STACKSIZE;
    void      *stackaddr = NULL;
    pthread_i *ltcb      = NULL;
    int       prio       = PTHREAD_DEFAULT_PRIORITY;
    int       internal_stack_flag = 0;
    qurt_thread_attr_t qurt_thread_attr;
    int ret_val;

    if (__builtin_expect(NULL == thread, 0))
    {
        errno = EINVAL;
        return -1;
    }

    if (__builtin_expect(create_thread, 1))
    {
        if (__builtin_expect(attr != NULL, 1))
        {
            stacksize = attr->stacksize;
            stackaddr = attr->stackaddr;
            prio      = attr->priority;
        }
        stacksize = stacksize & - 8;
        if (__builtin_expect(((unsigned int) stackaddr) & 7, 0))       
        {
            return -2;
        }
    
        if (__builtin_expect(stackaddr == NULL, 0))       
        {
            stackaddr = malloc(stacksize);
            if (__builtin_expect(stackaddr == NULL, 0))       
            {
                errno = ENOMEM;
                return -1;
            }
            internal_stack_flag = 1; /*set this flag so that we could free the stack later */
        }
    
        /* pthread priority order is the opposite of qurt thread priority order */
        prio = 255 - prio;
        if (prio > PTHREAD_MAX_PRIORITY) prio = PTHREAD_MAX_PRIORITY;
        if (prio < PTHREAD_MIN_PRIORITY) prio = PTHREAD_MIN_PRIORITY;

    }
    
    (void) pthread_mutex_lock(&pthread_mtx);

    ltcb = (pthread_i*) malloc(sizeof(pthread_i));
    if (__builtin_expect(ltcb == NULL, 0))       
    {
        (void) pthread_mutex_unlock(&pthread_mtx);
        if (internal_stack_flag)
        {
            free(stackaddr);
        }
        errno = ENOMEM;
        return -1;
    }
    memset(ltcb, 0, sizeof(pthread_i));

    ltcb->magic         = POSIX_THREAD_MAGIC;
    ltcb->pthread       = pthread_id_new(ltcb).raw;
    ltcb->start_routine = start;
    ltcb->args          = arg;
    ltcb->attr.internal_stack = internal_stack_flag;

    qurt_assert(ltcb->pthread); //ensure we got a pthread id

    if (__builtin_expect(attr == NULL, 0))       
    {
        /* Set default values for all attributes */
        strlcpy(ltcb->attr.name, pthread_default_name, sizeof(ltcb->attr.name));
        ltcb->attr.name[PTHREAD_NAME_LEN-1] = '\0';
        ltcb->attr.stacksize                = PTHREAD_DEFAULT_STACKSIZE;
        ltcb->attr.stackaddr                = stackaddr;
        ltcb->attr.priority                 = PTHREAD_DEFAULT_PRIORITY;
        ltcb->attr.timetest_id              = -2; // -2 is the default TID for QURT
        ltcb->attr.cpumask                  = CONFIG_NR_CPUS;        
        ltcb->attr.ext_context              = EXTERNAL_CONTEXT_BITMASK;
    }
    else
    {
        strlcpy(ltcb->attr.name, attr->name, sizeof(ltcb->attr.name));
        ltcb->attr.stacksize                = stacksize;
        ltcb->attr.stackaddr                = stackaddr;
        ltcb->attr.priority                 = prio;
        ltcb->attr.timetest_id              = attr->timetest_id;
        ltcb->attr.cpumask = (attr->cpumask)? attr->cpumask : CONFIG_NR_CPUS;        
        ltcb->attr.ext_context              = attr->ext_context;
    }

    qurt_anysignal_init(&ltcb->sigs);

    if (__builtin_expect(create_thread, 1))
    {
        qurt_sem_init_val(&ltcb->start_lock, 0);

        qurt_thread_attr_init (&qurt_thread_attr);
        qurt_thread_attr_set_name (&qurt_thread_attr, ltcb->attr.name);
        qurt_thread_attr_set_stack_size (&qurt_thread_attr, (unsigned int)ltcb->attr.stacksize);
        qurt_thread_attr_set_stack_addr (&qurt_thread_attr, ltcb->attr.stackaddr);
        qurt_thread_attr_set_priority (&qurt_thread_attr, (unsigned short)ltcb->attr.priority);
        qurt_thread_attr_set_timetest_id (&qurt_thread_attr, (unsigned short)ltcb->attr.timetest_id);
        qurt_thread_attr_set_affinity (&qurt_thread_attr, (unsigned char)ltcb->attr.cpumask);

        ret_val = qurt_thread_create (&id, &qurt_thread_attr, _pthread_stub, (void *)ltcb);
        if (__builtin_expect(ret_val == -1, 0))
        {
            /* free start_lock in ltcb */
            qurt_sem_destroy(&ltcb->start_lock);

            /* clean up ltcb */
            _deinit_ltcb(ltcb->pthread);
            
            (void) pthread_mutex_unlock(&pthread_mtx);
            errno = EINTR;
            return -1;
        }
    }
    else
    {
        /* already a qurt thread */
        id = qurt_thread_get_id();
        
        /* set pthread tcb into qurt tls */
        qurt_tls_set_specific(pthread_tcb_key, (const void *)ltcb);
    }

    if (__builtin_expect(thread != NULL, 1))
    {
        *thread = ltcb->pthread;
    }

    ltcb->qurtid = id;

    (void) pthread_mutex_unlock(&pthread_mtx);

    /* to let go of the child thread */
    qurt_sem_up(&ltcb->start_lock);
    
    return 0;
}

int pthread_create(pthread_t * restrict thread, const pthread_attr_t * restrict attr,
                   void *(*start)(void *), void * restrict arg)
{
    return pthread_create_internal(thread, attr, start, arg, 1);
}

/* Calling non-pthread calls this function to create pthred tcb w/o creating actual thread */

int pthread_fake(pthread_t * restrict thread, const pthread_attr_t * restrict attr)
{
   return pthread_create_internal(thread, attr, NULL, 0, 0);
}

int pthread_fake_destroy(pthread_t thread)
{
    pthread_i *ltcb;
    int       ret;

    ret = _getltcb(&ltcb, thread);

    if (0 != ret || NULL == ltcb)
    {
        /* No thread could be found by the given thread id */
        return ESRCH;
    }

    qurt_anysignal_destroy(&ltcb->sigs);

    /* do not free start_lock in ltcb, since this is a faked pthread */
    /* do not free stack here, since this is a faked pthread */
    /* do not free (faked) REX TCB, since this is a faked pthread */

    _deinit_ltcb(ltcb->pthread); /* Release Thread local info */
    return EOK;    
}

/* Termination, Cleanup, and Deletion functions */

void pthread_exit(void *value_ptr)
{
    pthread_i *ltcb;
    int       ret;

    /* get my ltcb; */
    ret = _getltcb_self(&ltcb);

    while (0 != ret || NULL == ltcb)
    {
        qurt_assert(0); /* this is a bug we would like to capture */
    }

    ltcb->exitstatus = (int) value_ptr;
    qurt_anysignal_destroy(&ltcb->sigs);

    /* do not free ltcb here. free it when the thread was joined */
    /* do not free stack here. free it when the thread was joined */

    /* free faked REX TCB */
    if (ltcb->rex_tcb)
    {
	    rex_destroy_fake_tcb(ltcb->rex_tcb);
    }
    /* qurt_thread_stop() has mem leak issue. Call exit() instead */
    qurt_thread_exit((int) value_ptr); /* this function will not return */
    qurt_assert(0);           /* this is a bug we would like to capture */
    while (1)
    {
    }
}

int pthread_join(pthread_t thread, void **value_ptr)
{
    pthread_i *ltcb;
    int       ret;
    int       status;

    /* find the ltcb of the thread */
    ret = _getltcb(&ltcb, thread);
    if (0 != ret || NULL == ltcb)
    {
        /* No thread could be found by the given thread id */
        return ESRCH;
    }

    qurt_thread_join(ltcb->qurtid, &status);

    /* free start_lock in ltcb */
    qurt_sem_destroy(&ltcb->start_lock);

    /* free stack */
    if (ltcb->attr.internal_stack)
    {
        free(ltcb->attr.stackaddr);
    }

    if (value_ptr)
        *value_ptr = (void*) ltcb->exitstatus;
    _deinit_ltcb(ltcb->pthread); /* Release Thread local info */
    return EOK;
}

int pthread_cancel(pthread_t thread)
{
    return -1;
}

pthread_t pthread_self(void)
{
    pthread_i* ltcb;
    ltcb = (pthread_i*)qurt_tls_get_specific(pthread_tcb_key);
    if (ltcb == NULL)
        return 0;          /* Returning 0 to support rex_get_pri() from crashing here;
                              Could handle better by creating a key if it doesn't exist. */
    return ltcb->pthread;
}

int pthread_kill(pthread_t thread, int sig)
{
    if (SIGKILL == sig)
    {
        _deinit_ltcb(thread);

        /* QURT does not delete thread. Do nothing right now */
        return 0;
    }
    else
    {
        pthread_i *ltcb = 0;
        int       ret   = _getltcb(&ltcb, thread);

        if (!ret && ltcb)
        {
            if (sigismember(&ltcb->sigwaiting, sig))
            {
                sigdelset(&ltcb->sigwaiting, sig);
                sigaddset(&ltcb->sigpending, sig);

                /* FIXME: will check the return value of this function
                   when it is clarified by QURT team. */
                qurt_anysignal_set(&ltcb->sigs, POSIX_SIGNAL_MASK);
            }
            else
            {
                sigaddset(&ltcb->sigpending, sig);
            }
            return 0;
        }
        return -1;
    }
}

/* Attribute functions */

int pthread_attr_init(pthread_attr_t *attr)
{
    if (__builtin_expect(NULL != attr, 1))
    {
        /* Set default values for all attributes */
        strlcpy(attr->name, pthread_default_name, sizeof(attr->name));
        attr->name[PTHREAD_NAME_LEN-1] = '\0';
        attr->stacksize                = PTHREAD_DEFAULT_STACKSIZE;
        attr->stackaddr                = 0;
        attr->internal_stack           = 0;
        attr->priority                 = PTHREAD_DEFAULT_PRIORITY;
        attr->timetest_id              = -2; // -2 is the default TID for QURT
        attr->cpumask                  = CONFIG_NR_CPUS;        
        attr->ext_context              = EXTERNAL_CONTEXT_BITMASK;
        return 0;
    }

    return -1;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    /* nothing to do here */
    return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *restrict attr,
                               const sched_param *restrict param)
{
    if (attr == NULL || param == NULL)
        return EINVAL;

    if (param->sched_priority >= PTHREAD_MIN_PRIORITY &&
        param->sched_priority <= PTHREAD_MAX_PRIORITY)
    {
        attr->priority = param->sched_priority;
        return 0;
    }
    else
    {
        return ENOTSUP;
    }
}

int pthread_attr_getschedparam(const pthread_attr_t *restrict attr,
                               sched_param *restrict param)
{
    if (attr && param)
    {
        param->sched_priority = attr->priority;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if (attr)
    {
        attr->stacksize = stacksize;
        return 0;
    }    
    return EINVAL;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
    if (attr && stacksize)
    {
        *stacksize = attr->stacksize;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_setthreadname(pthread_attr_t *attr, const char * name)
{
    if (attr && name)
    {
        strlcpy(attr->name, name, sizeof(attr->name));        
        return 0;
    }
    return EINVAL;
}

int pthread_attr_getthreadname(const pthread_attr_t *attr, char * name, int size)
{
    if (attr && name)
    {
        strlcpy(name, attr->name, size);        
        return 0;
    }
    return EINVAL;
}

int pthread_attr_settimetestid(pthread_attr_t *attr, unsigned int tid)
{
    if (attr)
    {
        attr->timetest_id = tid;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_gettimetestid(const pthread_attr_t *attr, unsigned int* tid)
{
    if (attr && tid)
    {
        *tid = attr->timetest_id;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void * stackaddr)
{
    if (attr && stackaddr)
    {
        attr->stackaddr = stackaddr;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_getstackaddr(const pthread_attr_t *attr, void ** stackaddr)
{
    if (attr && stackaddr)
    {
        *stackaddr = attr->stackaddr;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_setaffinity_np(pthread_attr_t *attr, size_t cpusetsize, const cpu_set_t *cpuset)
{
    if (attr && cpuset && (*cpuset & ~CONFIG_NR_CPUS)==0 )
    {
        attr->cpumask = *cpuset;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_getaffinity_np(pthread_attr_t *attr, size_t cpusetsize, cpu_set_t *cpuset)
{
    if (attr && cpuset)
    {
        *cpuset = attr->cpumask;
        return 0;
    }
    return EINVAL;
}

int pthread_attr_setexternalcontext_np(pthread_attr_t *attr, int context)
{
    if (attr)
    {
        attr->ext_context = context;
        return 0;
    }
    return EINVAL;
}

/* Mutexes */
#ifdef PTHREAD_MUTEX_OPAQUE
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
    qurt_mutex_t    *p_mutex_obj;
    _pthread_mutex_t *_p_mutex;

    if (mutex == NULL)
    {
        return EINVAL;
    }

    _p_mutex = (_pthread_mutex_t*) malloc(sizeof(_pthread_mutex_t));
    if (_p_mutex == NULL)
    {
        return ENOMEM;
    }

    if (attr == NULL)
    {
        pthread_mutexattr_init(&_p_mutex->attr);
    }
    else
    {
        _p_mutex->attr.is_initialized = attr->is_initialized;
        _p_mutex->attr.type           = attr->type;
        _p_mutex->attr.pshared        = attr->pshared;
        _p_mutex->attr.protocol       = attr->protocol;
    }

    p_mutex_obj = (qurt_mutex_t*) malloc(sizeof(qurt_mutex_t));
    if (p_mutex_obj == NULL)
    {
        free(_p_mutex);
        return ENOMEM;
    }
    memset(p_mutex_obj, 0, sizeof(qurt_mutex_t));

    if (_p_mutex->attr.type == PTHREAD_MUTEX_RECURSIVE)
    {
#ifdef CONFIG_PRIORITY_INHERITANCE    
        if (_p_mutex->attr.protocol == PTHREAD_PRIO_INHERIT)
        {
            qurt_pimutex_init(p_mutex_obj);
            _p_mutex->lock    = qurt_pimutex_lock;
            _p_mutex->trylock = qurt_pimutex_try_lock;
            _p_mutex->unlock  = qurt_pimutex_unlock;    
        }
        else if (_p_mutex->attr.protocol == PTHREAD_PRIO_NONE)
#endif        
        {
            qurt_rmutex_init(p_mutex_obj);
            _p_mutex->lock    = qurt_rmutex_lock;
            _p_mutex->trylock = qurt_rmutex_try_lock;
            _p_mutex->unlock  = qurt_rmutex_unlock;
        }
#ifdef CONFIG_PRIORITY_INHERITANCE          
        else
        {
            free(_p_mutex);
            free(p_mutex_obj);
            /* other protocols are not supported */
            return EINVAL;            
        }
#endif        
    }
    else if (_p_mutex->attr.type == PTHREAD_MUTEX_NORMAL)
    {
        /* PI Mutex is not supported for Normal Mutex */
        qurt_mutex_init(p_mutex_obj);
        _p_mutex->lock    = qurt_mutex_lock;
        _p_mutex->trylock = qurt_mutex_try_lock;
        _p_mutex->unlock  = qurt_mutex_unlock;
    }
    else
    {
        free(_p_mutex);
        free(p_mutex_obj);        
        /* other types are not supported */
        return EINVAL;
    }
    _p_mutex->mutex = p_mutex_obj;

    *mutex = (pthread_mutex_t) _p_mutex;
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    pthread_mutex_t mtx_temp;
    int             ret;

    if (__builtin_expect(mutex == NULL || *mutex==0, 0))
    {
        return EINVAL;
    }

    //If static initialization for mutex, need to call init
    if (__builtin_expect(PTHREAD_MUTEX_INITIALIZER == *mutex, 0))
    {
        ret = pthread_mutex_init(&mtx_temp, NULL);
        if (0 != ret)
        {
            return ret;
        }
        
        if (0 == qurt_atomic_compare_and_set(
                     (unsigned int *)mutex,
                     (unsigned int)PTHREAD_MUTEX_INITIALIZER,
                     (unsigned int)mtx_temp))
        {
            /* oops, someone else set it already. Free my mutex. */
            pthread_mutex_destroy(&mtx_temp);            
        }
    }

    ((_pthread_mutex_t*)(*mutex))->lock(((_pthread_mutex_t*)(*mutex))->mutex);
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (__builtin_expect(mutex == NULL || *mutex==0, 0))
        return EINVAL;

    ((_pthread_mutex_t*)(*mutex))->unlock(((_pthread_mutex_t*)(*mutex))->mutex);
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    pthread_mutex_t mtx_temp;
    int             ret;

    if (__builtin_expect(mutex == NULL || *mutex==0, 0))
    {
        return EINVAL;
    }

    //If static initialization for mutex, need to call init
    if (__builtin_expect(PTHREAD_MUTEX_INITIALIZER == *mutex, 0))
    {
        ret = pthread_mutex_init(&mtx_temp, NULL);
        if (0 != ret)
        {
            return ret;
        }
        
        if (0 == qurt_atomic_compare_and_set(
                     (unsigned int *)mutex,
                     (unsigned int)PTHREAD_MUTEX_INITIALIZER,
                     (unsigned int)mtx_temp))
        {
            /* oops, someone else set it already. Free my mutex. */
            pthread_mutex_destroy(&mtx_temp);            
        }
    }

    if (0 == ((_pthread_mutex_t*)(*mutex))->trylock(((_pthread_mutex_t*)(*mutex))->mutex))
        return 0;
    else
        return EBUSY;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    _pthread_mutex_t *_p_mutex;

    if (mutex == NULL)
    {
        return EINVAL;
    }

    _p_mutex = (_pthread_mutex_t*) *mutex;    
    if (_p_mutex == NULL)
    {
        return EINVAL;
    }

    if (_p_mutex->mutex == 0)
    {
        return EINVAL;
    }
    else
    {
        /* Destroy the attributes of the mutex */
        pthread_mutexattr_destroy(&_p_mutex->attr);

        if (_p_mutex->attr.type == PTHREAD_MUTEX_RECURSIVE)
        {
            qurt_rmutex_destroy((qurt_mutex_t *) _p_mutex->mutex);
            free((qurt_mutex_t *) _p_mutex->mutex);
        }
        else
        {
            qurt_mutex_destroy((qurt_mutex_t *) _p_mutex->mutex);
            free((qurt_mutex_t *) _p_mutex->mutex);
        }
    }
    _p_mutex->lock    = 0;
    _p_mutex->unlock  = 0;
    _p_mutex->trylock = 0;

    free(_p_mutex);

    *mutex = 0;
    return 0;
}
#else
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
    qurt_mutex_t    *bmutex;

    if (mutex == NULL)
    {
        return EINVAL;
    }

    if (attr == NULL)
    {
        pthread_mutexattr_init(&mutex->attr);
    }
    else
    {
        mutex->attr.is_initialized = attr->is_initialized;
        mutex->attr.type           = attr->type;
        mutex->attr.pshared        = attr->pshared;
        mutex->attr.protocol       = attr->protocol;
    }

    bmutex = (qurt_mutex_t *)malloc(sizeof(qurt_mutex_t));
    if (bmutex == NULL)
    {
        return ENOMEM;
    }
    memset(bmutex, 0, sizeof(qurt_mutex_t));

    if (mutex->attr.type == PTHREAD_MUTEX_RECURSIVE)
    {
        qurt_rmutex_init(bmutex);
        mutex->lock    = qurt_rmutex_lock;
        mutex->trylock = qurt_rmutex_try_lock;
        mutex->unlock  = qurt_rmutex_unlock;
    }
    else if (mutex->attr.type == PTHREAD_MUTEX_NORMAL)
    {
        qurt_mutex_init(bmutex);
        mutex->lock    = qurt_mutex_lock;
        mutex->trylock = qurt_mutex_try_lock;
        mutex->unlock  = qurt_mutex_unlock;
    }
    else
    {
        /* other types are not supported */
        return EINVAL;
    }
    mutex->kmutex = bmutex;
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    qurt_mutex_t   *mtx_temp;

    if (__builtin_expect(mutex == NULL, 0))
    {
        return EINVAL;
    }

    //If static initialization for mutex, need to call init
    if (__builtin_expect(0 == mutex->kmutex, 0))
    {
        mtx_temp = (qurt_mutex_t*)malloc(sizeof(qurt_mutex_t));
        if (0 == mtx_temp)
        {
            return ENOMEM;
        }
        memset(mtx_temp, 0, sizeof(qurt_mutex_t));
        qurt_rmutex_init(mtx_temp);        
        
        if (0 == qurt_atomic_compare_and_set(
                     (unsigned int *)&mutex->kmutex,
                     (unsigned int)NULL,
                     (unsigned int)mtx_temp))
        {
            /* oops, someone else set it already. Free my mutex. */
            qurt_rmutex_destroy(mtx_temp);
            free(mtx_temp);
        }
    }

    mutex->lock(mutex->kmutex);
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (__builtin_expect(mutex == NULL, 0))
        return EINVAL;

    mutex->unlock(mutex->kmutex);
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    qurt_mutex_t   *mtx_temp;

    if (__builtin_expect(mutex == NULL, 0))
    {
        return EINVAL;
    }

    //If static initialization for mutex, need to call init
    if (__builtin_expect(0 == mutex->kmutex, 0))
    {
        mtx_temp = (qurt_mutex_t*)malloc(sizeof(qurt_mutex_t));
        if (0 == mtx_temp)
        {
            return ENOMEM;
        }
        memset(mtx_temp, 0, sizeof(qurt_mutex_t));
        qurt_rmutex_init(mtx_temp);        
        
        if (0 == qurt_atomic_compare_and_set(
                     (unsigned int *)&mutex->kmutex,
                     (unsigned int)NULL,
                     (unsigned int)mtx_temp))
        {
            /* oops, someone else set it already. Free my mutex. */
            qurt_rmutex_destroy(mtx_temp);
            free(mtx_temp);        
        }
        
    }

    if (0 == mutex->trylock(mutex->kmutex))
        return 0;
    else
        return EBUSY;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    qurt_mutex_t    *p_bmutex_obj;
    //_pthread_mutex_t *_p_mutex;

    if (mutex == NULL)
    {
        return EINVAL;
    }

    p_bmutex_obj = (qurt_mutex_t *) mutex->kmutex;

    if (p_bmutex_obj == 0)
    {
        return EINVAL;
    }
    else
    {
        /* Destroy the attributes of the mutex */
        pthread_mutexattr_destroy(&mutex->attr);

        if (mutex->attr.type == PTHREAD_MUTEX_RECURSIVE)
        {
            qurt_rmutex_destroy((qurt_mutex_t *) mutex->kmutex);
            free((qurt_mutex_t *) mutex->kmutex);
        }
        else
        {
            qurt_mutex_destroy((qurt_mutex_t *) mutex->kmutex);
            free((qurt_mutex_t *) mutex->kmutex);
        }
    }
    mutex->lock    = 0;
    mutex->unlock  = 0;
    mutex->trylock = 0;
    mutex->kmutex  = 0;

    //free(mutex); //leave it to user
    return 0;
}
#endif /* PTHREAD_MUTEX_OPAQUE */

int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
    if(!lock) 
        return EINVAL;
  
    *lock = PTHREAD_SPINLOCK_UNLOCKED;
    return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock)
{
    if (!lock) 
        return EINVAL;
    return 0;
}
int pthread_spin_lock(pthread_spinlock_t *lock)
{
    /*
    qurt_atomic_compare_and_set (lock, old, new)
    {
        If lock == oldValue:
            lock = newValue
            Return true
        Else:
            Return false
    } */
    while (0 == qurt_atomic_compare_and_set(
                 (unsigned int *)lock,
                 (unsigned int)PTHREAD_SPINLOCK_UNLOCKED,
                 (unsigned int)PTHREAD_SPINLOCK_LOCKED));
  
    return 0;
}

int pthread_spin_trylock(pthread_spinlock_t *lock)
{
    if (0 == qurt_atomic_compare_and_set(
                 (unsigned int *)lock,
                 (unsigned int)PTHREAD_SPINLOCK_UNLOCKED,
                 (unsigned int)PTHREAD_SPINLOCK_LOCKED))
    {
        return EBUSY;
    }
    else
    {
        return 0;
    }
}

int pthread_spin_unlock(pthread_spinlock_t *lock)
{
    /*
    qurt_atomic_compare_and_set (lock, old, new)
    {
        If lock == oldValue:
            lock = newValue
            Return true
        Else:
            Return false
    } */
    if (qurt_atomic_compare_and_set(
                 (unsigned int *)lock,
                 (unsigned int)PTHREAD_SPINLOCK_LOCKED,
                 (unsigned int)PTHREAD_SPINLOCK_UNLOCKED))
    {
        return 0;
    }
    else
    {
        return EINVAL;
    }
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (attr)
    {
        attr->type           = PTHREAD_MUTEX_DEFAULT;
        attr->pshared        = PTHREAD_PROCESS_PRIVATE;
        attr->protocol       = PTHREAD_PRIO_INHERIT;
        attr->is_initialized = PTHREAD_MUTEX_ATTR_INITIALIZED;
        return 0;
    }
    return EINVAL;
}
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if (attr)
    {
        //only do if initialized
        if (PTHREAD_MUTEX_ATTR_INITIALIZED == attr->is_initialized)
        {
            attr->is_initialized = PTHREAD_MUTEX_ATTR_UNINITIALIZED;
            attr->type           = 0;
            attr->pshared        = 0;
            attr->protocol       = 0;
        }
        //always return success
        return 0;
    }
    //attr ptr is invalid
    return EINVAL;
}
int pthread_mutexattr_gettype(const pthread_mutexattr_t *restrict attr, int *restrict type)
{
    if (attr && type && (PTHREAD_MUTEX_ATTR_INITIALIZED == attr->is_initialized))
    {
        *type = attr->type;
        return 0;
    }
    return EINVAL;
}
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (attr && (PTHREAD_MUTEX_ATTR_INITIALIZED == attr->is_initialized))
    {
        attr->type = type;
        return 0;
    }
    return EINVAL;
}
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *restrict attr,
                                  int *restrict protocol)
{
    if (attr && protocol && (PTHREAD_MUTEX_ATTR_INITIALIZED == attr->is_initialized))
    {
        *protocol = attr->protocol;
        return 0;
    }
    return EINVAL;
}
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
    if (attr && (PTHREAD_MUTEX_ATTR_INITIALIZED == attr->is_initialized))
    {
        attr->protocol = protocol;
        return 0;
    }
    return EINVAL;
}
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *restrict attr,
                                 int *restrict pshared)
{
    if (attr && pshared && (PTHREAD_MUTEX_ATTR_INITIALIZED == attr->is_initialized))
    {
        *pshared = attr->pshared;
        return 0;
    }
    return EINVAL;
}
int pthread_mutexattr_setpshared(pthread_mutexattr_t * attr, int pshared)
{
    if (attr && (PTHREAD_MUTEX_ATTR_INITIALIZED == attr->is_initialized) &&
        ((int) PTHREAD_PROCESS_PRIVATE == pshared || (int) PTHREAD_PROCESS_SHARED == pshared))
    {
        attr->pshared = pshared;
        return 0;
    }
    return EINVAL;
}

#ifdef CONFIG_PRIORITY_SET
int pthread_getschedparam(pthread_t thread, int *restrict policy, struct sched_param *restrict param)
{
    pthread_i *ltcb;
    int       ret;
    
    if (policy == NULL || param == NULL)
        return EINVAL;

    *policy = SCHED_FIFO;
    ret     = _getltcb(&ltcb, thread);
    if (!ret && ltcb)    
    {
        param->sched_priority = 255 - qurt_thread_get_priority (ltcb->qurtid);
        return 0;
    }
    return EINVAL;
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
    pthread_i *ltcb;
    int       ret;
    
    if (policy != SCHED_FIFO)
        return ENOTSUP;

    if (param->sched_priority >= PTHREAD_MIN_PRIORITY &&
        param->sched_priority <= PTHREAD_MAX_PRIORITY)
    {
        ret = _getltcb(&ltcb, thread);
        if (!ret && ltcb)    
        {
            (void) qurt_thread_set_priority(ltcb->qurtid, 255 - param->sched_priority);
            return 0;
        }
        return EINVAL;        
    }
    else
    {
        return ENOTSUP;
    }
}

int pthread_setschedprio(pthread_t thread, int prio)
{
    struct sched_param param;

    param.sched_priority = prio;
    return pthread_setschedparam(thread, SCHED_FIFO, &param);
}
#endif /* CONFIG_PRIORITY_SET */

int pthread_condattr_init(pthread_condattr_t *attr)
{
    if (attr)
    {
        /* Set default values for attributes */
        attr->pshared        = PTHREAD_PROCESS_PRIVATE;
        attr->is_initialized = PTHREAD_COND_ATTR_INITIALIZED;
        return 0;
    }
    return EINVAL;
}

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    if (attr)
    {
        //only do if initialized
        if (PTHREAD_COND_ATTR_INITIALIZED == attr->is_initialized)
        {
            attr->is_initialized = PTHREAD_COND_ATTR_UNINITIALIZED;
        }
        return 0;
    }
    return EINVAL;
}

int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
    if (attr && (PTHREAD_COND_ATTR_INITIALIZED == attr->is_initialized) &&
        ((int) PTHREAD_PROCESS_PRIVATE == pshared))
    {
        attr->pshared = pshared;
        return 0;
    }
    return EINVAL;
}

int pthread_condattr_getpshared(const pthread_condattr_t * restrict attr, int *restrict pshared)
{
    if (attr && pshared && (PTHREAD_COND_ATTR_INITIALIZED == attr->is_initialized))
    {
        *pshared = attr->pshared;
        return 0;
    }
    return EINVAL;
}

int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t * attr)
{
    qurt_cond_t    *p_cond_obj;
    _pthread_cond_t *_p_cond;

    _p_cond = (_pthread_cond_t*) malloc(sizeof(_pthread_cond_t));
    if (_p_cond == NULL)
    {
        return ENOMEM;
    }

    if (attr == NULL)
    {
        pthread_condattr_init(&_p_cond->attr);
    }
    else
    {
        _p_cond->attr.is_initialized = attr->is_initialized;
        _p_cond->attr.pshared        = attr->pshared;
    }

    p_cond_obj = (qurt_cond_t*) malloc(sizeof(qurt_cond_t));
    if (p_cond_obj == NULL)
    {
        free(_p_cond);
        return ENOMEM;
    }

    qurt_cond_init(p_cond_obj);
    _p_cond->qurt_cond = p_cond_obj;

    *cond = (pthread_cond_t) _p_cond;
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    qurt_cond_t    *p_cond_obj;
    _pthread_cond_t *_p_cond;

    if (cond == NULL)
    {
        return EINVAL;
    }

    _p_cond = (_pthread_cond_t*) *cond;
    if (_p_cond == NULL)
       return EINVAL;

    p_cond_obj = _p_cond->qurt_cond;

    if (p_cond_obj == 0)
    {
        return EINVAL;
    }
    else
    {
        qurt_cond_destroy(p_cond_obj);
        free(p_cond_obj);
    }

    free(_p_cond);

    *cond = 0;
    return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    _pthread_cond_t *_p_cond;

    if (cond == NULL)
        return EINVAL;

    _p_cond = (_pthread_cond_t*) *cond;
    if (_p_cond)
    {
        //If static initialization for cond, need to call init
        if (PTHREAD_COND_INITIALIZER == *cond)
        {
            int ret = pthread_cond_init(cond, NULL);
            if (0 != ret)
                return ret;
            _p_cond = (_pthread_cond_t*) *cond;
        }

        qurt_cond_signal(_p_cond->qurt_cond);
        return 0;
    }
    return EINVAL;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    _pthread_cond_t *_p_cond;

    if (cond == NULL)
        return EINVAL;

    _p_cond = (_pthread_cond_t*) *cond;
    if (_p_cond)
    {
        //If static initialization for cond, need to call init
        if (PTHREAD_COND_INITIALIZER == *cond)
        {
            int ret = pthread_cond_init(cond, NULL);
            if (0 != ret)
                return ret;
            _p_cond = (_pthread_cond_t*) *cond;
        }

        qurt_cond_broadcast(_p_cond->qurt_cond);
        return 0;
    }
    return EINVAL;
}
#ifdef PTHREAD_MUTEX_OPAQUE
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    _pthread_cond_t  *_p_cond;
    _pthread_mutex_t *_p_mutex;
    int              mutex_type = 0;

    if (cond == NULL || mutex == NULL)
        return EINVAL;

    _p_cond  = (_pthread_cond_t*) *cond;
    _p_mutex = (_pthread_mutex_t*) *mutex;

    if (_p_cond == NULL || _p_mutex == NULL)
        return EINVAL;

    pthread_mutexattr_gettype(&(_p_mutex->attr), &mutex_type);

    if(PTHREAD_MUTEX_NORMAL != mutex_type)
    {
        /* A recursive mutex should never be used with condition variables, 
         * because the unlock performed for a pthread_cond_wait or 
         * pthread_cond_timedwait might not actually release the mutex. 
         * In that case, no other thread can satisfy the condition of the 
         * predicate, and the thread waits indefinitely. */
        qurt_assert(0);
        return EINVAL;
    }

    //If static initialization for cond, need to call init
    if (PTHREAD_COND_INITIALIZER == *cond)
    {
        int ret = pthread_cond_init(cond, NULL);
        if (0 != ret)
            return ret;
        _p_cond = (_pthread_cond_t*) *cond;
    }

    //If static initialization for mutex, need to call init
    if (PTHREAD_MUTEX_INITIALIZER == *mutex)
    {
        int ret = pthread_mutex_init(mutex, NULL);
        if (0 != ret)
            return ret;
        _p_mutex = (_pthread_mutex_t*) *mutex;
    }

    qurt_cond_wait(_p_cond->qurt_cond, (qurt_mutex_t *) _p_mutex->mutex);
    return 0;
}
#else
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    _pthread_cond_t  *_p_cond;
    //_pthread_mutex_t *_p_mutex;
    qurt_mutex_t    *mtx_temp;
    int              mutex_type;

    if (cond == NULL || mutex == NULL)
        return EINVAL;

    _p_cond  = (_pthread_cond_t*) *cond;
    //_p_mutex = (_pthread_mutex_t*) *mutex;

    if (_p_cond == NULL)
        return EINVAL;

    pthread_mutexattr_gettype(&(mutex->attr), &mutex_type);

    if(PTHREAD_MUTEX_NORMAL != mutex_type)
    {
        /* A recursive mutex should never be used with condition variables, 
         * because the unlock performed for a pthread_cond_wait or 
         * pthread_cond_timedwait might not actually release the mutex. 
         * In that case, no other thread can satisfy the condition of the 
         * predicate, and the thread waits indefinitely. */
        qurt_assert(0);
        return EINVAL;
    }        

    //If static initialization for cond, need to call init
    if (PTHREAD_COND_INITIALIZER == *cond)
    {
        int ret = pthread_cond_init(cond, NULL);
        if (0 != ret)
            return ret;
        _p_cond = (_pthread_cond_t*) *cond;
    }

    //If static initialization for mutex, need to call init
    //If static initialization for mutex, need to call init
    if (__builtin_expect(0 == mutex->kmutex, 0))
    {
        mtx_temp = (qurt_mutex_t*)malloc(sizeof(qurt_mutex_t));
        if (0 == mtx_temp)
        {
            return ENOMEM;
        }
        memset(mtx_temp, 0, sizeof(qurt_mutex_t));
        qurt_rmutex_init(mtx_temp);        
        
        if (0 == qurt_atomic_compare_and_set(
                     (unsigned int *)&mutex->kmutex,
                     (unsigned int)NULL,
                     (unsigned int)mtx_temp))
        {
            /* oops, someone else set it already. Free my mutex. */
            qurt_rmutex_destroy(mtx_temp);
            free(mtx_temp);        
        }        
    }

    qurt_cond_wait(_p_cond->qurt_cond, (qurt_mutex_t *) mutex->kmutex);
    return 0;
}
#endif

int pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
    if (attr)
    {
        /* Set default values for attributes */
        attr->pshared        = PTHREAD_PROCESS_PRIVATE;
        attr->is_initialized = PTHREAD_COND_ATTR_INITIALIZED;
        return 0;
    }
    return EINVAL;
}

int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
    if (attr)
    {
        //only do if initialized
        if (PTHREAD_COND_ATTR_INITIALIZED == attr->is_initialized)
        {
            attr->is_initialized = PTHREAD_COND_ATTR_UNINITIALIZED;
        }
        return 0;
    }
    return EINVAL;
}

int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
    if (attr && (PTHREAD_COND_ATTR_INITIALIZED == attr->is_initialized) &&
        ((int) PTHREAD_PROCESS_PRIVATE == pshared))
    {
        attr->pshared = pshared;
        return 0;
    }
    return EINVAL;
}

int pthread_barrierattr_getpshared(const pthread_barrierattr_t * restrict attr, int *restrict pshared)
{
    if (attr && pshared && (PTHREAD_COND_ATTR_INITIALIZED == attr->is_initialized))
    {
        *pshared = attr->pshared;
        return 0;
    }
    return EINVAL;
}

int pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned count)
{
    qurt_barrier_t    *p_barrier_obj;
    _pthread_barrier_t *_p_barrier;

    _p_barrier = (_pthread_barrier_t*) malloc(sizeof(_pthread_barrier_t));
    if (_p_barrier == NULL)
    {
        return ENOMEM;
    }

    if (attr == NULL)
    {
        pthread_barrierattr_init(&_p_barrier->attr);
    }
    else
    {
        _p_barrier->attr.is_initialized = attr->is_initialized;
        _p_barrier->attr.pshared        = attr->pshared;
    }

    p_barrier_obj = (qurt_barrier_t*) malloc(sizeof(qurt_barrier_t));
    if (p_barrier_obj == NULL)
    {
        free(_p_barrier);
        return ENOMEM;
    }

    qurt_barrier_init(p_barrier_obj, count);
    _p_barrier->qurt_barrier = p_barrier_obj;

    *barrier = (pthread_barrier_t) _p_barrier;
    return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    qurt_barrier_t    *p_barrier_obj;
    _pthread_barrier_t *_p_barrier;

    if (barrier == NULL)
    {
        return EINVAL;
    }

    _p_barrier = (_pthread_barrier_t*) *barrier;
    if (_p_barrier == NULL)
       return EINVAL;

    p_barrier_obj = _p_barrier->qurt_barrier;

    if (p_barrier_obj == 0)
    {
        return EINVAL;
    }
    else
    {
        qurt_barrier_destroy(p_barrier_obj);
        free(p_barrier_obj);
    }

    free(_p_barrier);

    *barrier = 0;
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    _pthread_barrier_t *_p_barrier;

    if (barrier == NULL)
        return EINVAL;

    _p_barrier = (_pthread_barrier_t*) *barrier;

    if (_p_barrier == NULL)
        return EINVAL;

    qurt_barrier_wait(_p_barrier->qurt_barrier);
    return 0;
}

int sched_get_priority_max(int policy)
{
    return PTHREAD_MAX_PRIORITY;
}

int sched_get_priority_min(int policy)
{
    return PTHREAD_MIN_PRIORITY;
}

int pthread_key_create(pthread_key_t *p_key, void (*destructor)(void*))
{
    int err_code = qurt_tls_create_key(p_key, destructor);
    
    if (0 != err_code)
    {
        return EAGAIN;
    }
    
    return 0;
}

int pthread_key_delete(pthread_key_t key)
{
    int err_code = qurt_tls_delete_key(key);

    if (0 != err_code)
    {
        return EAGAIN;
    }
    
    return 0;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
    int err_code = qurt_tls_set_specific(key, value);

    if (0 != err_code)
    {
        return EINVAL;
    }

    return 0;
}

void *pthread_getspecific(pthread_key_t key)
{
    return qurt_tls_get_specific(key);
}

int pthread_getattr_np(pthread_t thread, pthread_attr_t * restrict attr)
{
    pthread_i *ltcb;
    int       ret;

    if (!attr)
    {
      errno = EINVAL;
      return -1;
    }
    ret = _getltcb(&ltcb, thread);

    if (0 != ret || NULL == ltcb)
    {
        /* No thread could be found by the given thread id */
        errno = ESRCH;
        return -1;
    }
    strlcpy(attr->name, ltcb->attr.name, sizeof(attr->name));
    attr->stacksize = ltcb->attr.stacksize;
    attr->stackaddr = ltcb->attr.stackaddr;
    attr->priority = ltcb->attr.priority;
    attr->timetest_id = ltcb->attr.timetest_id;
    attr->cpumask = ltcb->attr.cpumask;

    return 0;
}
