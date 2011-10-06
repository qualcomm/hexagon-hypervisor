/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ANYSIGNAL_H
#define H2_ANYSIGNAL_H 1

/** @file h2_allsignal.h
 @brief All-signals wake up the waiter when all bits in the mask are set
*/
/** @addtogroup h2 
@{ */

/** @brief Any-signal type definition.  Please do not access directly. */
typedef union {
	unsigned long long int volatile raw;
	struct {
		unsigned int volatile signals;
		unsigned int volatile waiting;
	};
} h2_anysignal_t;

/**
Initialize an any-signal.  No signals are being waited on, and no signals are set.
@param[in] signal	Address of the all-signal to initialize
@returns None
@dependencies None
*/

static inline void h2_anysignal_init(h2_anysignal_t *signal) { signal->raw = 0; };

/**
Wait for one of a set of signals.  When any of the signals arrive, the thread should be woken.
@param[in] signal	Address of the all-signal to wait on
@param[in] mask		Mask of signals to wait on
@returns The signal value after waking
@dependencies None
*/
unsigned int h2_anysignal_wait(h2_anysignal_t *signal, unsigned int mask);

/**
Set signals in an any-signal.  If any of the signals are being waited on, the waiter is woken.
@param[in] signal	Address of the all-signal to change
@param[in] mask		Mask of signals to set
@returns The old value of the set signals
@dependencies None
*/
unsigned int h2_anysignal_set(h2_anysignal_t *signal, unsigned int mask);

/**
Clear signals in an any-signal.  
@param[in] signal	Address of the all-signal to change
@param[in] mask		Signals to clear.
@returns The old value of the set signals
@dependencies None
*/
unsigned int h2_anysignal_clear(h2_anysignal_t *signal, unsigned int mask);

/**
Get the signals set in an any-signal.  This does not change the any-signal or wake any threads.
@param[in] signal	Address of the all-signal to examine
@returns None
@dependencies None
*/
static inline unsigned int h2_anysignal_get(h2_anysignal_t *signal) { return signal->signals; };

/** @} */

#endif

