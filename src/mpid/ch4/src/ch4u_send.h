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
#define FUNCNAME ch4_do_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int ch4_do_send(const void *buf,
                              int count,
                              MPI_Datatype datatype,
                              int rank,
                              int tag,
                              MPID_Comm * comm,
                              int context_offset, MPID_Request ** request, int type)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPID_Request *sreq = NULL;
    MPI_Aint last;
    char *send_buf;
    uint64_t match_bits;
    MPIDI_CH4U_AM_Hdr_t am_hdr;
    MPIDI_CH4U_Ssend_req_msg_t ssend_req;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_DO_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_DO_SEND);

    sreq = MPIDI_CH4U_Create_req();
    MPIU_Assert(sreq);
    sreq->kind = MPID_REQUEST_SEND;

    *request = sreq;
    match_bits = MPID_CH4U_init_send_tag(comm->context_id + context_offset, comm->rank, tag);

    am_hdr.msg_tag = match_bits;
    if (type == MPIDI_CH4U_AM_SSEND_REQ) {
        ssend_req.hdr = am_hdr;
        ssend_req.sreq_ptr = (uint64_t) sreq;
        MPID_cc_incr(sreq->cc_ptr, &c);
        MPIU_RC_POP(MPIDI_netmod_send_am(rank, comm, MPIDI_CH4U_AM_SSEND_REQ,
                                         &ssend_req, sizeof(ssend_req),
                                         buf, count, datatype, sreq, NULL));
    }
    else {
        MPIU_RC_POP(MPIDI_netmod_send_am(rank, comm, MPIDI_CH4U_AM_SEND,
                                         &am_hdr, sizeof(am_hdr),
                                         buf, count, datatype, sreq, NULL));
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_DO_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME ch4_nm_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int ch4_nm_send(const void *buf, int count, MPI_Datatype datatype,
                              int rank, int tag, MPID_Comm * comm, int context_offset,
                              MPID_Request ** request, int noreq, int type)
{
    int mpi_errno;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_NM_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_NM_SEND);

    if (unlikely(rank == MPI_PROC_NULL)) {
        mpi_errno = MPI_SUCCESS;
        if (!noreq) {
            *request = MPIDI_CH4U_Create_req();
            (*request)->kind = MPID_REQUEST_SEND;
            MPIDI_Request_complete((*request));
        }
        goto fn_exit;
    }

    mpi_errno = ch4_do_send(buf, count, datatype, rank, tag, comm, context_offset, request, type);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_NM_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME ch4_nm_psend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int ch4_nm_psend(const void *buf,
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

    sreq = MPIDI_CH4U_Create_req();
    *request = sreq;

    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    match_bits = MPID_CH4U_init_send_tag(comm->context_id + context_offset, rank, tag);

    MPIU_CH4U_REQUEST(sreq, buffer) = (void *) buf;
    MPIU_CH4U_REQUEST(sreq, count) = count;
    MPIU_CH4U_REQUEST(sreq, datatype) = datatype;
    MPIU_CH4U_REQUEST(sreq, tag) = match_bits;

    sreq->partner_request = NULL;
    MPIDI_Request_complete(sreq);

    dtype_add_ref_if_not_builtin(datatype);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NM_PSEND);
    return MPI_SUCCESS;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Send(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_SEND);
    mpi_errno = ch4_nm_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 1, 0ULL);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Isend(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ISEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ISEND);
    mpi_errno = ch4_nm_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 0, 0ULL);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ISEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Rsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Rsend(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RSEND);
    mpi_errno = ch4_nm_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 1, 0ULL);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Irsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Irsend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_IRSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_IRSEND);
    mpi_errno = ch4_nm_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 0, 0ULL);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Ssend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Ssend(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_SSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_SSEND);
    mpi_errno = ch4_nm_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 1, MPIDI_CH4U_AM_SSEND_REQ);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Issend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Issend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ISSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ISSEND);
    mpi_errno = ch4_nm_send(buf, count, datatype, rank, tag, comm,
                            context_offset, request, 0, MPIDI_CH4U_AM_SSEND_REQ);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ISSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Startall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Startall(int count, MPID_Request * requests[])
{
    int mpi_errno = MPI_SUCCESS, i;
    int rank, tag, context_offset;
    MPI_Datatype datatype;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_STARTALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_STARTALL);

    for (i = 0; i < count; i++) {
        MPID_Request *const preq = requests[i];
        tag = MPIDI_CH4U_Get_tag(MPIU_CH4U_REQUEST(preq, tag));
        rank = MPIDI_CH4U_Get_source(MPIU_CH4U_REQUEST(preq, tag));
        context_offset = MPIDI_CH4U_Get_context(MPIU_CH4U_REQUEST(preq, tag) -
                                                preq->comm->context_id);
        datatype = MPIU_CH4U_REQUEST(preq, datatype);

        switch (MPIU_CH4U_REQUEST(preq, p_type)) {

        case MPIDI_PTYPE_RECV:
            mpi_errno = MPIDI_Irecv(MPIU_CH4U_REQUEST(preq, buffer),
                                    MPIU_CH4U_REQUEST(preq, count),
                                    datatype, rank, tag,
                                    preq->comm, context_offset, &preq->partner_request);
            break;

        case MPIDI_PTYPE_SEND:
            mpi_errno = MPIDI_Isend(MPIU_CH4U_REQUEST(preq, buffer),
                                    MPIU_CH4U_REQUEST(preq, count),
                                    datatype, rank, tag,
                                    preq->comm, context_offset, &preq->partner_request);
            break;

        case MPIDI_PTYPE_SSEND:
            mpi_errno = MPIDI_Issend(MPIU_CH4U_REQUEST(preq, buffer),
                                     MPIU_CH4U_REQUEST(preq, count),
                                     datatype, rank, tag,
                                     preq->comm, context_offset, &preq->partner_request);
            break;

        case MPIDI_PTYPE_BSEND:
            mpi_errno = MPIR_Bsend_isend(MPIU_CH4U_REQUEST(preq, buffer),
                                         MPIU_CH4U_REQUEST(preq, count),
                                         datatype, rank, tag,
                                         preq->comm, BSEND_INIT, &preq->partner_request);

            if (preq->partner_request != NULL)
                MPIU_Object_add_ref(preq->partner_request);

            break;

        default:
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL, __FUNCTION__,
                                             __LINE__, MPI_ERR_INTERN, "**ch3|badreqtype",
                                             "**ch3|badreqtype %d", MPIU_CH4U_REQUEST(preq,
                                                                                      p_type));
        }

        if (mpi_errno == MPI_SUCCESS) {
            preq->status.MPI_ERROR = MPI_SUCCESS;

            if (MPIU_CH4U_REQUEST(preq, p_type) == MPIDI_PTYPE_BSEND) {
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

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_STARTALL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Send_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Send_init(const void *buf,
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
    mpi_errno = ch4_nm_psend(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPIU_CH4U_REQUEST((*request), p_type) = MPIDI_PTYPE_SEND;
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SEND_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Ssend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Ssend_init(const void *buf,
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
    mpi_errno = ch4_nm_psend(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPIU_CH4U_REQUEST((*request), p_type) = MPIDI_PTYPE_SSEND;
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_SSEND_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Bsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Bsend_init(const void *buf,
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
    mpi_errno = ch4_nm_psend(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPIU_CH4U_REQUEST((*request), p_type) = MPIDI_PTYPE_BSEND;
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_BSEND_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Rsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Rsend_init(const void *buf,
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
    mpi_errno = ch4_nm_psend(buf, count, datatype, rank, tag, comm, context_offset, request);
    MPIU_CH4U_REQUEST((*request), p_type) = MPIDI_PTYPE_SEND;
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RSEND_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_Cancel_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_Cancel_send(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_CANCEL_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_CANCEL_SEND);
    /* cannot cancel send */
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_CANCEL_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* MPIDCH4U_SEND_H_INCLUDED */