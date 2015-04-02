/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURTOS_TIMER_SERVER_H
#define QURTOS_TIMER_SERVER_H

int qurt_timer_lib_timer_stop ( qurt_timer_client_ptr timer);
int qurt_timer_lib_timer_restart ( qurt_timer_client_ptr timer, qurt_timer_duration_t time);
int qurt_timer_lib_timer_activate ( qurt_timer_t *client, qurt_timer_duration_t duration, qurt_timer_time_t expiry,  
																		qurt_timer_type_t type, const qurt_anysignal_t *signal, unsigned int mask , unsigned int group);
int qurt_timer_lib_get_attr ( qurt_timer_client_ptr client, qurt_timer_attr_t *attr );
int qurt_timer_lib_timer_cancel ( qurt_timer_client_ptr client );
int qurt_timer_lib_sleep_activate (  qurt_timer_duration_t duration, qurt_anysignal_t *signal, unsigned int mask);

#endif
