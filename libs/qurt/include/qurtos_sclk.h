/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURTOS_SCLK_H
#define QURTOS_SCLK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Max. client supported by QURT system sclk module
 */
#define QURT_SCLK_CLIENTS_MAX                      4

#define QURT_SCLK_STACK_SIZE                    1024

typedef unsigned int                        qurt_sclk_t;

typedef struct qurt_sclk_client {
   unsigned char allocated;
   qurt_anysignal_t *signal;
   unsigned int signal_mask;
   unsigned long long match_value;
   unsigned long long reference;
} qurt_sclk_client_t;

struct qurt_sclk_hw_status {
   unsigned long long match_value;
   unsigned long long count_value;
};

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif /* __cplusplus */

#endif /* QURTOS_SCLK_H */
