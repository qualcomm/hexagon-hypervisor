/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef UTIL_H_
#define UTIL_H_

#define MAX_LIST_ITEMS 32

typedef struct list_str
{
	unsigned int data[MAX_LIST_ITEMS];
	unsigned int current;
} List;

typedef struct {
    unsigned int ch_addr;
    List msgqs;
}Channels_t;

#endif /* UTIL_H_ */
