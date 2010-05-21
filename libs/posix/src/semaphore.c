/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdlib.h> //malloc
#include <string.h> //memset
#include <blast.h>
#include <semaphore.h>

/*****************************************************************************
* Initialize an unamed semaphore
*****************************************************************************/
int sem_init(sem_t *__sem, int __pshared, unsigned int __sval)
{
    blast_sem_t *p_sem_obj;

    /* ensure validity of the pointer */
    if (__sem == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }

    /* ensure sem value is greater than 0 */
    if (__sval > SEM_VALUE_MAX)
    {
        errno = EINVAL; 
        return -1;    
    }

    /* support only Thread shared sem */
    if (__pshared != 0)
    {
        errno = ENOSYS; 
        return -1;    
    }

    // Allocate space on the heap for this sem object
    p_sem_obj = (blast_sem_t*) malloc(sizeof(blast_sem_t));
    if (p_sem_obj == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }
    
    memset(p_sem_obj, 0, sizeof(blast_sem_t));
    blast_sem_init_val(p_sem_obj, __sval);

    // Update __sem with pointer address
    *__sem = (sem_t) p_sem_obj;
    return 0;
}

/*****************************************************************************
* Wait for a semaphore (blocking)
*****************************************************************************/
int sem_wait(sem_t *__sem)
{
    blast_sem_t *p_sem_obj;

    /* ensure validity of the pointer */
    if (__sem == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }

    p_sem_obj = (blast_sem_t*) *__sem;
    if (p_sem_obj == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }
    
    //FIXME: will check the return value of this function when it is clarified by BLAST team.
    blast_sem_down(p_sem_obj);
    return 0;
}

/*****************************************************************************
* Try a wait for a semaphore (non-blocking)
*****************************************************************************/
int sem_trywait(sem_t *__sem)
{
    blast_sem_t *p_sem_obj;

    /* ensure validity of the pointer */
    if (__sem == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }

    p_sem_obj = (blast_sem_t*) *__sem;
    if (p_sem_obj == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }
    
    return blast_sem_trydown(p_sem_obj);
}

/*****************************************************************************
* Post a semaphore
*****************************************************************************/
int sem_post(sem_t *__sem)
{
    blast_sem_t *p_sem_obj;

    /* ensure validity of the pointer */
    if (__sem == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }

    p_sem_obj = (blast_sem_t*) *__sem;
    if (p_sem_obj == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }
    
    //FIXME: will check the return value of this function when it is clarified by BLAST team.
    blast_sem_up(p_sem_obj);
    return 0;
}

/*****************************************************************************
* Get the semaphore value
*****************************************************************************/
int sem_getvalue(sem_t *__sem, int *__sval)
{
    blast_sem_t *p_sem_obj;
    
    /* ensure validity of the pointer */
    if (__sem == NULL || __sval == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }

    p_sem_obj = (blast_sem_t*) *__sem;
    if (p_sem_obj == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }
    
    *__sval = (int)blast_sem_get_val(p_sem_obj);
    return 0;
}

/*****************************************************************************
* Destroy a semaphore
*****************************************************************************/
int sem_destroy(sem_t *__sem)
{
    blast_sem_t *p_sem_obj;

    /* ensure validity of the pointer */
    if (__sem == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }

    p_sem_obj = (blast_sem_t *) *__sem;
    if (p_sem_obj == NULL)
    {
        errno = EINVAL; 
        return -1;    
    }

    blast_sem_destroy(p_sem_obj);
    free(p_sem_obj);
    *__sem = 0;
    return 0;
}

#if 0
// **************************** //
// UNSUPPORTED APIs definitions //
// **************************** //
/*****************************************************************************
* Wait for a semaphore (blocking with timeout)
*****************************************************************************/
int sem_timedwait(sem_t *__sem, const struct timespec *__abstime)
{
    //UNSUPPORTED
    errno = EINVAL;
    return(-1);
}

/*****************************************************************************
* Open a named semaphore
*****************************************************************************/
sem_t *sem_open(const char *__name, int __oflag, ...)
{
    //UNSUPPORTED
    errno = EINVAL;
    return SEM_FAILED;
}

/*****************************************************************************
* Close a semaphore
*****************************************************************************/
int sem_close(sem_t*__sem)
{
    //UNSUPPORTED
    errno = EINVAL;
    return(-1);
}

/*****************************************************************************
* Unlink a named semaphore
*****************************************************************************/
int sem_unlink(const char *__name)
{
    //UNSUPPORTED
    errno = EINVAL;
    return(-1);
}
#endif
