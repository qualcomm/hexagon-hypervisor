/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <string.h> //memset
#include <blast.h>
#include "atomic_ops.h"
#include <pthread.h>
#include "pthread_internal.h"

extern void * __attribute__((weak)) rex_create_fake_tcb(void* sp, size_t siz);

#define BLAST_INIT_ID                       0xFEFEFEFE

//  Not present in tools errno.h
#define EOK 0

pthread_mutex_t pthread_mtx = PTHREAD_MUTEX_INITIALIZER;

static void* _pthread_stub(pthread_i *ltcb)
{
	void               *exit_status;
	unsigned long long name0;
	unsigned long long name1;

	/* set timetest id */
	//  dunno what this timetest stuff is
	//blast_thread_set_tid(ltcb->attr.timetest_id);
    
	/* set pthread tcb into blast tls */
	//blast_tls_setspecific(pthread_tcb_key, (const void *)ltcb);
	_allocltcb((void *)ltcb,blast_thread_myid());
    
	/* set thread name */
	memcpy(&name0, ltcb->attr.name, 8);
	memcpy(&name1, &(ltcb->attr.name[8]), 8);

	// looks like a debugging thign
	//blast_thread_set_name(name0, name1);

	if(!rex_create_fake_tcb(
		(void*)ltcb->attr.stackaddr, 
		ltcb->attr.stacksize))
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
    unsigned long long name0;
    unsigned long long name1;
    
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
    
        /* pthread priority order is the opposite of blast thread priority order */
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

    ltcb->pthread       = pthread_id_new(ltcb).raw;
    ltcb->start_routine = start;
    ltcb->args          = arg;
    ltcb->attr.internal_stack = internal_stack_flag;

    if (attr == NULL)
    {
        /* Set default values for all attributes */
        strncpy(ltcb->attr.name, PTHREAD_DEFAULT_NAME, PTHREAD_NAME_LEN - 1);
        ltcb->attr.name[PTHREAD_NAME_LEN-1] = '\0';
        ltcb->attr.stacksize                = PTHREAD_DEFAULT_STACKSIZE;
        ltcb->attr.stackaddr                = stackaddr;
        ltcb->attr.priority                 = PTHREAD_DEFAULT_PRIORITY;
        ltcb->attr.timetest_id              = 0;        
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

    blast_anysignal_init(&ltcb->sigs);

    if (create_thread)
    {
        /* OSAM and Modem tools need 0xF8 as stack init value for debugging purpose */
        memset(ltcb->attr.stackaddr, 0xF8, ltcb->attr.stacksize);
    
        id = blast_thread_create(_pthread_stub, 
                                (void*) (ltcb->attr.stackaddr + ltcb->attr.stacksize), 
                                ltcb, 
                                prio, 
                                0, 
                                ltcb->attr.cpumask
                                );
        if (id == -1)
        {
            _deinit_ltcb(ltcb->pthread);
            (void) pthread_mutex_unlock(&pthread_mtx);
            return -1;
        }
    }
    else
    {
	/*  This indicates that this fake pthread_create is 
	    actually called by the thread that wants to 
	    turn into a pthread.  */

        /* already a blast thread */
        id = blast_thread_myid();
        
        /* set timetest id */
        //blast_thread_set_tid(ltcb->attr.timetest_id);
        
        /* set pthread tcb into blast tls */
        //blast_tls_setspecific(pthread_tcb_key, (const void *)ltcb);
        _allocltcb((void *)ltcb,id);

        /* set thread name */
        memcpy(&name0, ltcb->attr.name, 8);
        memcpy(&name1, &(ltcb->attr.name[8]), 8);
        //blast_thread_set_name(name0, name1);    
    }

    if (thread)
    {
        *thread = ltcb->pthread;
    }

    ltcb->blastid = id;

    (void) pthread_mutex_unlock(&pthread_mtx);

    blast_sem_init_val(&ltcb->join_lock, 0);
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
    //blast_anysignal_destroy(&ltcb->sigs);

    /* sem_post on join_lock. TBD: post sem for multiple threads */
    blast_sem_up(&ltcb->join_lock);

    /* do not free ltcb here. free it when the thread was joined */
    /* do not free stack here. free it when the thread was joined */

    /* blast_thread_stop() has mem leak issue. Call exit() instead */
    blast_thread_exit(0); /* this function will not return */
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

    /* sem wait on its join_lock */
    blast_sem_down(&ltcb->join_lock);

    /* free join_lock in ltcb */
    blast_sem_destroy(&ltcb->join_lock);

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
    pthread_i *ltcb;
	//
	_getltcb_self(&ltcb);
    //ltcb = (pthread_i*)blast_tls_getspecific(pthread_tcb_key);
    return ltcb->pthread;
}

int pthread_kill(pthread_t thread, int sig)
{
    if (SIGKILL == sig)
    {
        _deinit_ltcb(thread);

        /* BLAST does not delete thread. Do nothing right now */
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
                   when it is clarified by BLAST team. */
                blast_anysignal_set(&ltcb->sigs, POSIX_SIGNAL_MASK);
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
#ifdef PTHREAD_MUTEX_OPAQUE
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
	//blast_mutex_t    *p_mutex_obj;
	h2_pthread_mutex_t *p_mutex_obj;
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

	p_mutex_obj = (h2_pthread_mutex_t*) malloc(sizeof(h2_pthread_mutex_t));
	if (p_mutex_obj == NULL)
	{
		return ENOMEM;
	}
	memset(p_mutex_obj, 0, sizeof(h2_pthread_mutex_t));

	//  blast_mutex_t and blast_rmutex_t are not interchangeable
	if (_p_mutex->attr.type == PTHREAD_MUTEX_RECURSIVE)
	{
		blast_rmutex_init(&p_mutex_obj->rmutex);
//		_p_mutex->lock    = blast_rmutex_lock;
//		_p_mutex->trylock = blast_rmutex_trylock;
//		_p_mutex->unlock  = blast_rmutex_unlock;
	}
	else if (_p_mutex->attr.type == PTHREAD_MUTEX_NORMAL)
	{
		blast_mutex_init(&p_mutex_obj->mutex);
//		_p_mutex->lock    = blast_mutex_lock;
//		_p_mutex->trylock = blast_mutex_trylock;
//		_p_mutex->unlock  = blast_mutex_unlock;
	}
	else
	{
		/* other types are not supported */
		return EINVAL;
	}
	_p_mutex->mutex = p_mutex_obj; 

	*mutex = (pthread_mutex_t) _p_mutex;  //  So they're using an int to hold a pointer to the _pthread_mutex type.
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	pthread_mutex_t mtx_temp;
	int             ret;
	h2_pthread_mutex_t *p_mutex_obj = ((_pthread_mutex_t *) (*mutex))->mutex;

	if (__builtin_expect(mutex == NULL || *mutex==NULL, 0))
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
	
		if (0 == atomic_compare_and_set(
			(atomic_word_t *)mutex,
			(atomic_plain_word_t)PTHREAD_MUTEX_INITIALIZER,
			(atomic_plain_word_t)mtx_temp))
		{
			/* oops, someone else set it already. Free my mutex. */
			pthread_mutex_destroy(&mtx_temp);            
		}
	}

	//((_pthread_mutex_t*)(*mutex))->lock(((_pthread_mutex_t*)(*mutex))->mutex);

	switch(((_pthread_mutex_t*) *mutex)->attr.type) {
		case PTHREAD_MUTEX_RECURSIVE:
			h2_rmutex_lock(&p_mutex_obj->rmutex);
			break;
		case PTHREAD_MUTEX_NORMAL:
			h2_mutex_lock(&p_mutex_obj->mutex);
	};

	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	h2_pthread_mutex_t *p_mutex_obj = ((_pthread_mutex_t *) (*mutex))->mutex;

	if (__builtin_expect(mutex == NULL || *mutex==NULL, 0))
		return EINVAL;

	//((_pthread_mutex_t*)(*mutex))->unlock(((_pthread_mutex_t*)(*mutex))->mutex);

	switch(((_pthread_mutex_t *)*mutex)->attr.type) {
		case PTHREAD_MUTEX_RECURSIVE:
			h2_rmutex_unlock(&p_mutex_obj->rmutex);
			break;
		case PTHREAD_MUTEX_NORMAL:
			h2_mutex_unlock(&p_mutex_obj->mutex);
	};

	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	h2_pthread_mutex_t *p_mutex_obj = ((_pthread_mutex_t *) (*mutex))->mutex;
	pthread_mutex_t mtx_temp;
	int             ret=0;

	if (__builtin_expect(mutex == NULL || *mutex==NULL, 0))
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
	
		if (0 == atomic_compare_and_set(
			(atomic_word_t *)mutex,
			(atomic_plain_word_t)PTHREAD_MUTEX_INITIALIZER,
			(atomic_plain_word_t)mtx_temp))
		{
			/* oops, someone else set it already. Free my mutex. */
			pthread_mutex_destroy(&mtx_temp);            
		}
	}

	switch(((_pthread_mutex_t *) *mutex)->attr.type) {
		case PTHREAD_MUTEX_RECURSIVE:
			ret = h2_rmutex_trylock(&p_mutex_obj->rmutex);
			break;
		case PTHREAD_MUTEX_NORMAL:
			ret = h2_mutex_trylock(&p_mutex_obj->mutex);
			break;
	};

	//if (0 == ((_pthread_mutex_t*)(*mutex))->trylock(((_pthread_mutex_t*)(*mutex))->mutex))
	if (0 == ret)
		return 0;
	else
		return EBUSY;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	//blast_mutex_t    *p_bmutex_obj;
	h2_pthread_mutex_t *p_bmutex_obj;
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

	p_bmutex_obj = (h2_pthread_mutex_t *) _p_mutex->mutex;

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
			//blast_rmutex_destroy((blast_mutex_t *) _p_mutex->mutex);
			//free((blast_mutex_t *) _p_mutex->mutex);
		}
		else
		{
			//blast_mutex_destroy((blast_mutex_t *) _p_mutex->mutex);
			//free((blast_mutex_t *) _p_mutex->mutex);
		}
		free(p_bmutex_obj);
	}
	_p_mutex->lock    = 0;  //  uh, planning to reuse these or something?
	_p_mutex->unlock  = 0;
	_p_mutex->trylock = 0;
	
	free(_p_mutex);

	*mutex = 0;  //  what?
	return 0;
}
#else
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
    blast_mutex_t    *bmutex;

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

    bmutex = (blast_mutex_t *)malloc(sizeof(blast_mutex_t));
    if (bmutex == NULL)
    {
        return ENOMEM;
    }
    memset(bmutex, 0, sizeof(blast_mutex_t));

    if (mutex->attr.type == PTHREAD_MUTEX_RECURSIVE)
    {
        blast_rmutex_init(bmutex);
        mutex->lock    = blast_rmutex_lock;
        mutex->trylock = blast_rmutex_trylock;
        mutex->unlock  = blast_rmutex_unlock;
    }
    else if (mutex->attr.type == PTHREAD_MUTEX_NORMAL)
    {
        blast_mutex_init(bmutex);
        mutex->lock    = blast_mutex_lock;
        mutex->trylock = blast_mutex_trylock;
        mutex->unlock  = blast_mutex_unlock;
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
    blast_mutex_t   *mtx_temp;

    if (__builtin_expect(mutex == NULL, 0))
    {
        return EINVAL;
    }

    //If static initialization for mutex, need to call init
    if (__builtin_expect(0 == mutex->kmutex, 0))
    {
        mtx_temp = (blast_mutex_t*)malloc(sizeof(blast_mutex_t));
        if (0 == mtx_temp)
        {
            return ENOMEM;
        }
        memset(mtx_temp, 0, sizeof(blast_mutex_t));
        blast_rmutex_init(mtx_temp);        
        
        if (0 == atomic_compare_and_set(
                     (atomic_word_t *)&mutex->kmutex,
                     (atomic_plain_word_t)NULL,
                     (atomic_plain_word_t)mtx_temp))
        {
            /* oops, someone else set it already. Free my mutex. */
            blast_rmutex_destroy(mtx_temp);
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
    blast_mutex_t   *mtx_temp;

    if (__builtin_expect(mutex == NULL, 0))
    {
        return EINVAL;
    }

    //If static initialization for mutex, need to call init
    if (__builtin_expect(0 == mutex->kmutex, 0))
    {
        mtx_temp = (blast_mutex_t*)malloc(sizeof(blast_mutex_t));
        if (0 == mtx_temp)
        {
            return ENOMEM;
        }
        memset(mtx_temp, 0, sizeof(blast_mutex_t));
        blast_rmutex_init(mtx_temp);        
        
        if (0 == atomic_compare_and_set(
                     (atomic_word_t *)&mutex->kmutex,
                     (atomic_plain_word_t)NULL,
                     (atomic_plain_word_t)mtx_temp))
        {
            /* oops, someone else set it already. Free my mutex. */
            blast_rmutex_destroy(mtx_temp);
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
    blast_mutex_t    *p_bmutex_obj;
    //_pthread_mutex_t *_p_mutex;

    if (mutex == NULL)
    {
        return EINVAL;
    }

    p_bmutex_obj = (blast_mutex_t *) mutex->kmutex;

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
            blast_rmutex_destroy((blast_mutex_t *) mutex->kmutex);
            free((blast_mutex_t *) mutex->kmutex);
        }
        else
        {
            blast_mutex_destroy((blast_mutex_t *) mutex->kmutex);
            free((blast_mutex_t *) mutex->kmutex);
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
        param->sched_priority = 255 - blast_prio_get(ltcb->blastid);
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
            (void) blast_prio_set(ltcb->blastid, 255 - param->sched_priority);
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
    blast_cond_t    *p_cond_obj;
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

    p_cond_obj = (blast_cond_t*) malloc(sizeof(blast_cond_t));
    if (p_cond_obj == NULL)
    {
        return ENOMEM;
    }

    blast_cond_init(p_cond_obj);
    _p_cond->blast_cond = p_cond_obj;

    *cond = (pthread_cond_t) _p_cond;
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    blast_cond_t    *p_cond_obj;
    _pthread_cond_t *_p_cond = (_pthread_cond_t*) *cond;

    if (cond == NULL)
    {
        return EINVAL;
    }

    p_cond_obj = _p_cond->blast_cond;

    if (p_cond_obj == 0)
    {
        return EINVAL;
    }
    else
    {
        blast_cond_destroy(p_cond_obj);
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

        blast_cond_signal(_p_cond->blast_cond);
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

        blast_cond_broadcast(_p_cond->blast_cond);
        return 0;
    }
    return EINVAL;
}
#ifdef PTHREAD_MUTEX_OPAQUE
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    _pthread_cond_t  *_p_cond;
    _pthread_mutex_t *_p_mutex;

    if (cond == NULL || mutex == NULL)
        return EINVAL;

    _p_cond  = (_pthread_cond_t*) *cond;
    _p_mutex = (_pthread_mutex_t*) *mutex;

    if (_p_cond == NULL || _p_mutex == NULL)
        return EINVAL;

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

    blast_cond_wait(_p_cond->blast_cond, (blast_mutex_t *) _p_mutex->mutex);
    return 0;
}
#else
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    _pthread_cond_t  *_p_cond;
    _pthread_mutex_t *_p_mutex;
    blast_mutex_t    *mtx_temp;

    if (cond == NULL || mutex == NULL)
        return EINVAL;

    _p_cond  = (_pthread_cond_t*) *cond;
    _p_mutex = (_pthread_mutex_t*) *mutex;

    if (_p_cond == NULL)
        return EINVAL;

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
        mtx_temp = (blast_mutex_t*)malloc(sizeof(blast_mutex_t));
        if (0 == mtx_temp)
        {
            return ENOMEM;
        }
        memset(mtx_temp, 0, sizeof(blast_mutex_t));
        blast_rmutex_init(mtx_temp);        
        
        if (0 == atomic_compare_and_set(
                     (atomic_word_t *)&mutex->kmutex,
                     (atomic_plain_word_t)NULL,
                     (atomic_plain_word_t)mtx_temp))
        {
            /* oops, someone else set it already. Free my mutex. */
            blast_rmutex_destroy(mtx_temp);
            free(mtx_temp);        
        }        
    }

    blast_cond_wait(_p_cond->blast_cond, (blast_mutex_t *) mutex->kmutex);
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
    blast_barrier_t    *p_barrier_obj;
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

    p_barrier_obj = (blast_barrier_t*) malloc(sizeof(blast_barrier_t));
    if (p_barrier_obj == NULL)
    {
        return ENOMEM;
    }

    blast_barrier_init(p_barrier_obj, count);
    _p_barrier->blast_barrier = p_barrier_obj;

    *barrier = (pthread_barrier_t) _p_barrier;
    return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    blast_barrier_t    *p_barrier_obj;
    _pthread_barrier_t *_p_barrier = (_pthread_barrier_t*) *barrier;

    if (barrier == NULL)
    {
        return EINVAL;
    }

    p_barrier_obj = _p_barrier->blast_barrier;

    if (p_barrier_obj == 0)
    {
        return EINVAL;
    }
    else
    {
        blast_barrier_destroy(p_barrier_obj);
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

    blast_barrier_wait(_p_barrier->blast_barrier);
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
