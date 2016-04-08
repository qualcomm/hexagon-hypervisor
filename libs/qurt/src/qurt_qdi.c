/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qurt.h"
#include "qurt_atomic_ops.h"
// #include "qurt_qdi_internal.h"
#include "qurt_qdi_driver.h"
#include "qurt_utcb.h"
#include <stdio.h>
#include <string.h>

int qurt_qdi_method_default(int client_handle,
                            qurt_qdi_obj_t *obj,
                            int method,
                            qurt_qdi_arg_t a1, qurt_qdi_arg_t a2, qurt_qdi_arg_t a3,
                            qurt_qdi_arg_t a4, qurt_qdi_arg_t a5, qurt_qdi_arg_t a6,
                            qurt_qdi_arg_t a7, qurt_qdi_arg_t a8, qurt_qdi_arg_t a9)
{
   switch (method) {
   case QDI_CLOSE:
      return qurt_qdi_release_handle(client_handle, a1.num);
   default:
      return -1;
   }
}
