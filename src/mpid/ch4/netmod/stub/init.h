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
#ifndef NETMOD_STUB_INIT_H_INCLUDED
#define NETMOD_STUB_INIT_H_INCLUDED

#include "impl.h"

static inline int MPIDI_CH4_NM_init(int rank,
                                    int size,
                                    int appnum,
                                    int *tag_ub,
                                    MPID_Comm * comm_world,
                                    MPID_Comm * comm_self,
                                    int spawned,
                                    int num_contexts,
                                    void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS;


    MPIU_Assert(0);
    return mpi_errno;
}

static inline int MPIDI_CH4_NM_finalize(void)
{
    int mpi_errno = MPI_SUCCESS;
    MPIU_Assert(0);
    return mpi_errno;
}


static inline int MPIDI_CH4_NM_comm_get_lpid(MPID_Comm * comm_ptr,
                                             int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_gpid_get(MPID_Comm * comm_ptr, int rank, MPIR_Gpid * gpid)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_getallincomm(MPID_Comm * comm_ptr,
                                            int local_size, MPIR_Gpid local_gpids[], int *singlePG)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_create_intercomm_from_lpids(MPID_Comm * newcomm_ptr,
                                                           int size, const int lpids[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_free_mem(void *ptr)
{
    return MPIDI_CH4U_free_mem(ptr);
}

static inline void *MPIDI_CH4_NM_alloc_mem(size_t size, MPID_Info * info_ptr)
{
    return MPIDI_CH4U_alloc_mem(size, info_ptr);
}


#endif /* NETMOD_STUB_INIT_H_INCLUDED */
