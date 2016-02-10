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
#ifndef MPIDCH4U_RECV_H_INCLUDED
#define MPIDCH4U_RECV_H_INCLUDED

#include "ch4_impl.h"
#include "ch4r_proc.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_prepare_recv_req
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_prepare_recv_req(void *buf, int count, MPI_Datatype datatype,
                                              MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_PREPARE_RECV_BUFFER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_PREPARE_RECV_BUFFER);

    MPIDI_CH4R_REQUEST(rreq, datatype) = datatype;
    MPIDI_CH4R_REQUEST(rreq, buffer) = (char *) buf;
    MPIDI_CH4R_REQUEST(rreq, count) = count;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_PREPARE_RECV_BUFFER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_handle_unexpected
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_handle_unexpected(void *buf,
                                               int count,
                                               MPI_Datatype datatype,
                                               MPID_Comm * comm,
                                               int context_offset, MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    int dt_contig;
    MPI_Aint dt_true_lb, last;
    MPID_Datatype *dt_ptr;
    size_t in_data_sz, dt_sz, nbytes;
    MPID_Segment *segment_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_HANDLE_UNEXPECTED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_HANDLE_UNEXPECTED);

    in_data_sz = MPIDI_CH4R_REQUEST(rreq, count);
    MPID_Datatype_get_size_macro(datatype, dt_sz);

    if (in_data_sz > dt_sz * count) {
        rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        nbytes = dt_sz * count;
    }
    else {
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        nbytes = in_data_sz;
        count = dt_sz ? nbytes/dt_sz : 0;
    }
    MPIR_STATUS_SET_COUNT(rreq->status, nbytes);
    MPIDI_CH4R_REQUEST(rreq, datatype) = datatype;
    MPIDI_CH4R_REQUEST(rreq, count) = nbytes;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, dt_sz, dt_ptr, dt_true_lb);

    if (!dt_contig) {
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, segment_ptr, 0);

        last = nbytes;
        MPID_Segment_unpack(segment_ptr, 0, &last, MPIDI_CH4R_REQUEST(rreq, buffer));
        MPID_Segment_free(segment_ptr);
        if (last != (MPI_Aint)(nbytes)) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __FUNCTION__, __LINE__,
                                             MPI_ERR_TYPE, "**dtypemismatch", 0);
            rreq->status.MPI_ERROR = mpi_errno;
        }
    }
    else {
        MPIU_Memcpy((char *) buf + dt_true_lb, MPIDI_CH4R_REQUEST(rreq, buffer), nbytes);
    }

    MPIDI_CH4R_REQUEST(rreq, req->status) &= ~MPIDI_CH4R_REQ_UNEXPECTED;
    MPL_free(MPIDI_CH4R_REQUEST(rreq, buffer));
        
    rreq->status.MPI_SOURCE = MPIDI_CH4R_get_source(MPIDI_CH4R_REQUEST(rreq, tag));
    rreq->status.MPI_TAG = MPIDI_CH4R_get_tag(MPIDI_CH4R_REQUEST(rreq, tag));

    if (MPIDI_CH4R_REQUEST(rreq, req->status) & MPIDI_CH4R_REQ_PEER_SSEND) {
        mpi_errno = MPIDI_CH4R_reply_ssend(rreq);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }
    MPIDI_CH4I_am_request_complete(rreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_HANDLE_UNEXPECTED);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_do_irecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_do_irecv(void          *buf,
                                      int            count,
                                      MPI_Datatype   datatype,
                                      int            rank,
                                      int            tag,
                                      MPID_Comm     *comm,
                                      int            context_offset,
                                      MPID_Request **request,
                                      int            alloc_req,
                                      uint64_t       flags)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL, *unexp_req = NULL;
    uint64_t match_bits, mask_bits;
    MPIU_Context_id_t context_id = comm->recvcontext_id + context_offset;
    MPID_Comm *root_comm;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_DO_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_DO_IRECV);

    match_bits = MPIDI_CH4R_init_recvtag(&mask_bits, context_id, rank, tag);
    root_comm  = MPIDI_CH4R_context_id_to_comm(comm->recvcontext_id);
    unexp_req  = MPIDI_CH4R_dequeue_unexp(match_bits, mask_bits,
                                          &MPIDI_CH4R_COMM(root_comm, unexp_list));

    if (unexp_req) {
        MPIR_Comm_release(root_comm); /* -1 for removing from unexp_list */
        if (MPIDI_CH4R_REQUEST(unexp_req, req->status) & MPIDI_CH4R_REQ_BUSY) {
            MPIDI_CH4R_REQUEST(unexp_req, req->status) |= MPIDI_CH4R_REQ_MATCHED;
        }
        else if (MPIDI_CH4R_REQUEST(unexp_req, req->status) & MPIDI_CH4R_REQ_LONG_RTS) {
            MPIDI_CH4R_Send_long_ack_msg_t msg;
            int c;

            /* Matching receive is now posted, sending CTS to the peer */
            msg.sreq_ptr = MPIDI_CH4R_REQUEST(unexp_req, req->rreq.peer_req_ptr);
            msg.rreq_ptr = (uint64_t) unexp_req;
            MPIU_Assert((void *)msg.sreq_ptr != NULL);
            MPIR_cc_incr(unexp_req->cc_ptr, &c);
            mpi_errno = MPIDI_CH4_NM_send_am_hdr_reply(MPIDI_CH4R_REQUEST(unexp_req, req->rreq.reply_token),
                                                       MPIDI_CH4R_SEND_LONG_ACK,
                                                       &msg, sizeof(msg), unexp_req);
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);

            dtype_add_ref_if_not_builtin(datatype);
            MPIDI_CH4R_REQUEST(unexp_req, datatype) = datatype;
            MPIDI_CH4R_REQUEST(unexp_req, buffer) = (char *) buf;
            MPIDI_CH4R_REQUEST(unexp_req, count) = count;
            *request = unexp_req;
            goto fn_exit;
        }
        else {
            *request = unexp_req;
            mpi_errno = MPIDI_CH4I_handle_unexpected(buf, count, datatype,
                                                     root_comm, context_id, unexp_req);
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);
            goto fn_exit;
        }
    }

    if (alloc_req) {
        rreq = MPIDI_CH4I_am_request_create();
    }
    else {
        rreq = *request;
        MPIU_Assert(0);
    }

    rreq->kind = MPID_REQUEST_RECV;
    *request = rreq;
    if (unlikely(rank == MPI_PROC_NULL)) {
        rreq->kind = MPID_REQUEST_RECV;
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        rreq->status.MPI_SOURCE = rank;
        rreq->status.MPI_TAG = tag;
        MPIDI_CH4I_am_request_complete(rreq);
        goto fn_exit;
    }

    dtype_add_ref_if_not_builtin(datatype);
    MPIDI_CH4R_REQUEST(rreq, tag) = match_bits;
    MPIDI_CH4R_REQUEST(rreq, req->rreq.ignore) = mask_bits;
    MPIDI_CH4R_REQUEST(rreq, datatype) = datatype;

    mpi_errno = MPIDI_CH4I_prepare_recv_req(buf, count, datatype, rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);


    if (!unexp_req) {
        /* MPIDI_CS_ENTER(); */
        /* Increment refcnt for comm before posting rreq to posted_list,
           to make sure comm is alive while holding an entry in the posted_list */
        MPIR_Comm_add_ref(root_comm);
        MPIDI_CH4R_enqueue_posted(rreq, &MPIDI_CH4R_COMM(root_comm, posted_list));
        /* MPIDI_CS_EXIT(); */
    } else {
        MPIDI_CH4R_REQUEST(unexp_req, req->rreq.match_req) = (uint64_t) rreq;
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_DO_IRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_recv(void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm,
                                   int context_offset, MPI_Status * status, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RECV);

    mpi_errno = MPIDI_CH4I_do_irecv(buf, count, datatype, rank, tag,
                                    comm, context_offset, request, 1, 0ULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_recv_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_recv_init(void *buf,
                                        int count,
                                        MPI_Datatype datatype,
                                        int rank,
                                        int tag,
                                        MPID_Comm * comm,
                                        int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RECV_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RECV_INIT);

    rreq = MPIDI_CH4I_am_request_create();

    *request = rreq;
    rreq->kind = MPID_PREQUEST_RECV;
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);

    MPIDI_CH4R_REQUEST(rreq, buffer) = (void *) buf;
    MPIDI_CH4R_REQUEST(rreq, count) = count;
    MPIDI_CH4R_REQUEST(rreq, datatype) = datatype;
    MPIDI_CH4R_REQUEST(rreq, tag) =
        MPIDI_CH4R_init_send_tag(comm->context_id + context_offset, rank, tag);
    rreq->partner_request = NULL;
    MPIDI_CH4I_am_request_complete(rreq);
    MPIDI_CH4R_REQUEST(rreq, p_type) = MPIDI_PTYPE_RECV;
    dtype_add_ref_if_not_builtin(datatype);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RECV_INIT);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_imrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_imrecv(void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     MPID_Request * message, MPID_Request ** rreqp)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_IMRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_IMRECV);

    if (message == NULL) {
        MPIDI_Request_create_null_rreq(rreq, mpi_errno, fn_fail);
        *rreqp = rreq;
        goto fn_exit;
    }

    MPIU_Assert(message->kind == MPID_REQUEST_MPROBE);
    MPIDI_CH4R_REQUEST(message, req->rreq.mrcv_buffer) = buf;
    MPIDI_CH4R_REQUEST(message, req->rreq.mrcv_count) = count;
    MPIDI_CH4R_REQUEST(message, req->rreq.mrcv_datatype) = datatype;
    *rreqp = message;

    /* MPIDI_CS_ENTER(); */
    if (MPIDI_CH4R_REQUEST(message, req->status) & MPIDI_CH4R_REQ_BUSY) {
        MPIDI_CH4R_REQUEST(message, req->status) |= MPIDI_CH4R_REQ_UNEXP_CLAIMED;
    }
    else if (MPIDI_CH4R_REQUEST(message, req->status) & MPIDI_CH4R_REQ_LONG_RTS) {
        MPIDI_CH4R_Send_long_ack_msg_t msg;
        int c;

        /* Matching receive is now posted, sending CTS to the peer */
        msg.sreq_ptr = MPIDI_CH4R_REQUEST(message, req->rreq.peer_req_ptr);
        msg.rreq_ptr = (uint64_t) message;
        MPIU_Assert((void *)msg.sreq_ptr != NULL);
        MPIR_cc_incr(message->cc_ptr, &c);
        mpi_errno = MPIDI_CH4_NM_send_am_hdr_reply(MPIDI_CH4R_REQUEST(message, req->rreq.reply_token),
                                                   MPIDI_CH4R_SEND_LONG_ACK,
                                                   &msg, sizeof(msg), message);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);

        message->kind = MPID_REQUEST_RECV;
        dtype_add_ref_if_not_builtin(datatype);
        MPIDI_CH4R_REQUEST(message, datatype) = datatype;
        MPIDI_CH4R_REQUEST(message, buffer) = (char *) buf;
        MPIDI_CH4R_REQUEST(message, count) = count;
    }
    else {
        mpi_errno = MPIDI_CH4R_unexp_mrecv_cmpl_handler(message);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }
    /* MPIDI_CS_EXIT(); */

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_IMRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_mrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_mrecv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    MPID_Request * message, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS, active_flag;
    MPI_Request req_handle;
    MPID_Request *rreq = NULL;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_MRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_MRECV);

    mpi_errno = MPIDI_Imrecv(buf, count, datatype, message, &rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    while (!MPID_Request_is_complete(rreq)) {
        MPIDI_Progress_test();
    }

    MPIR_Request_extract_status(rreq, status);

    mpi_errno = MPIR_Request_complete(&req_handle, rreq, status, &active_flag);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_MRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_irecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_irecv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_IRECV);

    mpi_errno = MPIDI_CH4I_do_irecv(buf, count, datatype, rank, tag,
                                    comm, context_offset, request, 1, 0ULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_IRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_cancel_recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_cancel_recv(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, found;
    MPID_Comm *root_comm;
    uint64_t msg_tag;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_CANCEL_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_CANCEL_RECV);

    msg_tag   = MPIDI_CH4R_REQUEST(rreq, tag);
    root_comm = MPIDI_CH4R_context_id_to_comm(MPIDI_CH4R_get_context(MPIDI_CH4R_get_context(msg_tag)));

    /* MPIDI_CS_ENTER(); */
    found = MPIDI_CH4R_delete_posted(&rreq->dev.ch4.ch4r.req->rreq, &MPIDI_CH4R_COMM(root_comm, posted_list));
    /* MPIDI_CS_EXIT(); */

    if (found) {
        MPIR_STATUS_SET_CANCEL_BIT(rreq->status, TRUE);
        MPIR_STATUS_SET_COUNT(rreq->status, 0);
        MPIR_Comm_release(root_comm); /* -1 for posted_list */
        MPIDI_CH4I_am_request_complete(rreq);
    }
    else {
        MPIR_STATUS_SET_CANCEL_BIT(rreq->status, FALSE);
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_CANCEL_RECV);
    return mpi_errno;
}

#endif /* MPIDCH4U_RECV_H_INCLUDED */
