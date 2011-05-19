/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qube.h>

/**
 * Initializes a callback
 *
 * @param cb       [OUT] Callback object
 * @param msgq           Message queue object
 * @param msgtype        Type of the message
 * @param msg            Message data
 * @param size           Size of the message
 * @return               EOK:       Operation is successful,
 *                       EMEM:      Out of memory error,
 *                       EFAILED:   IPC related failures,
 *                       EINVALID:  Wrong channel, attribute pointer, etc,
 *                       EVAL:      Wrong values for parameters like 
 *                                  null value for queue name, 
 *                       ENO_MSGQ:  Message Queue doesn't exist.
 */
int qmsgq_callback_init ( qmsgq_callback_t *cb, qmsgq_t msgq, int msgtype, const void *msg, size_t size )
{
	/* Initialize the callback */
	cb->msgq         =   msgq;
	cb->msg_type     =   msgtype;
	cb->msg_size     =   size;
	cb->msg          =   msg;

	/* Return success */
	return EOK;
} /* qmsgq_callback_init */

/**
 * Allocates a message node structure and copies data from callback
 * 
 * Note: This function is executed from message queue server.
 *
 * @param cb          Pointer to the qmsgq_callback_t.
 * @param token [out] Pointer to the message node structure.
 * @return            EOK if send is successful, error code otherwise
 */
int qmsgq_lib_allocate_callback (qmsgq_callback_t* cb, unsigned int *token)
{
	/* Pointer to the message structure */
	qmsgq_callback_t *new_cb;

	/* Allocate space for message structure */
	if ((new_cb = (qmsgq_callback_t *)blast_malloc (cb->msg_size + sizeof (qmsgq_callback_t))) == NULL) {
		return EMEM;
	}

	/* Initialize the newly allocated message structure */
	new_cb->msgq = cb->msgq;
	new_cb->msg_type = cb->msg_type;
	new_cb->msg_size = cb->msg_size;
        new_cb->msg = (void *)(new_cb + 1);

	/* Copy Message to the message queue buffer, when size is non-zero */
	if (cb->msg_size) {
		/* Copy message from source to destination */
		memcpy ((char *)new_cb->msg, cb->msg, cb->msg_size);
	}

	/* Store token */
	*token = (unsigned int)new_cb;

	/* Return result */
	return EOK;
}

/**
 * Complete message send operation using message callback
 * Note: This function is executed from message queue server pd.
 *
 * The qmsgq_send is used to send a message asynchronously to a message queue.
 * The node identifies the queue where message should be added. 
 * 
 * @param token     Pointer to the message node structure created using qmsgq_lib_allocate_callback.
 * @return          EOK if send is successful, error code otherwise
 */
int qmsgq_lib_process_callback (unsigned int token)
{
	/* Result, success or failure */
	int result;
	/* Pointer to the Message */
	qmsgq_callback_t *cb;
    
	/* Assign the message pointer */
	cb = (qmsgq_callback_t *)token;
	/* Perform the requested operation and return result */
        result = qmsgq_send (cb->msgq, cb->msg_type,  cb->msg,  cb->msg_size, NULL, QPERM_FULL);

	/* Return result */
	return result;
} /* qmsgq_lib_process_callback */

/**
 * This function will free the message structure allocated for callback
 * Note: This function is executed from message queue server pd.
 *
 * @param token     Pointer to the message node structure created using qmsgq_lib_allocate_callback.
 * @return          EOK if send is successful, error code otherwise
 */
int qmsgq_lib_free_callback (unsigned int token )
{
	/* Pointer to the callback structure */
	qmsgq_callback_t *cb;
   
	/* Assign pointer */
	cb = (qmsgq_callback_t *)token;
   
	/* Verify Pointer */
	if (cb) {
		free (cb);
		return EOK;
	}
	return EINVALID;
}
