/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef CH4R_PROC_H_INCLUDED
#define CH4R_PROC_H_INCLUDED

#include "ch4_types.h"
#include "build_nodemap.h"

static inline int MPIDI_CH4U_rank_is_local(int rank, MPIR_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_CH4U_STATE_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_CH4U_STATE_IS_LOCAL);

#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    if (NULL == MPIDI_CH4U_COMM(comm,locality)) {
        ret = 0;
    } else {
        ret = MPIDI_CH4U_COMM(comm,locality)[rank].is_local;

        MPL_DBG_MSG_FMT(MPIDI_CH4_DBG_GENERAL, VERBOSE, (MPL_DBG_FDEST,
                    "Rank %d %s local", rank, ret ? "is" : "is not"));
    }
#else
    ret = 0;
#endif

    MPIDI_FUNC_EXIT(MPIDI_CH4U_STATE_IS_LOCAL);
    return ret;
}


static inline int MPIDI_CH4U_rank_to_lpid(int rank, MPIR_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_CH4U_STATE_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_CH4U_STATE_IS_LOCAL);

#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    ret = MPIDI_CH4U_COMM(comm,locality)[rank].index;
#else
    ret = -1;
#endif

    MPIDI_FUNC_EXIT(MPIDI_CH4U_STATE_IS_LOCAL);
    return ret;
}

static inline int MPIDI_CH4U_get_node_id(MPIR_Comm *comm, int rank, MPID_Node_id_t *id_p)
{
    int mpi_errno = MPI_SUCCESS;
#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    *id_p = MPIDI_CH4_Global.node_map[MPIDI_CH4U_COMM(comm,locality)[rank].index];
#else
    /* If the locality info isn't being built, this function shouldn't be getting called. */
    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**ch4|invalid_locality");
#endif

    return mpi_errno;
}

static inline int MPIDI_CH4U_get_max_node_id(MPIR_Comm *comm, MPID_Node_id_t *max_id_p)
{
    int mpi_errno = MPI_SUCCESS;
#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    *max_id_p = MPIDI_CH4_Global.max_node_id;
#else
    /* If the locality info isn't being built, this function shouldn't be getting called. */
    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**ch4|invalid_locality");
#endif

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_build_nodemap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_build_nodemap(int myrank,
                                           MPIR_Comm *comm,
                                           int sz,
                                           MPID_Node_id_t *out_nodemap,
                                           MPID_Node_id_t *sz_out)
{
    return MPIR_NODEMAP_build_nodemap(sz, myrank, out_nodemap, sz_out);
}

#endif /* CH4R_PROC_H_INCLUDED */
