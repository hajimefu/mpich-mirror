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
#ifndef NETMOD_AM_OFI_REQUEST_H_INCLUDED
#define NETMOD_AM_OFI_REQUEST_H_INCLUDED

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

static inline void MPIDI_netmod_request_set_completed(MPID_Request * req)
{
    MPIU_Assert(0);
    return;
}

extern MPIU_Object_alloc_t MPIDI_Request_mem;
static inline MPID_Request *MPIDI_netmod_request_alloc_and_init(int count)
{
    MPID_Request *req;
    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPID_cc_set(&req->cc, 1);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, count);
    req->greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG = MPI_UNDEFINED;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->comm = NULL;
    AMREQ_OFI(req, req_hdr) = NULL;
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_am_ofi_clear_req
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_netmod_am_ofi_clear_req(MPID_Request *sreq)
{
    MPIDI_am_ofi_req_hdr_t *req_hdr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);

    req_hdr = AMREQ_OFI(sreq, req_hdr);
    if (!req_hdr)
        return;

    if (req_hdr->am_hdr != &req_hdr->am_hdr_buf[0]) {
        MPIU_Free(req_hdr->am_hdr);
    }
    MPIU_CH4U_release_buf(req_hdr);
    AMREQ_OFI(sreq, req_hdr) = NULL;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);
    return;
}

static inline void MPIDI_netmod_request_release(MPID_Request * req)
{
    int count;
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIU_Object_release_ref(req, &count);
    MPIU_Assert(count >= 0);
    if (count == 0) {
        MPIU_Assert(MPID_cc_is_complete(&req->cc));
        MPIDI_netmod_am_ofi_clear_req(req);

        if (req->comm)
            MPIR_Comm_release(req->comm);

        if (req->greq_fns)
            MPIU_Free(req->greq_fns);

        MPIU_Handle_obj_free(&MPIDI_Request_mem, req);
    }
}

static inline MPID_Request *MPIDI_netmod_request_create(void)
{
    MPID_Request *req = MPIDI_netmod_request_alloc_and_init(1);
    return req;
}

static inline void MPIDI_netmod_am_ofi_req_complete(MPID_Request *req)
{
    int count;
    MPID_cc_decr(req->cc_ptr, &count);
    MPIU_Assert(count >= 0);
    if (count == 0)
        MPIDI_Request_release(req);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_am_ofi_init_req
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_am_ofi_init_req(const void *am_hdr,
                                               size_t am_hdr_sz,
                                               MPID_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_am_ofi_req_hdr_t *req_hdr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);

    if (AMREQ_OFI(sreq, req_hdr) == NULL) {
        req_hdr = (MPIDI_am_ofi_req_hdr_t *)
            MPIU_CH4U_get_buf(MPIDI_Global.buf_pool);
        MPIU_Assert(req_hdr);
        AMREQ_OFI(sreq, req_hdr) = req_hdr;

        req_hdr->am_hdr = (void *) &req_hdr->am_hdr_buf[0];
        req_hdr->am_hdr_sz = MPIDI_MAX_AM_HDR_SZ;
    } else {
        req_hdr = AMREQ_OFI(sreq, req_hdr);
    }

    if (am_hdr_sz > req_hdr->am_hdr_sz) {
        if (req_hdr->am_hdr != &req_hdr->am_hdr_buf[0])
            MPIU_Free(req_hdr->am_hdr);
        req_hdr->am_hdr = MPIU_Malloc(am_hdr_sz);
        MPIU_Assert(req_hdr->am_hdr);
        req_hdr->am_hdr_sz = am_hdr_sz;
    }

    if (am_hdr) {
        MPIU_Memcpy(req_hdr->am_hdr, am_hdr, am_hdr_sz);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);
    return mpi_errno;
}

#endif /* NETMOD_AM_OFI_REQUEST_H_INCLUDED */
