/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <string.h> //memset
#include <qurt.h>
#include "atomic_ops.h"
#include <pthread.h>
#include "pthread_internal.h"

/*  This file has too much crap in it -- split into multiple files!  */

extern void * __attribute__((weak)) rex_create_fake_tcb(void* sp, size_t siz);

#define QURT_INIT_ID                       0xFEFEFEFE

static const char pthread_default_name[] = "Anonymous";
/*  I think this is a "big pthreads subsystem lock"  */
pthread_mutex_t pthread_mtx = PTHREAD_MUTEX_INITIALIZER; 

static void* _pthread_stub(pthread_i *ltcb)
{
	void               *exit_status;
	unsigned long long name0;
	unsigned long long name1;

	/* set timetest id */
	//  dunno what this timetest stuff is
	//  Default answer for stuff I don't know about:  delete.
	//qurt_thread_set_tid(ltcb->attr.timetest_id);
    
	//  This TLS/UGP setting stuff used to happen in qurt_thread_create via some trampoline, which was
	//  basically this stub.  So hopefully the only people using it are the same ones using this
	//  pthread API in the first place.

	struct QURT_ugp_ptr *pUgp;
	pUgp = (struct QURT_ugp_ptr *)malloc (sizeof (struct QURT_ugp_ptr));
	memset (pUgp, 0, sizeof (struct QURT_ugp_ptr));
	asm volatile("ugp = %0\n" : : "r" (pUgp));

	/* set pthread tcb into qurt tls */
	qurt_tls_setspecific(pthread_tcb_key, (const void *)ltcb);
    
	/* set thread name */
	memcpy(&name0, ltcb->attr.name, 8);
	memcpy(&name1, &(ltcb->attr.name[8]), 8);

	// looks like a debugging thing
	//qurt_thread_set_name(name0, name1);

//	if(!rex_create_fake_tcb(
//		(void*)ltcb->attr.stackaddr, 
//		ltcb->attr.stacksize))
//	assert(0);

	exit_status = ltcb->start_routine(ltcb->args);
	pthread_exit(exit_status);

//	assert(0);  //should never reach here
	return (void*) 0;
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
    
    if (NULL == thread)
    {
        errno = EINVAL;
        return -1;
    }

    if (create_thread)
    {
        if (attr != NULL)
        {
            stacksize = attr->stacksize;
            stackaddr = attr->stackaddr;
            prio      = attr->priority;
        }
        stacksize = stacksize & - 8;
        if (((unsigned int) stackaddr) & 7)
        {
            return -2;
        }
    
        if (stackaddr == NULL)
        {
            if ((stackaddr = malloc(stacksize)) == NULL)
                return -1;
            internal_stack_flag = 1; /*set this flag so that we could free the stack later */
        }
    
        /* pthread priority order is the opposite of qurt thread priority order */
        prio = 255 - prio;
        if (prio > PTHREAD_MAX_PRIORITY) prio = PTHREAD_MAX_PRIORITY;
        if (prio < PTHREAD_MIN_PRIORITY) prio = PTHREAD_MIN_PRIORITY;

    }
    
    (void) pthread_mutex_lock(&pthread_mtx);

    ltcb = (pthread_i*) malloc(sizeof(pthread_i));
    if (!ltcb)
    {
        (void) pthread_mutex_unlock(&pthread_mtx);
        errno = ENOMEM;
        return -1;
    }
    memset(ltcb, 0, sizeof(pthread_i));

    ltcb->magic         = POSIX_THREAD_MAGIC;
    ltcb->pthread       = pthread_id_new(ltcb).raw;
    ltcb->start_routine = start;
    ltcb->args          = arg;
    ltcb->attr.internal_stack = internal_stack_flag;

    if (attr == NULL)
    {
        /* Set default values for all attributes */
        memcpy(ltcb->attr.name, pthread_default_name, sizeof(pthread_default_name));
        ltcb->attr.name[PTHREAD_NAME_LEN-1] = '\0';
        ltcb->attr.stacksize                = PTHREAD_DEFAULT_STACKSIZE;
        ltcb->attr.stackaddr                = stackaddr;
        ltcb->attr.priority                 = PTHREAD_DEFAULT_PRIORITY;
        ltcb->attr.timetest_id              = -2; // -2 is the default TID for QURT
        ltcb->attr.cpumask                  = CONFIG_NR_CPUS;        
    }
    else
    {
        memcpy(ltcb->attr.name, attr->name, PTHREAD_NAME_LEN);
        ltcb->attr.stacksize                = stacksize;
        ltcb->attr.stackaddr                = stackaddr;
        ltcb->attr.priority                 = prio;
        ltcb->attr.timetest_id              = attr->timetest_id;
        ltcb->attr.cpumask = (attr->cpumask)? attr->cpumask : CONFIG_NR_CPUS;        
    }

    qurt_anysignal_init(&ltcb->sigs);

    if (create_thread)
    {
        qurt_sem_init_val(&ltcb->start_lock, 0);

        /* OSAM and Modem tools need 0xF8 as stack init value for debugging purpose */
        memset(ltcb->attr.stackaddr, 0xF8, ltcb->attr.stacksize);

        qurt_thread_attr_init (&qurt_thread_attr);
        qurt_thread_attr_set_name (&qurt_thread_attr, ltcb->attr.name);
        qurt_thread_attr_set_stack_size (&qurt_thread_attr, (unsigned int)ltcb->attr.stacksize);
        qurt_thread_attr_set_stack_addr (&qurt_thread_attr, ltcb->attr.stackaddr);
        qurt_thread_attr_set_priority (&qurt_thread_attr, (unsigned short)ltcb->attr.priority);
        qurt_thread_attr_set_timetest_id (&qurt_thread_attr, (unsigned short)ltcb->attr.timetest_id);
        qurt_thread_attr_set_affinity (&qurt_thread_attr, (unsigned char)ltcb->attr.cpumask);

        ret_val = qurt_thread_create (&id, &qurt_thread_attr, _pthread_stub, (void *)ltcb);
        if (ret_val == -1)
        {
            /* free start_lock in ltcb */
            qurt_sem_destroy(&ltcb->start_lock);

            /* clean up ltcb */
            _deinit_ltcb(ltcb->pthread);
            
            (void) pthread_mutex_unlock(&pthread_mtx);
            return -1;
        }
    }
    else
    {
        /* already a qurt thread */
//        id = qurt_thread_get_id();
	    id = qurt_thread_myid();
	    
        /* set pthread tcb into qurt tls */
//        qurt_tls_set_specific(pthread_tcb_key, (const void *)ltcb);
        qurt_tls_setspecific(pthread_tcb_key, (const void *)ltcb);
    }

    if (thread)
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

/* Termination, Cleanup, and Deletion functions */

void pthread_exit(void *value_ptr)
{
    pthread_i * ltcb;
    int       ret;

    /* get my ltcb; */
    ret = _getltcb_self(&ltcb);

//    if (0 != ret || NULL == ltcb)
//    {
//        assert(0); /* this is a bug we would like to capture */
//    }

    ltcb->exitstatus = (int) value_ptr;
    qurt_anysignal_destroy(&ltcb->sigs);

    /* sem_post on start_lock. TBD: post sem for multiple threads */
    qurt_sem_up(&ltcb->start_lock);

    /* do not free ltcb here. free it when the thread was joined */
    /* do not free stack here. free it when the thread was joined */

    /* qurt_thread_stop() has mem leak issue. Call exit() instead */
    qurt_thread_exit(0); /* this function will not return */
//    assert(0);           /* this is a bug we would like to capture */
    while (1)
    {
    }
}

int pthread_join(pthread_t thread, void **value_ptr)
{
    pthread_i * ltcb;
    int       ret;

    /* find the ltcb of the thread */
    ret = _getltcb(&ltcb, thread);
    if (0 != ret || NULL == ltcb)
    {
        /* No thread could be found by the given thread id */
        return ESRCH;
    }

    /* sem wait on its start_lock */
    qurt_sem_down(&ltcb->start_lock);

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
    ltcb = (pthread_i*)qurt_tls_getspecific(pthread_tcb_key);

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
    if (attr)
    {
        /* Set default values for all attributes */
        strncpy(attr->name, PTHREAD_DEFAULT_NAME, PTHREAD_NAME_LEN - 1);
        attr->name[PTHREAD_NAME_LEN-1] = '\0';
        attr->stacksize                = PTHREAD_DEFAULT_STACKSIZE;
        attr->stackaddr                = 0;
        attr->internal_stack           = 0;
        attr->priority                 = PTHREAD_DEFAULT_PRIORITY;
        attr->timetest_id              = 0;        
        attr->cpumask                  = CONFIG_NR_CPUS;        
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
        strncpy(attr->name, name, PTHREAD_NAME_LEN - 1);
        return 0;
    }
    return EINVAL;
}

int pthread_attr_getthreadname(const pthread_attr_t *attr, char * name, int size)
{
    if (attr && name)
    {
        /* copy name string */
        name[size - 1] = '\0';
        strncpy(name, (char *) attr->name, size - 1);
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

/* Mutexes */

/*  maybe assert.  maybe redfine to nothing for TURBO BOOST  */
#define CHECK_PTR(ptr)		if (!ptr) return EINVAL
#define CHECK_INIT(attr)	if ((attr)->type == PTHREAD_MUTEX_ERRORCHECK) return EINVAL

int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
	pthread_mutex_t temp = PTHREAD_MUTEX_INITIALIZER;
	CHECK_PTR(mutex);

	if (attr) temp.attributes = *attr;
	*mutex = temp;

	return 0;
}

/*  The option is to use function call pointers.  Maybe.  */

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	CHECK_PTR(mutex);
	CHECK_INIT(&mutex->attributes);

	switch (mutex->attributes.type) {
		case PTHREAD_MUTEX_RECURSIVE:
			h2_rmutex_lock(&mutex->rmutex);
			break;
		case PTHREAD_MUTEX_NORMAL:
			h2_mutex_lock(&mutex->mutex);
			break;
	}
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	CHECK_PTR(mutex);
	CHECK_INIT(&mutex->attributes);

	switch (mutex->attributes.type) {
		case PTHREAD_MUTEX_RECURSIVE:
			h2_rmutex_unlock(&mutex->rmutex);
			break;
		case PTHREAD_MUTEX_NORMAL:
			h2_mutex_unlock(&mutex->mutex);
			break;

	}
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	CHECK_PTR(mutex);
	CHECK_INIT(&mutex->attributes);

	switch (mutex->attributes.type) {
		case PTHREAD_MUTEX_RECURSIVE:
			return h2_rmutex_trylock(&mutex->rmutex);
		case PTHREAD_MUTEX_NORMAL:
			return h2_mutex_trylock(&mutex->mutex);
	}
	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	/*
	 *  Should "de-initialize" the mutex...  which we don't really need to
	 *  do.  We should only check that we aren't destroying something that's
	 *  locked.  That would be inappropriate.
	 */
	CHECK_PTR(mutex);

	/*  TODO:
	 *  Should a recursive mutex not be destroyed if it's still locked by
	 *  the owner?  (Since trylock should pass)  There an official way to 
	 *  detect that?
	 *  Also, zero means "success" */

	if (pthread_mutex_trylock(mutex)) return EBUSY;

	/*  Guess we'll just use this to mark it as destroyed  */
	pthread_mutexattr_destroy(&mutex->attributes);

	return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	pthread_mutexattr_t temp = PTHREAD_MUTEXATTR_T_INIT;
	CHECK_PTR(attr);
	*attr = temp;
    	return 0;

}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	CHECK_PTR(attr);
	/*  Slightly abusing this errorcheck thing as our invalid/uninitialized value  */
	attr->type = PTHREAD_MUTEX_ERRORCHECK;  // macroize?
	return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *restrict attr, int *restrict type)
{

	CHECK_PTR(attr);
	CHECK_PTR(type);
	CHECK_INIT(attr);

	*type = attr->type;
	return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	CHECK_PTR(attr);

	/*  
	 * Technically I guess we need a separate "initialized" member,
	 * but I think this should suffice.  If your code really is 
	 * setting the type of an uninitialized mutexattr, fix your code.
	 */

	attr->type = type;  
        return 0;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *restrict attr,
                                  int *restrict protocol)
{
	CHECK_PTR(attr);
	CHECK_PTR(protocol);
	CHECK_INIT(attr);

	*protocol = attr->protocol;
	return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
	CHECK_PTR(attr);
	CHECK_INIT(attr);

	attr->protocol = protocol;
	return 0;
}

int pthread_mutexattr_getpshared(const pthread_mutexattr_t *restrict attr,
                                 int *restrict pshared)
{
	CHECK_PTR(attr);
	CHECK_PTR(pshared);
	CHECK_INIT(attr);

	*pshared = attr->pshared;
        return 0;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t * attr, int pshared)
{
	CHECK_PTR(attr);
	CHECK_INIT(attr);

	//  TODO:  check pshared value for legal values.
        attr->pshared = pshared;
	return 0;
}

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
    atomic_compare_and_set (lock, old, new)
    {
        If lock == oldValue:
            lock = newValue
            Return true
        Else:
            Return false
    } */
    while (0 == atomic_compare_and_set(
                 (atomic_word_t *)lock,
                 (atomic_plain_word_t)PTHREAD_SPINLOCK_UNLOCKED,
                 (atomic_plain_word_t)PTHREAD_SPINLOCK_LOCKED));
  
    return 0;
}

int pthread_spin_trylock(pthread_spinlock_t *lock)
{
    if (0 == atomic_compare_and_set(
                 (atomic_word_t *)lock,
                 (atomic_plain_word_t)PTHREAD_SPINLOCK_UNLOCKED,
                 (atomic_plain_word_t)PTHREAD_SPINLOCK_LOCKED))
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
    atomic_compare_and_set (lock, old, new)
    {
        If lock == oldValue:
            lock = newValue
            Return true
        Else:
            Return false
    } */
    if (atomic_compare_and_set(
                 (atomic_word_t *)lock,
                 (atomic_plain_word_t)PTHREAD_SPINLOCK_LOCKED,
                 (atomic_plain_word_t)PTHREAD_SPINLOCK_UNLOCKED))
    {
        return 0;
    }
    else
    {
        return EINVAL;
    }
}

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
        param->sched_priority = 255 - qurt_prio_get(ltcb->qurtid);
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
            (void) qurt_prio_set(ltcb->qurtid, 255 - param->sched_priority);
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
    _pthread_cond_t *_p_cond = (_pthread_cond_t*) *cond;

    if (cond == NULL)
    {
        return EINVAL;
    }

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

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	_pthread_cond_t  *_p_cond;
	//_pthread_mutex_t *_p_mutex;
	//qurt_mutex_t    *mtx_temp;

	CHECK_PTR(cond);
	CHECK_PTR(mutex);

	_p_cond  = (_pthread_cond_t*) *cond;
	//_p_mutex = (_pthread_mutex_t*) *mutex;

	CHECK_PTR(_p_cond); 

	//If static initialization for cond, need to call init
	if (PTHREAD_COND_INITIALIZER == *cond)
	{
		int ret = pthread_cond_init(cond, NULL);
		if (0 != ret)
			return ret;
		_p_cond = (_pthread_cond_t*) *cond;
	}

	//  If static initialization for mutex, need to call init
	//  No, static initialization should be just fine.
	if (mutex->attributes.type == PTHREAD_MUTEX_ERRORCHECK) 
		pthread_mutex_init(mutex,NULL);  //  does this need to be "atomic"?

	qurt_cond_wait(_p_cond->qurt_cond, (qurt_mutex_t *) mutex->mutex);
	return 0;
}

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
    _pthread_barrier_t *_p_barrier = (_pthread_barrier_t*) *barrier;

    if (barrier == NULL)
    {
        return EINVAL;
    }

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
    if (SCHED_FIFO != policy)
        return EINVAL;
    return PTHREAD_MAX_PRIORITY;
}

int sched_get_priority_min(int policy)
{
    if (SCHED_FIFO != policy)
        return EINVAL;
    return PTHREAD_MIN_PRIORITY;
}
