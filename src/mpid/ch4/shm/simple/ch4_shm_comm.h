/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef SHM_COMM_H_INCLUDED
#define SHM_COMM_H_INCLUDED

#include "ch4_shm_impl.h"
#include "impl.h"
#include "mpl_utlist.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_SHM_VCRT_Create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_SHM_VCRT_Create(int size, struct MPIDI_SHM_VCRT **vcrt_ptr)
{
    struct MPIDI_SHM_VCRT *vcrt;
    int i,mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SHM_VCRT_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SHM_VCRT_CREATE);

    vcrt = (struct MPIDI_SHM_VCRT *)MPIU_Malloc(sizeof(struct MPIDI_SHM_VCRT) +
                                            size*sizeof(MPIDI_SHM_VCR));

    if(vcrt != NULL) {
        MPIU_Object_set_ref(vcrt, 1);
        vcrt->size = size;
        *vcrt_ptr = vcrt;

        for(i=0; i<size; i++)
            vcrt->vcr_table[i].pg_rank = i;

        mpi_errno = MPI_SUCCESS;
    } else
        mpi_errno = MPIR_ERR_MEMALLOCFAILED;

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SHM_VCRT_CREATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_SHM_VCRT_Release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_SHM_VCRT_Release(struct MPIDI_SHM_VCRT *vcrt)
{
    int count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SHM_VCRT_RELEASE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SHM_VCRT_RELEASE);
    MPIU_Object_release_ref(vcrt, &count);

    if(count == 0)
        MPIU_Free(vcrt);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SHM_VCRT_RELEASE);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME shm_dup_vcrt
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void shm_dup_vcrt(struct MPIDI_SHM_VCRT *src_vcrt,
                            struct MPIDI_SHM_VCRT **dest_vcrt,
                            MPIR_Comm_map_t * mapper,
                            int src_comm_size, int vcrt_size, int vcrt_offset)
{
    int flag, i;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SHM_DUP_VCRT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SHM_DUP_VCRT);

    /* try to find the simple case where the new comm is a simple
     * duplicate of the previous comm.  in that case, we simply add a
     * reference to the previous VCRT instead of recreating it. */
    if (mapper->type == MPIR_COMM_MAP_DUP && src_comm_size == vcrt_size) {
        *dest_vcrt = src_vcrt;
        MPIU_Object_add_ref(src_vcrt);
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
            MPIU_Object_add_ref(src_vcrt);
            goto fn_exit;
        }
    }

    /* we are in the more complex case where we need to allocate a new
     * VCRT */

    if (!vcrt_offset)
        MPIDI_SHM_VCRT_Create(vcrt_size, dest_vcrt);

    if (mapper->type == MPIR_COMM_MAP_DUP) {
        for (i = 0; i < src_comm_size; i++)
            (*dest_vcrt)->vcr_table[i + vcrt_offset] = src_vcrt->vcr_table[i];
    }
    else {
        for (i = 0; i < mapper->src_mapping_size; i++)
        {
            (*dest_vcrt)->vcr_table[i + vcrt_offset] = src_vcrt->vcr_table[mapper->src_mapping[i]];
        }
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SHM_DUP_VCRT);
    return;
}

static inline int shm_map_size(MPIR_Comm_map_t map)
{
    if (map.type == MPIR_COMM_MAP_IRREGULAR)
        return map.src_mapping_size;
    else if (map.dir == MPIR_COMM_MAP_DIR_L2L || map.dir == MPIR_COMM_MAP_DIR_L2R)
        return map.src_comm->local_size;
    else
        return map.src_comm->remote_size;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_shm_comm_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_shm_comm_create(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Comm_map_t *mapper;
    MPID_Comm *src_comm;
    int vcrt_size, vcrt_offset;

    MPIDI_STATE_DECL(MPID_STATE_SHM_COMM_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_SHM_COMM_CREATE);

    /* It may happen shm vcrt isn't created yet */
    if( COMM_SHM_SIMPLE(comm,vcrt) == NULL ) {
        mpi_errno = MPIDI_SHM_VCRT_Create(comm->remote_size, &COMM_SHM_SIMPLE(comm,vcrt));
        if (mpi_errno)
            MPIR_ERR_POP(mpi_errno);
        if( comm == MPIR_Process.comm_self ) {
            COMM_SHM_SIMPLE(comm,vcrt)->vcr_table[0].pg_rank = comm->rank;
        }
    }
    /* First, handle all the mappers that contribute to the local part
     * of the comm */
    vcrt_size = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R || mapper->dir == MPIR_COMM_MAP_DIR_R2R)
            continue;

        vcrt_size += shm_map_size(*mapper);
    }
    vcrt_offset = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        src_comm = mapper->src_comm;
        /* It may happen shm vcrt isn't created yet */
        MPIU_Assert( COMM_SHM_SIMPLE(src_comm,vcrt) );

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R || mapper->dir == MPIR_COMM_MAP_DIR_R2R)
            continue;

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L) {
            if (src_comm->comm_kind == MPID_INTRACOMM && comm->comm_kind == MPID_INTRACOMM) {
                shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,vcrt), &COMM_SHM_SIMPLE(comm,vcrt), mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            }
            else if (src_comm->comm_kind == MPID_INTRACOMM && comm->comm_kind == MPID_INTERCOMM) {
                shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,vcrt), &COMM_SHM_SIMPLE(comm,local_vcrt), mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            }
            else if (src_comm->comm_kind == MPID_INTERCOMM && comm->comm_kind == MPID_INTRACOMM) {
                shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,local_vcrt), &COMM_SHM_SIMPLE(comm,vcrt), mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            }
            else {
                shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,local_vcrt), &COMM_SHM_SIMPLE(comm,local_vcrt), mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            }
        }
        else {  /* mapper->dir == MPIR_COMM_MAP_DIR_R2L */
            MPIU_Assert(src_comm->comm_kind == MPID_INTERCOMM);

            if (comm->comm_kind == MPID_INTRACOMM) {
                shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,vcrt), &COMM_SHM_SIMPLE(comm,vcrt), mapper,
                         mapper->src_comm->remote_size, vcrt_size, vcrt_offset);
            }
            else {
                shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,vcrt), &COMM_SHM_SIMPLE(comm,local_vcrt), mapper,
                         mapper->src_comm->remote_size, vcrt_size, vcrt_offset);
            }
        }

        vcrt_offset += shm_map_size(*mapper);
    }

    /* Next, handle all the mappers that contribute to the remote part
     * of the comm (only valid for intercomms) */
    vcrt_size = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L || mapper->dir == MPIR_COMM_MAP_DIR_R2L)
            continue;

        vcrt_size += shm_map_size(*mapper);
    }
    vcrt_offset = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        src_comm = mapper->src_comm;

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L || mapper->dir == MPIR_COMM_MAP_DIR_R2L)
            continue;

        MPIU_Assert(comm->comm_kind == MPID_INTERCOMM);

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R) {
            if (src_comm->comm_kind == MPID_INTRACOMM) {
                shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,vcrt), &COMM_SHM_SIMPLE(comm,vcrt), mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            }
            else {
                shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,local_vcrt), &COMM_SHM_SIMPLE(comm,vcrt), mapper,
                         mapper->src_comm->local_size, vcrt_size, vcrt_offset);
            }
        }
        else {  /* mapper->dir == MPIR_COMM_MAP_DIR_R2R */
            MPIU_Assert(src_comm->comm_kind == MPID_INTERCOMM);
            shm_dup_vcrt(COMM_SHM_SIMPLE(src_comm,vcrt), &COMM_SHM_SIMPLE(comm,vcrt), mapper,
                     mapper->src_comm->remote_size, vcrt_size, vcrt_offset);
        }

        vcrt_offset += shm_map_size(*mapper);
    }

    if (comm->comm_kind == MPID_INTERCOMM) {
        /* setup the vcrt for the local_comm in the intercomm */
        if (comm->local_comm) {
            COMM_SHM_SIMPLE(comm->local_comm,vcrt) = COMM_SHM_SIMPLE(comm,local_vcrt);
            MPIU_Object_add_ref(COMM_SHM_SIMPLE(comm,local_vcrt));
        }
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_SHM_COMM_CREATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_shm_comm_destroy
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_shm_comm_destroy(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_SHM_COMM_DESTROY);
    MPIDI_FUNC_ENTER(MPID_STATE_SHM_COMM_DESTROY);

    mpi_errno = MPIDI_SHM_VCRT_Release(COMM_SHM_SIMPLE(comm,vcrt));

    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    if (comm->comm_kind == MPID_INTERCOMM) {
        mpi_errno = MPIDI_SHM_VCRT_Release(COMM_SHM_SIMPLE(comm,local_vcrt));

        if (mpi_errno)
            MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_SHM_COMM_DESTROY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#endif /* SHM_COMM_H_INCLUDED */
