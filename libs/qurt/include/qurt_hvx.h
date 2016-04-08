/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_HVX_H
#define QURT_HVX_H
/**
  @file qurt_hvx.h 
  @brief   Prototypes of Qurt HVX API.  

EXTERNAL FUNCTIONS
   None.

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2014  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================*/

typedef enum {
    QURT_HVX_MODE_64B = 0,      /**< HVX mode of 64 bytes */
    QURT_HVX_MODE_128B = 1      /**< HVX mode of 128 bytes */
} qurt_hvx_mode_t;

#define QURT_HVX_HW_UNITS_2X128B_4X64B        0x00000204       /* bit#15-#8 are for number of 128B units   */
                                                               /* bit#7-#0 are for number of 64B units     */
/* Input value for HVX reservation */

#define QURT_HVX_RESERVE_ALL                  (4)              /* All the HVX units in terms of 64B_MODE are requested to be reserved */

/* Return values for HVX reservation */

#define QURT_HVX_RESERVE_NOT_SUPPORTED        (-1)             /* There is no HVX hardware, or less units in the hardware than requested */
#define QURT_HVX_RESERVE_NOT_SUCCESSFUL       (-2)             /* Some HVX units are already locked/reserved by other PD, thus not enough units left for the reservation. */
#define QURT_HVX_RESERVE_ALREADY_MADE         (-3)             /* There is already a HVX reservation made. */
#define QURT_HVX_RESERVE_CANCEL_ERR           (-4)             /* The action of cancling the reservation fails because this protection domain has no reservation made before. */

/**@ingroup func_qurt_hvx_lock
  Lock one HVX unit specified by the HVX mode.
  
  @note1hang Input variable can be 128B_MODE or 64B_MODE etc. If a HVX unit in this mode 
             is available, this function will lock the unit and returns right away;
             if the current HVX mode is different from the requested mode, the current 
             thread will be blocked. When all HVX units become idle, Qurt will change 
             the mode, lock the HVX unit, and return.  
  
  @datatypes
  #qurt_mode_t
  
  @param[in]  lock_mode, either QURT_HVX_MODE_64B or QURT_HVX_MODE_128B

  @param[out] 
  None.

  @return
  QURT_EOK - successful return
  Others - failure

  @dependencies
  None.
  
 */
int qurt_hvx_lock(qurt_hvx_mode_t lock_mode);

/**@ingroup func_qurt_hvx_unlock
  Unlock the HVX unit held by this software thread.
  
  @note1hang  Unlock the HVX unit hold by this software thread.  
  
  @datatypes
  None.
  
  @param[out] 
  None.

  @return
  QURT_EOK - successful return
  Others - failure

  @dependencies
  None.
  
 */
int qurt_hvx_unlock(void);

/**@ingroup func_qurt_hvx_try_lock
  Try to lock one HVX unit specified by the HVX mode.
  
  @note1hang Input variable can be 128B_MODE or 64B_MODE etc. If a HVX unit in this mode 
             is available, this function will lock the unit and returns QURT_EOK; Otherwise,
             the function will return a failure, but will not block the current software 
             thread to wait for the HVX unit.
  
  @datatypes
  #qurt_mode_t
  
  @param[out] 
  None.

  @return
  QURT_EOK - successful return
  Others - failure

  @dependencies
  None.
  
 */
int qurt_hvx_try_lock(qurt_hvx_mode_t lock_mode);

/**@ingroup func_qurt_hvx_get_mode
  Get the current HVX mode configured by Qurt.
  
  @note1hang The function returns QURT_HVX_MODE_128B or QURT_HVX_MODE_64B, based on 
             the current HVX configuration
  
  @datatypes
  None.
  
  @param[out] 
  None.

  @return
  QURT_HVX_MODE_128B 
  QURT_HVX_MODE_64B
  -1:  not available

  @dependencies
  None.
  
 */
int qurt_hvx_get_mode(void);

/**@ingroup func_qurt_hvx_get_units
  Get the HVX hardware configuration supported by the chipset.
  
  @note1hang The function returns the HVX hardware configuration supported by the chipset.
  
  @datatypes
  None.
  
  @param[out] 
  None.

  @return
     The bit-mask of the units, 1X64, 2X64, 4X64, 1X128, 2X128, etc.
      - QURT_HVX_HW_UNITS_2X126B_4X64B : for V60A HVX
      - 0:  not available

  @dependencies
  None.
  
 */
int qurt_hvx_get_units(void);

/**@ingroup func_qurt_hvx_reserve
  Reserve HVX units in terms of 64-byte mode for the caller's protection domain (PD).
  
  @note1hang Only one HVX reservation in the system is supported, as of now.
  
  @datatypes
  None.

  @param[in]  num_units  Number of HVX units in terms of 64B_MODE to be reserved for the PD
                         or QURT_HVX_RESERVE_ALL to reserve all the HVX units.

  @param[out] 
  None.

  @return
    - number of units successfully reserved 
    - QURT_HVX_RESERVE_NOT_SUPPORTED      
    - QURT_HVX_RESERVE_NOT_SUCCESSFUL     
    - QURT_HVX_RESERVE_ALREADY_MADE    

  @dependencies
  None.
  
 */
int qurt_hvx_reserve(int num_units);

/**@ingroup func_qurt_hvx_cancel_reserve
  Cancel the HVX reservation in the caller's protection domain (PD).
  
  @note1hang Only one HVX reservation in the system is supported, as of now.
  
  @datatypes
  None.

  @param[in]
  None.

  @param[out] 
  None.

  @return
    - 0, sucess
    - QURT_HVX_RESERVE_CANCEL_ERR, failure      

  @dependencies
  None.
  
 */
int qurt_hvx_cancel_reserve(void);

#endif /* QURT_HVX_H */

