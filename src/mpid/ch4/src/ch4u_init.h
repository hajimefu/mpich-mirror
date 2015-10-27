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
#include "ch4u_util.h"

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
#define FUNCNAME MPIDI_CH4I_am_put_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_PUT_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_PUT_ACK_TX_HANDLER);
    MPIDI_Request_complete(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_PUT_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_ACC_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_ACC_ACK_TX_HANDLER);
    MPIDI_Request_complete(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_ACC_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_acc_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_GET_ACC_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_GET_ACC_ACK_TX_HANDLER);
    MPIU_Free(MPIU_CH4U_REQUEST(req, areq.data));
    MPIDI_Request_complete(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_GET_ACC_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_CSWAP_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_CSWAP_ACK_TX_HANDLER);

    MPIU_Free(MPIU_CH4U_REQUEST(req, creq.data));
    MPIDI_Request_complete(req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_CSWAP_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_GET_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_GET_ACK_TX_HANDLER);

    if (MPIU_CH4U_REQUEST(req, greq.dt_iov)) {
        MPIU_Free(MPIU_CH4U_REQUEST(req, greq.dt_iov));
    }

    MPIDI_Request_complete(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_GET_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_PUT_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_PUT_TX_HANDLER);

    MPID_cc_decr(sreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);
    if (c == 0) {
        MPIDI_Request_release(sreq);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_PUT_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_CSWAP_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_CSWAP_TX_HANDLER);

    MPID_cc_decr(sreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);
    if (c == 0) {
        MPIDI_Request_release(sreq);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_CSWAP_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_ACC_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_ACC_TX_HANDLER);

    if (MPIU_CH4U_REQUEST(sreq, areq.dt_iov)) {
        MPIU_Free(MPIU_CH4U_REQUEST(sreq, areq.dt_iov));
    }

    MPID_cc_decr(sreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);
    if (c == 0) {
        MPIDI_Request_release(sreq);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_ACC_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_GET_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_GET_TX_HANDLER);

    MPID_cc_decr(sreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);
    if (c == 0) {
        MPIDI_Request_release(sreq);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_GET_TX_HANDLER);
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
    ack_msg.sreq_ptr = MPIU_CH4U_REQUEST(rreq, rreq.peer_req_ptr);
    MPIDU_RC_POP(MPIDI_netmod_send_am_hdr_reply(MPIU_CH4U_REQUEST(rreq, rreq.reply_token),
                                               MPIDI_CH4U_AM_SSEND_ACK, &ack_msg, sizeof(ack_msg),
                                               rreq));
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_REPLY_SSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_ack_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_ack_put(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_put_ack_msg_t ack_msg;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACK_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACK_PUT);
    MPID_cc_incr(rreq->cc_ptr, &c);
    ack_msg.preq_ptr = MPIU_CH4U_REQUEST(rreq, preq.preq_ptr);
    MPIDU_RC_POP(MPIDI_netmod_send_am_hdr_reply(MPIU_CH4U_REQUEST(rreq, preq.reply_token),
                                                MPIDI_CH4U_AM_PUT_ACK, &ack_msg, sizeof(ack_msg),
                                                rreq));
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACK_PUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_ack_cswap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_ack_cswap(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_cswap_ack_msg_t ack_msg;
    void *result_addr;
    uint64_t dt_contig, data_sz, dt_true_lb;
    MPID_Datatype *dt_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACK_CSWAP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACK_CSWAP);

    MPIDI_Datatype_get_info(1, MPIU_CH4U_REQUEST(rreq, creq.datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);
    result_addr = ((char *)MPIU_CH4U_REQUEST(rreq, creq.data)) + data_sz;

    MPID_cc_incr(rreq->cc_ptr, &c);
    ack_msg.req_ptr = MPIU_CH4U_REQUEST(rreq, creq.creq_ptr);
    MPIDU_RC_POP(MPIDI_netmod_send_am_reply(MPIU_CH4U_REQUEST(rreq, creq.reply_token),
                                            MPIDI_CH4U_AM_CSWAP_ACK, 
                                            &ack_msg, sizeof(ack_msg),
                                            result_addr, 1, 
                                            MPIU_CH4U_REQUEST(rreq, creq.datatype),
                                            rreq));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACK_CSWAP);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_ack_acc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_ack_acc(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_acc_ack_msg_t ack_msg;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACK_ACC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACK_ACC);

    MPID_cc_incr(rreq->cc_ptr, &c);
    ack_msg.req_ptr = MPIU_CH4U_REQUEST(rreq, areq.req_ptr);
    MPIDU_RC_POP(MPIDI_netmod_send_am_hdr_reply(MPIU_CH4U_REQUEST(rreq, areq.reply_token),
                                                MPIDI_CH4U_AM_ACC_ACK, &ack_msg, sizeof(ack_msg),
                                                rreq));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACK_ACC);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_ack_get_acc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_ack_get_acc(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_acc_ack_msg_t ack_msg;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACK_GET_ACC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACK_GET_ACC);

    MPID_cc_incr(rreq->cc_ptr, &c);
    ack_msg.req_ptr = MPIU_CH4U_REQUEST(rreq, areq.req_ptr);
    MPIDU_RC_POP(MPIDI_netmod_send_am_reply(MPIU_CH4U_REQUEST(rreq, areq.reply_token),
                                            MPIDI_CH4U_AM_GET_ACC_ACK,
                                            &ack_msg, sizeof(ack_msg),
                                            MPIU_CH4U_REQUEST(rreq, areq.data),
                                            MPIU_CH4U_REQUEST(rreq, areq.data_sz),
                                            MPI_BYTE, rreq));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACK_GET_ACC);
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

    buf = MPIU_CH4U_REQUEST(rreq, rreq.mrcv_buffer);
    count = MPIU_CH4U_REQUEST(rreq, rreq.mrcv_count);
    datatype = MPIU_CH4U_REQUEST(rreq, rreq.mrcv_datatype);

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
        MPID_Segment_unpack(segment_ptr, 0, &last, MPIU_CH4U_REQUEST(rreq, buffer));
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

    dtype_release_if_not_builtin(MPIU_CH4U_REQUEST(match_req, datatype));
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

    dtype_release_if_not_builtin(MPIU_CH4U_REQUEST(rreq, datatype));
    MPIDI_Request_complete(rreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_RECV_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_acc_ack_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_ack_cmpl_handler(MPID_Request * areq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_GET_ACC_ACK_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_GET_ACC_ACK_CMPL_HANDLER);

    if (MPIU_CH4U_REQUEST(areq, status) & MPIDI_CH4U_REQ_RCV_NON_CONTIG) {
        MPIU_Free(MPIU_CH4U_REQUEST(areq, iov));
    }

    win = (MPID_Win *) MPIU_CH4U_REQUEST(areq, areq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)--;
    /* MPIDI_CS_EXIT(); */

    dtype_release_if_not_builtin(MPIU_CH4U_REQUEST(areq, areq.result_datatype));
    MPID_cc_decr(areq->cc_ptr, &c);
    MPIU_Assert(c >= 0);
    if (c == 0) {
        MPIDI_Request_release(areq);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_GET_ACC_ACK_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_PUT_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_PUT_CMPL_HANDLER);

    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_RCV_NON_CONTIG) {
        MPIU_Free(MPIU_CH4U_REQUEST(rreq, iov));
    }

    if (MPIU_CH4U_REQUEST(rreq, preq.dt_iov)) {
        MPIU_Free(MPIU_CH4U_REQUEST(rreq, preq.dt_iov));
    }

    MPIDU_RC_POP(MPIDI_CH4I_ack_put(rreq));
    MPID_cc_decr(rreq->cc_ptr, &c);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_PUT_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    void *compare_addr;
    void *origin_addr;
    uint64_t dt_contig, data_sz, dt_true_lb;
    MPID_Datatype *dt_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_CSWAP_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_CSWAP_CMPL_HANDLER);

    MPIDI_Datatype_get_info(1, MPIU_CH4U_REQUEST(rreq, creq.datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);

    origin_addr = MPIU_CH4U_REQUEST(rreq, creq.data);
    compare_addr = ((char *)MPIU_CH4U_REQUEST(rreq, creq.data)) + data_sz;
    
    /* MPIDI_CS_ENTER(); */

    if (MPIR_Compare_equal((void *)MPIU_CH4U_REQUEST(rreq, creq.addr), compare_addr,
                           MPIU_CH4U_REQUEST(rreq, creq.datatype))) {
        MPIU_Memcpy(compare_addr, (void *)MPIU_CH4U_REQUEST(rreq, creq.addr), data_sz);
        MPIU_Memcpy((void *)MPIU_CH4U_REQUEST(rreq, creq.addr), origin_addr, data_sz);
    } else {
        MPIU_Memcpy(compare_addr, (void *)MPIU_CH4U_REQUEST(rreq, creq.addr), data_sz);
    }
    
    /* MPIDI_CS_EXIT(); */

    MPIDU_RC_POP(MPIDI_CH4I_ack_cswap(rreq));
    MPID_cc_decr(rreq->cc_ptr, &c);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_CSWAP_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_do_accumulate_op
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_do_accumulate_op(void *source_buf, int source_count, MPI_Datatype source_dtp,
                                   void *target_buf, int target_count, MPI_Datatype target_dtp,
                                   MPI_Aint stream_offset, MPI_Op acc_op)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_User_function *uop = NULL;
    MPI_Aint source_dtp_size = 0, source_dtp_extent = 0;
    int is_empty_source = FALSE;
    MPIDI_STATE_DECL(MPID_STATE_DO_ACCUMULATE_OP);

    MPIDI_FUNC_ENTER(MPID_STATE_DO_ACCUMULATE_OP);

    /* first Judge if source buffer is empty */
    if (acc_op == MPI_NO_OP)
        is_empty_source = TRUE;

    if (is_empty_source == FALSE) {
        MPIU_Assert(MPIR_DATATYPE_IS_PREDEFINED(source_dtp));
        MPID_Datatype_get_size_macro(source_dtp, source_dtp_size);
        MPID_Datatype_get_extent_macro(source_dtp, source_dtp_extent);
    }

    if (HANDLE_GET_KIND(acc_op) == HANDLE_KIND_BUILTIN) {
        /* get the function by indexing into the op table */
        uop = MPIR_OP_HDL_TO_FN(acc_op);
    }
    else {
        /* --BEGIN ERROR HANDLING-- */
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                         FCNAME, __LINE__, MPI_ERR_OP,
                                         "**opnotpredefined", "**opnotpredefined %d", acc_op);
        return mpi_errno;
        /* --END ERROR HANDLING-- */
    }


    if (is_empty_source == TRUE || MPIR_DATATYPE_IS_PREDEFINED(target_dtp)) {
        /* directly apply op if target dtp is predefined dtp OR source buffer is empty */
        MPI_Aint real_stream_offset;
        void *curr_target_buf;

        if (is_empty_source == FALSE) {
            MPIU_Assert(source_dtp == target_dtp);
            real_stream_offset = (stream_offset / source_dtp_size) * source_dtp_extent;
            curr_target_buf = (void *) ((char *) target_buf + real_stream_offset);
        }
        else {
            curr_target_buf = target_buf;
        }

        (*uop) (source_buf, curr_target_buf, &source_count, &source_dtp);
    }
    else {
        /* derived datatype */
        MPID_Segment *segp;
        DLOOP_VECTOR *dloop_vec;
        MPI_Aint first, last;
        int vec_len, i, count;
        MPI_Aint type_extent, type_size;
        MPI_Datatype type;
        MPID_Datatype *dtp;
        MPI_Aint curr_len;
        void *curr_loc;
        int accumulated_count;

        segp = MPID_Segment_alloc();
        /* --BEGIN ERROR HANDLING-- */
        if (!segp) {
            mpi_errno =
                MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, FCNAME, __LINE__,
                                     MPI_ERR_OTHER, "**nomem", 0);
            MPIDI_FUNC_EXIT(MPID_STATE_DO_ACCUMULATE_OP);
            return mpi_errno;
        }
        /* --END ERROR HANDLING-- */
        MPID_Segment_init(NULL, target_count, target_dtp, segp, 0);
        first = stream_offset;
        last = first + source_count * source_dtp_size;

        MPID_Datatype_get_ptr(target_dtp, dtp);
        vec_len = dtp->max_contig_blocks * target_count + 1;
        /* +1 needed because Rob says so */
        dloop_vec = (DLOOP_VECTOR *)
            MPIU_Malloc(vec_len * sizeof(DLOOP_VECTOR));
        /* --BEGIN ERROR HANDLING-- */
        if (!dloop_vec) {
            mpi_errno =
                MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, FCNAME, __LINE__,
                                     MPI_ERR_OTHER, "**nomem", 0);
            MPIDI_FUNC_EXIT(MPID_STATE_DO_ACCUMULATE_OP);
            return mpi_errno;
        }
        /* --END ERROR HANDLING-- */

        MPID_Segment_pack_vector(segp, first, &last, dloop_vec, &vec_len);

        type = dtp->basic_type;
        MPIU_Assert(type != MPI_DATATYPE_NULL);

        MPIU_Assert(type == source_dtp);
        type_size = source_dtp_size;
        type_extent = source_dtp_extent;

        i = 0;
        curr_loc = dloop_vec[0].DLOOP_VECTOR_BUF;
        curr_len = dloop_vec[0].DLOOP_VECTOR_LEN;
        accumulated_count = 0;
        while (i != vec_len) {
            if (curr_len < type_size) {
                MPIU_Assert(i != vec_len);
                i++;
                curr_len += dloop_vec[i].DLOOP_VECTOR_LEN;
                continue;
            }

            MPIU_Assign_trunc(count, curr_len / type_size, int);

            (*uop) ((char *) source_buf + type_extent * accumulated_count,
                    (char *) target_buf + MPIU_PtrToAint(curr_loc), &count, &type);

            if (curr_len % type_size == 0) {
                i++;
                if (i != vec_len) {
                    curr_loc = dloop_vec[i].DLOOP_VECTOR_BUF;
                    curr_len = dloop_vec[i].DLOOP_VECTOR_LEN;
                }
            }
            else {
                curr_loc = (void *) ((char *) curr_loc + type_extent * count);
                curr_len -= type_size * count;
            }

            accumulated_count += count;
        }

        MPID_Segment_free(segp);
        MPIU_Free(dloop_vec);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_DO_ACCUMULATE_OP);

    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_handle_acc_cmpl
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_handle_acc_cmpl(MPID_Request * rreq, int do_get)
{
    int mpi_errno = MPI_SUCCESS, c, i;
    uint64_t dt_contig, data_sz, dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPI_Aint basic_sz, count, offset = 0;
    struct iovec *iov;
    char *src_ptr, *original = NULL;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_HANDLE_ACC_CMPL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_HANDLE_ACC_CMPL);

    MPID_Datatype_get_size_macro(MPIU_CH4U_REQUEST(rreq, areq.target_datatype),
                                 basic_sz);
    
    /* MPIDI_CS_ENTER(); */

    if (do_get) {
        original = (char *)MPIU_Malloc(MPIU_CH4U_REQUEST(rreq, areq.data_sz));
        MPIU_Assert(original);
    }
    
    if (MPIU_CH4U_REQUEST(rreq, areq.dt_iov) == NULL) {

        if (original) {
            MPIU_Memcpy(original, MPIU_CH4U_REQUEST(rreq, areq.target_addr),
                        basic_sz * MPIU_CH4U_REQUEST(rreq, areq.target_count));
        }
        
        MPIDU_RC_POP(MPIDI_CH4I_do_accumulate_op(MPIU_CH4U_REQUEST(rreq, areq.data),
                                      MPIU_CH4U_REQUEST(rreq, areq.origin_count),
                                      MPIU_CH4U_REQUEST(rreq, areq.origin_datatype),
                                      MPIU_CH4U_REQUEST(rreq, areq.target_addr),
                                      MPIU_CH4U_REQUEST(rreq, areq.target_count),
                                      MPIU_CH4U_REQUEST(rreq, areq.target_datatype), 0,
                                      MPIU_CH4U_REQUEST(rreq, areq.op)));
    } else {
        iov = (struct iovec *)MPIU_CH4U_REQUEST(rreq, areq.dt_iov);
        src_ptr = (char *)MPIU_CH4U_REQUEST(rreq, areq.data);
        for (i = 0; i < MPIU_CH4U_REQUEST(rreq, areq.n_iov); i++) {
            count = iov[i].iov_len / basic_sz;
            MPIU_Assert(count > 0);

            if (original) {
                MPIU_Memcpy(original + offset, iov[i].iov_base, count * basic_sz);
                offset += count * basic_sz;
            }

            MPIDU_RC_POP(MPIDI_CH4I_do_accumulate_op(src_ptr, count,
                                          MPIU_CH4U_REQUEST(rreq, areq.origin_datatype),
                                          iov[i].iov_base, count,
                                          MPIU_CH4U_REQUEST(rreq, areq.target_datatype), 0,
                                          MPIU_CH4U_REQUEST(rreq, areq.op)));
            src_ptr += count * basic_sz;
        }
        MPIU_Free(iov);
    }
    
    /* MPIDI_CS_EXIT(); */
    MPIU_Free(MPIU_CH4U_REQUEST(rreq, areq.data));

    if (original) {
        MPIU_CH4U_REQUEST(rreq, areq.data) = original;
        MPIDU_RC_POP(MPIDI_CH4I_ack_get_acc(rreq));
    } else {
        MPIDU_RC_POP(MPIDI_CH4I_ack_acc(rreq));
    }

    MPID_cc_decr(rreq->cc_ptr, &c);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_HANDLE_ACC_CMPL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_ACC_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_ACC_HANDLER);
    MPIDU_RC_POP(MPIDI_CH4I_handle_acc_cmpl(rreq, 0));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_ACC_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_acc_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_GET_ACC_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_GET_ACC_HANDLER);
    MPIDU_RC_POP(MPIDI_CH4I_handle_acc_cmpl(rreq, 1));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_GET_ACC_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_ack_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_ack_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_GET_ACK_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_GET_ACK_CMPL_HANDLER);

    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_RCV_NON_CONTIG) {
        MPIU_Free(MPIU_CH4U_REQUEST(rreq, iov));
    }

    win = (MPID_Win *) MPIU_CH4U_REQUEST(rreq, greq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)--;
    /* MPIDI_CS_EXIT(); */

    MPID_cc_decr(rreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);

    if (c == 0) {
        MPIDI_Request_release(rreq);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_GET_ACK_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_ack_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_ack_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_CSWAP_ACK_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_CSWAP_ACK_CMPL_HANDLER);


    win = (MPID_Win *) MPIU_CH4U_REQUEST(rreq, creq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)--;
    /* MPIDI_CS_EXIT(); */

    MPIU_Free(MPIU_CH4U_REQUEST(rreq, creq.data));
    MPID_cc_decr(rreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);

    if (c == 0) {
        MPIDI_Request_release(rreq);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_CSWAP_ACK_CMPL_HANDLER);
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
    MPIU_CH4U_REQUEST(*req, rreq.peer_req_ptr) = msg_hdr->sreq_ptr;
    MPIU_CH4U_REQUEST(*req, rreq.reply_token) = reply_token;
    MPIU_CH4U_REQUEST(*req, status) |= MPIDI_CH4U_REQ_PEER_SSEND;
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_SSEND_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_ssend_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_ssend_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
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
#define FUNCNAME MPIDI_CH4I_am_put_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                       void *reply_token, void **data,
                                                       size_t * p_data_sz, int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_put_ack_msg_t *msg_hdr = (MPIDI_CH4U_put_ack_msg_t *) am_hdr;
    MPID_Win *win;
    MPID_Request *preq;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_PUT_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_PUT_ACK_HANDLER);

    preq = (MPID_Request *)msg_hdr->preq_ptr;
    win = (MPID_Win *) MPIU_CH4U_REQUEST(preq, preq.win_ptr);

    if (MPIU_CH4U_REQUEST(preq, preq.dt_iov)) {
        MPIU_Free(MPIU_CH4U_REQUEST(preq, preq.dt_iov));
    }

    MPID_cc_decr(preq->cc_ptr, &c);
    MPIU_Assert(c >= 0);

    if (c == 0) {
        MPIDI_Request_release(preq);
    }

    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)--;
    /* MPIDI_CS_EXIT(); */

    if (req)
        *req = NULL;
    if (cmpl_handler_fn)
        *cmpl_handler_fn = NULL;
    
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_PUT_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                       void *reply_token, void **data,
                                                       size_t * p_data_sz, int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_acc_ack_msg_t *msg_hdr = (MPIDI_CH4U_acc_ack_msg_t *) am_hdr;
    MPID_Win *win;
    MPID_Request *areq;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_ACC_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_ACC_ACK_HANDLER);

    areq = (MPID_Request *)msg_hdr->req_ptr;
    win = (MPID_Win *) MPIU_CH4U_REQUEST(areq, areq.win_ptr);

    if (MPIU_CH4U_REQUEST(areq, areq.dt_iov)) {
        MPIU_Free(MPIU_CH4U_REQUEST(areq, areq.dt_iov));
    }

    MPID_cc_decr(areq->cc_ptr, &c);
    MPIU_Assert(c >= 0);

    if (c == 0) {
        MPIDI_Request_release(areq);
    }

    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)--;
    /* MPIDI_CS_EXIT(); */

    if (req)
        *req = NULL;
    if (cmpl_handler_fn)
        *cmpl_handler_fn = NULL;
    
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_ACC_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_acc_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                       void *reply_token, void **data,
                                                       size_t * p_data_sz, int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_acc_ack_msg_t *msg_hdr = (MPIDI_CH4U_acc_ack_msg_t *) am_hdr;
    MPID_Request *areq;

    size_t data_sz;
    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_ACC_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_ACC_ACK_HANDLER);

    areq = (MPID_Request *)msg_hdr->req_ptr;

    if (MPIU_CH4U_REQUEST(areq, areq.dt_iov)) {
        MPIU_Free(MPIU_CH4U_REQUEST(areq, areq.dt_iov));
    }

    MPIDI_Datatype_get_info(MPIU_CH4U_REQUEST(areq, areq.result_count),
                            MPIU_CH4U_REQUEST(areq, areq.result_datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);
    *is_contig = dt_contig;

    if (dt_contig) {
        *p_data_sz = data_sz;
        *data = (char *) MPIU_CH4U_REQUEST(areq, areq.result_addr) + dt_true_lb;
    }
    else {
        segment_ptr = MPID_Segment_alloc();
        MPIU_Assert(segment_ptr);

        MPID_Segment_init(MPIU_CH4U_REQUEST(areq, areq.result_addr),
                          MPIU_CH4U_REQUEST(areq, areq.result_count),
                          MPIU_CH4U_REQUEST(areq, areq.result_datatype),
                          segment_ptr, 0);

        last = data_sz;
        MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, (MPI_Aint *) &n_iov);
        MPIU_Assert(n_iov > 0);
        MPIU_CH4U_REQUEST(areq, iov) = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
        MPIU_Assert(MPIU_CH4U_REQUEST(areq, iov));

        last = data_sz;
        MPID_Segment_pack_vector(segment_ptr, 0, &last, MPIU_CH4U_REQUEST(areq, iov), &n_iov);
        MPIU_Assert(last == (MPI_Aint)data_sz);
        *data = MPIU_CH4U_REQUEST(areq, iov);
        *p_data_sz = n_iov;
        MPIU_CH4U_REQUEST(areq, status) |= MPIDI_CH4U_REQ_RCV_NON_CONTIG;
        MPIU_Free(segment_ptr);
    }

    *req = areq;
    *cmpl_handler_fn = MPIDI_CH4I_am_get_acc_ack_cmpl_handler;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_ACC_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                       void *reply_token, void **data,
                                                       size_t * p_data_sz, int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_cswap_ack_msg_t *msg_hdr = (MPIDI_CH4U_cswap_ack_msg_t *) am_hdr;
    MPID_Request *creq;
    uint64_t dt_contig, data_sz, dt_true_lb;
    MPID_Datatype *dt_ptr;


    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_CSWAP_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_CSWAP_ACK_HANDLER);

    creq = (MPID_Request *)msg_hdr->req_ptr;
    MPIDI_Datatype_get_info(1, MPIU_CH4U_REQUEST(creq, creq.datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);

    *data = MPIU_CH4U_REQUEST(creq, creq.result_addr);
    *p_data_sz = data_sz;

    *req = creq;
    *cmpl_handler_fn = MPIDI_CH4I_am_cswap_ack_cmpl_handler;
    
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_CSWAP_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_win_ctrl_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_win_ctrl_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                        void *reply_token, void **data,
                                                        size_t * p_data_sz, int *is_contig,
                                                        MPIDI_netmod_am_completion_handler_fn *
                                                        cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, sender_rank;
    MPIDI_CH4U_win_cntrl_msg_t *msg_hdr = (MPIDI_CH4U_win_cntrl_msg_t *) am_hdr;
    MPID_Win *win;
    MPID_Request *preq;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_WIN_CTRL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_WIN_CTRL_HANDLER);

    win = (MPID_Win *)MPIDI_CH4I_map_lookup(MPIDI_CH4_Global.win_map, msg_hdr->win_id);
    sender_rank = msg_hdr->origin_rank;

    switch (msg_hdr->type) {

    case MPIDI_CH4U_WIN_LOCK:
    case MPIDI_CH4U_WIN_LOCKALL:
        MPIDI_CH4I_win_lock_req_proc(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_LOCK_ACK:
    case MPIDI_CH4U_WIN_LOCKALL_ACK:
        MPIDI_CH4I_win_lock_ack_proc(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_UNLOCK:
    case MPIDI_CH4U_WIN_UNLOCKALL:
        MPIDI_CH4I_win_unlock_proc(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_UNLOCK_ACK:
    case MPIDI_CH4U_WIN_UNLOCKALL_ACK:
        MPIDI_CH4I_win_unlock_done_cb(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_COMPLETE:
        MPIDI_CH4I_win_complete_proc(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_POST:
        MPIDI_CH4I_win_post_proc(msg_hdr, win, sender_rank);
        break;

    default:
        fprintf(stderr, "not implemented: %d\n", msg_hdr->type);
    }

    if (req)
        *req = NULL;
    if (cmpl_handler_fn)
        *cmpl_handler_fn = NULL;
    
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_WIN_CTRL_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
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
            MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
            MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list, 
                                curr, tmp) {
                MPL_DL_DELETE(MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list, curr);
                MPL_DL_APPEND(MPIU_CH4U_COMM(comm, unexp_list), curr);
            }
            MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list = NULL;
        }
    }
    MPIU_CH4U_COMM(comm, window_instance) = 0;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT_COMM);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_target_handler(void *am_hdr,
                                                   size_t am_hdr_sz,
                                                   void *reply_token,
                                                   void **data,
                                                   size_t * p_data_sz,
                                                   int *is_contig,
                                                    MPIDI_netmod_am_completion_handler_fn *
                                                   cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    size_t data_sz;
    struct iovec *iov, *dt_iov;

    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;

    MPIDI_CH4U_put_msg_t *msg_hdr = (MPIDI_CH4U_put_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_PUT_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_PUT_HANDLER);

    rreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    *req = rreq;

    MPIU_CH4U_REQUEST(*req, preq.preq_ptr) = msg_hdr->preq_ptr;
    MPIU_CH4U_REQUEST(*req, preq.reply_token) = reply_token;

    *cmpl_handler_fn = MPIDI_CH4I_am_put_cmpl_handler;
    if (msg_hdr->n_iov) {
        dt_iov = (struct iovec *)MPIU_Malloc(sizeof(struct iovec) * msg_hdr->n_iov);
        MPIU_Assert(dt_iov);

        iov = (struct iovec *)((char *)am_hdr + sizeof(*msg_hdr));
        MPIU_Memcpy(dt_iov, iov, sizeof(struct iovec) * msg_hdr->n_iov);
        MPIU_CH4U_REQUEST(rreq, preq.dt_iov) = dt_iov;
        *is_contig = 0;
        *data = iov;
        *p_data_sz = n_iov;
        goto fn_exit;
    }

    MPIU_CH4U_REQUEST(rreq, preq.dt_iov) = NULL;
    MPIDI_Datatype_get_info(msg_hdr->count, msg_hdr->datatype,
                            dt_contig, data_sz, dt_ptr, dt_true_lb);
    *is_contig = dt_contig;

    if (dt_contig) {
        *p_data_sz = data_sz;
        *data = (char *) (msg_hdr->addr + dt_true_lb);
    }
    else {
        segment_ptr = MPID_Segment_alloc();
        MPIU_Assert(segment_ptr);

        MPID_Segment_init((void *)msg_hdr->addr, msg_hdr->count, msg_hdr->datatype,
                          segment_ptr, 0);
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

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_PUT_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_target_handler(void *am_hdr,
                                                     size_t am_hdr_sz,
                                                     void *reply_token,
                                                     void **data,
                                                     size_t * p_data_sz,
                                                     int *is_contig,
                                                     MPIDI_netmod_am_completion_handler_fn *
                                                     cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    size_t data_sz;

    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;
    void *p_data;

    MPIDI_CH4U_cswap_req_msg_t *msg_hdr = (MPIDI_CH4U_cswap_req_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_CSWAP_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_CSWAP_HANDLER);

    rreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    *req = rreq;

    *cmpl_handler_fn = MPIDI_CH4I_am_cswap_cmpl_handler;
    MPIDI_Datatype_get_info(1, msg_hdr->datatype, dt_contig, data_sz,
                            dt_ptr, dt_true_lb);
    *is_contig = dt_contig;

    MPIU_CH4U_REQUEST(*req, creq.creq_ptr) = msg_hdr->req_ptr;
    MPIU_CH4U_REQUEST(*req, creq.reply_token) = reply_token;
    MPIU_CH4U_REQUEST(*req, creq.datatype) = msg_hdr->datatype;
    MPIU_CH4U_REQUEST(*req, creq.addr) = msg_hdr->addr;

    MPIU_Assert(dt_contig == 1);
    p_data = MPIU_Malloc(data_sz * 2);
    MPIU_Assert(p_data);

    *p_data_sz = data_sz * 2;
    *data = p_data;
    MPIU_CH4U_REQUEST(*req, creq.data) = p_data;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_CSWAP_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_handle_acc_request(void *am_hdr,
                                                size_t am_hdr_sz,
                                                void *reply_token,
                                                void **data,
                                                size_t * p_data_sz,
                                                int *is_contig,
                                                MPIDI_netmod_am_completion_handler_fn *
                                                cmpl_handler_fn, MPID_Request ** req,
                                                int do_get)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    size_t data_sz;
    int dt_contig, n_iov;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    void *p_data;
    struct iovec *iov, *dt_iov;

    MPIDI_CH4U_acc_req_msg_t *msg_hdr = (MPIDI_CH4U_acc_req_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_HANDLE_ACC_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_HANDLE_ACC_REQ);

    rreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    *req = rreq;

    MPIDI_Datatype_get_info(msg_hdr->origin_count, msg_hdr->origin_datatype,
                            dt_contig, data_sz, dt_ptr, dt_true_lb);
    p_data = MPIU_Malloc(data_sz);
    MPIU_Assert(p_data);

    /* todo: pkt header should contain data_sz ? */

    *cmpl_handler_fn = (do_get) ? MPIDI_CH4I_am_get_acc_cmpl_handler :
        MPIDI_CH4I_am_acc_cmpl_handler;
    *is_contig = 1;
    *p_data_sz = data_sz;
    *data = p_data;

    MPIU_CH4U_REQUEST(*req, areq.req_ptr) = msg_hdr->req_ptr;
    MPIU_CH4U_REQUEST(*req, areq.reply_token) = reply_token;
    MPIU_CH4U_REQUEST(*req, areq.origin_datatype) = msg_hdr->origin_datatype;
    MPIU_CH4U_REQUEST(*req, areq.target_datatype) = msg_hdr->target_datatype;
    MPIU_CH4U_REQUEST(*req, areq.origin_count) = msg_hdr->origin_count;
    MPIU_CH4U_REQUEST(*req, areq.target_count) = msg_hdr->target_count;
    MPIU_CH4U_REQUEST(*req, areq.target_addr) = (void *)msg_hdr->target_addr;
    MPIU_CH4U_REQUEST(*req, areq.op) = msg_hdr->op;
    MPIU_CH4U_REQUEST(*req, areq.data) = p_data;
    MPIU_CH4U_REQUEST(*req, areq.n_iov) = msg_hdr->n_iov;
    MPIU_CH4U_REQUEST(*req, areq.data_sz) = data_sz;

    if (!msg_hdr->n_iov) {
        MPIU_CH4U_REQUEST(rreq, areq.dt_iov) = NULL;
        goto fn_exit;
    }

    dt_iov = (struct iovec *) MPIU_Malloc(sizeof(struct iovec) * msg_hdr->n_iov);
    MPIU_Assert(dt_iov);

    iov = (struct iovec *) ((char *) msg_hdr + sizeof(*msg_hdr));
    MPIU_Memcpy(dt_iov, iov, sizeof(struct iovec) * msg_hdr->n_iov);
    MPIU_CH4U_REQUEST(rreq, areq.dt_iov) = dt_iov;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_HANDLE_ACC_REQ);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_target_handler(void *am_hdr,
                                                   size_t am_hdr_sz,
                                                   void *reply_token,
                                                   void **data,
                                                   size_t * p_data_sz,
                                                   int *is_contig,
                                                   MPIDI_netmod_am_completion_handler_fn *
                                                   cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_ACC_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_ACC_HANDLER);
    MPIDU_RC_POP(MPIDI_CH4I_handle_acc_request(am_hdr, am_hdr_sz, reply_token,
                                               data, p_data_sz, is_contig,
                                               cmpl_handler_fn, req, 0));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_ACC_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_target_handler(void *am_hdr,
                                                       size_t am_hdr_sz,
                                                       void *reply_token,
                                                       void **data,
                                                       size_t * p_data_sz,
                                                       int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_ACC_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_ACC_HANDLER);
    MPIDU_RC_POP(MPIDI_CH4I_handle_acc_request(am_hdr, am_hdr_sz, reply_token,
                                               data, p_data_sz, is_contig,
                                               cmpl_handler_fn, req, 1));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_ACC_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_target_handler(void *am_hdr,
                                                   size_t am_hdr_sz,
                                                   void *reply_token,
                                                   void **data,
                                                   size_t * p_data_sz,
                                                   int *is_contig,
                                                   MPIDI_netmod_am_completion_handler_fn *
                                                   cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, i;
    MPID_Request *rreq = NULL;
    int dt_contig, n_iov;
    uint64_t data_sz, offset;
    MPI_Aint dt_true_lb, last;
    MPID_Datatype *dt_ptr;
    MPIDI_CH4U_get_req_msg_t *msg_hdr = (MPIDI_CH4U_get_req_msg_t *) am_hdr;
    MPIDI_CH4U_get_ack_msg_t get_ack;
    struct iovec *iov;
    void *p_data;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_GET_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_GET_HANDLER);

    rreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    get_ack.greq_ptr = msg_hdr->greq_ptr;
    *req = NULL;
    *cmpl_handler_fn = NULL;
    
    if (!msg_hdr->n_iov) {
        MPIU_CH4U_REQUEST(rreq, greq.dt_iov) = NULL;
        MPIDI_Datatype_get_info(msg_hdr->count, msg_hdr->datatype,
                                dt_contig, data_sz, dt_ptr, dt_true_lb);
        MPIDU_RC_POP(MPIDI_netmod_send_am_reply(reply_token,
                                                MPIDI_CH4U_AM_GET_ACK,
                                                &get_ack, sizeof(get_ack),
                                                (void *)msg_hdr->addr, msg_hdr->count,
                                                msg_hdr->datatype, rreq));
        goto fn_exit;
    }
        
    iov = (struct iovec *)((char *)am_hdr + sizeof(*msg_hdr));
    data_sz = 0;
    for(i = 0; i < msg_hdr->n_iov; i++) {
        data_sz += iov[i].iov_len;
    }

    p_data = MPIU_Malloc(data_sz);
    MPIU_Assert(p_data);

    offset = 0;
    for(i = 0; i < msg_hdr->n_iov; i++) {
        MPIU_Memcpy((char *)p_data + offset, iov[i].iov_base, iov[i].iov_len);
        offset += iov[i].iov_len;
    }

    MPIU_CH4U_REQUEST(rreq, greq.dt_iov) = p_data;
    MPIDU_RC_POP(MPIDI_netmod_send_am_reply(reply_token,
                                            MPIDI_CH4U_AM_GET_ACK,
                                            &get_ack, sizeof(get_ack),
                                            p_data, data_sz, MPI_BYTE, rreq));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_GET_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_ack_target_handler(void *am_hdr,
                                                       size_t am_hdr_sz,
                                                       void *reply_token,
                                                       void **data,
                                                       size_t * p_data_sz,
                                                       int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    size_t data_sz;

    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;

    MPIDI_CH4U_get_ack_msg_t *msg_hdr = (MPIDI_CH4U_get_ack_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_AM_GET_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_AM_GET_ACK_HANDLER);

    rreq = (MPID_Request *) msg_hdr->greq_ptr;
    MPIU_Assert(rreq->kind == MPID_WIN_REQUEST);

    if (MPIU_CH4U_REQUEST(rreq, greq.dt_iov)) {
        MPIU_Free(MPIU_CH4U_REQUEST(rreq, greq.dt_iov));
    }

    *cmpl_handler_fn = MPIDI_CH4I_am_get_ack_cmpl_handler;
    MPIDI_Datatype_get_info(MPIU_CH4U_REQUEST(rreq, greq.count),
                            MPIU_CH4U_REQUEST(rreq, greq.datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);

    *is_contig = dt_contig;
    *req = rreq;

    if (dt_contig) {
        *p_data_sz = data_sz;
        *data = (char *) (MPIU_CH4U_REQUEST(rreq, greq.addr) + dt_true_lb);
    }
    else {
        segment_ptr = MPID_Segment_alloc();
        MPIU_Assert(segment_ptr);

        MPID_Segment_init((void *)MPIU_CH4U_REQUEST(rreq, greq.addr),
                          MPIU_CH4U_REQUEST(rreq, greq.count),
                          MPIU_CH4U_REQUEST(rreq, greq.datatype),
                          segment_ptr, 0);
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

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_AM_GET_ACK_HANDLER);
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
                                             &MPIDI_CH4I_am_ssend_ack_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_PUT_REQ,
                                             &MPIDI_CH4I_am_put_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_put_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_PUT_ACK,
                                             &MPIDI_CH4I_am_put_ack_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_put_ack_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_GET_REQ,
                                             &MPIDI_CH4I_am_get_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_get_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_GET_ACK,
                                             &MPIDI_CH4I_am_get_ack_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_get_ack_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_CSWAP_REQ,
                                              &MPIDI_CH4I_am_cswap_origin_cmpl_handler,
                                              &MPIDI_CH4I_am_cswap_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_CSWAP_ACK,
                                              &MPIDI_CH4I_am_cswap_ack_origin_cmpl_handler,
                                              &MPIDI_CH4I_am_cswap_ack_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_ACC_REQ,
                                              &MPIDI_CH4I_am_acc_origin_cmpl_handler,
                                              &MPIDI_CH4I_am_acc_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_ACC_ACK,
                                              &MPIDI_CH4I_am_acc_ack_origin_cmpl_handler,
                                              &MPIDI_CH4I_am_acc_ack_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_GET_ACC_REQ,
                                              &MPIDI_CH4I_am_acc_origin_cmpl_handler,
                                              &MPIDI_CH4I_am_get_acc_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_GET_ACC_ACK,
                                              &MPIDI_CH4I_am_get_acc_ack_origin_cmpl_handler,
                                              &MPIDI_CH4I_am_get_acc_ack_target_handler));

    MPIDU_RC_POP(MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_WIN_CTRL,
                                              NULL,
                                              &MPIDI_CH4I_am_win_ctrl_target_handler));


    MPIDU_RC_POP(MPIDI_CH4U_init_comm(comm_world));
    MPIDU_RC_POP(MPIDI_CH4U_init_comm(comm_self));
    MPIDI_CH4I_map_create(&MPIDI_CH4_Global.win_map);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT);
  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4U_finalize()
{
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FINALIZE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FINALIZE);
    MPIDI_CH4I_map_destroy(MPIDI_CH4_Global.win_map);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FINALIZE);
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
