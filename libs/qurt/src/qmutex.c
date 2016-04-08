/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qmutex.h>

int qmutex_create(qmutex_t *mutex, qmutex_attr_t *attr)
{
	if ((*mutex = (qmutex_t)qurt_malloc(sizeof(qmutex_struct))) == NULL) {
		return QURT_EMEM;
	}
	(*mutex)->type = (attr == NULL) ? QMUTEX_LOCAL : attr->type;
	qurt_pimutex_init(&(*mutex)->qurt_mutex);
	return QURT_EOK;
}

