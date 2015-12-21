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

static inline void MPIDI_netmod_request_release(MPID_Request * req)
{
    int count;
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIU_Object_release_ref(req, &count);
    MPIU_Assert(count >= 0);

    if (count == 0) {
        MPIU_Assert(MPID_cc_is_complete(&req->cc));

        if (req->comm)
            MPIR_Comm_release(req->comm);

        if (req->greq_fns)
            MPIU_Free(req->greq_fns);

        MPIU_Handle_obj_free(&MPIDI_Request_mem, req);
    }

    return;
}

static inline MPID_Request *MPIDI_netmod_request_create(void)
{
    MPID_Request *req;
    req = MPIDI_Request_alloc_and_init(1);
    return req;
}

#endif /* NETMOD_OFI_REQUEST_H_INCLUDED */
