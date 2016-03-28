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
#ifndef CH4_COMM_H_INCLUDED
#define CH4_COMM_H_INCLUDED

#include "ch4_impl.h"

__CH4_INLINE__ int MPIDI_Comm_AS_enabled(MPID_Comm * comm)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_reenable_anysource(MPID_Comm * comm, MPID_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_remote_group_failed(MPID_Comm * comm, MPID_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_group_failed(MPID_Comm * comm_ptr, MPID_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Comm_failure_ack(MPID_Comm * comm_ptr)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_failure_get_acked(MPID_Comm * comm_ptr,
                                                MPID_Group ** failed_group_ptr)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_revoke(MPID_Comm * comm_ptr, int is_remote)
{
    MPIU_Assert(0);
    return 0;
}

__CH4_INLINE__ int MPIDI_Comm_get_all_failed_procs(MPID_Comm * comm_ptr, MPID_Group ** failed_group,
                                                   int tag)
{
    MPIU_Assert(0);
    return 0;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_split_type
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_split_type(MPID_Comm  *comm_ptr,
                                         int         split_type,
                                         int         key,
                                         MPID_Info  *info_ptr,
                                         MPID_Comm **newcomm_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    int            idx;
    MPID_Node_id_t node_id;

    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_SPLIT_TYPE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_SPLIT_TYPE);

    if(split_type == MPI_COMM_TYPE_SHARED) {
        MPIDI_Comm_get_lpid(comm_ptr,comm_ptr->rank,&idx,FALSE);
        MPIDI_Get_node_id(comm_ptr, comm_ptr->rank, &node_id);
        mpi_errno              = MPIR_Comm_split_impl(comm_ptr,node_id,key,newcomm_ptr);
    } else
        mpi_errno              = MPIR_Comm_split_impl(comm_ptr, MPI_UNDEFINED, key, newcomm_ptr);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_SPLIT_TYPE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_create(MPID_Comm * comm)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_CREATE);
    mpi_errno = MPIDI_CH4_NM_comm_create(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#if defined(MPIDI_BUILD_CH4_SHM)
    mpi_errno = MPIDI_CH4_SHM_comm_create(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif
#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    if (comm != MPIR_Process.comm_world && comm != MPIR_Process.comm_self) {
        int i, lpid, is_local;

        MPIDI_CH4R_COMM(comm,locality) = (MPIDI_CH4R_locality_t*)
            MPL_malloc(comm->remote_size * sizeof(MPIDI_CH4R_locality_t));

        /* For now, we'll only deal with locality for intracommunicators. For
         * intercommunicators, we'll just set all locality to remote. */
        if (comm->comm_kind == MPID_INTRACOMM) {
            for (i = 0; i < comm->remote_size; i++) {
                MPIDI_Comm_get_lpid(comm, i, &lpid, TRUE);
                is_local = MPIDI_CH4_rank_is_local(lpid, MPIR_Process.comm_world);

                MPIDI_CH4R_COMM(comm,locality)[i].is_local = is_local;
                MPIDI_CH4R_COMM(comm,locality)[i].index    = lpid;
            }
        } else {
            /* TODO - Set up locality information for intercommunicators. */
            for (i = 0; i < comm->remote_size; i++) {
                MPIDI_Comm_get_lpid(comm, i, &lpid, TRUE);

                MPIDI_CH4R_COMM(comm,locality)[i].is_local = 0;
                MPIDI_CH4R_COMM(comm,locality)[i].index    = lpid;
            }
        }
    }

#endif
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_CREATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_destroy
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_destroy(MPID_Comm * comm)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_DESTROY);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_DESTROY);
    mpi_errno = MPIDI_CH4_NM_comm_destroy(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#if defined(MPIDI_BUILD_CH4_SHM)
    mpi_errno = MPIDI_CH4_SHM_comm_destroy(comm);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
#endif
#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    MPL_free(MPIDI_CH4R_COMM(comm,locality));
#endif
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_DESTROY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#endif /* CH4_COMM_H_INCLUDED */
