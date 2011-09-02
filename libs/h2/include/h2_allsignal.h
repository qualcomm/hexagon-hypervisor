/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ALLSIGNAL_H
#define H2_ALLSIGNAL_H

/** @file h2_allsignal.h
 @brief All-signals wake up the waiter when all bits in the mask are set
*/
/** @addtogroup h2 
@{ */

/** @brief All-signal type definition.  Please do not access directly. */
typedef union {
	unsigned long long int raw;
	struct {
		unsigned int waiting;
		unsigned int signals_in;
	};
} h2_allsignal_t;

/**
Initialize an all-signal.  No signals are being waited on, and no signals are set.
@param[in] signal	Address of the all-signal to initialize
@returns None
@dependencies None
*/

static inline void h2_allsignal_init(h2_allsignal_t *signal) { signal->raw = 0; };

/**
Wait for signals.  When all the signals in mask have been set, the thread should be woken.
@param[in] signal	Address of the all-signal to initialize
@param[in] mask		Mask of signals to wait on
@returns None
@dependencies None
*/
void h2_allsignal_wait(h2_allsignal_t *signal, unsigned int mask);

/**
Add signals to the all-signal.  This may wake up a thread waiting on signal bits.
@param[in] signal	Address of the all-signal to initialize
@param[in] mask		Signals to set in the all-signal
@returns None
@dependencies None
*/
void h2_allsignal_signal(h2_allsignal_t *signal, unsigned int mask);

/** @} */

#endif

