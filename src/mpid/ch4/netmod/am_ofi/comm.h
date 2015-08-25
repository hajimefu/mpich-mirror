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
#ifndef NETMOD_AM_OFI_COMM_H_INCLUDED
#define NETMOD_AM_OFI_COMM_H_INCLUDED

#include "impl.h"
#include "mpl_utlist.h"

static inline int dup_vcr(MPIDI_VCR orig_vcr, MPIDI_VCR * new_vcr)
{
    *new_vcr = orig_vcr;
    return MPI_SUCCESS;
}

static inline int addref_vcrt(struct MPIDI_VCRT *vcrt)
{
    MPIU_Object_add_ref(vcrt);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME dup_vcrt
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void dup_vcrt(struct MPIDI_VCRT *src_vcrt,
                            struct MPIDI_VCRT **dest_vcrt,
                            MPIR_Comm_map_t * mapper,
                            int src_comm_size, int vcrt_size, int vcrt_offset)
{
    int flag, i;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DUP_VCRT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DUP_VCRT);

    /* try to find the simple case where the new comm is a simple
     * duplicate of the previous comm.  in that case, we simply add a
     * reference to the previous VCRT instead of recreating it. */
    if (mapper->type == MPIR_COMM_MAP_DUP && src_comm_size == vcrt_size) {
        *dest_vcrt = src_vcrt;
        addref_vcrt(src_vcrt);
        goto fn_exit;
    }
    else if (mapper->type == MPIR_COMM_MAP_IRREGULAR && mapper->src_mapping_size == vcrt_size) {
        /* if the mapping array is exactly the same as the original
         * comm's VC list, there is no need to create a new VCRT.
         * instead simply point to the original comm's VCRT and bump
         * up it's reference count */
        flag = 1;

        for (i = 0; i < mapper->src_mapping_size; i++)
            if (mapper->src_mapping[i] != i)
                flag = 0;

        if (flag) {
            *dest_vcrt = src_vcrt;
            addref_vcrt(src_vcrt);
            goto fn_exit;
        }
    }

    /* we are in the more complex case where we need to allocate a new
     * VCRT */

    if (!vcrt_offset)
        MPIDI_VCRT_Create(vcrt_size, dest_vcrt);

    if (mapper->type == MPIR_COMM_MAP_DUP) {
        for (i = 0; i < src_comm_size; i++)
            dup_vcr(src_vcrt->vcr_table[i], &((*dest_vcrt)->vcr_table[i + vcrt_offset]));
    }
    else {
        for (i = 0; i < mapper->src_mapping_size; i++)
            dup_vcr(src_vcrt->vcr_table[mapper->src_mapping[i]],
                    &((*dest_vcrt)->vcr_table[i + vcrt_offset]));
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DUP_VCRT);
    return;
}

static inline int map_size(MPIR_Comm_map_t map)
{
    if (map.type == MPIR_COMM_MAP_IRREGULAR)
        return map.src_mapping_size;
    else if (map.dir == MPIR_COMM_MAP_DIR_L2L || map.dir == MPIR_COMM_MAP_DIR_L2R)
        return map.src_comm->local_size;
    else
        return map.src_comm->remote_size;
}

#undef FUNCNAME
#define FUNCNAME alloc_tables
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int alloc_tables(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Comm_map_t *mapper;
    MPID_Comm *src_comm;
    int vcrt_size, vcrt_offset;


    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_ALLOC_TABLES);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_ALLOC_TABLES);

    /* do some sanity checks */
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        if (mapper->src_comm->comm_kind == MPID_INTRACOMM)
            MPIU_Assert(mapper->dir == MPIR_COMM_MAP_DIR_L2L ||
                        mapper->dir == MPIR_COMM_MAP_DIR_L2R);

        if (comm->comm_kind == MPID_INTRACOMM)
            MPIU_Assert(mapper->dir == MPIR_COMM_MAP_DIR_L2L ||
                        mapper->dir == MPIR_COMM_MAP_DIR_R2L);
    }

    /* First, handle all the mappers that contribute to the local part
     * of the comm */
    vcrt_size = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R || mapper->dir == MPIR_COMM_MAP_DIR_R2R)
            continue;

        vcrt_size += map_size(*mapper);
    }
    vcrt_offset = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        src_comm = mapper->src_comm;

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R || mapper->dir == MPIR_COMM_MAP_DIR_R2R)
            continue;

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L) {
            if (src_comm->comm_kind == MPID_INTRACOMM && comm->comm_kind == MPID_INTRACOMM) {
                dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->vcrt, mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            }
            else if (src_comm->comm_kind == MPID_INTRACOMM && comm->comm_kind == MPID_INTERCOMM)
                dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->local_vcrt, mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            else if (src_comm->comm_kind == MPID_INTERCOMM && comm->comm_kind == MPID_INTRACOMM) {
                dup_vcrt(COMM_OFI(src_comm)->local_vcrt, &COMM_OFI(comm)->vcrt, mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            }
            else
                dup_vcrt(COMM_OFI(src_comm)->local_vcrt, &COMM_OFI(comm)->local_vcrt, mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
        }
        else {  /* mapper->dir == MPIR_COMM_MAP_DIR_R2L */
            MPIU_Assert(src_comm->comm_kind == MPID_INTERCOMM);

            if (comm->comm_kind == MPID_INTRACOMM) {
                dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->vcrt, mapper,
                         mapper->src_comm->remote_size, vcrt_size, vcrt_offset);
            }
            else
                dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->local_vcrt, mapper,
                         mapper->src_comm->remote_size, vcrt_size, vcrt_offset);
        }

        vcrt_offset += map_size(*mapper);
    }

    /* Next, handle all the mappers that contribute to the remote part
     * of the comm (only valid for intercomms) */
    vcrt_size = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L || mapper->dir == MPIR_COMM_MAP_DIR_R2L)
            continue;

        vcrt_size += map_size(*mapper);
    }
    vcrt_offset = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        src_comm = mapper->src_comm;

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L || mapper->dir == MPIR_COMM_MAP_DIR_R2L)
            continue;

        MPIU_Assert(comm->comm_kind == MPID_INTERCOMM);

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R) {
            if (src_comm->comm_kind == MPID_INTRACOMM)
                dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->vcrt, mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            else
                dup_vcrt(COMM_OFI(src_comm)->local_vcrt, &COMM_OFI(comm)->vcrt, mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
        }
        else {  /* mapper->dir == MPIR_COMM_MAP_DIR_R2R */
            MPIU_Assert(src_comm->comm_kind == MPID_INTERCOMM);
            dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->vcrt, mapper,
                     mapper->src_comm->remote_size, vcrt_size, vcrt_offset);
        }

        vcrt_offset += map_size(*mapper);
    }

    if (comm->comm_kind == MPID_INTERCOMM) {
        /* setup the vcrt for the local_comm in the intercomm */
        if (comm->local_comm) {
            COMM_OFI(comm->local_comm)->vcrt = COMM_OFI(comm)->local_vcrt;
            addref_vcrt(COMM_OFI(comm)->local_vcrt);
        }
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_ALLOC_TABLES);
    return mpi_errno;
}

static inline int MPIDI_netmod_comm_create(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_COMM_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_COMM_CREATE);
    uint64_t mapid;

    alloc_tables(comm);

    mapid = (((uint64_t) COMM_TO_EP(comm, comm->rank)) << 32) | comm->context_id;
    MPIDI_Map_set(MPIDI_Global.comm_map, mapid, comm);
    COMM_OFI(comm)->window_instance = 0;
    MPIDI_CH4U_Init_comm(comm);

    /* Do not handle intercomms */
    if (comm->comm_kind == MPID_INTERCOMM)
        goto fn_exit;

    MPIU_Assert(comm->coll_fns != NULL);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_COMM_CREATE);
    return mpi_errno;
}

static inline int MPIDI_netmod_comm_destroy(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    uint64_t mapid;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_COMM_DESTROY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_COMM_DESTROY);

    MPIDI_CH4U_Destroy_comm(comm);

    mapid = (((uint64_t) COMM_TO_EP(comm, comm->rank)) << 32) | comm->context_id;
    MPIDI_Map_erase(MPIDI_Global.comm_map, mapid);
    mpi_errno = MPIDI_VCRT_Release(COMM_OFI(comm)->vcrt);

    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    if (comm->comm_kind == MPID_INTERCOMM) {
        mpi_errno = MPIDI_VCRT_Release(COMM_OFI(comm)->local_vcrt);

        if (mpi_errno)
            MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_COMM_DESTROY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#endif /* NETMOD_AM_OFI_COMM_H_INCLUDED */
