/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qurt.h"
#include "qurt_atomic_ops.h"
//#include "qurt_qdi_internal.h"
#include "qurt_qdi_driver.h"
#include "qurt_utcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
  qurt_qdi_local_client_create() creates a new handle
  which represents the local process.  This is used
  for handling QDI calls when the client is in the
  same context as the driver, so most of the calls
  can be greatly simplified.
*/

struct qurt_qdi_local_client {
   qurt_qdi_obj_t qdiobj;
   int handle;
};

#if 0
#define qurt_qdi_refcnt_fail(x) ((void)0)
#else
/* Declare this varargs which should prevent it from being inlined;
   it's only here to give us a breakpoint to set to find refcnt
   violations. */

void qurt_qdi_refcnt_fail(unsigned x, ...)
{
   void * volatile dummy; // what?

   if (x)
      qurt_qdi_refcnt_fail(x>>1);

   dummy = &x;
   (void)dummy;     // Suppress warning
}
#endif

/* Increment reference count. */
/* Since we don't do anything on a refcount fail, we shouldn't check too much */
int qurt_qdi_obj_ref_inc(qurt_qdi_obj_t *objptr)
{
	unsigned int ival;
	unsigned int nval;
	while (1) {
		ival = objptr->refcnt;
		if (ival == QDI_REFCNT_PERM) return 0;
		if (ival == QDI_REFCNT_INIT) {
			nval = 0;
		} else {
			/* For maxing out, it turns permanent. */
			nval = ival + 1;
		}
		if (qurt_atomic_compare_and_set(&objptr->refcnt,ival,nval)) return 0;
	}
}

int qurt_qdi_obj_ref_dec(qurt_qdi_obj_t *objptr)
{
	unsigned ival;
	unsigned nval;
	while (1) {
		ival = objptr->refcnt;
		if (ival == QDI_REFCNT_PERM) return 0;
		nval = ival - 1;
		if (qurt_atomic_compare_and_set(&objptr->refcnt,ival,nval)) return (nval == 0);
	}
}

/* EJP: FIXME: breaks on LP64 */
static inline int obj2handle(const qurt_qdi_obj_t *objptr)
{
	union {
		const qurt_qdi_obj_t *p;
		unsigned int i;
	} x;
	x.p = objptr;
	return (x.i >> 2);
}

static inline qurt_qdi_obj_t *handle2obj(unsigned int handle)
{
	union {
		qurt_qdi_obj_t *p;
		unsigned int i;
	} x;
	x.i = (handle << 2);
	return x.p;
}

static int qurt_qdi_state_local_new_handle_from_obj(qurt_qdi_obj_t *objptr)
{
	if (qurt_qdi_obj_ref_inc(objptr) < 0) return -1;
	return obj2handle(objptr);
}

static int qurt_qdi_state_local_release_handle(int h)
{
	qurt_qdi_obj_t *objptr = handle2obj(h);
	if (qurt_qdi_obj_ref_dec(objptr) == 1) {
		/* EJP: note: race here with increment */
		objptr->release(objptr);
	}
	return 0;
}

static void qurt_qdi_local_client_release(qurt_qdi_obj_t *obj)
{
}

static qurt_mutex_t qurt_qdi_namespace_lock;

struct qurt_qdi_namespace_struct {
	const char *path;
	int handle;
	struct qurt_qdi_namespace_struct *next;
	unsigned short namelen;
	unsigned short matchend;
};

static struct qurt_qdi_namespace_struct *qdi_namespace = NULL;

static int qurt_qdi_local_open(const char *path, unsigned int flags, unsigned int mode)
{
	/* Search pathlist for matching driver */
	struct qurt_qdi_namespace_struct *tmp;
	qurt_rmutex_lock(&qurt_qdi_namespace_lock);
	for (tmp = qdi_namespace; tmp != NULL; tmp = tmp->next) {
		if (strncmp(tmp->path,path,tmp->namelen + tmp->matchend) == 0) {
			break;
		}
	}
	qurt_rmutex_unlock(&qurt_qdi_namespace_lock);
	/* if not found, return failure */
	if (tmp == NULL) return -1;
	/* Found it! Hand off to that object's invoke method */
	return qurt_qdi_handle_invoke(tmp->handle, QDI_OPEN, path, flags, mode);
}

static int qurt_qdi_local_register(const char *path, const qurt_qdi_obj_t *obj)
{
	struct qurt_qdi_namespace_struct *tmp;
	/* Allocate new path structure */
	if ((tmp = malloc(sizeof(*tmp))) == NULL) {
		return -1;
	}
	/* Fill it out */
	tmp->path = path;
	tmp->handle = obj2handle(obj);
	tmp->namelen = strlen(path);
	if (path[tmp->namelen-1] == '/') {
		tmp->matchend = 0;
	} else {
		tmp->matchend = 1;
	}
	/* Insert into list */
	qurt_rmutex_lock(&qurt_qdi_namespace_lock);
	tmp->next = qdi_namespace;
	qdi_namespace = tmp;
	qurt_rmutex_unlock(&qurt_qdi_namespace_lock);
	return 0;
}

/* EJP: no way to remove a device from namespace? */

static int qurt_qdi_local_client_invocation(int client_handle, qurt_qdi_obj_t *obj, int method,
                                            qurt_qdi_arg_t a1, qurt_qdi_arg_t a2, qurt_qdi_arg_t a3,
                                            qurt_qdi_arg_t a4, qurt_qdi_arg_t a5, qurt_qdi_arg_t a6,
                                            qurt_qdi_arg_t a7, qurt_qdi_arg_t a8, qurt_qdi_arg_t a9)
{
	qurt_qdi_obj_t *tmp;
	switch (method) {
	case QDI_CLIENT_HANDLE_HANDLE_CREATE_FROM_OBJ_T:
		return qurt_qdi_state_local_new_handle_from_obj(a1.ptr);
	case QDI_CLIENT_HANDLE_HANDLE_RELEASE:
		return qurt_qdi_state_local_release_handle(a1.num);
	case QDI_CLIENT_HANDLE_OBJREF_GET:
		if ((tmp = qurt_qdi_objref_get_from_pointer(handle2obj(a1.num))) != NULL) {
			*(qurt_qdi_obj_t **)(a2.ptr) = tmp;
			return 0;
		}
		return -1;
	case QDI_CLIENT_HANDLE_COPY_FROM_USER:
	case QDI_CLIENT_HANDLE_COPY_TO_USER:
		memmove(a1.ptr, a2.ptr, a3.num);
		return 0;
	case QDI_CLIENT_HANDLE_BUFFER_LOCK:
		*(void **)a4.ptr = a1.ptr;
		return 0;
	case QDI_CLIENT_HANDLE_USER_MALLOC:
		tmp = malloc(a1.num);
		if (tmp != NULL) {
			*((void **)a2.ptr) = tmp;
			return 0;
		} else {
			return -1;
		}
	case QDI_CLIENT_HANDLE_USER_FREE:
		free(a1.ptr);
		return 0;
	case QDI_OS_CLIENT_INFO_GET:
	case QDI_OS_SYSENV:
	case QDI_CLIENT_ASID:
	case QDI_CLIENT_ERROR_HANDLER:
		/* Local client doesn't support this -- it's not suppose to */
		return -1;
	case QDI_CLOSE:
		if (a1.num == QDI_HANDLE_LOCAL_CLIENT) return -1;
	case QDI_DEVNAME_REGISTER:
		return qurt_qdi_local_register(a1.ptr,a2.ptr);
	case QDI_OPEN:
		return qurt_qdi_local_open(a1.ptr,a2.num,a3.num);
	default:
		if (method != QDI_CLOSE) qurt_printf("%s unknown method %d\n", __FUNCTION__, method);
		return qurt_qdi_method_default(client_handle, obj, method,
                                     a1, a2, a3, a4, a5, a6, a7, a8, a9);
   }
   return -1;
}

static qurt_qdi_obj_t local_client_obj = {
	qurt_qdi_local_client_invocation,
	QDI_REFCNT_PERM,
	qurt_qdi_local_client_release
};

unsigned int QDI_HANDLE_LOCAL_CLIENT;

void qurt_qdi_local_client_init(void)
{
	qurt_rmutex_init(&qurt_qdi_namespace_lock);
	QDI_HANDLE_LOCAL_CLIENT = obj2handle(&local_client_obj);
}
