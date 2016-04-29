/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS



#include <mpidimpl.h>


#undef FUNCNAME
#define FUNCNAME MPIDI_UCX_VCRT_Create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_UCX_VEPT_Create(int size, struct MPIDI_VEPT **vept_ptr)
{
    struct MPIDI_VEPT *vept;
    int i, mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_VEPT_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_VEPT_CREATE);

    vept = (struct MPIDI_VEPT *) MPL_malloc(sizeof(struct MPIDI_VEPT) + size * sizeof(MPIDI_VEP));

    if (vept != NULL) {
        MPIU_Object_set_ref(vept, 1);
        vept->size = size;
        *vept_ptr = vept;

        for (i = 0; i < size; i++)
            vept->vep_table[i].addr_idx = i;
        for (i = 0; i < size; i++)
            vept->vep_table[i].is_local = 0;

        mpi_errno = MPI_SUCCESS;
    }
    else
        mpi_errno = MPIR_ERR_MEMALLOCFAILED;

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_VEPT_CREATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_UCX_VEPT_Release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_UCX_VEPT_Release(struct MPIDI_VEPT *vept)
{
    int count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_VEPT_RELEASE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_VEPT_RELEASE);
    MPIU_Object_release_ref(vept, &count);

    if (count == 0)
        MPL_free(vept);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_VEPT_RELEASE);
    return MPI_SUCCESS;
}
