/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_SLEEP_H
#define H2_SLEEP_H 1

unsigned long long int h2_nanosleep(unsigned long long int time);
static inline unsigned long long int h2_microsleep(unsigned long long int time) { return h2_nanosleep(time*1000)/1000; }
static inline unsigned long long int h2_millisleep(unsigned long long int time) { return h2_microsleep(time*1000)/1000; }
static inline unsigned long long int h2_sleep(unsigned long long int time) { return h2_millisleep(time*1000)/1000; }

#endif

