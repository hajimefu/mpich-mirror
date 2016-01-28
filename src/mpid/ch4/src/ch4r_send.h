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
#ifndef MPIDCH4U_SEND_H_INCLUDED
#define MPIDCH4U_SEND_H_INCLUDED

#include "ch4_impl.h"

#include <mpibsend.h>
#include <../mpi/pt2pt/bsendutil.h>

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_do_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_do_send(const void    *buf,
                                     int            count,
                                     MPI_Datatype   datatype,
                                     int            rank,
                                     int            tag,
                                     MPID_Comm     *comm,
                                     int            context_offset,
                                     MPID_Request **request,
                                     int            type)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPID_Request *sreq = NULL;
    uint64_t match_bits;
    MPIDI_CH4R_Hdr_t am_hdr;
    MPIDI_CH4R_Ssend_req_msg_t ssend_req;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_DO_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_DO_SEND);

    sreq = MPIDI_CH4I_am_request_create();
    MPIU_Assert(sreq);
    sreq->kind = MPID_REQUEST_SEND;

    *request = sreq;
    match_bits = MPIDI_CH4R_init_send_tag(comm->context_id + context_offset, comm->rank, tag);

    am_hdr.msg_tag = match_bits;
    if (type == MPIDI_CH4R_SSEND_REQ) {
        ssend_req.hdr = am_hdr;
        ssend_req.sreq_ptr = (uint64_t) sreq;
        MPIR_cc_incr(sreq->cc_ptr, &c);

        mpi_errno = MPIDI_CH4_NM_send_am(rank, comm, MPIDI_CH4R_SSEND_REQ,
                                         &ssend_req, sizeof(ssend_req),
                                         buf, count, datatype, sreq, NULL);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }
    else {
        mpi_errno = MPIDI_CH4_NM_send_am(rank, comm, MPIDI_CH4R_SEND,
                                         &am_hdr, sizeof(am_hdr),
                                         buf, count, datatype, sreq, NULL);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_DO_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_send(const void *buf, int count, MPI_Datatype datatype,
                              int rank, int tag, MPID_Comm * comm, int context_offset,
                              MPID_Request ** request, int noreq, int type)
{
    int mpi_errno;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_NM_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_NM_SEND);

    if (unlikely(rank == MPI_PROC_NULL)) {
        mpi_errno = MPI_SUCCESS;
        if (!noreq) {
            *request = MPIDI_CH4I_am_request_create();
            (*request)->kind = MPID_REQUEST_SEND;
            MPIDI_Request_complete((*request));
        }
        goto fn_exit;
    }

    mpi_errno = MPIDI_CH4I_do_send(buf, count, datatype, rank, tag, comm, context_offset, request, type);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_NM_SEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_psend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_psend(const void *buf,
                               int count,
                               MPI_Datatype datatype,
                               int rank,
                               int tag,
                               MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    MPID_Request *sreq;
    uint64_t match_bits;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_NM_PSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_NM_PSEND);

    sreq = MPIDI_CH4I_am_request_create();
    *request = sreq;

    sreq->kind = MPID_PREQUEST_SEND;
    MPIDI_CH4R_REQUEST(sreq, util_comm) = comm;
    match_bits = MPIDI_CH4R_init_send_tag(comm->context_id + context_offset, rank, tag);

    MPIDI_CH4R_REQUEST(sreq, buffer) = (void *) buf;
    MPIDI_CH4R_REQUEST(sreq, count) = count;
    MPIDI_CH4R_REQUEST(sreq, datatype) = datatype;
    MPIDI_CH4R_REQUEST(sreq, tag) = match_bits;

    sreq->partner_request = NULL;
    MPIDI_Request_complete(sreq);

    dtype_add_ref_if_not_builtin(datatype);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NM_PSEND);
    return MPI_SUCCESS;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_send(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_SEND);
    mpi_errno = MPIDI_CH4I_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 1, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_isend(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ISEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ISEND);
    mpi_errno = MPIDI_CH4I_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 0, 0ULL);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ISEND);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_rsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_rsend(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RSEND);
    mpi_errno = MPIDI_CH4I_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 1, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RSEND);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_irsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_irsend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_IRSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_IRSEND);
    mpi_errno = MPIDI_CH4I_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 0, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_ssend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_ssend(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_SSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_SSEND);
    mpi_errno = MPIDI_CH4I_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 1, MPIDI_CH4R_SSEND_REQ);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SSEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_issend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_issend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ISSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ISSEND);
    mpi_errno = MPIDI_CH4I_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 0, MPIDI_CH4R_SSEND_REQ);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ISSEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_startall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_startall(int count, MPID_Request * requests[])
{
    int mpi_errno = MPI_SUCCESS, i;
    int rank, tag, context_offset;
    MPI_Datatype datatype;
    uint64_t msg_tag;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_STARTALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_STARTALL);

    for (i = 0; i < count; i++) {
        MPID_Request *const preq = requests[i];
        MPI_Request sreq_handle;

        msg_tag = MPIDI_CH4R_REQUEST(preq, tag);
        datatype = MPIDI_CH4R_REQUEST(preq, datatype);

        tag = MPIDI_CH4R_get_tag(msg_tag);
        rank = MPIDI_CH4R_get_source(msg_tag);
        context_offset = MPIDI_CH4R_get_context(msg_tag - MPIDI_CH4R_REQUEST(preq, util_comm)->context_id);

        switch (MPIDI_CH4R_REQUEST(preq, p_type)) {

        case MPIDI_PTYPE_RECV:
            mpi_errno = MPIDI_Irecv(MPIDI_CH4R_REQUEST(preq, buffer),
                                    MPIDI_CH4R_REQUEST(preq, count),
                                    datatype, rank, tag,
                                    MPIDI_CH4R_REQUEST(preq, util_comm),
                                    context_offset, &preq->partner_request);
            break;

        case MPIDI_PTYPE_SEND:
            mpi_errno = MPIDI_Isend(MPIDI_CH4R_REQUEST(preq, buffer),
                                    MPIDI_CH4R_REQUEST(preq, count),
                                    datatype, rank, tag,
                                    MPIDI_CH4R_REQUEST(preq, util_comm),
                                    context_offset, &preq->partner_request);
            break;

        case MPIDI_PTYPE_SSEND:
            mpi_errno = MPIDI_Issend(MPIDI_CH4R_REQUEST(preq, buffer),
                                     MPIDI_CH4R_REQUEST(preq, count),
                                     datatype, rank, tag,
                                     MPIDI_CH4R_REQUEST(preq, util_comm),
                                     context_offset, &preq->partner_request);
            break;

        case MPIDI_PTYPE_BSEND:
            mpi_errno = MPIR_Ibsend_impl(MPIDI_CH4R_REQUEST(preq, buffer),
                                         MPIDI_CH4R_REQUEST(preq, count),
                                         datatype, rank, tag,
                                         MPIDI_CH4R_REQUEST(preq, util_comm),
                                         &sreq_handle);
            if (mpi_errno == MPI_SUCCESS)
                MPID_Request_get_ptr(sreq_handle, preq->partner_request);

            break;

        default:
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL, __FUNCTION__,
                                             __LINE__, MPI_ERR_INTERN, "**ch3|badreqtype",
                                             "**ch3|badreqtype %d", MPIDI_CH4R_REQUEST(preq, p_type));
        }

        if (mpi_errno == MPI_SUCCESS) {
            preq->status.MPI_ERROR = MPI_SUCCESS;

            if (MPIDI_CH4R_REQUEST(preq, p_type) == MPIDI_PTYPE_BSEND) {
                preq->cc_ptr = &preq->cc;
                MPIDI_Request_set_completed(preq);
            }
            else
                preq->cc_ptr = &preq->partner_request->cc;
        }
        else {
            preq->partner_request = NULL;
            preq->status.MPI_ERROR = mpi_errno;
            preq->cc_ptr = &preq->cc;
            MPIDI_Request_set_completed(preq);
        }
        dtype_release_if_not_builtin(datatype);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_STARTALL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_send_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_send_init(const void *buf,
                                        int count,
                                        MPI_Datatype datatype,
                                        int rank,
                                        int tag,
                                        MPID_Comm * comm,
                                        int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_SEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_SEND_INIT);
    mpi_errno = MPIDI_CH4I_psend(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPIDI_CH4R_REQUEST((*request), p_type) = MPIDI_PTYPE_SEND;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_ssend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_ssend_init(const void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm,
                                         int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_SSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_SSEND_INIT);
    mpi_errno = MPIDI_CH4I_psend(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPIDI_CH4R_REQUEST((*request), p_type) = MPIDI_PTYPE_SSEND;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_bsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_bsend_init(const void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm,
                                         int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_BSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_BSEND_INIT);
    mpi_errno = MPIDI_CH4I_psend(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPIDI_CH4R_REQUEST((*request), p_type) = MPIDI_PTYPE_BSEND;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_BSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_rsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_rsend_init(const void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm,
                                         int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RSEND_INIT);
    mpi_errno = MPIDI_CH4I_psend(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPIDI_CH4R_REQUEST((*request), p_type) = MPIDI_PTYPE_SEND;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_cancel_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_cancel_send(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_CANCEL_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_CANCEL_SEND);
    /* cannot cancel send */
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_CANCEL_SEND);
    return mpi_errno;
}

#endif /* MPIDCH4U_SEND_H_INCLUDED */
