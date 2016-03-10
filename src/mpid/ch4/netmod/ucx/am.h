/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2015 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */
#ifndef NETMOD_UCX_AM_H_INCLUDED
#define NETMOD_UCX_AM_H_INCLUDED

#include "impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_reg_hdr_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_reg_hdr_handler(int handler_id,
                                               MPIDI_CH4_NM_am_origin_handler_fn origin_handler_fn,
                                               MPIDI_CH4_NM_am_target_handler_fn target_handler_fn)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REG_HDR_HANDLER);

    MPIDI_CH4_NMI_UCX_Global.am_handlers[handler_id] = target_handler_fn;
    MPIDI_CH4_NMI_UCX_Global.send_cmpl_handlers[handler_id] = origin_handler_fn;
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


static inline void MPIDI_CH4_NMI_UCX_Am_request_complete(MPID_Request *req)
{
    int count;
    MPIR_cc_decr(req->cc_ptr, &count);
    MPIU_Assert(count >= 0);

    if(count == 0) {
        MPIDI_CH4R_Request_release(req);
    }
}

static inline void MPIDI_CH4_NMI_UCX_Send_am_callback(void *request, ucs_status_t status)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_CH4_NMI_UCX_Ucp_request_t* ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) request;
    if(ucp_request->req){
        MPID_Request *req = ucp_request->req;
        if (req->dev.ch4.ch4r.netmod_am.ucx.pack_buffer) {
            MPL_free(req->dev.ch4.ch4r.netmod_am.ucx.pack_buffer);
        }
        MPIDI_CH4_NMI_UCX_Am_request_complete(req);
        MPIDI_CH4R_send_origin_cmpl_handler(req);
        ucp_request->req = NULL;
    } else {
        ucp_request->req = (void *)TRUE;
    }

fn_exit:
    return;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_am_hdr(int           rank,
                                           MPID_Comm    *comm,
                                           int           handler_id,
                                           const void   *am_hdr,
                                           size_t        am_hdr_sz,
                                           MPID_Request *sreq,
                                           void         *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_CH4_NMI_UCX_Am_header_t ucx_hdr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR);

    ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_CH4_NMI_UCX_init_tag(0, 0, MPIDI_CH4_NMI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.context_id = comm->context_id;
    ucx_hdr.src_rank = comm->rank;
    ucx_hdr.data_sz = 0;

    MPIR_cc_incr(sreq->cc_ptr, &c);

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIU_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIU_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);

    ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     &MPIDI_CH4_NMI_UCX_Send_am_callback);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_CH4_NMI_UCX_Am_request_complete(sreq);
        MPIDI_CH4_NMI_UCX_Global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_CH4_NMI_UCX_Am_request_complete(sreq);
        MPIDI_CH4_NMI_UCX_Global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4r.netmod_am.ucx.pack_buffer = send_buf;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_am(int rank,
                                       MPID_Comm * comm,
                                       int handler_id,
                                       const void *am_hdr,
                                       size_t am_hdr_sz,
                                       const void *data,
                                       MPI_Count count,
                                       MPI_Datatype datatype,
                                       MPID_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_msg_sz_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPID_Datatype  *dt_ptr;
    int             dt_contig;
    MPIDI_CH4_NMI_UCX_Am_header_t ucx_hdr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_CH4_NMI_UCX_init_tag(0, 0, MPIDI_CH4_NMI_UCX_AM_TAG);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.context_id = comm->context_id;
    ucx_hdr.src_rank = comm->rank;
    ucx_hdr.data_sz = data_sz;

    MPIR_cc_incr(sreq->cc_ptr, &c);

    if (dt_contig) {
        /* just pack and send for now */
        send_buf = MPL_malloc(data_sz + am_hdr_sz + sizeof(ucx_hdr));
        MPIU_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
        MPIU_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
        MPIU_Memcpy(send_buf + am_hdr_sz + sizeof(ucx_hdr), data + dt_true_lb, data_sz);

        ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                         data_sz + am_hdr_sz + sizeof(ucx_hdr),
                                                                         ucp_dt_make_contig(1), ucx_tag,
                                                                         &MPIDI_CH4_NMI_UCX_Send_am_callback);
        MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);
    }

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_CH4_NMI_UCX_Am_request_complete(sreq);
        MPIDI_CH4_NMI_UCX_Global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_CH4_NMI_UCX_Am_request_complete(sreq);
        MPIDI_CH4_NMI_UCX_Global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4r.netmod_am.ucx.pack_buffer = send_buf;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_amv_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_amv_hdr(int rank,
                                        MPID_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        MPID_Request * sreq, void *netmod_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_amv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_amv(int rank,
                                        MPID_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        const void *data,
                                        MPI_Count count,
                                        MPI_Datatype datatype,
                                        MPID_Request * sreq, void *netmod_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_am_hdr_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_am_hdr_reply(uint64_t reply_token,
                                                 int handler_id,
                                                 const void *am_hdr,
                                                 size_t am_hdr_sz, MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_am_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_am_reply(uint64_t reply_token,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz,
                                             const void *data, MPI_Count count,
                                             MPI_Datatype datatype, MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_msg_sz_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPID_Datatype  *dt_ptr;
    int             dt_contig;
    MPIDI_CH4_NMI_UCX_Am_header_t ucx_hdr;
    MPIDI_CH4_NMI_UCX_Am_reply_token_t use_token;
    MPID_Comm *use_comm;
    int use_rank;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    use_token.val = reply_token;
    use_comm = MPIDI_CH4R_context_id_to_comm(use_token.data.context_id);
    use_rank = use_token.data.src_rank;
    ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(use_comm, use_rank);
    ucx_tag = MPIDI_CH4_NMI_UCX_init_tag(0, 0, MPIDI_CH4_NMI_UCX_AM_TAG);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.context_id = use_comm->context_id;
    ucx_hdr.src_rank = use_comm->rank;
    ucx_hdr.data_sz = data_sz;

    MPIR_cc_incr(sreq->cc_ptr, &c);

    if (dt_contig) {
        /* just pack and send for now */
        send_buf = MPL_malloc(data_sz + am_hdr_sz + sizeof(ucx_hdr));
        MPIU_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
        MPIU_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
        MPIU_Memcpy(send_buf + am_hdr_sz + sizeof(ucx_hdr), data + dt_true_lb, data_sz);

        ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                         data_sz + am_hdr_sz + sizeof(ucx_hdr),
                                                                         ucp_dt_make_contig(1), ucx_tag,
                                                                         &MPIDI_CH4_NMI_UCX_Send_am_callback);
        MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);
    }

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
        MPIDI_CH4_NMI_UCX_Am_request_complete(sreq);
        MPIDI_CH4_NMI_UCX_Global.send_cmpl_handlers[handler_id](sreq);
        goto fn_exit;
    }

    /* request completed between the UCP call and now. free resources
       and complete the send request */
    if(ucp_request->req){
        MPL_free(send_buf);
        MPIDI_CH4_NMI_UCX_Am_request_complete(sreq);
        MPIDI_CH4_NMI_UCX_Global.send_cmpl_handlers[handler_id](sreq);
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    } else {
        /* set the ch4r request inside the UCP request */
        sreq->dev.ch4.ch4r.netmod_am.ucx.pack_buffer = send_buf;
        ucp_request->req = sreq;
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_send_amv_reply(uint64_t reply_token,
                                              int handler_id,
                                              struct iovec *am_hdr,
                                              size_t iov_len,
                                              const void *data, MPI_Count count,
                                              MPI_Datatype datatype, MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_CH4_NM_am_hdr_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}

static inline int MPIDI_CH4_NM_inject_am_hdr(int rank,
                                             MPID_Comm * comm,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_CH4_NMI_UCX_Am_header_t ucx_hdr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR);

    ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_CH4_NMI_UCX_init_tag(0, 0, MPIDI_CH4_NMI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.context_id = comm->context_id;
    ucx_hdr.src_rank = comm->rank;
    ucx_hdr.data_sz = 0;

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIU_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIU_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);

    ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     NULL);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
    } else {
        while (!ucp_request_is_completed(ucp_request)) {
            ucp_worker_progress(MPIDI_CH4_NMI_UCX_Global.worker);
        }
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_inject_am_hdr_reply(uint64_t reply_token,
                                                   int handler_id,
                                                   const void *am_hdr, size_t am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
    ucp_ep_h ep;
    uint64_t ucx_tag;
    char *send_buf;
    MPIDI_CH4_NMI_UCX_Am_header_t ucx_hdr;
    MPIDI_CH4_NMI_UCX_Am_reply_token_t use_token;
    MPID_Comm *use_comm;
    int use_rank;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_INJECT_AM_HDR_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_INJECT_AM_HDR_REPLY);

    use_token.val = reply_token;
    use_comm = MPIDI_CH4R_context_id_to_comm(use_token.data.context_id);
    use_rank = use_token.data.src_rank;
    ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(use_comm, use_rank);
    ucx_tag = MPIDI_CH4_NMI_UCX_init_tag(0, 0, MPIDI_CH4_NMI_UCX_AM_TAG);

    /* initialize our portion of the hdr */
    ucx_hdr.handler_id = handler_id;
    ucx_hdr.context_id = use_comm->context_id;
    ucx_hdr.src_rank = use_comm->rank;

    /* just pack and send for now */
    send_buf = MPL_malloc(am_hdr_sz + sizeof(ucx_hdr));
    MPIU_Memcpy(send_buf, &ucx_hdr, sizeof(ucx_hdr));
    MPIU_Memcpy(send_buf + sizeof(ucx_hdr), am_hdr, am_hdr_sz);
    ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_send_nb(ep, send_buf,
                                                                     am_hdr_sz + sizeof(ucx_hdr),
                                                                     ucp_dt_make_contig(1), ucx_tag,
                                                                     NULL);
    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    /* send is done. free all resources and complete the request */
    if (ucp_request == NULL) {
        MPL_free(send_buf);
    } else {
        while (!ucp_request_is_completed(ucp_request)) {
            ucp_worker_progress(MPIDI_CH4_NMI_UCX_Global.worker);
        }
        ucp_request_release(ucp_request);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline size_t MPIDI_CH4_NM_am_inject_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}


#endif /* NETMOD_UCX_AM_H_INCLUDED */
