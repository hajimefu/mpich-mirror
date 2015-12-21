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
#ifndef MPIDCH4_RECV_H_INCLUDED
#define MPIDCH4_RECV_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_Recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Recv(void *buf,
                              int count,
                              MPI_Datatype datatype,
                              int rank,
                              int tag,
                              MPID_Comm * comm,
                              int context_offset, MPI_Status * status, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_RECV);

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno =
        MPIDI_netmod_recv(buf, count, datatype, rank, tag, comm, context_offset, status, request);
#else
    if (unlikely(rank == MPI_ANY_SOURCE))
    {
         mpi_errno = MPIDI_shm_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);

        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        mpi_errno = MPIDI_netmod_irecv(buf, count, datatype, rank, tag, comm, context_offset,
                                       &(MPIU_CH4_REQUEST(*request, anysource_partner_request)));

        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
        else if (*request) {
            MPIU_CH4_REQUEST(*request, is_local) = 1;
            MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(*request, anysource_partner_request), is_local) = 0;
        }

        MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(*request, anysource_partner_request), anysource_partner_request) = *request;
    }
    else
    {
        int r;
        if ((r = MPIDI_CH4U_rank_is_local(rank, comm)))
            mpi_errno =
                MPIDI_shm_recv(buf, count, datatype, rank, tag, comm, context_offset, status, request);
        else
            mpi_errno =
                MPIDI_netmod_recv(buf, count, datatype, rank, tag, comm, context_offset, status,
                                  request);
        if(mpi_errno == MPI_SUCCESS && *request) {
            MPIU_CH4_REQUEST(*request, is_local) = r;
            MPIU_CH4_REQUEST(*request, anysource_partner_request) = NULL;
        }
    }
#endif

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_RECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Recv_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Recv_init(void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_RECV_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_RECV_INIT);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_recv_init(buf, count, datatype, rank, tag,
                                       comm, context_offset, request);
#else
    if (unlikely(rank == MPI_ANY_SOURCE))
    {
         mpi_errno = MPIDI_shm_recv_init(buf, count, datatype, rank, tag, comm, context_offset, request);

        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        mpi_errno = MPIDI_netmod_recv_init(buf, count, datatype, rank, tag, comm, context_offset,
                                       &(MPIU_CH4_REQUEST(*request, anysource_partner_request)));

        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        MPIU_CH4_REQUEST(*request, is_local) = 1;
        MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(*request, anysource_partner_request), is_local) = 0;

        MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(*request, anysource_partner_request), anysource_partner_request) = *request;
    }
    else {
        int r;
        if ((r = MPIDI_CH4U_rank_is_local(rank, comm)))
            mpi_errno = MPIDI_shm_recv_init(buf, count, datatype, rank, tag,
                                        comm, context_offset, request);
        else
            mpi_errno = MPIDI_netmod_recv_init(buf, count, datatype, rank, tag,
                                           comm, context_offset, request);
        if(mpi_errno == MPI_SUCCESS) {
            MPIU_CH4_REQUEST(*request, is_local) = r;
            MPIU_CH4_REQUEST(*request, anysource_partner_request) = NULL;
        }
    }
#endif
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_RECV_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Mrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Mrecv(void *buf,
                               int count,
                               MPI_Datatype datatype,
                               MPID_Request * message,
                               MPI_Status * status)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_MRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_MRECV);

    MPI_Request   req_handle;
    int           active_flag;
    MPID_Request *rreq = NULL;

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_imrecv(buf, count, datatype, message, &rreq);
#else
    if (message == NULL) {
        /* treat as though MPI_MESSAGE_NO_PROC was passed */
        MPIR_Status_set_procnull(status);
        goto fn_exit;
    }

    if (unlikely(message->status.MPI_SOURCE == MPI_ANY_SOURCE))
    {
        mpi_errno = MPIDI_shm_imrecv(buf, count, datatype, message, &rreq);

        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        mpi_errno = MPIDI_netmod_imrecv(buf, count, datatype, message, &(MPIU_CH4_REQUEST(rreq, anysource_partner_request)));

        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(rreq, anysource_partner_request), anysource_partner_request) = rreq;
        MPIU_CH4_REQUEST(rreq, is_local) = 1;
        MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(rreq, anysource_partner_request), is_local) = 0;
    }
    else {
        int r;
        if (r = MPIDI_CH4U_rank_is_local(message->status.MPI_SOURCE, message->comm))
            mpi_errno = MPIDI_shm_imrecv(buf, count, datatype, message, &rreq);
        else
            mpi_errno = MPIDI_netmod_imrecv(buf, count, datatype, message, &rreq);

        if (mpi_errno == MPI_SUCCESS) {
            MPIU_CH4_REQUEST(rreq, anysource_partner_request) = NULL;
            MPIU_CH4_REQUEST(rreq, is_local) = r;
        }
    }
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
    while (!MPID_Request_is_complete(rreq)) {
        MPIDI_netmod_progress(MPIDI_CH4_Global.netmod_context[0], 0);
#ifdef MPIDI_CH4_EXCLUSIVE_SHM
        MPIDI_shm_progress(0);
#endif
    }

    /* This should probably be moved to MPICH (above device) level */
    /* Someone neglected to put the blocking at the MPICH level    */
    MPIR_Request_extract_status(rreq, status);
    MPIR_Request_complete(&req_handle, rreq, status, &active_flag);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_MRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Imrecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Imrecv(void *buf,
                                int count,
                                MPI_Datatype datatype,
                                MPID_Request * message, MPID_Request ** rreqp)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4_IMRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_IMRECV);

    if (message == NULL)
    {
        MPID_Request *rreq;
        MPIDI_Request_create_null_rreq(rreq, mpi_errno, goto fn_fail);
        *rreqp = rreq;
        goto fn_exit;
    }

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_imrecv(buf, count, datatype, message, rreqp);
#else
    int r;
    if (r = MPIDI_CH4U_rank_is_local(message->status.MPI_SOURCE, message->comm))
        mpi_errno = MPIDI_shm_imrecv(buf, count, datatype, message, rreqp);
    else
        mpi_errno = MPIDI_netmod_imrecv(buf, count, datatype, message, rreqp);
    if (mpi_errno == MPI_SUCCESS) {
        MPIU_CH4_REQUEST(*rreqp, is_local) = r;
        MPIU_CH4_REQUEST(*rreqp, anysource_partner_request) = NULL;
    }
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_IMRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Irecv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Irecv(void *buf,
                               int count,
                               MPI_Datatype datatype,
                               int rank,
                               int tag,
                               MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_IRECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_IRECV);

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);
#else
    if (unlikely(rank == MPI_ANY_SOURCE))
    {
         mpi_errno = MPIDI_shm_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);

        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }

        mpi_errno = MPIDI_netmod_irecv(buf, count, datatype, rank, tag, comm, context_offset,
                                       &(MPIU_CH4_REQUEST(*request, anysource_partner_request)));

        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
        else if (*request) {
            MPIU_CH4_REQUEST(*request, is_local) = 1;
            MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(*request, anysource_partner_request), is_local) = 0;
       }

        MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(*request, anysource_partner_request), anysource_partner_request) = *request;
    }
    else
    {
        int r;
        if ((r = MPIDI_CH4U_rank_is_local(rank, comm)))
            mpi_errno = MPIDI_shm_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);
        else
            mpi_errno =
                MPIDI_netmod_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);
        if(mpi_errno == MPI_SUCCESS && *request) {
            MPIU_CH4_REQUEST(*request, is_local) = r;
            MPIU_CH4_REQUEST(*request, anysource_partner_request) = NULL;
        }
    }
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_IRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Cancel_Recv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Cancel_recv(MPID_Request * rreq)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_CANCEL_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_CANCEL_RECV);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_cancel_recv(rreq);
#else
    if (MPIU_CH4_REQUEST(rreq, is_local))
        mpi_errno = MPIDI_shm_cancel_recv(rreq);
    else
        mpi_errno = MPIDI_netmod_cancel_recv(rreq);
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_CANCEL_RECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* MPIDCH4_RECV_H_INCLUDED */
