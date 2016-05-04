/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#include <mpidimpl.h>
#include "impl.h"

/* ------------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/src/ch3i_comm.c          */
/* ------------------------------------------------------- */

#undef FUNCNAME
#define FUNCNAME MPIDI_SIMPLE_barrier_vars_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_SIMPLE_barrier_vars_init(MPIDI_SIMPLE_barrier_vars_t *barrier_region)
{
    int mpi_errno = MPI_SUCCESS;
    int i;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SIMPLE_BARRIER_VARS_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SIMPLE_BARRIER_VARS_INIT);

    if(MPIDI_SIMPLE_mem_region.local_rank == 0)
        for(i = 0; i < MPIDI_SIMPLE_NUM_BARRIER_VARS; ++i) {
            OPA_store_int(&barrier_region[i].context_id, -1);
            OPA_store_int(&barrier_region[i].usage_cnt, 0);
            OPA_store_int(&barrier_region[i].cnt, 0);
            OPA_store_int(&barrier_region[i].sig0, 0);
            OPA_store_int(&barrier_region[i].sig, 0);
        }

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SIMPLE_BARRIER_VARS_INIT);
    return mpi_errno;
}


