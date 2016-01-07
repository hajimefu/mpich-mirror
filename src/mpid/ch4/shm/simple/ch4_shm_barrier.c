/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
#include <mpidimpl.h>
#include "ch4_shm_impl.h"

/* ------------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/src/ch3i_comm.c          */
/* ------------------------------------------------------- */

#define NULL_CONTEXT_ID -1

#undef FUNCNAME
#define FUNCNAME MPID_nem_barrier_vars_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_nem_barrier_vars_init(MPID_nem_barrier_vars_t * barrier_region)
{
    int mpi_errno = MPI_SUCCESS;
    int i;
    MPIDI_STATE_DECL(MPID_STATE_MPID_NEM_BARRIER_VARS_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPID_NEM_BARRIER_VARS_INIT);
    if (MPID_nem_mem_region.local_rank == 0)
        for (i = 0; i < MPID_NEM_NUM_BARRIER_VARS; ++i) {
            OPA_store_int(&barrier_region[i].context_id, NULL_CONTEXT_ID);
            OPA_store_int(&barrier_region[i].usage_cnt, 0);
            OPA_store_int(&barrier_region[i].cnt, 0);
            OPA_store_int(&barrier_region[i].sig0, 0);
            OPA_store_int(&barrier_region[i].sig, 0);
        }

    MPIDI_FUNC_EXIT(MPID_STATE_MPID_NEM_BARRIER_VARS_INIT);
    return mpi_errno;
}

