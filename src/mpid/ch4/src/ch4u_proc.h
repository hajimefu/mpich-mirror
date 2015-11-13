/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef MPIDCH4U_PROC_H_INCLUDED
#define MPIDCH4U_PROC_H_INCLUDED

#include "ch4_types.h"

static inline int MPIDI_CH4U_rank_is_local(int rank, MPID_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_CH4U_STATE_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_CH4U_STATE_IS_LOCAL);

#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    if (NULL == MPIU_CH4U_COMM(comm,locality)) {
        ret = 0;
    } else {
        ret = MPIU_CH4U_COMM(comm,locality)[rank].is_local;

        MPIU_DBG_MSG_FMT(CH4, VERBOSE, (MPIU_DBG_FDEST,
                    "Rank %d %s local", rank, ret ? "is" : "is not"));
    }
#else
    ret = 0;
#endif

    MPIDI_FUNC_EXIT(MPIDI_CH4U_STATE_IS_LOCAL);
    return ret;
}


static inline int MPIDI_CH4U_rank_to_lpid(int rank, MPID_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_CH4U_STATE_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_CH4U_STATE_IS_LOCAL);

#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    ret = MPIU_CH4U_COMM(comm,locality)[rank].index;
#else
    ret = -1;
#endif

    MPIDI_FUNC_EXIT(MPIDI_CH4U_STATE_IS_LOCAL);
    return ret;
}

typedef struct node_map_t{
    uint32_t node_id;
    int      rank;
}node_map_t;

static inline int MPIDI_CH4U_cmpfunc (const void * a, const void * b)
{
    node_map_t *n_a, *n_b;
    n_a = (node_map_t*)a;
    n_b = (node_map_t*)b;
    return ( n_a->node_id - n_b->node_id);
}

static inline int MPIDI_CH4U_build_nodemap(int             myrank,
                                           MPID_Comm      *comm,
                                           int             sz,
                                           MPID_Node_id_t *out_nodemap,
                                           MPID_Node_id_t *sz_out)
{
    int             i,mpi_errno,idx,*nodeids;
    node_map_t     *node_map;
    MPIR_Errflag_t  errflag = MPIR_ERR_NONE;

    MPIU_CHKLMEM_DECL(2);

    MPIU_CHKLMEM_MALLOC(nodeids,int *,sz*sizeof(int),
                        mpi_errno,"initial node list");
    MPIU_CHKLMEM_MALLOC(node_map,node_map_t*,sz*sizeof(node_map_t),
                        mpi_errno,"node map");

    nodeids[myrank] = gethostid();

    mpi_errno = MPIR_Allgather_impl(MPI_IN_PLACE,
				    0,
				    MPI_DATATYPE_NULL,
				    nodeids,
				    sizeof(*nodeids),
				    MPI_BYTE,
				    comm,
				    &errflag);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    *sz_out = -1;
    for(i=0;i<sz;i++) {
        node_map[i].node_id=nodeids[i];
        node_map[i].rank=i;
    }
    qsort(node_map, sz, sizeof(node_map_t), MPIDI_CH4U_cmpfunc);

    idx=0;
    out_nodemap[0]=0;
    for(i=1;i<sz;i++) {
        if(node_map[i-1].node_id != node_map[i].node_id)
            idx++;
        out_nodemap[node_map[i].rank]=idx;
    }
    *sz_out = idx+1;
fn_exit:
    MPIU_CHKLMEM_FREEALL();
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4U_get_node_id(MPID_Comm *comm, int rank, MPID_Node_id_t *id_p)
{
    int mpi_errno = MPI_SUCCESS;
#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    *id_p = MPIDI_CH4_Global.node_map[MPIU_CH4U_COMM(comm,locality)[rank].index];
#else
    /* If the locality info isn't being built, this function shouldn't be getting called. */
    MPIR_ERR_SET(mpi_errno, MPI_ERR_OTHER, "**ch4|invalid_locality");
#endif

    return mpi_errno;
}

static inline int MPIDI_CH4U_get_max_node_id(MPID_Comm *comm, MPID_Node_id_t *max_id_p)
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

#endif /*MPIDCH4U_PROC_H_INCLUDED */
