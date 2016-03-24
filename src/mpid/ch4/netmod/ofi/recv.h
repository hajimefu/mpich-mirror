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

#define MPIDI_CH4_NMI_OFI_ON_HEAP      0
#define MPIDI_CH4_NMI_OFI_USE_EXISTING 1

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Do_irecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NMI_OFI_Do_irecv(void          *buf,
                                                 int            count,
                                                 MPI_Datatype   datatype,
                                                 int            rank,
                                                 int            tag,
                                                 MPID_Comm     *comm,
                                                 int            context_offset,
                                                 MPID_Request **request,
                                                 int            mode,
                                                 uint64_t       flags)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    uint64_t match_bits, mask_bits;
    MPIU_Context_id_t context_id = comm->recvcontext_id + context_offset;
    size_t data_sz;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    struct fi_msg_tagged msg;
    char *recv_buf;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_IRECV);

    if(mode == MPIDI_CH4_NMI_OFI_ON_HEAP)         /* Branch should compile out */
        MPIDI_CH4_NMI_OFI_REQUEST_CREATE(rreq);
    else if(mode == MPIDI_CH4_NMI_OFI_USE_EXISTING)
        rreq = *request;

    rreq->kind = MPID_REQUEST_RECV;
    *request = rreq;

    if(unlikely(rank == MPI_PROC_NULL)) {
        rreq->kind = MPID_REQUEST_RECV;
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        rreq->status.MPI_SOURCE = rank;
        rreq->status.MPI_TAG = tag;
        MPIDI_CH4U_request_complete(rreq);
        goto fn_exit;
    }

    match_bits = MPIDI_CH4_NMI_OFI_Init_recvtag(&mask_bits, context_id, rank, tag);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, datatype) = datatype;
    dtype_add_ref_if_not_builtin(datatype);

    recv_buf = (char *) buf + dt_true_lb;

    if(!dt_contig) {
        MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig) = (MPIDI_CH4_NMI_OFI_Noncontig_t*) MPL_malloc(data_sz+sizeof(MPID_Segment));
        MPIR_ERR_CHKANDJUMP1(MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig->pack_buffer) == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv Pack Buffer alloc");
        recv_buf = MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig->pack_buffer);
        MPID_Segment_init(buf, count, datatype, &MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig->segment), 0);
    }
    else
        MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig) = NULL;

    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_comm) = comm;
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_id) = context_id;

    if(unlikely(data_sz > MPIDI_Global.max_send)) {
        MPIDI_CH4_NMI_OFI_REQUEST(rreq, event_id) = MPIDI_CH4_NMI_OFI_EVENT_RECV_HUGE;
        data_sz = MPIDI_Global.max_send;
    } else
        MPIDI_CH4_NMI_OFI_REQUEST(rreq, event_id) = MPIDI_CH4_NMI_OFI_EVENT_RECV;

    if(!flags)  /* Branch should compile out */
        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_trecv(MPIDI_CH4_NMI_OFI_EP_RX_TAG(0),
                                              recv_buf,
                                              data_sz,
                                              NULL,
                                              (MPI_ANY_SOURCE == rank) ? FI_ADDR_UNSPEC : MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank,
                                                      MPIDI_CH4_NMI_OFI_API_TAG),
                                              match_bits, mask_bits, (void *) &(MPIDI_CH4_NMI_OFI_REQUEST(rreq, context))), trecv);
    else {
        MPIDI_CH4_NMI_OFI_REQUEST(rreq,util.iov).iov_base = recv_buf;
        MPIDI_CH4_NMI_OFI_REQUEST(rreq,util.iov).iov_len  = data_sz;

        msg.msg_iov   = &MPIDI_CH4_NMI_OFI_REQUEST(rreq,util.iov);
        msg.desc      = NULL;
        msg.iov_count = 1;
        msg.tag       = match_bits;
        msg.ignore    = mask_bits;
        msg.context   = (void *) &(MPIDI_CH4_NMI_OFI_REQUEST(rreq, context));
        msg.data      = 0;
        msg.addr      = FI_ADDR_UNSPEC;

        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_trecvmsg(MPIDI_CH4_NMI_OFI_EP_RX_TAG(0), &msg, flags), trecv);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_IRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_recv(void *buf,
                                        int count,
                                        MPI_Datatype datatype,
                                        int rank,
                                        int tag,
                                        MPID_Comm *comm,
                                        int context_offset,
                                        MPI_Status *status, MPID_Request **request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV);
    mpi_errno = MPIDI_CH4_NMI_OFI_Do_irecv(buf, count, datatype, rank, tag, comm,
                                           context_offset, request, MPIDI_CH4_NMI_OFI_ON_HEAP, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_recv_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_recv_init(void          *buf,
                                             int            count,
                                             MPI_Datatype   datatype,
                                             int            rank,
                                             int            tag,
                                             MPID_Comm     *comm,
                                             int            context_offset,
                                             MPID_Request **request)
{
    MPID_Request *rreq;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV_INIT);

    MPIDI_CH4_NMI_OFI_REQUEST_CREATE((rreq));

    *request = rreq;
    rreq->kind = MPID_PREQUEST_RECV;
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);

    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util.persist.buf)   = (void *) buf;
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util.persist.count) = count;
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, datatype)           = datatype;
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util.persist.rank)  = rank;
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util.persist.tag)   = tag;
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_comm)          = comm;
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_id)            = comm->context_id + context_offset;
    rreq->partner_request             = NULL;

    MPIDI_CH4U_request_complete(rreq);

    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util.persist.type) = MPIDI_PTYPE_RECV;

    if(HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN) {
        MPID_Datatype *dt_ptr;
        MPID_Datatype_get_ptr(datatype, dt_ptr);
        MPID_Datatype_add_ref(dt_ptr);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV_INIT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_imrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_imrecv(void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          MPID_Request *message, MPID_Request **rreqp)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_IMRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_IMRECV);

    if(message == NULL) {
        MPIDI_CH4_NMI_OFI_Request_create_null_rreq(rreq, mpi_errno, fn_fail);
        *rreqp = rreq;
        goto fn_exit;
    }

    MPIU_Assert(message != NULL);
    MPIU_Assert(message->kind == MPID_REQUEST_MPROBE);

    *rreqp = rreq = message;

    mpi_errno = MPIDI_CH4_NMI_OFI_Do_irecv(buf, count, datatype, message->status.MPI_SOURCE,
                                           message->status.MPI_TAG, rreq->comm, 0,
                                           &rreq, MPIDI_CH4_NMI_OFI_USE_EXISTING, FI_CLAIM | FI_COMPLETION);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_IMRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_irecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_irecv(void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm *comm, int context_offset, MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_IRECV);
    mpi_errno = MPIDI_CH4_NMI_OFI_Do_irecv(buf, count, datatype, rank, tag, comm,
                                           context_offset, request, MPIDI_CH4_NMI_OFI_ON_HEAP, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_IRECV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_cancel_recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_cancel_recv(MPID_Request *rreq)
{

    int mpi_errno = MPI_SUCCESS;
    ssize_t ret;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CANCEL_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CANCEL_RECV);

#ifndef MPIDI_BUILD_CH4_SHM
    MPIDI_CH4_NMI_OFI_PROGRESS();
#endif /* MPIDI_BUILD_CH4_SHM */
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);
    ret = fi_cancel((fid_t) MPIDI_CH4_NMI_OFI_EP_RX_TAG(0), &(MPIDI_CH4_NMI_OFI_REQUEST(rreq, context)));
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);

    if(ret == 0) {
        while((!MPIR_STATUS_GET_CANCEL_BIT(rreq->status)) && (!MPIR_cc_is_complete(&rreq->cc))) {
            if((mpi_errno = MPIDI_CH4_NM_progress(&MPIDI_CH4_NMI_OFI_REQUEST(rreq, context), 0)) != MPI_SUCCESS)
                goto fn_exit;
        }

        if(MPIR_STATUS_GET_CANCEL_BIT(rreq->status)) {
            MPIR_STATUS_SET_CANCEL_BIT(rreq->status, TRUE);
            MPIR_STATUS_SET_COUNT(rreq->status, 0);
            MPIDI_CH4U_request_complete(rreq);
        }
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CANCEL_RECV);
    return mpi_errno;
#ifndef MPIDI_BUILD_CH4_SHM
fn_fail:
    goto fn_exit;
#endif
}

#endif /* NETMOD_OFI_RECV_H_INCLUDED */
