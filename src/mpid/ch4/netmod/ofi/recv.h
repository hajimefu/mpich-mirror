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
#ifndef NETMOD_OFI_RECV_H_INCLUDED
#define NETMOD_OFI_RECV_H_INCLUDED

#include "impl.h"

enum recv_mode {
    ON_HEAP, USE_EXISTING
};

#undef FUNCNAME
#define FUNCNAME recv_callback
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int recv_callback(cq_tagged_entry_t * wc, MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint last;
    size_t count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV_CALLBACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV_CALLBACK);

    rreq->status.MPI_ERROR = MPI_SUCCESS;
    rreq->status.MPI_SOURCE = get_source(wc->tag);
    rreq->status.MPI_TAG = get_tag(wc->tag);
    count = wc->len;
    MPIR_STATUS_SET_COUNT(rreq->status, count);

    if (REQ_OFI(rreq, pack_buffer)) {
        last = count;
        MPID_Segment_unpack(REQ_OFI(rreq, segment_ptr), 0, &last, REQ_OFI(rreq, pack_buffer));
        MPIU_Free(REQ_OFI(rreq, pack_buffer));
        MPID_Segment_free(REQ_OFI(rreq, segment_ptr));

        if (last != count) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __FUNCTION__, __LINE__,
                                             MPI_ERR_TYPE, "**dtypemismatch", 0);
            rreq->status.MPI_ERROR = mpi_errno;
        }
    }

    dtype_release_if_not_builtin(REQ_OFI(rreq, datatype));

    /* If syncronous, ack and complete when the ack is done */
    if (unlikely(is_tag_sync(wc->tag))) {
        uint64_t ss_bits = init_sendtag(REQ_OFI(rreq, util_id),
                                        REQ_OFI(rreq, util_comm->rank),
                                        rreq->status.MPI_TAG,
                                        MPID_SYNC_SEND_ACK);
        MPID_Comm *c = REQ_OFI(rreq, util_comm);
        int r = rreq->status.MPI_SOURCE;
        FI_RC_RETRY(fi_tinject(G_TXC_TAG(0), NULL, 0, _comm_to_phys(c, r, MPIDI_API_TAG),
                               ss_bits), tsendsync);

    }

    MPIDI_Request_complete(rreq);

    /* Polling loop will check for truncation */
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV_CALLBACK);
    return mpi_errno;
  fn_fail:
    rreq->status.MPI_ERROR = mpi_errno;
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME recv_huge_callback
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int recv_callback_huge(cq_tagged_entry_t * wc, MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_Huge_recv_t *recv;
    MPIDI_Huge_chunk_t *hc;
    MPID_Comm *comm_ptr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV_HUGE_CALLBACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV_HUGE_CALLBACK);

    /* Look up the receive sequence number and chunk queue */
    comm_ptr = REQ_OFI(rreq, util_comm);
    recv = (MPIDI_Huge_recv_t *) MPIDI_OFI_Map_lookup(COMM_OFI(comm_ptr)->huge_recv_counters,
                                                      get_source(wc->tag));

    if (recv == MPIDI_MAP_NOT_FOUND) {
        recv = (MPIDI_Huge_recv_t *) MPIU_Malloc(sizeof(*recv));
        recv->seqno = 0;
        MPIDI_OFI_Map_create(&recv->chunk_q);
        MPIDI_OFI_Map_set(COMM_OFI(comm_ptr)->huge_recv_counters, get_source(wc->tag), recv);
    }

    /* Look up the receive in the chunk queue */
    hc = (MPIDI_Huge_chunk_t *) MPIDI_OFI_Map_lookup(recv->chunk_q, recv->seqno);

    if (hc == MPIDI_MAP_NOT_FOUND) {
        hc = (MPIDI_Huge_chunk_t *) MPIU_Malloc(sizeof(*hc));
        memset(hc, 0, sizeof(*hc));
        hc->callback = MPIDI_OFI_Gethuge_callback;
        MPIDI_OFI_Map_set(recv->chunk_q, recv->seqno, hc);
    }

    recv->seqno++;
    hc->localreq = rreq;
    hc->done_fn = recv_callback;
    hc->wc = *wc;
    MPIDI_OFI_Gethuge_callback(NULL, (MPID_Request *) hc);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV_HUGE_CALLBACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME do_irecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_irecv(void *buf,
                           int count,
                           MPI_Datatype datatype,
                           int rank,
                           int tag,
                           MPID_Comm * comm,
                           int context_offset,
                           MPID_Request ** request, enum recv_mode mode, uint64_t flags)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    uint64_t match_bits, mask_bits;
    MPIU_Context_id_t context_id = comm->recvcontext_id + context_offset;
    MPIDI_msg_sz_t data_sz;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    iovec_t iov;
    msg_tagged_t msg;
    char *recv_buf;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_IRECV);

    if (mode == ON_HEAP)        /* Branch should compile out */
        REQ_CREATE(rreq);
    else if (mode == USE_EXISTING)
        rreq = *request;

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

    match_bits = init_recvtag(&mask_bits, context_id, rank, tag);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    REQ_OFI(rreq, datatype) = datatype;
    dtype_add_ref_if_not_builtin(datatype);

    recv_buf = (char *) buf + dt_true_lb;

    if (!dt_contig) {
        REQ_OFI(rreq, segment_ptr) = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(REQ_OFI(rreq, segment_ptr) == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, REQ_OFI(rreq, segment_ptr), 0);

        REQ_OFI(rreq, pack_buffer) = (char *) MPIU_Malloc(data_sz);
        MPIR_ERR_CHKANDJUMP1(REQ_OFI(rreq, pack_buffer) == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv Pack Buffer alloc");
        recv_buf = REQ_OFI(rreq, pack_buffer);
    }
    else
        REQ_OFI(rreq, pack_buffer) = NULL;

    REQ_OFI(rreq, util_comm) = comm;
    REQ_OFI(rreq, util_id) = context_id;

    if (unlikely(data_sz > MPIDI_Global.max_send)) {
        REQ_OFI(rreq, callback) = recv_callback_huge;
        data_sz = MPIDI_Global.max_send;
    }
    else
        REQ_OFI(rreq, callback) = recv_callback;

    if (!flags) /* Branch should compile out */
        FI_RC_RETRY(fi_trecv(G_RXC_TAG(0),
                             recv_buf,
                             data_sz,
                             MPIDI_Global.mr,
                             (MPI_ANY_SOURCE == rank) ? FI_ADDR_UNSPEC : _comm_to_phys(comm, rank,
                                                                                       MPIDI_API_TAG),
                             match_bits, mask_bits, (void *) &(REQ_OFI(rreq, context))), trecv);
    else {
        iov.iov_base = recv_buf;
        iov.iov_len = data_sz;

        msg.msg_iov = &iov;
        msg.desc = NULL;
        msg.iov_count = 1;
        msg.tag = match_bits;
        msg.ignore = mask_bits;
        msg.context = (void *) &(REQ_OFI(rreq, context));
        msg.data = 0;
        msg.addr = FI_ADDR_UNSPEC;

        MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex);
        FI_RC_RETRY(fi_trecvmsg(G_RXC_TAG(0), &msg, flags), trecv);
        MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_IRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_recv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm,
                                    int context_offset,
                                    MPI_Status * status, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV);
    mpi_errno = do_irecv(buf, count, datatype, rank, tag, comm,
                         context_offset, request, ON_HEAP, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_recv_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_recv_init(void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm,
                                         int context_offset, MPID_Request ** request)
{
    MPID_Request *rreq;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV_INIT);

    REQ_CREATE((rreq));

    *request = rreq;
    rreq->kind = MPID_PREQUEST_RECV;
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);

    REQ_OFI(rreq, p_buf) = (void *) buf;
    REQ_OFI(rreq, p_count) = count;
    REQ_OFI(rreq, datatype) = datatype;
    REQ_OFI(rreq, p_rank) = rank;
    REQ_OFI(rreq, p_tag) = tag;
    REQ_OFI(rreq, util_comm) = comm;
    REQ_OFI(rreq, util_id) = comm->context_id + context_offset;
    rreq->partner_request = NULL;

    MPIDI_Request_complete(rreq);

    REQ_OFI(rreq, p_type) = MPIDI_PTYPE_RECV;

    if (HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN) {
        MPID_Datatype *dt_ptr;
        MPID_Datatype_get_ptr(datatype, dt_ptr);
        MPID_Datatype_add_ref(dt_ptr);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV_INIT);
    return MPI_SUCCESS;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_mrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_mrecv(void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     MPID_Request * message, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Request req_handle;
    int active_flag;
    MPID_Request *rreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_MRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_MRECV);

    MPI_RC_POP(MPIDI_Imrecv(buf, count, datatype, message, &rreq));

    if (!MPID_Request_is_complete(rreq))
        PROGRESS_WHILE(!MPID_Request_is_complete(rreq));

    MPIR_Request_extract_status(rreq, status);
    MPI_RC_POP(MPIR_Request_complete(&req_handle, rreq, status, &active_flag));

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_MRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_imrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_imrecv(void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      MPID_Request * message, MPID_Request ** rreqp)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    MPID_Comm *comm;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_IMRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_IMRECV);

    if (message == NULL) {
        MPIDI_Request_create_null_rreq(rreq, mpi_errno, fn_fail);
        *rreqp = rreq;
        goto fn_exit;
    }

    MPIU_Assert(message != NULL);
    MPIU_Assert(message->kind == MPID_REQUEST_MPROBE);

    *rreqp = rreq = message;
    comm = rreq->comm;

    mpi_errno = do_irecv(buf, count, datatype, message->status.MPI_SOURCE,
                         message->status.MPI_TAG, rreq->comm, 0,
                         &rreq, USE_EXISTING, FI_CLAIM | FI_COMPLETION);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_IMRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_irecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_irecv(void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_IRECV);
    mpi_errno = do_irecv(buf, count, datatype, rank, tag, comm,
                         context_offset, request, ON_HEAP, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_IRECV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_cancel_recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_cancel_recv(MPID_Request * rreq)
{

    int mpi_errno = MPI_SUCCESS;
    ssize_t ret;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CANCEL_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CANCEL_RECV);

    PROGRESS();
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex);
    ret = fi_cancel((fid_t) G_RXC_TAG(0), &(REQ_OFI(rreq, context)));
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);

    if (ret == 0) {
        while (!MPIR_STATUS_GET_CANCEL_BIT(rreq->status)) {
            if ((mpi_errno = MPIDI_Progress_test()) != MPI_SUCCESS)
                goto fn_exit;
        }

        MPIR_STATUS_SET_CANCEL_BIT(rreq->status, TRUE);
        MPIR_STATUS_SET_COUNT(rreq->status, 0);
        MPIDI_Request_complete(rreq);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CANCEL_RECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* NETMOD_OFI_RECV_H_INCLUDED */