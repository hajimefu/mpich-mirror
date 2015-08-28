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
#ifndef NETMOD_STUB_RMA_H_INCLUDED
#define NETMOD_STUB_RMA_H_INCLUDED

#include "impl.h"

static inline int MPIDI_netmod_put(const void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count, MPI_Datatype target_datatype, MPID_Win * win)
{
    return MPIDI_CH4U_Put(origin_addr, origin_count, origin_datatype,
                          target_rank, target_disp, target_count, target_datatype, win);
}

static inline int MPIDI_netmod_get(void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count, MPI_Datatype target_datatype, MPID_Win * win)
{
    return MPIDI_CH4U_Get(origin_addr, origin_count, origin_datatype,
                          target_rank, target_disp, target_count, target_datatype, win);
}

static inline int MPIDI_netmod_rput(const void *origin_addr,
                                    int origin_count,
                                    MPI_Datatype origin_datatype,
                                    int target_rank,
                                    MPI_Aint target_disp,
                                    int target_count,
                                    MPI_Datatype target_datatype,
                                    MPID_Win * win, MPID_Request ** request)
{
    return MPIDI_CH4U_Rput(origin_addr, origin_count, origin_datatype,
                           target_rank, target_disp, target_count, target_datatype, win, request);
}


static inline int MPIDI_netmod_compare_and_swap(const void *origin_addr,
                                                const void *compare_addr,
                                                void *result_addr,
                                                MPI_Datatype datatype,
                                                int target_rank,
                                                MPI_Aint target_disp, MPID_Win * win)
{
    return MPIDI_CH4U_Compare_and_swap(origin_addr, compare_addr, result_addr,
                                       datatype, target_rank, target_disp, win);
}

static inline int MPIDI_netmod_raccumulate(const void *origin_addr,
                                           int origin_count,
                                           MPI_Datatype origin_datatype,
                                           int target_rank,
                                           MPI_Aint target_disp,
                                           int target_count,
                                           MPI_Datatype target_datatype,
                                           MPI_Op op, MPID_Win * win, MPID_Request ** request)
{
    return MPIDI_CH4U_Raccumulate(origin_addr, origin_count, origin_datatype,
                                  target_rank, target_disp, target_count,
                                  target_datatype, op, win, request);
}

static inline int MPIDI_netmod_rget_accumulate(const void *origin_addr,
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
    return MPIDI_CH4U_Rget_accumulate(origin_addr, origin_count, origin_datatype,
                                      result_addr, result_count, result_datatype,
                                      target_rank, target_disp, target_count,
                                      target_datatype, op, win, request);
}

static inline int MPIDI_netmod_fetch_and_op(const void *origin_addr,
                                            void *result_addr,
                                            MPI_Datatype datatype,
                                            int target_rank,
                                            MPI_Aint target_disp, MPI_Op op, MPID_Win * win)
{
    return MPIDI_CH4U_Fetch_and_op(origin_addr, result_addr, datatype,
                                   target_rank, target_disp, op, win);
}


static inline int MPIDI_netmod_rget(void *origin_addr,
                                    int origin_count,
                                    MPI_Datatype origin_datatype,
                                    int target_rank,
                                    MPI_Aint target_disp,
                                    int target_count,
                                    MPI_Datatype target_datatype,
                                    MPID_Win * win, MPID_Request ** request)
{
    return MPIDI_CH4U_Rget(origin_addr, origin_count, origin_datatype,
                           target_rank, target_disp, target_count, target_datatype, win, request);
}


static inline int MPIDI_netmod_get_accumulate(const void *origin_addr,
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
    return MPIDI_CH4U_Get_accumulate(origin_addr, origin_count, origin_datatype,
                                     result_addr, result_count, result_datatype,
                                     target_rank, target_disp, target_count,
                                     target_datatype, op, win);
}

static inline int MPIDI_netmod_accumulate(const void *origin_addr,
                                          int origin_count,
                                          MPI_Datatype origin_datatype,
                                          int target_rank,
                                          MPI_Aint target_disp,
                                          int target_count,
                                          MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win)
{
    return MPIDI_CH4U_Accumulate(origin_addr, origin_count, origin_datatype,
                                 target_rank, target_disp, target_count, target_datatype, op, win);
}

#endif /* NETMOD_STUB_RMA_H_INCLUDED */
