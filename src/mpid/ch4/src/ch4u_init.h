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
#ifndef MPIDCH4U_INIT_H_INCLUDED
#define MPIDCH4U_INIT_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_send_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_send_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_SEND_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_SEND_TX_HANDLER);

    MPID_cc_decr(sreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);
    if (c == 0) {
        MPIDI_Request_release(sreq);
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_SEND_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_ssend_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_ssend_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_SSEND_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_SSEND_ACK_TX_HANDLER);
    MPID_cc_decr(req->cc_ptr, &c);
    MPIU_Assert(c >= 0);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_SSEND_ACK_TX_HANDLER);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_reply_ssend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_reply_ssend(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_Ssend_ack_msg_t ack_msg;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_REPLY_SSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_REPLY_SSEND);
    MPID_cc_incr(rreq->cc_ptr, &c);
    ack_msg.sreq_ptr = MPIU_CH4U_REQUEST(rreq, peer_req_ptr);
    MPIDU_RC_POP(MPIDI_netmod_send_am_hdr_reply(MPIU_CH4U_REQUEST(rreq, reply_token),
                                               MPIDI_CH4U_AM_SSEND_ACK, &ack_msg, sizeof(ack_msg),
                                               rreq));
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_REPLY_SSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_unexp_mrecv_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_unexp_mrecv_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    uint64_t msg_tag;
    size_t message_sz;
    MPI_Aint last;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPIDI_msg_sz_t data_sz, dt_sz;
    MPID_Segment *segment_ptr;
    void *buf;
    int count;
    MPI_Datatype datatype;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_UNEXP_MRECV_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_UNEXP_MRECV_CMPL_HANDLER);

    msg_tag = MPIU_CH4U_REQUEST(rreq, tag);
    rreq->status.MPI_SOURCE = MPIDI_CH4I_get_source(msg_tag);
    rreq->status.MPI_TAG = MPIDI_CH4I_get_tag(msg_tag);

    buf = MPIU_CH4U_REQUEST(rreq, mrcv_buffer);
    count = MPIU_CH4U_REQUEST(rreq, mrcv_count);
    datatype = MPIU_CH4U_REQUEST(rreq, mrcv_datatype);

    message_sz = MPIU_CH4U_REQUEST(rreq, count);
    MPID_Datatype_get_size_macro(datatype, dt_sz);

    if (message_sz > count * dt_sz) {
        rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
    }
    else {
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        count = message_sz / dt_sz;
    }

    MPIR_STATUS_SET_COUNT(rreq->status, count * dt_sz);
    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    if (!dt_contig) {
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, segment_ptr, 0);

        last = count * dt_sz;
        MPID_Segment_unpack(segment_ptr, 0, &last, MPIU_CH4U_REQUEST(rreq, buffer));
        MPID_Segment_free(segment_ptr);
        if (last != (MPI_Aint)(count * dt_sz)) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __FUNCTION__, __LINE__,
                                             MPI_ERR_TYPE, "**dtypemismatch", 0);
            rreq->status.MPI_ERROR = mpi_errno;
        }
    }
    else {
        MPIU_Memcpy((char *) buf + dt_true_lb, MPIU_CH4U_REQUEST(rreq, buffer), data_sz);
    }

    MPIU_Free(MPIU_CH4U_REQUEST(rreq, buffer));
    rreq->kind = MPID_REQUEST_RECV;

    MPIDI_Request_complete(rreq);
    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_PEER_SSEND) {
        MPIDU_RC_POP(MPIDI_CH4I_reply_ssend(rreq));
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_UNEXP_MRECV_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_unexp_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_unexp_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, comm_idx, c;
    MPID_Comm *root_comm;
    MPID_Request *match_req = NULL;
    uint64_t msg_tag;
    size_t count;
    MPI_Aint last;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPIDI_msg_sz_t dt_sz;
    MPID_Segment *segment_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_UNEXP_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_UNEXP_CMPL_HANDLER);

    /* MPIDI_CS_ENTER(); */
    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_UNEXP_DQUED) {
        if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_UNEXP_CLAIMED) {
            MPIDI_CH4I_unexp_mrecv_cmpl_handler(rreq);
        }
        /* MPIDI_CS_EXIT(); */
        goto fn_exit;
    }
    /* MPIDI_CS_EXIT(); */

    msg_tag = MPIU_CH4U_REQUEST(rreq, tag);
    comm_idx = MPIDI_CH4I_get_context_index(MPIDI_CH4I_get_context(msg_tag));
    root_comm = MPIDI_CH4_Global.comm_req_lists[comm_idx].comm;

    /* MPIDI_CS_ENTER(); */
    if (root_comm)
        match_req = MPIDI_CH4I_dequeue_posted(msg_tag, &MPIU_CH4U_COMM(root_comm, posted_list));

    if (!match_req) {
        MPIU_CH4U_REQUEST(rreq, status) &= ~MPIDI_CH4U_REQ_BUSY;
        /* MPIDI_CS_EXIT(); */
        goto fn_exit;
    }

    MPIDI_CH4I_delete_unexp(rreq, &MPIU_CH4U_COMM(root_comm, unexp_list));
    /* MPIDI_CS_EXIT(); */

    match_req->status.MPI_SOURCE = MPIDI_CH4I_get_source(msg_tag);
    match_req->status.MPI_TAG = MPIDI_CH4I_get_tag(msg_tag);

    MPIDI_Datatype_get_info(MPIU_CH4U_REQUEST(match_req, count),
                            MPIU_CH4U_REQUEST(match_req, datatype),
                            dt_contig, dt_sz, dt_ptr, dt_true_lb);
    MPID_Datatype_get_size_macro(MPIU_CH4U_REQUEST(match_req, datatype), dt_sz);

    if (MPIU_CH4U_REQUEST(rreq, count) > dt_sz * MPIU_CH4U_REQUEST(match_req, count)) {
        rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        count = MPIU_CH4U_REQUEST(match_req, count);
    }
    else {
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        count = MPIU_CH4U_REQUEST(rreq, count) / dt_sz;
    }

    MPIR_STATUS_SET_COUNT(match_req->status, count * dt_sz);
    MPIU_CH4U_REQUEST(rreq, count) = count;

    if (!dt_contig) {
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv MPID_Segment_alloc");
        MPID_Segment_init(MPIU_CH4U_REQUEST(match_req, buffer), count,
                          MPIU_CH4U_REQUEST(match_req, datatype), segment_ptr, 0);

        last = count * dt_sz;
        MPID_Segment_unpack(segment_ptr, 0, &last, MPIU_CH4U_REQUEST(match_req, buffer));
        MPID_Segment_free(segment_ptr);
        if (last != (MPI_Aint)(count * dt_sz)) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __FUNCTION__, __LINE__,
                                             MPI_ERR_TYPE, "**dtypemismatch", 0);
            match_req->status.MPI_ERROR = mpi_errno;
        }
    }
    else {
        MPIU_Memcpy((char *) MPIU_CH4U_REQUEST(match_req, buffer) + dt_true_lb,
                    MPIU_CH4U_REQUEST(rreq, buffer), count * dt_sz);
    }

    MPIU_CH4U_REQUEST(rreq, status) &= ~MPIDI_CH4U_REQ_UNEXPECTED;
    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_PEER_SSEND) {
        MPIDU_RC_POP(MPIDI_CH4I_reply_ssend(rreq));
    }

    MPIU_Free(MPIU_CH4U_REQUEST(rreq, buffer));
    MPIU_Object_release_ref(rreq, &c);
    MPIDI_Request_complete(rreq);
    MPIDI_Request_complete(match_req);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_UNEXP_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_recv_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_recv_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_RECV_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_RECV_CMPL_HANDLER);

    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_RCV_NON_CONTIG) {
        MPIU_Free(MPIU_CH4U_REQUEST(rreq, iov));
    }

    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_UNEXPECTED) {
        MPIDU_RC_POP(MPIDI_CH4I_unexp_cmpl_handler(rreq));
        goto fn_exit;
    }

    rreq->status.MPI_SOURCE = MPIDI_CH4I_get_source(MPIU_CH4U_REQUEST(rreq, tag));
    rreq->status.MPI_TAG = MPIDI_CH4I_get_tag(MPIU_CH4U_REQUEST(rreq, tag));

    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_PEER_SSEND) {
        MPIDU_RC_POP(MPIDI_CH4I_reply_ssend(rreq));
    }
    MPIDI_Request_complete(rreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_RECV_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_send_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_send_target_handler(void *am_hdr,
                                                    size_t am_hdr_sz,
                                                    void *reply_token,
                                                    void **data,
                                                    size_t * p_data_sz,
                                                    int *is_contig,
                                                    MPIDI_netmod_am_completion_handler_fn *
                                                    cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, comm_idx;
    MPID_Request *rreq = NULL;
    MPID_Comm *root_comm;
    size_t data_sz;

    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;

    MPIDI_CH4U_AM_Hdr_t *hdr = (MPIDI_CH4U_AM_Hdr_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_SEND_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_SEND_HANDLER);

    comm_idx = MPIDI_CH4I_get_context_index(MPIDI_CH4I_get_context(hdr->msg_tag));
    root_comm = MPIDI_CH4_Global.comm_req_lists[comm_idx].comm;

    if (root_comm) {
        /* MPIDI_CS_ENTER(); */
        rreq = MPIDI_CH4I_dequeue_posted(hdr->msg_tag, &MPIU_CH4U_COMM(root_comm, posted_list));
        /* MPIDI_CS_EXIT(); */
    }

    if (rreq == NULL) {
        rreq = MPIDI_CH4I_create_req();
        rreq->kind = MPID_REQUEST_RECV;
        MPIU_CH4U_REQUEST(rreq, buffer) = (char *) MPIU_Malloc(*p_data_sz);
        MPIU_CH4U_REQUEST(rreq, datatype) = MPI_BYTE;
        MPIU_CH4U_REQUEST(rreq, count) = *p_data_sz;
        MPIU_CH4U_REQUEST(rreq, tag) = hdr->msg_tag;
        MPIU_CH4U_REQUEST(rreq, status) |= MPIDI_CH4U_REQ_BUSY;
        MPIU_CH4U_REQUEST(rreq, status) |= MPIDI_CH4U_REQ_UNEXPECTED;
        /* MPIDI_CS_ENTER(); */
        if (root_comm)
            MPIDI_CH4I_enqueue_unexp(rreq, &MPIU_CH4U_COMM(root_comm, unexp_list));
        else
            MPIDI_CH4I_enqueue_unexp(rreq, &MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list);
        /* MPIDI_CS_EXIT(); */
    }

    MPIU_CH4U_REQUEST(rreq, tag) = hdr->msg_tag;
    *req = rreq;
    *cmpl_handler_fn = MPIDI_CH4I_am_recv_cmpl_handler;

    MPIDI_Datatype_get_info(MPIU_CH4U_REQUEST(rreq, count),
                            MPIU_CH4U_REQUEST(rreq, datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);
    *is_contig = dt_contig;

    if (dt_contig) {
        *p_data_sz = data_sz;
        *data = (char *) MPIU_CH4U_REQUEST(rreq, buffer) + dt_true_lb;
    }
    else {
        segment_ptr = MPID_Segment_alloc();
        MPIU_Assert(segment_ptr);

        MPID_Segment_init(MPIU_CH4U_REQUEST(rreq, buffer),
                          MPIU_CH4U_REQUEST(rreq, count),
                          MPIU_CH4U_REQUEST(rreq, datatype), segment_ptr, 0);

        last = data_sz;
        MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, (MPI_Aint *) & n_iov);
        MPIU_Assert(n_iov > 0);
        MPIU_CH4U_REQUEST(rreq, iov) = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
        MPIU_Assert(MPIU_CH4U_REQUEST(rreq, iov));

        last = data_sz;
        MPID_Segment_pack_vector(segment_ptr, 0, &last, MPIU_CH4U_REQUEST(rreq, iov), &n_iov);
        MPIU_Assert(last == (MPI_Aint)data_sz);
        *data = MPIU_CH4U_REQUEST(rreq, iov);
        *p_data_sz = n_iov;
        MPIU_CH4U_REQUEST(rreq, status) |= MPIDI_CH4U_REQ_RCV_NON_CONTIG;
        MPIU_Free(segment_ptr);
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_SEND_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_ssend_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_ssend_target_handler(void *am_hdr,
                                                     size_t am_hdr_sz,
                                                     void *reply_token,
                                                     void **data,
                                                     size_t * p_data_sz,
                                                     int *is_contig,
                                                     MPIDI_netmod_am_completion_handler_fn *
                                                     cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_CH4U_Ssend_req_msg_t *msg_hdr = (MPIDI_CH4U_Ssend_req_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_SSEND_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_SSEND_HANDLER);

    MPIDU_RC_POP(MPIDI_CH4I_am_send_target_handler(am_hdr, sizeof(MPIDI_CH4U_AM_Hdr_t),
                                                  reply_token, data, p_data_sz,
                                                  is_contig, cmpl_handler_fn, req));
    MPIU_Assert(req);
    MPIU_CH4U_REQUEST(*req, peer_req_ptr) = msg_hdr->sreq_ptr;
    MPIU_CH4U_REQUEST(*req, reply_token) = reply_token;
    MPIU_CH4U_REQUEST(*req, status) |= MPIDI_CH4U_REQ_PEER_SSEND;
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_SSEND_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_am_ssend_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_am_ssend_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                         void *reply_token, void **data,
                                                         size_t * p_data_sz, int *is_contig,
                                                         MPIDI_netmod_am_completion_handler_fn *
                                                         cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPID_Request *sreq;
    MPIDI_CH4U_Ssend_ack_msg_t *msg_hdr = (MPIDI_CH4U_Ssend_ack_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_SSEND_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_SSEND_ACK_HANDLER);


    sreq = (MPID_Request *)msg_hdr->sreq_ptr;
    MPID_cc_decr(sreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);

    if (c == 0) {
        MPIDI_Request_release(sreq);
    }

    if (req)
        *req = NULL;
    if (cmpl_handler_fn)
        *cmpl_handler_fn = NULL;
    
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_SSEND_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_init_comm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_init_comm(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS, comm_idx;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_INIT_COMM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_INIT_COMM);

    comm_idx = MPIDI_CH4I_get_context_index(comm->recvcontext_id);
    if (!MPIDI_CH4_Global.comm_req_lists[comm_idx].comm) {
        MPIU_CH4U_COMM(comm, posted_list) = NULL;
        MPIU_CH4U_COMM(comm, unexp_list) = NULL;
        MPIDI_CH4_Global.comm_req_lists[comm_idx].comm = comm;
        if (MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list) {
            MPIDI_CH4U_Devreq_t *curr, *tmp;
            MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list, 
                                curr, tmp) {
                MPL_DL_DELETE(MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list, curr);
                MPL_DL_APPEND(MPIU_CH4U_COMM(comm, unexp_list), curr);
            }
            MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list = NULL;
        }
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT_COMM);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_destroy_comm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_destroy_comm(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS, comm_idx;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_INIT_COMM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_INIT_COMM);

    comm_idx = MPIDI_CH4I_get_context_index(comm->recvcontext_id);
    if (MPIDI_CH4_Global.comm_req_lists[comm_idx].comm) {
        MPIU_Assert(MPIDI_CH4_Global.comm_req_lists[comm_idx].comm->dev.ch4.ch4u.posted_list == NULL);
        MPIU_Assert(MPIDI_CH4_Global.comm_req_lists[comm_idx].comm->dev.ch4.ch4u.unexp_list == NULL);
        MPIDI_CH4_Global.comm_req_lists[comm_idx].comm = NULL;
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT_COMM);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_init(MPID_Comm * comm_world, MPID_Comm * comm_self,
                                   int num_contexts, void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_INIT);

#ifndef MPIDI_CH4U_USE_PER_COMM_QUEUE
    MPIDI_CH4_Global.posted_list = NULL;
    MPIDI_CH4_Global.unexp_list = NULL;
#endif

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_SEND,
                                             &MPIDI_CH4I_am_send_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_send_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_SSEND_REQ,
                                             &MPIDI_CH4I_am_send_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_ssend_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_SSEND_ACK,
                                             &MPIDI_CH4I_am_ssend_ack_origin_cmpl_handler,
                                             &MPIDI_CH4U_am_ssend_ack_target_handler));

    MPIDU_RC_POP(MPIDI_CH4U_init_comm(comm_world));
    MPIDU_RC_POP(MPIDI_CH4U_init_comm(comm_self));
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT);
  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_alloc_mem
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void *MPIDI_CH4U_alloc_mem(size_t size, MPID_Info * info_ptr)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ALLOC_MEM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ALLOC_MEM);
    void *p;
    p = MPIU_Malloc(size);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ALLOC_MEM);
    return p;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_free_mem
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_free_mem(void *ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FREE_MEM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FREE_MEM);
    MPIU_Free(ptr);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FREE_MEM);
    return mpi_errno;
}


#endif /* MPIDCH4_INIT_H_INCLUDED */
