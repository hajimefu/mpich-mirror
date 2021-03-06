/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef NETMOD_UCX_AM_H_INCLUDED
#define NETMOD_UCX_AM_H_INCLUDED

#include "ucx_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_reg_hdr_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_reg_hdr_handler(int handler_id,
                                               MPIDI_NM_am_origin_handler_fn origin_handler_fn,
                                               MPIDI_NM_am_target_handler_fn target_handler_fn)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_REG_HDR_HANDLER);

    MPIDI_UCX_global.am_handlers[handler_id] = target_handler_fn;
    MPIDI_UCX_global.send_cmpl_handlers[handler_id] = origin_handler_fn;
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


static inline void MPIDI_UCX_send_am_callback(void *request, ucs_status_t status)
{
    MPIDI_UCX_ucp_request_t* ucp_request = (MPIDI_UCX_ucp_request_t*) request;

    if(ucp_request->req){
        MPIR_Request *req = ucp_request->req;
        int handler_id = req->dev.ch4.ch4u.netmod_am.ucx.handler_id;

        MPL_free(req->dev.ch4.ch4u.netmod_am.ucx.pack_buffer);
        req->dev.ch4.ch4u.netmod_am.ucx.pack_buffer = NULL;
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](req);
        ucp_request->req = NULL;
    } else {
        ucp_request->req = (void *)TRUE;
    }

fn_exit:
    return;
fn_fail:
    goto fn_exit;
}

static inline void MPIDI_UCX_inject_am_callback(void *request, ucs_status_t status)
{
    MPIDI_UCX_ucp_request_t* ucp_request = (MPIDI_UCX_ucp_request_t*) request;

    if(ucp_request->req){
        MPL_free(ucp_request->req);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        ucp_request->req = (void *)TRUE;
    }

fn_exit:
    return;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_am_hdr(int           rank,
                                           MPIR_Comm    *comm,
                                           int           handler_id,
                                           const void   *am_hdr,
                                           size_t        am_hdr_sz,
                                           MPIR_Request *sreq,
                                           void         *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_UCX_am_header_t ucx_hdr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR);

    ep = MPIDI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.data_sz = 0;

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);

    ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     &MPIDI_UCX_send_am_callback);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4u.netmod_am.ucx.pack_buffer = send_buf;
        sreq->dev.ch4.ch4u.netmod_am.ucx.handler_id = handler_id;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_am(int rank,
                                       MPIR_Comm * comm,
                                       int handler_id,
                                       const void *am_hdr,
                                       size_t am_hdr_sz,
                                       const void *data,
                                       MPI_Count count,
                                       MPI_Datatype datatype,
                                       MPIR_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPIR_Datatype  *dt_ptr;
    int             dt_contig;
    MPIDI_UCX_am_header_t ucx_hdr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_SEND_AM);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    if (handler_id == MPIDI_CH4U_SEND &&
        am_hdr_sz + sizeof(MPIDI_UCX_am_header_t) + data_sz > MPIDI_UCX_MAX_AM_EAGER_SZ) {
        MPIDI_CH4U_send_long_req_msg_t lreq_hdr;

        MPIR_Memcpy(&lreq_hdr.hdr, am_hdr, am_hdr_sz);
        lreq_hdr.data_sz = data_sz;
        lreq_hdr.sreq_ptr = (uint64_t)sreq;
        MPIDI_CH4U_REQUEST(sreq, req->lreq).src_buf = data;
        MPIDI_CH4U_REQUEST(sreq, req->lreq).count = count;
        dtype_add_ref_if_not_builtin(datatype);
        MPIDI_CH4U_REQUEST(sreq, req->lreq).datatype = datatype;
        MPIDI_CH4U_REQUEST(sreq, req->lreq).msg_tag = lreq_hdr.hdr.msg_tag;
        MPIDI_CH4U_REQUEST(sreq, src_rank) = rank;
        mpi_errno = MPIDI_NM_inject_am_hdr(rank, comm, MPIDI_CH4U_SEND_LONG_REQ,
                                           &lreq_hdr, sizeof(lreq_hdr), NULL);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        goto fn_exit;
    }

    ep = MPIDI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.data_sz = data_sz;

    if (dt_contig) {
        /* just pack and send for now */
        send_buf = MPL_malloc(data_sz + am_hdr_sz + sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
        MPIR_Memcpy(send_buf + am_hdr_sz + sizeof(ucx_hdr), data + dt_true_lb, data_sz);
       }
    else {
        size_t segment_first;
        struct MPIDU_Segment *segment_ptr;
        segment_ptr = MPIDU_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send MPIDU_Segment_alloc");
        MPIDU_Segment_init(data, count, datatype, segment_ptr, 0);
        segment_first = 0;
        last = data_sz;
        send_buf = MPL_malloc(data_sz + am_hdr_sz + sizeof(ucx_hdr)); 

        MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
        MPIDU_Segment_pack(segment_ptr, segment_first, &last, send_buf+am_hdr_sz+sizeof(ucx_hdr));
        MPIDU_Segment_free(segment_ptr);
    }

        ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                         data_sz + am_hdr_sz + sizeof(ucx_hdr),
                                                                         ucp_dt_make_contig(1), ucx_tag,
                                                                         &MPIDI_UCX_send_am_callback);
        MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);
    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4u.netmod_am.ucx.pack_buffer = send_buf;
        sreq->dev.ch4.ch4u.netmod_am.ucx.handler_id = handler_id;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_amv_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_amv_hdr(int rank,
                                        MPIR_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        MPIR_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    size_t am_hdr_sz = 0, i;
    char *am_hdr_buf;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_UCX_SEND_AMV_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_UCX_SEND_AMV_HDR);

    for(i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    am_hdr_buf = (char *) MPL_malloc(am_hdr_sz);
    MPIR_Assert(am_hdr_buf);
    am_hdr_sz = 0;

    for(i = 0; i < iov_len; i++) {
        MPIR_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_NM_send_am_hdr(rank, comm, handler_id, am_hdr_buf, am_hdr_sz,
                                         sreq, netmod_context);
    MPL_free(am_hdr_buf);
     MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_UCX_SEND_AMV_HDR);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_amv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_amv(int rank,
                                        MPIR_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        const void *data,
                                        MPI_Count count,
                                        MPI_Datatype datatype,
                                        MPIR_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    size_t am_hdr_sz = 0, i;
    char *am_hdr_buf;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_UCX_SEND_AMV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_UCX_SEND_AMV);

    for(i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    am_hdr_buf = (char *) MPL_malloc(am_hdr_sz);
    MPIR_Assert(am_hdr_buf);
    am_hdr_sz = 0;

    for(i = 0; i < iov_len; i++) {
        MPIR_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_NM_send_am(rank, comm, handler_id, am_hdr_buf, am_hdr_sz,
                                     data, count, datatype, sreq, netmod_context);

    MPL_free(am_hdr_buf);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_UCX_SEND_AMV);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_am_hdr_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_am_hdr_reply(MPIR_Context_id_t context_id,
                                                 int src_rank,
                                                 int handler_id,
                                                 const void *am_hdr,
                                                 size_t am_hdr_sz, MPIR_Request * sreq)
{

   return    MPIDI_NM_send_am_hdr(src_rank, MPIDI_CH4U_context_id_to_comm(context_id), handler_id,
                         am_hdr, am_hdr_sz,sreq, NULL);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_send_am_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_send_am_reply(MPIR_Context_id_t context_id,
                                             int src_rank,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz,
                                             const void *data, MPI_Count count,
                                             MPI_Datatype datatype, MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPIR_Datatype  *dt_ptr;
    int             dt_contig;
    MPIDI_UCX_am_header_t ucx_hdr;
    MPIR_Comm *use_comm;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_SEND_AM);

    use_comm = MPIDI_CH4U_context_id_to_comm(context_id);
    ep = MPIDI_UCX_COMM_TO_EP(use_comm, src_rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.data_sz = data_sz;

    if (dt_contig) {
        /* just pack and send for now */
        send_buf = MPL_malloc(data_sz + am_hdr_sz + sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
        MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
        MPIR_Memcpy(send_buf + am_hdr_sz + sizeof(ucx_hdr), data + dt_true_lb, data_sz);

        ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                         data_sz + am_hdr_sz + sizeof(ucx_hdr),
                                                                         ucp_dt_make_contig(1), ucx_tag,
                                                                         &MPIDI_UCX_send_am_callback);
        MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);
    }

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_UCX_global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4u.netmod_am.ucx.pack_buffer = send_buf;
        sreq->dev.ch4.ch4u.netmod_am.ucx.handler_id = handler_id;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
        
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_NM_send_amv_reply(MPIR_Context_id_t context_id,
                                              int src_rank,
                                              int handler_id,
                                              struct iovec *am_hdr,
                                              size_t iov_len,
                                              const void *data, MPI_Count count,
                                              MPI_Datatype datatype, MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    size_t am_hdr_sz = 0, i;
    char *am_hdr_buf;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_UCX_SEND_AMV_REPLY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_UCX_SEND_AMV_REPLY);

    for(i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    am_hdr_buf = (char *) MPL_malloc(am_hdr_sz);

    MPIR_Assert(am_hdr_buf);
    am_hdr_sz = 0;

    for(i = 0; i < iov_len; i++) {
        MPIR_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_NM_send_am_reply(context_id, src_rank, handler_id, am_hdr_buf, am_hdr_sz,
                                           data, count, datatype, sreq);
    MPL_free(am_hdr_buf);
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_UCX_SEND_AMV_REPLY);
    return mpi_errno;
}

static inline size_t MPIDI_NM_am_hdr_max_sz(void)
{
    return (MPIDI_UCX_MAX_AM_EAGER_SZ - sizeof(MPIDI_UCX_am_header_t));
}

static inline int MPIDI_NM_inject_am_hdr(int rank,
                                             MPIR_Comm * comm,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_UCX_am_header_t ucx_hdr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR);

    ep = MPIDI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.data_sz = 0;

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);

    ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     &MPIDI_UCX_inject_am_callback);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    if (ucp_request == NULL) {
        /* inject is done */
        MPL_free(send_buf);
    } else if (ucp_request->req) {
        MPL_free(send_buf);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        ucp_request->req = send_buf;
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_NM_inject_am_hdr_reply(MPIR_Context_id_t context_id,
                                                   int src_rank,
                                                   int handler_id,
                                                   const void *am_hdr, size_t am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_UCX_ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_UCX_am_header_t ucx_hdr;
    MPIR_Comm *use_comm;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_INJECT_AM_HDR_REPLY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_INJECT_AM_HDR_REPLY);

    use_comm = MPIDI_CH4U_context_id_to_comm(context_id);
    ep = MPIDI_UCX_COMM_TO_EP(use_comm, src_rank);
    ucx_tag = MPIDI_UCX_init_tag(0, 0, MPIDI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIR_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
    ucp_request = (MPIDI_UCX_ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     &MPIDI_UCX_inject_am_callback);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    if (ucp_request == NULL) {
        /* inject is done */
        MPL_free(send_buf);
    } else if (ucp_request->req) {
        MPL_free(send_buf);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        ucp_request->req = send_buf;
    }

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline size_t MPIDI_NM_am_inject_max_sz(void)
{
    return MPIDI_NM_am_hdr_max_sz();
}

static inline int MPIDI_NM_long_am_matched(MPIR_Request * req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_send_long_ack_msg_t msg;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_UCX_AM_MATCHED);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_UCX_AM_MATCHED);

    msg.sreq_ptr = (MPIDI_CH4U_REQUEST(req, req->rreq.peer_req_ptr));
    msg.rreq_ptr = (uint64_t)req;
    MPIR_Assert((void *)msg.sreq_ptr != NULL);
    mpi_errno = MPIDI_NM_inject_am_hdr_reply(MPIDI_CH4U_get_context(MPIDI_CH4U_REQUEST(req, tag)),
                                             MPIDI_CH4U_REQUEST(req, src_rank),
                                             MPIDI_CH4U_SEND_LONG_ACK,
                                             &msg, sizeof(msg));
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

 fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_UCX_AM_MATCHED);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}


#endif /* NETMOD_UCX_AM_H_INCLUDED */
