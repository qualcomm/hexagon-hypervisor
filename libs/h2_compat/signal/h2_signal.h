/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_SIGNAL_H
#define H2_SIGNAL_H 1

#include <h2_atomic.h>

/** @file h2_signal.h
 @brief Any-signals wake up the waiter when any bits in the mask are set
*/
/** @addtogroup h2 
@{ */

/** @brief Any-signal type definition.  Please do not access directly. */
typedef union {
	unsigned long long int volatile raw;
	struct {
		unsigned int volatile signals;
		unsigned int volatile mask;
	};
} h2_signal_t;

enum {
	H2_SIGNAL_WAIT_ANY = 0,
	H2_SIGNAL_WAIT_ALL,
};

/**
Initialize an any-signal.  No signals are being waited on, and no signals are set.
@param[in] signal	Address of the any-signal to initialize
@returns None
@dependencies None
*/
void h2_signal_init(h2_signal_t *signal);

/**
Wait for one of a set of signals.  When any of the signals arrive, the thread should be woken.
@param[in] signal	Address of the any-signal to wait on
@param[in] mask		Mask of signals to wait on
@returns The signal value after waking
@dependencies None
*/
unsigned int h2_signal_wait_any(h2_signal_t *signal, unsigned int mask);

/**
Wait for all of a set of signals.  When any of the signals arrive, the thread should be woken.
@param[in] signal	Address of the any-signal to wait on
@param[in] mask		Mask of signals to wait on
@returns The signal value after waking
@dependencies None
*/
unsigned int h2_signal_wait_all(h2_signal_t *signal, unsigned int mask);

/**
Wait for all of a set of signals.  When any of the signals arrive, the thread should be woken.
@param[in] signal	Address of the any-signal to wait on
@param[in] mask		Mask of signals to wait on
@returns The signal value after waking
@dependencies None
*/
unsigned int h2_signal_wait(h2_signal_t *signal, unsigned int mask, unsigned int type);

/**
Set signals in an any-signal.  If any of the signals are being waited on, the waiter is woken.
@param[in] signal	Address of the any-signal to change
@param[in] mask		Mask of signals to set
@returns The new value of the set signals
@dependencies None
*/
unsigned int h2_signal_set(h2_signal_t *signal, unsigned int mask);

/**
Clear signals in an any-signal.  Bits set in mask are cleared in the signal.
@param[in] signal	Address of the any-signal to change
@param[in] mask		Signals to clear.
@returns The new value of the set signals
@dependencies None
*/
unsigned int h2_signal_clear(h2_signal_t *signal, unsigned int mask);

/**
Get the signals set in an any-signal.  This does not change the any-signal or wake any threads.
@param[in] signal	Address of the any-signal to examine
@returns None
@dependencies None
*/
unsigned int h2_signal_get(h2_signal_t *signal);

/** @} */

#endif
