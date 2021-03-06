/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef NETMOD_STUBNM_INIT_H_INCLUDED
#define NETMOD_STUBNM_INIT_H_INCLUDED

#include "stubnm_impl.h"

static inline int MPIDI_NM_init(int rank,
                                    int size,
                                    int appnum,
                                    int *tag_ub,
                                    MPIR_Comm * comm_world,
                                    MPIR_Comm * comm_self,
                                    int spawned,
                                    int num_contexts,
                                    void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS;


    MPIR_Assert(0);
    return mpi_errno;
}

static inline int MPIDI_NM_finalize(void)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Assert(0);
    return mpi_errno;
}


static inline int MPIDI_NM_comm_get_lpid(MPIR_Comm * comm_ptr,
                                             int idx, int *lpid_ptr, MPL_bool is_remote)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_gpid_get(MPIR_Comm * comm_ptr, int rank, MPIR_Gpid * gpid)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_get_node_id(MPIR_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_get_max_node_id(MPIR_Comm * comm, MPID_Node_id_t * max_id_p)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_getallincomm(MPIR_Comm * comm_ptr,
                                            int local_size, MPIR_Gpid local_gpids[], int *singleAVT)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[])
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_create_intercomm_from_lpids(MPIR_Comm * newcomm_ptr,
                                                           int size, const int lpids[])
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_NM_free_mem(void *ptr)
{
    return MPIDI_CH4U_free_mem(ptr);
}

static inline void *MPIDI_NM_alloc_mem(size_t size, MPIR_Info * info_ptr)
{
    return MPIDI_CH4U_alloc_mem(size, info_ptr);
}


#endif /* NETMOD_STUBNM_INIT_H_INCLUDED */
