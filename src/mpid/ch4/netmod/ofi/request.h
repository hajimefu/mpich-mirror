/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef NETMOD_OFI_REQUEST_H_INCLUDED
#define NETMOD_OFI_REQUEST_H_INCLUDED

#include "impl.h"

static inline int MPIDI_netmod_request_is_anysource(MPID_Request * req)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_request_is_pending_failure(MPID_Request * req)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

/* Support for active message and direct requests are a bit tricky:
   MPI layer can call MPID_Request_create/release/complete and operate on
   a non-specific request, with the device layer implementing all bookkeeping.

   CH4 layer implements the device-level functions by calling the netmod.

   We assume that if CH4 is allocating the request, it must be an AM
   OR an unpsecified request that won't be used by communication
   directly.  So we just allocate an AM request and it should
   suffice for both instances.

   On release (also called by the upper layers), the netmod and ch4
   do not have any information about the type of this request and how
   to free the internal structures, so we have to query the type, which
   is a common offset/field in both am and direct requests.

   These functions should only be called by the upper layers because
   we can optimize away branches for internal release functions when
   the type is completely determined by the operation being used.
*/

static inline void MPIDI_CH4_NM_request_release(MPID_Request * req)
{
    switch (MPIDI_CH4I_REQUEST(req,reqtype)) {
    case MPIDI_CH4_DEVTYPE_DIRECT:
    case MPIDI_CH4_DEVTYPE_SHM:
    case MPIDI_CH4_DEVTYPE_UNSPECIFIED:
        MPIDI_CH4_NMI_OFI_request_release(req);
        break;
    case MPIDI_CH4_DEVTYPE_AM:
        MPIDI_CH4_NMI_OFI_AM_request_release(req);
        break;
    }
}

static inline MPID_Request *MPIDI_CH4_NM_request_create(void)
{
    return MPIDI_AM_netmod_request_create();
}

#endif /* NETMOD_OFI_REQUEST_H_INCLUDED */
