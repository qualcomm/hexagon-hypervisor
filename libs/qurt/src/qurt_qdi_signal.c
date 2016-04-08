/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qurt.h"
#include "qurt_signal.h"
#include "qurt_atomic_ops.h"
#include "qurt_qdi_driver.h"
#include "qurt_utcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct signal_imp {
   qurt_qdi_obj_t qdiobj;
   struct signal_group *pGroup;
   int signal_bit;
};

struct signal_group {
   qurt_qdi_obj_t qdiobj;
   unsigned bits_in_use;
   qurt_signal_t signalobj;
   int client_handle;
};

static int signal_invoke(int client_handle,
                         qurt_qdi_obj_t *obj,
                         int method,
                         qurt_qdi_arg_t a1, qurt_qdi_arg_t a2, qurt_qdi_arg_t a3,
                         qurt_qdi_arg_t a4, qurt_qdi_arg_t a5, qurt_qdi_arg_t a6,
                         qurt_qdi_arg_t a7, qurt_qdi_arg_t a8, qurt_qdi_arg_t a9)
{
   struct signal_imp *me;

   me = (struct signal_imp *)obj;

   switch (method) {
   case QDI_SIGNAL_SET:
      qurt_signal_set(&me->pGroup->signalobj, me->signal_bit);
      return 0;
   case QDI_SIGNAL_CLEAR:
      qurt_signal_clear(&me->pGroup->signalobj, me->signal_bit);
      return 0;
   case QDI_SIGNAL_WAIT:
      if (client_handle != QDI_HANDLE_LOCAL_CLIENT) {
         unsigned dummy;

         /* If client isn't local, treat it as cancellable */

         return qurt_signal_wait_cancellable(&me->pGroup->signalobj, me->signal_bit, QURT_SIGNAL_ATTR_WAIT_ANY, &dummy);
      }
      qurt_signal_wait_any(&me->pGroup->signalobj, me->signal_bit);
      return 0;
   case QDI_SIGNAL_POLL:
      return (qurt_signal_get(&me->pGroup->signalobj) & me->signal_bit) != 0;
   default:
      return qurt_qdi_method_default(client_handle, obj, method, a1, a2, a3, a4, a5, a6, a7, a8, a9);
   }
}

static void signal_release(qurt_qdi_obj_t *obj)
{
   struct signal_imp *me;
   unsigned our_bit;
   unsigned in_use;

   me = (void *)obj;

   our_bit = me->signal_bit;
   for (;;) {
      in_use = me->pGroup->bits_in_use;
      if (!(in_use & me->signal_bit)) {
         /* Problem - bail out silently */
         return;
      }
      if (qurt_atomic_compare_and_set(&me->pGroup->bits_in_use,
                                      in_use,
                                      in_use & ~our_bit))
         break;
   }

   if (in_use == our_bit) {
      /* We just released the only signal used in the group */
      if (me->pGroup->qdiobj.refcnt == 0) {
         /* All QDI references to the signal group are gone */
         /* We need to call the signal group's release function
            here, because we deferred the release when the
            last QDI reference went away due to active signals */
         (*me->pGroup->qdiobj.release)(&me->pGroup->qdiobj);
      }
   }

   free(me);
}

static int signal_new(int client_handle,
                      struct signal_group *me,
                      int *p_signal_handle_local,
                      int *p_signal_handle_remote)
{
   struct signal_imp *pSig;
   unsigned our_bit;
   unsigned in_use;
   int hRemote;
   int hLocal;

   if (client_handle != me->client_handle) {
      if (client_handle != QDI_HANDLE_LOCAL_CLIENT) {
         return -1;
      }
      client_handle = me->client_handle;
   }

   /* Allocate a bit from the signal group */
   for (;;) {
      in_use = me->bits_in_use;
      /* Next line isolates the least significant zero bit from "in_use" */
      our_bit = (~in_use) & (-(~in_use));
      if (our_bit == 0)
         return -1;
      if (qurt_atomic_compare_and_set(&me->bits_in_use,
                                      in_use,
                                      in_use | our_bit))
         break;
   }

   pSig = malloc(sizeof(*pSig));
   if (pSig) {
      pSig->qdiobj.invoke = signal_invoke;
      pSig->qdiobj.refcnt = QDI_REFCNT_INIT;
      pSig->qdiobj.release = signal_release;
      pSig->pGroup = me;
      pSig->signal_bit = our_bit;
      hLocal = qurt_qdi_new_handle_from_obj_t(QDI_HANDLE_LOCAL_CLIENT, &pSig->qdiobj);
      if (hLocal >= 0) {
         hRemote = qurt_qdi_new_handle_from_obj_t(client_handle, &pSig->qdiobj);
         if (hRemote >= 0) {
            if (qurt_qdi_copy_to_user(client_handle, p_signal_handle_local, &hLocal, sizeof(hLocal)) >= 0 &&
                qurt_qdi_copy_to_user(client_handle, p_signal_handle_remote, &hRemote, sizeof(hRemote)) >= 0) {
               return 0;
            }
            qurt_qdi_release_handle(client_handle, hRemote);
         }
         qurt_qdi_release_handle(QDI_HANDLE_LOCAL_CLIENT, hLocal);
      } else {
         free(pSig);
      }
   }
   return -1;
}

static int signal_group_invoke(int client_handle,
                               qurt_qdi_obj_t *obj,
                               int method,
                               qurt_qdi_arg_t a1, qurt_qdi_arg_t a2, qurt_qdi_arg_t a3,
                               qurt_qdi_arg_t a4, qurt_qdi_arg_t a5, qurt_qdi_arg_t a6,
                               qurt_qdi_arg_t a7, qurt_qdi_arg_t a8, qurt_qdi_arg_t a9)
{
   struct signal_group *me;

   me = (void *)obj;

   switch (method) {
   case QDI_SIGNAL_GROUP_SIGNAL_CREATE:
      return signal_new(client_handle, me, a1.ptr, a2.ptr);
   case QDI_SIGNAL_GROUP_WAIT:
      if (client_handle != QDI_HANDLE_LOCAL_CLIENT) {
         unsigned dummy;

         /* If client isn't local, treat it as cancellable */

         return qurt_signal_wait_cancellable(&me->signalobj, me->bits_in_use, QURT_SIGNAL_ATTR_WAIT_ANY, &dummy);
      }
      qurt_signal_wait_any(&me->signalobj, me->bits_in_use);
      return 0;
   case QDI_SIGNAL_GROUP_POLL:
      return (qurt_signal_get(&me->signalobj) & me->bits_in_use) != 0;
      return 0;
   default:
      return qurt_qdi_method_default(client_handle, obj, method, a1, a2, a3, a4, a5, a6, a7, a8, a9);
   }
}

static void signal_group_release(qurt_qdi_obj_t *obj)
{
   struct signal_group *pGroup;

   pGroup = (void *)obj;

   if (obj->refcnt == 0 && pGroup->bits_in_use == 0) {
      qurt_signal_destroy(&pGroup->signalobj);
      free(pGroup);
   }
}

/*
|| Function that creates a new signal group object, and returns
||  one or two handles to the object; a local handle suitable for
||  usage locally, and a remote handle suitable for usage by the
||  client whose client handle is provided.
*/

int qurt_qdi_client_signal_group_new(int client_handle,
                                     int *p_signal_group_handle_local,
                                     int *p_signal_group_handle_remote)
{
   struct signal_group *pGroup;
   int hRemote;
   int hLocal;

   pGroup = malloc(sizeof(*pGroup));
   if (pGroup) {
      pGroup->qdiobj.invoke = signal_group_invoke;
      pGroup->qdiobj.refcnt = QDI_REFCNT_INIT;
      pGroup->qdiobj.release = signal_group_release;
      pGroup->bits_in_use = 0;
      qurt_signal_init(&pGroup->signalobj);
      pGroup->client_handle = client_handle;
      hLocal = qurt_qdi_new_handle_from_obj_t(QDI_HANDLE_LOCAL_CLIENT, &pGroup->qdiobj);
      if (hLocal >= 0) {
         hRemote = qurt_qdi_new_handle_from_obj_t(client_handle, &pGroup->qdiobj);
         if (hRemote >= 0) {
            if (qurt_qdi_copy_to_user(client_handle, p_signal_group_handle_local, &hLocal, sizeof(hLocal)) >= 0 &&
                qurt_qdi_copy_to_user(client_handle, p_signal_group_handle_remote, &hRemote, sizeof(hRemote)) >= 0) {
               return 0;
            }
            qurt_qdi_release_handle(client_handle, hRemote);
         }
         qurt_qdi_release_handle(QDI_HANDLE_LOCAL_CLIENT, hLocal);
      } else {
         pGroup->qdiobj.refcnt = 0;
         signal_group_release(&pGroup->qdiobj);
      }
   }
   return -1;
}

