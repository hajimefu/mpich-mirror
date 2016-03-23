/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef REQUEST_H_INCLUDED
#define REQUEST_H_INCLUDED

#include "impl.h"
#include "mpidch4.h"
#include <ucp/api/ucp.h>
#include "mpidch4r.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_request_release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
extern MPIU_Object_alloc_t MPIDI_Request_mem;
static inline void MPIDI_CH4_NM_am_request_init(MPID_Request *req)
{
    req->dev.ch4.ch4u.netmod_am.ucx.pack_buffer = NULL;
}

static inline void MPIDI_CH4_NM_am_request_finalize(MPID_Request *req)
{
    if ((req)->dev.ch4.ch4u.netmod_am.ucx.pack_buffer) {
        MPL_free((req)->dev.ch4.ch4u.netmod_am.ucx.pack_buffer);
    }
    /* MPIDI_CH4R_Request_release(req); */
}


#if 0
static inline void MPIDI_CH4_NM_request_release(MPID_Request * req)
{
    int count;

    MPIDI_STATE_DECL(MPID_STATE_MPIDI_CH4_NM_REQUEST_RELEASE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_CH4_NM_REQUEST_RELEASE);


    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIU_Object_release_ref(req, &count);
    printf("release request %d %d\n", count,MPIDI_CH4_NMI_UCX_REQ(req).is_ucx_req);
    MPIU_Assert(count >= 0);
    if (count == 0) {
        MPIU_Assert(MPID_cc_is_complete(&req->cc));
        if (req->comm){
            MPIR_Comm_release(req->comm);
            req->comm = NULL;
        }
        if (req->greq_fns)
            MPL_free(req->greq_fns);
        if(!MPIDI_CH4_NMI_UCX_REQ(req).is_ucx_req)
             MPIU_Handle_obj_free(&MPIDI_Request_mem, req);
        else{
            MPIDI_CH4_NMI_UCX_REQ(req).is_call_done = 0;
            MPIDI_CH4_NMI_UCX_REQ(req).in_nb = 1;
            ucp_request_release(MPIDI_CH4_NMI_UCX_REQ(req).ucx_request);
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_CH4_NM_REQUEST_RELEASE);
    return;
}
#endif

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_request_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_netmod_request_init(MPID_Request* req)
{
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_NETMOD_REQUEST_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_NETMOD_REQUEST_INIT);

    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIR_cc_set(&req->cc, 1);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, 2);
    req->greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG = MPI_UNDEFINED;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->comm = NULL;
    req->errflag = MPIR_ERR_NONE;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_NETMODE_REQUEST_INIT);

}
#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_request_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline MPID_Request* MPIDI_CH4_NMI_UCX_Request_create()
{
    MPID_Request *req;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_NETMOD_REQUEST_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_NETMOD_REQUEST_CREATE);
    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
    if (req == NULL)
        MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request");
    MPIU_Assert(req != NULL);

    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIR_cc_set(&req->cc, 1);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, 2);
    req->greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG = MPI_UNDEFINED;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->comm = NULL;
    req->errflag = MPIR_ERR_NONE;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_NETMODE_REQUEST_CREATE);
    return req;
}
#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_UCX_Alloc_send_request_done()
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline MPID_Request *MPIDI_CH4_NMI_UCX_Alloc_send_request_done()
{
    MPID_Request *req;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_NETMOD_REQUEST_SEND_DONE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_NETMOD_REQUEST_SEND_DONE);

    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
    if (req == NULL)
        MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request");
    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIR_cc_set(&req->cc, 0);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, 1);
    req->greq_fns = NULL;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->kind = MPID_REQUEST_SEND;
    req->comm = NULL;
    req->errflag = MPIR_ERR_NONE;
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_NETMOD_REQUEST_SEND_DONE);
    return req;
}
static inline MPID_Request  *MPIDI_CH4_NM_UCX_Alloc_recv_request_done()
{
    MPID_Request *req;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_NETMOD_REQUEST_RECV_DONE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_NETMOD_REQUEST_RECV_DONE);
    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
    if (req == NULL)
        MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request");

    req->kind = MPID_REQUEST_RECV;
    req->partner_request   = NULL;
    MPIR_cc_set(&req->cc, 0);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, 1);
    req->greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->errflag = MPIR_ERR_NONE;
    req->kind = MPID_REQUEST_RECV;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_NETMOD_REQUEST_RECV_DONE);
    return req;
}
static inline void MPIDI_CH4_NMI_UCX_Request_init_callback(void *request)
{

    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) request;
    ucp_request->req = NULL;

}

static inline void MPIDI_CH4_NMI_UCX_Handle_send_callback(void *request, ucs_status_t status)
{
    int c;
    int mpi_errno;

    MPIDI_CH4_NMI_UCX_Ucp_request_t* ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) request;
    MPID_Request *req = NULL;
    if(ucp_request->req){
        req = ucp_request->req;
        MPIR_cc_decr(req->cc_ptr, &c);
        MPIU_Assert(c >= 0);

        if (c == 0) {
            MPIDI_Request_release(req);
        }
        ucp_request->req = NULL;
    }
    else {
        req = MPIDI_CH4_NMI_UCX_Alloc_send_request_done();
        ucp_request->req = req;
    }
fn_exit:
    return;
fn_fail:
    req->status.MPI_ERROR = mpi_errno;
}

static inline void MPIDI_CH4_NMI_UCX_Handle_recv_callback(void *request, ucs_status_t status,
                                                          ucp_tag_recv_info_t * info)
{
    int count;
    int mpi_errno;
    MPIDI_CH4_NMI_UCX_Ucp_request_t* ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) request;
    MPID_Request *rreq = NULL;
    if(!ucp_request->req) {
        rreq = MPIDI_CH4_NM_UCX_Alloc_recv_request_done();
        rreq->status.MPI_SOURCE = MPIDI_CH4_NMI_UCX_get_source(info->sender_tag);
        rreq->status.MPI_TAG = MPIDI_CH4_NMI_UCX_get_tag(info->sender_tag);
        count = info->length;
        MPIR_STATUS_SET_COUNT(rreq->status, count);
        ucp_request->req = rreq;
    }
    else {
        rreq = ucp_request->req;
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        rreq->status.MPI_SOURCE = MPIDI_CH4_NMI_UCX_get_source(info->sender_tag);
        rreq->status.MPI_TAG = MPIDI_CH4_NMI_UCX_get_tag(info->sender_tag);
        count = info->length;
        MPIR_STATUS_SET_COUNT(rreq->status, count);
        MPIDI_CH4R_request_complete(rreq);
        ucp_request->req = NULL;
    }

  fn_exit:
    return;
  fn_fail:
    rreq->status.MPI_ERROR = mpi_errno;
}

#endif /* REQUEST_H_INCLUDED */
