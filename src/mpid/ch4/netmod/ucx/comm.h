/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef COMM_H_INCLUDED
#define COMM_H_INCLUDED

#include "impl.h"
 static inline int dup_vep(MPIDI_VEP orig_vep, MPIDI_VEP * new_vep)
{
    *new_vep = orig_vep;
    return MPI_SUCCESS;
}


#undef FUNCNAME
#define FUNCNAME dup_vept
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void dup_vept(struct MPIDI_VEPT *src_vept,
                            struct MPIDI_VEPT **dest_vept,
                            MPIR_Comm_map_t * mapper,
                            int src_comm_size, int vept_size, int vept_offset)
{
    int flag, i;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DUP_VCRT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DUP_VCRT);

    /* try to find the simple case where the new comm is a simple
     * duplicate of the previous comm.  in that case, we simply add a
     * reference to the previous VCRT instead of recreating it. */
    if (mapper->type == MPIR_COMM_MAP_DUP && src_comm_size == vept_size) {
        *dest_vept = src_vept;
        MPIU_Object_add_ref(src_vept);
        goto fn_exit;
    }
    else if (mapper->type == MPIR_COMM_MAP_IRREGULAR && mapper->src_mapping_size == vept_size) {
        /* if the mapping array is exactly the same as the original
         * comm's VC list, there is no need to create a new VCRT.
         * instead simply point to the original comm's VCRT and bump
         * up it's reference count */
        flag = 1;

        for (i = 0; i < mapper->src_mapping_size; i++)
            if (mapper->src_mapping[i] != i)
                flag = 0;

        if (flag) {
            *dest_vept = src_vept;
            MPIU_Object_add_ref(src_vept);
            goto fn_exit;
        }
    }

    /* we are in the more complex case where we need to allocate a new
     * VCRT */

    if (!vept_offset)
        MPIDI_CH4_NMI_UCX_VEPT_Create(vept_size, dest_vept);

    if (mapper->type == MPIR_COMM_MAP_DUP) {
        for (i = 0; i < src_comm_size; i++)
            dup_vep(src_vept->vep_table[i], &((*dest_vept)->vep_table[i + vept_offset]));
    }
    else {
        for (i = 0; i < mapper->src_mapping_size; i++)
            dup_vep(src_vept->vep_table[mapper->src_mapping[i]],
                    &((*dest_vept)->vep_table[i + vept_offset]));
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
    int vept_size, vept_offset;


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
    vept_size = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R || mapper->dir == MPIR_COMM_MAP_DIR_R2R)
            continue;

        vept_size += map_size(*mapper);
    }
    vept_offset = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        src_comm = mapper->src_comm;

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R || mapper->dir == MPIR_COMM_MAP_DIR_R2R)
            continue;

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L) {
            if (src_comm->comm_kind == MPID_INTRACOMM && comm->comm_kind == MPID_INTRACOMM) {
                dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).vept, &MPIDI_CH4_NMI_UCX_COMM(comm).vept, mapper,
                         mapper->src_comm->local_size, vept_size, vept_offset);
            }
            else if (src_comm->comm_kind == MPID_INTRACOMM && comm->comm_kind == MPID_INTERCOMM)
                dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).vept, &MPIDI_CH4_NMI_UCX_COMM(comm).local_vept, mapper,
                         mapper->src_comm->local_size, vept_size, vept_offset);
            else if (src_comm->comm_kind == MPID_INTERCOMM && comm->comm_kind == MPID_INTRACOMM) {
                dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).local_vept, &MPIDI_CH4_NMI_UCX_COMM(comm).vept, mapper,
                         mapper->src_comm->local_size, vept_size, vept_offset);
            }
            else
                dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).local_vept, &MPIDI_CH4_NMI_UCX_COMM(comm).local_vept, mapper,
                         mapper->src_comm->local_size, vept_size, vept_offset);
        }
        else {  /* mapper->dir == MPIR_COMM_MAP_DIR_R2L */
            MPIU_Assert(src_comm->comm_kind == MPID_INTERCOMM);

            if (comm->comm_kind == MPID_INTRACOMM) {
                dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).vept, &MPIDI_CH4_NMI_UCX_COMM(comm).vept, mapper,
                         mapper->src_comm->remote_size, vept_size, vept_offset);
            }
            else
                dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).vept, &MPIDI_CH4_NMI_UCX_COMM(comm).local_vept, mapper,
                         mapper->src_comm->remote_size, vept_size, vept_offset);
        }

        vept_offset += map_size(*mapper);
    }

    /* Next, handle all the mappers that contribute to the remote part
     * of the comm (only valid for intercomms) */
    vept_size = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L || mapper->dir == MPIR_COMM_MAP_DIR_R2L)
            continue;

        vept_size += map_size(*mapper);
    }
    vept_offset = 0;
    MPL_LL_FOREACH(comm->mapper_head, mapper) {
        src_comm = mapper->src_comm;

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2L || mapper->dir == MPIR_COMM_MAP_DIR_R2L)
            continue;

        MPIU_Assert(comm->comm_kind == MPID_INTERCOMM);

        if (mapper->dir == MPIR_COMM_MAP_DIR_L2R) {
            if (src_comm->comm_kind == MPID_INTRACOMM)
                dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).vept, &MPIDI_CH4_NMI_UCX_COMM(comm).vept, mapper,
                         mapper->src_comm->local_size, vept_size, vept_offset);
            else
                dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).local_vept, &MPIDI_CH4_NMI_UCX_COMM(comm).vept, mapper,
                         mapper->src_comm->local_size, vept_size, vept_offset);
        }
        else {  /* mapper->dir == MPIR_COMM_MAP_DIR_R2R */
            MPIU_Assert(src_comm->comm_kind == MPID_INTERCOMM);
            dup_vept(MPIDI_CH4_NMI_UCX_COMM(src_comm).vept, &MPIDI_CH4_NMI_UCX_COMM(comm).vept, mapper,
                     mapper->src_comm->remote_size, vept_size, vept_offset);
        }

        vept_offset += map_size(*mapper);
    }

    if (comm->comm_kind == MPID_INTERCOMM) {
        /* setup the vept for the local_comm in the intercomm */
        if (comm->local_comm) {
            MPIDI_CH4_NMI_UCX_COMM(comm->local_comm).vept = MPIDI_CH4_NMI_UCX_COMM(comm).local_vept;
            MPIU_Object_add_ref(MPIDI_CH4_NMI_UCX_COMM(comm).local_vept);
        }
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_ALLOC_TABLES);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_create(MPID_Comm * comm)
{

    alloc_tables(comm);

    MPIDI_CH4U_init_comm(comm);

    return MPI_SUCCESS;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_destroy
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_destroy(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    int in_use;

    MPIDI_CH4U_destroy_comm(comm);
    MPIU_Object_release_ref(MPIDI_CH4_NMI_UCX_COMM(comm).vept, &in_use);
    if(in_use == 0)
        MPL_free(MPIDI_CH4_NMI_UCX_COMM(comm).vept);

    if (comm->comm_kind == MPID_INTERCOMM) {
        MPIU_Object_release_ref(MPIDI_CH4_NMI_UCX_COMM(comm).local_vept, &in_use);
       if(in_use == 0)
          MPL_free(MPIDI_CH4_NMI_UCX_COMM(comm).local_vept);


        }
fn_fail:
    return mpi_errno;
}

#endif /* COMM_H_INCLUDED */
