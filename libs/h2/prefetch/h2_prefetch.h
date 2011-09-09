/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PREFETCH_H
#define H2_PREFETCH_H

/** @file h2_prefetch.h
 @brief Configure prefetch settings
*/
/** @addtogroup h2 
@{ */

#define H2_PREFETCH_L1I_OFF 0		/**< Turn L1 Instruction Prefetching Off */
#define H2_PREFETCH_L1I_LOW 1		/**< Turn L1 Instruction Prefetching to Low */
#define H2_PREFETCH_L1I_MED 2		/**< Turn L1 Instruction Prefetching to Medium */
#define H2_PREFETCH_L1I_HIGH H2_PREFETCH_L1I_MED		/**< Turn L1 Instruction Prefetching to High */
#define H2_PREFETCH_L1I H2_PREFETCH_L1I_HIGH	/**< Default L1 Instruction Prefetching Setting */

#define H2_PREFETCH_L1D_OFF 0		/**< Turn L1 Data Prefetching Off */
#define H2_PREFETCH_L1D_LOW H2_PREFETCH_L1D_OFF		/**< Turn L1 Data Prefetching to Low */
#define H2_PREFETCH_L1D_MED H2_PREFETCH_L1D_LOW		/**< Turn L1 Data Prefetching to Medium */
#define H2_PREFETCH_L1D_HIGH H2_PREFETCH_L1D_MED	/**< Turn L1 Data Prefetching to High */
#define H2_PREFETCH_L1D H2_PREFETCH_L1D_HIGH		/**< Default L1 Data Prefetching Setting */

#define H2_PREFETCH_L2I_OFF 0x00	/**< Turn L2 Instruction Prefetching Off */
#define H2_PREFETCH_L2I_LOW 0x08	/**< Turn L2 Instruction Prefetching to Low */
#define H2_PREFETCH_L2I_MED 0x10	/**< Turn L2 Instruction Prefetching to Medium */
#define H2_PREFETCH_L2I_HIGH 0x18	/**< Turn L2 Instruction Prefetching to High */
#define H2_PREFETCH_L2I H2_PREFETCH_L2I_HIGH	/**< Default L2 Instruction Prefetching Setting */

#define H2_PREFETCH_L2D_OFF 0x00	/**< Turn L2 Data Prefetching Off */
#define H2_PREFETCH_L2D_LOW H2_PREFETCH_L2D_OFF	/**< Turn L2 Data Prefetching to Low */
#define H2_PREFETCH_L2D_MED H2_PREFETCH_L2D_LOW	/**< Turn L2 Data Prefetching to Medium */
#define H2_PREFETCH_L2D_HIGH H2_PREFETCH_L2D_MED	/**< Turn L2 Data Prefetching to High */
#define H2_PREFETCH_L2D H2_PREFETCH_L2D_HIGH	/**< Default L2 Data Prefetching Setting */

#define H2_PREFETCH_I (H2_PREFETCH_L1I | H2_PREFETCH_L2I)	/**< Default Instruction Prefetching */
#define H2_PREFETCH_D (H2_PREFETCH_L1D | H2_PREFETCH_L2D)	/**< Default Data Prefetching */
#define H2_PREFETCH_SW (0)					/**< Default SW Prefetching */

/**
Set prefetch settings.
@param[in] settings	Selected Prefetch Settings
@returns None
@dependencies None
*/

void h2_set_prefetch(unsigned int settings);

/** @} */

#endif

