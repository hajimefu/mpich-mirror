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
#ifndef MPIDCH4U_RMA_H_INCLUDED
#define MPIDCH4U_RMA_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_put(const void *origin_addr,
                                  int origin_count,
                                  MPI_Datatype origin_datatype,
                                  int target_rank,
                                  MPI_Aint target_disp,
                                  int target_count, MPI_Datatype target_datatype, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_PUT);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_PUT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_rput
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_rput(const void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count,
                                   MPI_Datatype target_datatype,
                                   MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RPUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RPUT);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RPUT);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_get
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_get(void *origin_addr,
                                  int origin_count,
                                  MPI_Datatype origin_datatype,
                                  int target_rank,
                                  MPI_Aint target_disp,
                                  int target_count, MPI_Datatype target_datatype, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_GET);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_accumulate(const void *origin_addr,
                                         int origin_count,
                                         MPI_Datatype origin_datatype,
                                         int target_rank,
                                         MPI_Aint target_disp,
                                         int target_count,
                                         MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win)
{
    int mpi_errno=MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACCUMULATE);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACCUMULATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_compare_and_swap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_compare_and_swap(const void *origin_addr,
                                               const void *compare_addr,
                                               void *result_addr,
                                               MPI_Datatype datatype,
                                               int target_rank,
                                               MPI_Aint target_disp, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_COMPARE_AND_SWAP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_COMPARE_AND_SWAP);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_COMPARE_AND_SWAP);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_raccumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_raccumulate(const void *origin_addr,
                                          int origin_count,
                                          MPI_Datatype origin_datatype,
                                          int target_rank,
                                          MPI_Aint target_disp,
                                          int target_count,
                                          MPI_Datatype target_datatype,
                                          MPI_Op op, MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RACCUMULATE);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RACCUMULATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_rget_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_rget_accumulate(const void *origin_addr,
                                              int origin_count,
                                              MPI_Datatype origin_datatype,
                                              void *result_addr,
                                              int result_count,
                                              MPI_Datatype result_datatype,
                                              int target_rank,
                                              MPI_Aint target_disp,
                                              int target_count,
                                              MPI_Datatype target_datatype,
                                              MPI_Op op, MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RGET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RGET_ACCUMULATE);

    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RGET_ACCUMULATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_fetch_and_op
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_fetch_and_op(const void *origin_addr,
                                           void *result_addr,
                                           MPI_Datatype datatype,
                                           int target_rank,
                                           MPI_Aint target_disp, MPI_Op op, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FETCH_AND_OP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FETCH_AND_OP);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FETCH_AND_OP);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_rget
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_rget(void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count,
                                   MPI_Datatype target_datatype,
                                   MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RGET);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RGET);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RGET);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_get_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_get_accumulate(const void *origin_addr,
                                             int origin_count,
                                             MPI_Datatype origin_datatype,
                                             void *result_addr,
                                             int result_count,
                                             MPI_Datatype result_datatype,
                                             int target_rank,
                                             MPI_Aint target_disp,
                                             int target_count,
                                             MPI_Datatype target_datatype,
                                             MPI_Op op, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_GET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_GET_ACCUMULATE);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET_ACCUMULATE);
    return mpi_errno;
}

#endif /* MPIDCH4U_RMA_H_INCLUDED */
