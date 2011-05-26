/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_SCLK_H
#define BLAST_SCLK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Max. client supported by BLAST system sclk module
 */
#define MAX_BLAST_SCLK_CLIENTS                      4

/**
 * Register a client for BLAST system sclk
 *
 * @param   signal       blast_signal_t to be registered
 * @param   signal_mask  mask to be set to indicate teh sclk expiry
 *
 * @return  client id, -1 for failure 
 *            
 */
int blast_system_sclk_register (blast_anysignal_t *signal, unsigned int signal_mask);

/**
 * Program the SCLK timer expiry
 *
 * @param   id          Client Id to be programmed for SCLK timer expiry
 * @param   ref_count   sclk count when the match value was calculated
 * @param   match_value match value to be programmed on SCLK HW
 *
 * @return  match value programmed
 *            
 */
int blast_system_sclk_alarm (int id, unsigned int ref_count, unsigned int match_value);

/**
 * Program the SCLK timer expiry
 *
 * @param   id          Client Id to be programmed for SCLK timer expiry
 * @param   duration    duration to be SCLK ticks
 * @param   match_value match value to be programmed on SCLK HW
 *
 * @return  EOK
 *            
 */
int blast_system_sclk_timer (int id, unsigned int duration);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif /* __cplusplus */

#endif /* BLAST_SCLK_H */
