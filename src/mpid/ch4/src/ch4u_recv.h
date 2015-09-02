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

    MPIU_CH4U_REQUEST(rreq, datatype) = datatype;
    MPIU_CH4U_REQUEST(rreq, buffer) = (char *) buf;
    MPIU_CH4U_REQUEST(rreq, count) = count;

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
    MPIDI_msg_sz_t in_data_sz, dt_sz, nbytes;
    MPID_Segment *segment_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_HANDLE_UNEXPECTED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_HANDLE_UNEXPECTED);

    in_data_sz = MPIU_CH4U_REQUEST(rreq, count);
    MPID_Datatype_get_size_macro(datatype, dt_sz);

    if (in_data_sz > dt_sz * count) {
        rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        nbytes = dt_sz * count;
    }
    else {
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        nbytes = in_data_sz;
    }
    MPIR_STATUS_SET_COUNT(rreq->status, nbytes);
    MPIU_CH4U_REQUEST(rreq, datatype) = datatype;
    MPIU_CH4U_REQUEST(rreq, count) = count;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, dt_sz, dt_ptr, dt_true_lb);

    if (!dt_contig) {
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv MPID_Segment_alloc");
        MPID_Segment_init(buf, nbytes/dt_sz, datatype, segment_ptr, 0);

        last = nbytes;
        MPID_Segment_unpack(segment_ptr, 0, &last, MPIU_CH4U_REQUEST(rreq, buffer));
        MPID_Segment_free(segment_ptr);
        if (last != (MPI_Aint)(nbytes)) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __FUNCTION__, __LINE__,
                                             MPI_ERR_TYPE, "**dtypemismatch", 0);
            rreq->status.MPI_ERROR = mpi_errno;
        }
    }
    else {
        MPIU_Memcpy((char *) buf + dt_true_lb, MPIU_CH4U_REQUEST(rreq, buffer), nbytes);
        MPIU_Free(MPIU_CH4U_REQUEST(rreq, buffer));
    }

    MPIU_CH4U_REQUEST(rreq, status) &= ~MPIDI_CH4U_REQ_UNEXPECTED;

    rreq->status.MPI_SOURCE = MPIDI_CH4I_get_source(MPIU_CH4U_REQUEST(rreq, tag));
    rreq->status.MPI_TAG = MPIDI_CH4I_get_tag(MPIU_CH4U_REQUEST(rreq, tag));

    if (MPIU_CH4U_REQUEST(rreq, status) & MPIDI_CH4U_REQ_PEER_SSEND) {
        MPIDU_RC_POP(MPIDI_CH4I_reply_ssend(rreq));
    }
    MPIDI_Request_complete(rreq);
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
static inline int MPIDI_CH4I_do_irecv(void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm,
                                      int context_offset,
                                      MPID_Request ** request, int alloc_req,
                                      uint64_t flags)
{
    int mpi_errno = MPI_SUCCESS, comm_idx;
    MPID_Request *rreq = NULL;
    uint64_t match_bits, mask_bits;
    MPIU_Context_id_t context_id = comm->recvcontext_id + context_offset;
    MPID_Comm *root_comm;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_DO_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_DO_IRECV);

    match_bits = MPIDI_CH4I_init_recvtag(&mask_bits, context_id, rank, tag);
    comm_idx = MPIDI_CH4I_get_context_index(comm->recvcontext_id);
    root_comm = MPIDI_CH4_Global.comms[comm_idx];

    /* MPIDI_CS_ENTER() */
    rreq = MPIDI_CH4I_dequeue_unexp_strict(match_bits, mask_bits,
                                           &MPIU_CH4U_COMM(root_comm, unexp_list));
    /* MPIDI_CS_EXIT() */

    if (rreq) {
        *request = rreq;
        MPIDU_RC_POP(MPIDI_CH4I_handle_unexpected(buf, count, datatype,
                                                 root_comm, context_id, rreq));
        goto fn_exit;
    }

    if (alloc_req) {
        rreq = MPIDI_CH4I_create_req();
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
        MPIDI_Request_complete(rreq);
        goto fn_exit;
    }

    MPIU_CH4U_REQUEST(rreq, tag) = match_bits;
    MPIU_CH4U_REQUEST(rreq, ignore) = mask_bits;
    MPIU_CH4U_REQUEST(rreq, datatype) = datatype;
    MPIDU_RC_POP(MPIDI_CH4I_prepare_recv_req(buf, count, datatype, rreq));

    /* MPIDI_CS_ENTER(); */
    MPIDI_CH4I_enqueue_posted(rreq, &MPIU_CH4U_COMM(root_comm, posted_list));
    /* MPIDI_CS_EXIT(); */
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_DO_IRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_recv(void *buf,
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
    MPIDU_RC_POP(MPIDI_CH4I_do_irecv(buf, count, datatype, rank, tag,
                                    comm, context_offset, request, 1, 0ULL));
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_recv_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_recv_init(void *buf,
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

    rreq = MPIDI_CH4I_create_req();

    *request = rreq;
    rreq->kind = MPID_PREQUEST_RECV;
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);

    MPIU_CH4U_REQUEST(rreq, buffer) = (void *) buf;
    MPIU_CH4U_REQUEST(rreq, count) = count;
    MPIU_CH4U_REQUEST(rreq, datatype) = datatype;
    MPIU_CH4U_REQUEST(rreq, tag) =
        MPIDI_CH4I_init_send_tag(comm->context_id + context_offset, rank, tag);
    rreq->partner_request = NULL;
    MPIDI_Request_complete(rreq);
    MPIU_CH4U_REQUEST(rreq, p_type) = MPIDI_PTYPE_RECV;
    dtype_add_ref_if_not_builtin(datatype);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RECV_INIT);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_imrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_imrecv(void *buf,
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
    MPIU_CH4U_REQUEST(message, mrcv_buffer) = buf;
    MPIU_CH4U_REQUEST(message, mrcv_count) = count;
    MPIU_CH4U_REQUEST(message, mrcv_datatype) = datatype;
    *rreqp = message;
    MPIR_Comm_add_ref(message->comm);

    /* MPIDI_CS_ENTER(); */
    if (MPIU_CH4U_REQUEST(message, status) & MPIDI_CH4U_REQ_BUSY) {
        MPIU_CH4U_REQUEST(message, status) |= MPIDI_CH4U_REQ_UNEXP_CLAIMED;
    }
    else {
        MPIDU_RC_POP(MPIDI_CH4I_unexp_mrecv_cmpl_handler(message));
    }
    /* MPIDI_CS_EXIT(); */

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_IMRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_mrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_mrecv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    MPID_Request * message, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS, active_flag;
    MPI_Request req_handle;
    MPID_Request *rreq = NULL;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_MRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_MRECV);

    MPIDU_RC_POP(MPIDI_Imrecv(buf, count, datatype, message, &rreq));

    while (!MPID_Request_is_complete(rreq)) {
        MPIDI_Progress_test();
    }

    MPIR_Request_extract_status(rreq, status);
    MPIDU_RC_POP(MPIR_Request_complete(&req_handle, rreq, status, &active_flag));

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_MRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_irecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_irecv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_IRECV);
    MPIDU_RC_POP(MPIDI_CH4I_do_irecv(buf, count, datatype, rank, tag,
                                    comm, context_offset, request, 1, 0ULL));
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_IRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_cancel_recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_cancel_recv(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, found, comm_idx;
    MPID_Comm *root_comm;
    uint64_t msg_tag;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_CANCEL_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_CANCEL_RECV);

    msg_tag = MPIU_CH4U_REQUEST(rreq, tag);
    comm_idx = MPIDI_CH4I_get_context_index(MPIDI_CH4I_get_context(msg_tag));
    root_comm = MPIDI_CH4_Global.comms[comm_idx];
    found = MPIDI_CH4I_delete_posted(&rreq->dev.ch4.ch4u, &MPIU_CH4U_COMM(root_comm, posted_list));

    if (found) {
        MPIR_STATUS_SET_CANCEL_BIT(rreq->status, TRUE);
        MPIR_STATUS_SET_COUNT(rreq->status, 0);
        MPIDI_Request_complete(rreq);
    }
    else {
        MPIR_STATUS_SET_CANCEL_BIT(rreq->status, FALSE);
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_CANCEL_RECV);
    return mpi_errno;
}

#endif /* MPIDCH4U_RECV_H_INCLUDED */
