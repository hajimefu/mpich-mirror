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
#ifndef MPIDCH4_SEND_H_INCLUDED
#define MPIDCH4_SEND_H_INCLUDED

#include "ch4_impl.h"
#include "ch4u_proc.h"
#undef FUNCNAME
#define FUNCNAME MPIDI_Send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Send(const void *buf,
                              int count,
                              MPI_Datatype datatype,
                              int rank,
                              int tag,
                              MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SEND);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_send(buf, count, datatype, rank, tag, comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_send(buf, count, datatype, rank, tag, comm, context_offset, request);
    else
        mpi_errno =
            MPIDI_netmod_send(buf, count, datatype, rank, tag, comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS && *request) MPIU_CH4_REQUEST(*request, is_local) = r;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Isend(const void *buf,
                               int count,
                               MPI_Datatype datatype,
                               int rank,
                               int tag,
                               MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_ISEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_ISEND);

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_isend(buf, count, datatype, rank, tag, comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_isend(buf, count, datatype, rank, tag, comm, context_offset, request);
    else
        mpi_errno =
            MPIDI_netmod_isend(buf, count, datatype, rank, tag, comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS) MPIU_CH4_REQUEST(*request, is_local) = r;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_ISEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Rsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Rsend(const void *buf,
                               int count,
                               MPI_Datatype datatype,
                               int rank,
                               int tag,
                               MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_RSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_RSEND);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_send(buf, count, datatype, rank, tag, comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_send(buf, count, datatype, rank, tag, comm, context_offset, request);
    else
        mpi_errno =
            MPIDI_netmod_send(buf, count, datatype, rank, tag, comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS && *request) MPIU_CH4_REQUEST(*request, is_local) = r;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_RSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Irsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Irsend(const void *buf,
                                int count,
                                MPI_Datatype datatype,
                                int rank,
                                int tag,
                                MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_IRSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_IRSEND);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_isend(buf, count, datatype, rank, tag, comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_isend(buf, count, datatype, rank, tag, comm, context_offset, request);
    else
        mpi_errno =
            MPIDI_netmod_isend(buf, count, datatype, rank, tag, comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS) MPIU_CH4_REQUEST(*request, is_local) = r;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Ssend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Ssend(const void *buf,
                               int count,
                               MPI_Datatype datatype,
                               int rank,
                               int tag,
                               MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SSEND);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_ssend(buf, count, datatype, rank, tag, comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_ssend(buf, count, datatype, rank, tag, comm, context_offset, request);
    else
        mpi_errno =
            MPIDI_netmod_ssend(buf, count, datatype, rank, tag, comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS && *request) MPIU_CH4_REQUEST(*request, is_local) = r;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Issend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Issend(const void *buf,
                                int count,
                                MPI_Datatype datatype,
                                int rank,
                                int tag,
                                MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_ISSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_ISSEND);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_issend(buf, count, datatype, rank, tag, comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno =
            MPIDI_shm_issend(buf, count, datatype, rank, tag, comm, context_offset, request);
    else
        mpi_errno =
            MPIDI_netmod_issend(buf, count, datatype, rank, tag, comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS) MPIU_CH4_REQUEST(*request, is_local) = r;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_ISSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Startall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Startall(int count, MPID_Request * requests[])
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_STARTALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_STARTALL);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_startall(count, requests);
#else
    int i;
    for(i=0; i<count; i++)
    {
        /* This is sub-optimal, can we do better? */
        if (MPIU_CH4_REQUEST(requests[i], anysource_partner_request)) {
            mpi_errno = MPIDI_shm_startall(1, &requests[i]);
            if (mpi_errno == MPI_SUCCESS) {
                mpi_errno = MPIDI_netmod_startall(1, &MPIU_CH4_REQUEST(requests[i], anysource_partner_request));
                MPIU_CH4_REQUEST(requests[i]->partner_request, anysource_partner_request) = MPIU_CH4_REQUEST(requests[i], anysource_partner_request)->partner_request;
                MPIU_CH4_REQUEST(MPIU_CH4_REQUEST(requests[i], anysource_partner_request)->partner_request, anysource_partner_request) = requests[i]->partner_request;
            }
        }
        else if(MPIU_CH4_REQUEST(requests[i], is_local))
            mpi_errno = MPIDI_shm_startall(1, &requests[i]);
        else
            mpi_errno = MPIDI_netmod_startall(1, &requests[i]);
    }
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_STARTALL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Send_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Send_init(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SEND_INIT);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_send_init(buf, count, datatype, rank, tag,
                                       comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_send_init(buf, count, datatype, rank, tag,
                                        comm, context_offset, request);
    else
        mpi_errno = MPIDI_netmod_send_init(buf, count, datatype, rank, tag,
                                           comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS) MPIU_CH4_REQUEST(*request, is_local) = r;
    MPIU_CH4_REQUEST(*request, anysource_partner_request) = NULL;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SEND_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Ssend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Ssend_init(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SSEND_INIT);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_ssend_init(buf, count, datatype, rank, tag,
                                        comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_ssend_init(buf, count, datatype, rank, tag,
                                         comm, context_offset, request);
    else
        mpi_errno = MPIDI_netmod_ssend_init(buf, count, datatype, rank, tag,
                                            comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS && *request) {
        MPIU_CH4_REQUEST(*request, is_local) = r;
        MPIU_CH4_REQUEST(*request, anysource_partner_request) = NULL;
    }
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SSEND_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Bsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Bsend_init(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_BSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_BSEND_INIT);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_bsend_init(buf, count, datatype, rank, tag,
                                        comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_bsend_init(buf, count, datatype, rank, tag,
                                         comm, context_offset, request);
    else
        mpi_errno = MPIDI_netmod_bsend_init(buf, count, datatype, rank, tag,
                                            comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS) MPIU_CH4_REQUEST(*request, is_local) = r;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_BSEND_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Rsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Rsend_init(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_RSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_RSEND_INIT);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_rsend_init(buf, count, datatype, rank, tag,
                                        comm, context_offset, request);
#else
    int r;
    if ((r = MPIDI_CH4_rank_is_local(rank, comm)))
        mpi_errno = MPIDI_shm_rsend_init(buf, count, datatype, rank, tag,
                                         comm, context_offset, request);
    else
        mpi_errno = MPIDI_netmod_rsend_init(buf, count, datatype, rank, tag,
                                            comm, context_offset, request);
    if(mpi_errno == MPI_SUCCESS) MPIU_CH4_REQUEST(*request, is_local) = r;
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_RSEND_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_Cancel_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Cancel_send(MPID_Request * sreq)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_CANCEL_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_CANCEL_SEND);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_cancel_send(sreq);
#else
    if(MPIU_CH4_REQUEST(sreq, is_local))
        mpi_errno = MPIDI_shm_cancel_send(sreq);
    else
        mpi_errno = MPIDI_netmod_cancel_send(sreq);
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_CANCEL_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* MPIDCH4_SEND_H_INCLUDED */
