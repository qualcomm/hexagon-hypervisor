/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>
#include <stdlib.h>

union error_msg_type {
	unsigned long long int raw;
	struct {
		qurt_sem_t *response;
		qurt_sysevent_error_t *info;
	};
};

#define PIPE_ELEMENTS 10
static qurt_pipe_t error_pipe;
static qurt_pipe_data_t error_pipe_buf[PIPE_ELEMENTS+1];

typedef void (*error_callback_t)(void *arg);
static error_callback_t error_callback = NULL;
void *error_callback_arg = NULL;

int qurt_exception_raise_nonfatal(int error)
{
	union error_msg_type x;
	qurt_sysevent_error_t my_err;
	qurt_sem_t response_sem;
	qurt_sem_init_val(&response_sem,0);
	my_err.thread_id = h2_thread_myid();
	my_err.fault_pc = (unsigned int)__builtin_return_address(0);
	my_err.sp = (unsigned int)(&my_err);
	my_err.badva = 0;
	my_err.cause = QURT_EXCEPT_EXIT;
	x.info = &my_err;
	x.response = &response_sem;
	qurt_printf("exception nonfatal: sending message %p\n",&my_err);
	qurt_pipe_send(&error_pipe,x.raw);
	qurt_sem_down(&response_sem);
	return 0;
}

void qurt_exception_wait_ext(qurt_sysevent_error_t *sys_err)
{
	union error_msg_type x;
	x.raw = qurt_pipe_receive(&error_pipe);
	qurt_printf("exception wait: msg %p recv'd\n",x.info);
	*sys_err = *x.info;
}

void qurt_exception_raise_fatal()
{
	qurt_printf("FATAL: from %p<%p<%p: STMODE\n",__builtin_return_address(0),__builtin_return_address(1),__builtin_return_address(2));
	h2_fatal_crash(H2_FATAL_STMODE);
}

void qurt_exception_shutdown_fatal()
{
	qurt_printf("Shutdown fatal.\n");
	h2_fatal_crash(H2_FATAL_CLEAN);
	if (error_callback != NULL) error_callback(error_callback_arg);
	qurt_printf("Turn off.\n");
	h2_fatal_crash(H2_FATAL_OFF);
}

void qurt_assert_error(const char *filename, int lineno)
{
	qurt_printf("assertion fail @ %s:%d\n",filename,lineno);
	while (1) qurt_exception_raise_nonfatal(12);
}

unsigned int qurt_exception_register_fatal_notification ( void(*entryfuncpoint)(void *), void *argp)
{
	error_callback = entryfuncpoint;
	error_callback_arg = argp;
	return QURT_EOK;
}

void qurt_exception_init()
{
	qurt_pipe_attr_t attrs;
	qurt_pipe_attr_init(&attrs);
	qurt_pipe_attr_set_buffer(&attrs,error_pipe_buf);
	qurt_pipe_attr_set_elements(&attrs,PIPE_ELEMENTS);
	qurt_pipe_init(&error_pipe,&attrs);
	qurt_printf("qurt exception init done\n");
}

