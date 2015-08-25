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
#ifndef NETMOD_OFI_SEND_H_INCLUDED
#define NETMOD_OFI_SEND_H_INCLUDED

#include "impl.h"
#include <mpibsend.h>
#include <../mpi/pt2pt/bsendutil.h>

#define SENDPARAMS const void *buf,int count,MPI_Datatype datatype, \
    int rank,int tag,MPID_Comm *comm,                               \
    int context_offset,MPID_Request **request

#define SENDARGS buf,count,datatype,rank,tag, \
                 comm,context_offset,request

#undef FUNCNAME
#define FUNCNAME send_lightweight
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int send_lightweight(const void *buf,MPIDI_msg_sz_t data_sz,
                                       int rank, int tag, MPID_Comm * comm,
                                       int context_offset)
{
    int mpi_errno = MPI_SUCCESS;
    ssize_t ret;
    uint64_t match_bits;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT);
    match_bits = init_sendtag(comm->context_id + context_offset, comm->rank, tag, 0);
    FI_RC_RETRY(fi_tinject(G_TXC_TAG(0), buf, data_sz,
                           _comm_to_phys(comm, rank, MPIDI_API_TAG), match_bits), tinject);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME send_normal
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int send_normal(const void *buf,
                              int count,
                              MPI_Datatype datatype,
                              int rank,
                              int tag,
                              MPID_Comm * comm,
                              int context_offset,
                              MPID_Request ** request,
                              int dt_contig,
                              MPIDI_msg_sz_t data_sz,
                              MPID_Datatype * dt_ptr, MPI_Aint dt_true_lb, uint64_t type)
{
    int mpi_errno = MPI_SUCCESS;
    ssize_t ret;
    MPID_Request *sreq = NULL;
    MPI_Aint last;
    char *send_buf;
    uint64_t match_bits;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_NORMAL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_NORMAL);

    REQ_CREATE(sreq);
    sreq->kind = MPID_REQUEST_SEND;
    *request = sreq;
    match_bits = init_sendtag(comm->context_id + context_offset, comm->rank, tag, type);
    REQ_OFI(sreq, event_id) = MPIDI_EVENT_SEND;
    REQ_OFI(sreq, datatype) = datatype;
    dtype_add_ref_if_not_builtin(datatype);

    if (type == MPID_SYNC_SEND) {       /* Branch should compile out */
        int c = 1;
        uint64_t ssend_match, ssend_mask;
        MPIDI_Ssendack_request *ackreq;
        SSENDACKREQ_CREATE(ackreq);
        ackreq->event_id = MPIDI_EVENT_SSEND_ACK;
        ackreq->signal_req = sreq;
        MPID_cc_incr(sreq->cc_ptr, &c);
        ssend_match = init_recvtag(&ssend_mask, comm->context_id + context_offset, rank, tag);
        ssend_match |= MPID_SYNC_SEND_ACK;
        FI_RC_RETRY(fi_trecv(G_RXC_TAG(0),      /* Socket      */
                             NULL,      /* recvbuf     */
                             0, /* data sz     */
                             MPIDI_Global.mr,   /* data descr  */
                             _comm_to_phys(comm, rank, MPIDI_API_TAG),  /* remote proc */
                             ssend_match,       /* match bits  */
                             0ULL,      /* mask bits   */
                             (void *) &(ackreq->context)), trecvsync);
    }

    send_buf = (char *) buf + dt_true_lb;

    if (!dt_contig) {
        MPIDI_msg_sz_t segment_first, segment_size;
        struct MPID_Segment *segment_ptr;
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, segment_ptr, 0);
        segment_first = 0;
        segment_size = data_sz;
        last = data_sz;
        REQ_OFI(sreq, pack_buffer) = (char *) MPIU_Malloc(data_sz);
        MPIR_ERR_CHKANDJUMP1(REQ_OFI(sreq, pack_buffer) == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send Pack buffer alloc");
        MPID_Segment_pack(segment_ptr, segment_first, &last, REQ_OFI(sreq, pack_buffer));
        MPID_Segment_free(segment_ptr);
        send_buf = REQ_OFI(sreq, pack_buffer);
    }
    else
        REQ_OFI(sreq, pack_buffer) = NULL;

    if (data_sz <= MPIDI_Global.max_buffered_send) {
        FI_RC_RETRY(fi_tinject(G_TXC_TAG(0), send_buf, data_sz,
                               _comm_to_phys(comm, rank, MPIDI_API_TAG), match_bits), tinject);
        send_event(NULL, sreq);
    }
    else if (data_sz <= MPIDI_Global.max_send)
        FI_RC_RETRY(fi_tsend(G_TXC_TAG(0), send_buf,
                             data_sz, MPIDI_Global.mr, _comm_to_phys(comm, rank, MPIDI_API_TAG),
                             match_bits, (void *) &(REQ_OFI(sreq, context))), tsend);
    else if (unlikely(1)) {
        MPIDI_Send_control_t ctrl;
        int c;
        MPIDI_Hugecntr *cntr;
        void *ptr;
        c = 1;
        REQ_OFI(sreq, event_id) = MPIDI_EVENT_SEND_HUGE;
        MPID_cc_incr(sreq->cc_ptr, &c);
        ptr = MPIDI_OFI_Map_lookup(COMM_OFI(comm)->huge_send_counters, rank);

        if (ptr == MPIDI_MAP_NOT_FOUND) {
            ptr = MPIU_Malloc(sizeof(int));
            cntr = (MPIDI_Hugecntr *) ptr;
            cntr->outstanding = 0;
            cntr->counter = 0;
            MPIDI_OFI_Map_set(COMM_OFI(comm)->huge_send_counters, rank, ptr);
        }

        cntr = (MPIDI_Hugecntr *) ptr;
        cntr->outstanding++;
        cntr->counter++;
        MPIU_Assert(cntr->outstanding != USHRT_MAX);
        MPIU_Assert(cntr->counter != USHRT_MAX);
        REQ_OFI(sreq, util_comm) = comm;
        REQ_OFI(sreq, util_id) = rank;
        FI_RC_RETRY(fi_tsend(G_TXC_TAG(0), send_buf,
                             MPIDI_Global.max_send, MPIDI_Global.mr, _comm_to_phys(comm, rank,
                                                                                   MPIDI_API_TAG),
                             match_bits, (void *) &(REQ_OFI(sreq, context))), tsend);
        ctrl.type = MPIDI_CTRL_HUGE;
        ctrl.seqno = cntr->counter - 1;
        MPI_RC_POP(do_control_send(&ctrl, send_buf, data_sz, rank, comm, sreq));
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_NORMAL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME nm_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int nm_send(SENDPARAMS, int noreq, uint64_t syncflag)
{
    int dt_contig, mpi_errno;
    MPIDI_msg_sz_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NM_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NM_SEND);

    if (unlikely(rank == MPI_PROC_NULL)) {
        mpi_errno = MPI_SUCCESS;

        if (!noreq) {
            REQ_CREATE((*request));
            (*request)->kind = MPID_REQUEST_SEND;
            MPIDI_Request_complete((*request));
        }

        goto fn_exit;
    }
    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    if (likely(noreq && dt_contig && (data_sz <= MPIDI_Global.max_buffered_send)))
        mpi_errno = send_lightweight((char *) buf + dt_true_lb, data_sz,
                                     rank, tag, comm, context_offset);
    else
        mpi_errno = send_normal(buf, count, datatype, rank, tag, comm,
                                context_offset, request, dt_contig,
                                data_sz, dt_ptr, dt_true_lb, syncflag);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NM_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME nm_psend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int nm_psend(const void *buf,
                           int count,
                           MPI_Datatype datatype,
                           int rank,
                           int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    MPID_Request *sreq;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NM_PSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NM_PSEND);

    REQ_CREATE(sreq);
    *request = sreq;

    MPIR_Comm_add_ref(comm);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    REQ_OFI(sreq, p_buf) = (void *) buf;
    REQ_OFI(sreq, p_count) = count;
    REQ_OFI(sreq, datatype) = datatype;
    REQ_OFI(sreq, p_rank) = rank;
    REQ_OFI(sreq, p_tag) = tag;
    REQ_OFI(sreq, util_comm) = comm;
    REQ_OFI(sreq, util_id) = comm->context_id + context_offset;
    sreq->partner_request = NULL;
    MPIDI_Request_complete(sreq);

    if (HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN) {
        MPID_Datatype *dt_ptr;
        MPID_Datatype_get_ptr(datatype, dt_ptr);
        MPID_Datatype_add_ref(dt_ptr);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NM_PSEND);
    return MPI_SUCCESS;
}

#define STARTALL_CASE(CASELABEL,FUNC,CONTEXTID) \
  case CASELABEL:                               \
  {                                             \
    rc = FUNC(REQ_OFI(preq,p_buf),              \
              REQ_OFI(preq,p_count),            \
              REQ_OFI(preq,datatype),           \
              REQ_OFI(preq,p_rank),             \
              REQ_OFI(preq,p_tag),              \
              preq->comm,                       \
              REQ_OFI(preq,util_id) -           \
              CONTEXTID,                        \
              &preq->partner_request);          \
    break;                                      \
  }

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_send(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND);
    mpi_errno = nm_send(SENDARGS, 1, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_rsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_rsend(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RSEND);
    mpi_errno = nm_send(SENDARGS, 1, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RSEND);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_irsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_irsend(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_IRSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_IRSEND);
    mpi_errno = nm_send(SENDARGS, 0, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_IRSEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ssend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_ssend(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SSEND);
    mpi_errno = nm_send(SENDARGS, 0, MPID_SYNC_SEND);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SSEND);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_isend(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_ISEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_ISEND);
    mpi_errno = nm_send(SENDARGS, 0, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_ISEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_issend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_issend(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_ISSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_ISSEND);
    mpi_errno = nm_send(SENDARGS, 0, MPID_SYNC_SEND);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_ISSEND);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_startall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_startall(int count, MPID_Request * requests[])
{
    int rc = MPI_SUCCESS, i;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_STARTALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_STARTALL);

    for (i = 0; i < count; i++) {
        MPID_Request *const preq = requests[i];

        switch (REQ_OFI(preq, p_type)) {
            STARTALL_CASE(MPIDI_PTYPE_RECV, MPIDI_Irecv, preq->comm->recvcontext_id);
            STARTALL_CASE(MPIDI_PTYPE_SEND, MPIDI_Isend, preq->comm->context_id);
            STARTALL_CASE(MPIDI_PTYPE_SSEND, MPIDI_Issend, preq->comm->context_id);

        case MPIDI_PTYPE_BSEND:{
                rc = MPIR_Bsend_isend(REQ_OFI(preq, p_buf),
                                      REQ_OFI(preq, p_count),
                                      REQ_OFI(preq, datatype),
                                      REQ_OFI(preq, p_rank),
                                      REQ_OFI(preq, p_tag),
                                      preq->comm, BSEND_INIT, &preq->partner_request);

                if (preq->partner_request != NULL)
                    MPIU_Object_add_ref(preq->partner_request);

                break;
            }

        default:
            rc = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL, __FUNCTION__,
                                      __LINE__, MPI_ERR_INTERN, "**ch3|badreqtype",
                                      "**ch3|badreqtype %d", REQ_OFI(preq, p_type));
        }

        if (rc == MPI_SUCCESS) {
            preq->status.MPI_ERROR = MPI_SUCCESS;

            if (REQ_OFI(preq, p_type) == MPIDI_PTYPE_BSEND) {
                preq->cc_ptr = &preq->cc;
                MPIDI_Request_set_completed(preq);
            }
            else
                preq->cc_ptr = &preq->partner_request->cc;
        }
        else {
            preq->partner_request = NULL;
            preq->status.MPI_ERROR = rc;
            preq->cc_ptr = &preq->cc;
            MPIDI_Request_set_completed(preq);
        }
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_STARTALL);
    return rc;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_send_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_send_init(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_INIT);
    mpi_errno = nm_psend(SENDARGS);
    REQ_OFI((*request), p_type) = MPIDI_PTYPE_SEND;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ssend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_ssend_init(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SSEND_INIT);
    mpi_errno = nm_psend(SENDARGS);
    REQ_OFI((*request), p_type) = MPIDI_PTYPE_SSEND;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_bsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_bsend_init(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_BSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_BSEND_INIT);
    mpi_errno = nm_psend(SENDARGS);
    REQ_OFI((*request), p_type) = MPIDI_PTYPE_BSEND;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_BSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_rsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_rsend_init(SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RSEND_INIT);
    mpi_errno = nm_psend(SENDARGS);
    REQ_OFI((*request), p_type) = MPIDI_PTYPE_SEND;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_cancel_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_netmod_cancel_send(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CANCEL_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CANCEL_SEND);
    /* Sends cannot be cancelled */

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CANCEL_SEND);
    return mpi_errno;
}

#endif /* NETMOD_OFI_SEND_H_INCLUDED */
