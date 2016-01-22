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
#ifndef MPIDCH4_REQUEST_H_INCLUDED
#define MPIDCH4_REQUEST_H_INCLUDED

#include "ch4_impl.h"
#include "ch4r_buf.h"

__CH4_INLINE__ int MPIDI_Request_is_anysource(MPID_Request * req)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Request_is_pending_failure(MPID_Request * req)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ void MPIDI_Request_set_completed(MPID_Request * req)
{
    MPID_cc_set(&req->cc, 0);
    return;
}

__CH4_INLINE__ void MPIDI_Request_add_ref(MPID_Request * req)
{
    MPIR_Request_add_ref(req);
    return;
}

__CH4_INLINE__ void MPIDI_Request_release_ref(MPID_Request * req)
{
    int inuse;
    MPIR_Request_release_ref(req, &inuse);
    return;
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

#undef FUNCNAME
#define FUNCNAME MPIDI_request_release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_Request_release(MPID_Request * req)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_REQUEST_RELEASE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_REQEUST_RELEASE);

    MPIDI_CH4I_Request_release(req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_REQUEST_RELEASE);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_request_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Request_complete(MPID_Request * req)
{
    int count;
    MPID_cc_decr(req->cc_ptr, &count);
    MPIU_Assert(count >= 0);
    MPIDI_CH4I_Request_release(req);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_request_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_Request_create(void)
{
    MPID_Request *req;
    MPIDI_STATE_DECL(MPID_STATE_CH4_REQUEST_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_REQEUST_CREATE);

    req = MPIDI_CH4I_alloc_and_init_req(2);
    MPIDI_CH4I_REQUEST(req,reqtype) = MPIDI_CH4_REQTYPE_MPI;
    MPIDI_CH4R_REQUEST(req, req)    = NULL;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_REQUEST_CREATE);
    return req;
}

#endif /* MPIDCH4_REQUEST_H_INCLUDED */
