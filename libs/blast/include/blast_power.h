/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_POWER_H
#define BLAST_POWER_H

#include <blast_error.h>

#define BLAST_POWER_SHUTDOWN       0
#define BLAST_TCXO_SHUTDOWN        1

#define BLAST_POWER_CMD_PREPARE    0
#define BLAST_POWER_CMD_PERFORM    1
#define BLAST_POWER_CMD_EXIT       2
#define BLAST_POWER_CMD_FAIL_EXIT  3

static inline int blast_power_control (int power, int tcxo_control)
{
  return EOK;
}

static inline int blast_power_shutdown_prepare (void)
{
  return EOK;
}

static inline int blast_power_shutdown_fail_exit (void)
{
  return EOK;
}

static inline int blast_power_shutdown (void)
{
  return EOK;
}

static inline int blast_power_shutdown_exit (void)
{
  return EOK;
}

static inline int blast_power_tcxo_prepare (void)
{
  return EOK;
}

static inline int blast_power_tcxo_fail_exit (void)
{
  return EOK;
}

static inline int blast_power_tcxo_enter (void)
{
  return EOK;
}

static inline int blast_power_tcxo_exit (void)
{
  return EOK;
}

static inline void blast_power_wait_for_idle (void)
{
}

static inline void blast_power_wait_for_active (void)
{
}

static inline void blast_system_ipend_clear (unsigned int ipend)
{
}

#endif

