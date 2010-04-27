/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <string.h> //memset
#include <pthread.h>
#include "pthread_internal.h"

pthread_id_table_t pthread_id_table;
unsigned long long pthread_objs[PTHREAD_MAX_THREADS]; //Each id has two words

/*  

Since we're going after other thread's TLS areas, that means
we have to be aware of other threads' TLS keys.

So we're back to pid->key lookups.  Again.  Probably what that
blast_fd_table was for.

The current thread's key will be thrown into UGP to cut
down on searching for own TCB.

I'm also assuming that pthread ID's are the same as BLAST ID's...
Kind of useful to document these things.  Just sayin'.

Dinkumware xtls.h file indicates the _Tls* functions under __QDSP6_RTOS__ 
should get defined into _Tls* functions...  Not sure what the
compile flags for that are though.  So I'm cheating.

*/

extern int sys_Tlsalloc(void ** key, void (*dtor)(void *));
extern int sys_Tlsfree(void * key);
extern int sys_Tlsset(void * key, const void *p);
extern void *sys_Tlsget(void *key);

#define _Tlsalloc(...)	sys_Tlsalloc(__VA_ARGS__)
#define _Tlsfree(...)	sys_Tlsfree(__VA_ARGS__)
#define _Tlsget(...)	sys_Tlsget(__VA_ARGS__)
#define _Tlsset(...)	sys_Tlsset(__VA_ARGS__)

typedef struct {
	int pid;
	void *key; 
} pid2key_table_t;

pid2key_table_t	pid2key_table[PTHREAD_MAX_THREADS];  
h2_mutex_t	pid2key_lock=H2_MUTEX_T_INIT;  

/*  lock whenever adding, freeing, or looking at someone else's tcb.
    There may be more situations, but let's see if we get away with this 
    first.  */

void *pid2key(int pid)
{
	int i;
	h2_mutex_lock(&pid2key_lock);
	for (i=0; i<PTHREAD_MAX_THREADS; i++) {
		if (pid2key_table[i].pid == pid) {
			h2_mutex_unlock(&pid2key_lock);
			return pid2key_table[i].key;
		}  //  not sure if this is really safe; need review
	}
	h2_mutex_unlock(&pid2key_lock);
	return (void *)-1;
}

/*
Called after TLS allocation; stuff the key inside pid lookup table
*/

void alloc_key_lookup(int pid, void *key)
{
	int i;
	h2_mutex_lock(&pid2key_lock);
	for (i=0; i<PTHREAD_MAX_THREADS; i++) {
		if (pid2key_table[i].pid == -1)
			pid2key_table[i].pid = pid;
			pid2key_table[i].key = key;
	} 
	h2_mutex_unlock(&pid2key_lock);
}

void free_key_lookup(int pid)
{
	int i;
	h2_mutex_lock(&pid2key_lock);
	for (i=0; i<PTHREAD_MAX_THREADS; i++) {
		if (pid2key_table[i].pid == pid)
			pid2key_table[i].pid = -1;
			pid2key_table[i].key = NULL;
	} 
	h2_mutex_unlock(&pid2key_lock);
}

void init_key_lookup()
{
	int i;
	for (i=0; i<PTHREAD_MAX_THREADS; i++) {
		pid2key_table[i].pid = -1;
		pid2key_table[i].key = NULL;
	}
}

static inline void *get_ugp(void) 
{
	void *key;
	asm ("%0 = UGP\n"
		: "=r" (key));
	return key;
}

static inline void set_ugp(void *key)
{
	asm ("UGP = %0\n" 
		:
		: "r" (key));

}

/*  Actual TCB fetch/set routines follow; should probably be static inlines  */

int _getltcb(pthread_i **ltcb, pthread_t pthreadid)
{
	void *key;
	key = pid2key(pthreadid);
	*ltcb = _Tlsget(key);
	return 0;
}

int _getltcb_self(pthread_i **ltcb)
{ 
	*ltcb = (pthread_i *) _Tlsget(get_ugp());
	return 0;
}

/*  There was no wrapper for _setltcb... */

int _setltcb(pthread_i *ltcb, pthread_t pthreadid)
{
	_Tlsset(get_ugp(),(void *)ltcb);
	//  need error checking
	return 0;
}

int _allocltcb(pthread_i *ltcb, pthread_t pthreadid)
{
	void *key;
	_Tlsalloc(&key,NULL);		//  get TLS key for this ltcb
	set_ugp(key);			//  use this key for ltcb
	_Tlsset(key,(void *)ltcb);	//  set the data at this key to point at the ltcb
	alloc_key_lookup(pthreadid,(void *)key);  //  create PID->key lookup
	//  need error checking and unrolling in case of errors
	return 0;
}

/*  Probably should have a free() function, but it's handled in deinit  */

void _deinit_ltcb(pthread_t pthreadid)
{
	pthread_i * ltcb;
	void *key;

	if (0 != _getltcb(&ltcb, pthreadid))
		return;

	//pthread_id_delete(pthreadid);
	//  original code never freed the TLS slot?  not necessary?
	alloc_key_lookup(pthreadid,&key);
	_Tlsfree(key);
	free_key_lookup(pthreadid);
    
	if (ltcb->select_mask)
	{
		free(ltcb->select_mask);
		ltcb->select_mask = 0;
	}

	if (ltcb)
		free(ltcb);
}

int * _geterrnoaddr(void)
{
	pthread_i *ltcb;

	//ltcb = (pthread_i*)blast_tls_getspecific(pthread_tcb_key);
	ltcb = (pthread_i *)_Tlsget(get_ugp());
	if (!ltcb)
		return NULL;

	return &ltcb->last_err;
}

int _posix_init(void)
{
//	assert(0); //  not sure if this is really called?
	init_key_lookup();
	return 0;
}

/* this is the weak version of the function in env that the actual funciton is not available */
void * __attribute__((weak)) rex_create_fake_tcb(void* sp, size_t siz)
{
    return (void*)1;
}

