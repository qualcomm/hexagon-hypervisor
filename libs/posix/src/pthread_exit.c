/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>

void pthread_exit(void *retval)
{
	/*
	 * Put retval in "TCB", decrement reference count, and if 
	 * detached just go ahead and free the "TCB".  Then h2_thread_stop().
	 */
	/* EJP: should this have attribute noreturn? */
	qurt_thread_exit((long)retval);
}

