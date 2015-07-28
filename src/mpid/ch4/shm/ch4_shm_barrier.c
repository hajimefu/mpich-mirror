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
int MPID_nem_barrier_vars_init (MPID_nem_barrier_vars_t *barrier_region)
{
    int mpi_errno = MPI_SUCCESS;
    int i;
    MPIDI_STATE_DECL(MPID_STATE_MPID_NEM_BARRIER_VARS_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPID_NEM_BARRIER_VARS_INIT);
    if (MPID_nem_mem_region.local_rank == 0)
        for (i = 0; i < MPID_NEM_NUM_BARRIER_VARS; ++i)
        {
            OPA_store_int(&barrier_region[i].context_id, NULL_CONTEXT_ID);
            OPA_store_int(&barrier_region[i].usage_cnt, 0);
            OPA_store_int(&barrier_region[i].cnt, 0);
            OPA_store_int(&barrier_region[i].sig0, 0);
            OPA_store_int(&barrier_region[i].sig, 0);
        }

    MPIDI_FUNC_EXIT(MPID_STATE_MPID_NEM_BARRIER_VARS_INIT);
    return mpi_errno;
}

/* ------------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/src/mpid_nem_barrier.c   */
/* ------------------------------------------------------- */

static int sense;
static int barrier_init = 0;

#undef FUNCNAME
#define FUNCNAME MPID_nem_barrier_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPID_nem_barrier_init(MPID_nem_barrier_t *barrier_region, int init_values)
{
    MPIDI_STATE_DECL(MPID_STATE_MPID_NEM_BARRIER_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPID_NEM_BARRIER_INIT);
    
    MPID_nem_mem_region.barrier = barrier_region;
    if (init_values) {
        OPA_store_int(&MPID_nem_mem_region.barrier->val, 0);
        OPA_store_int(&MPID_nem_mem_region.barrier->wait, 0);
        OPA_write_barrier();
    }
    sense = 0;
    barrier_init = 1;

    MPIDI_FUNC_EXIT(MPID_STATE_MPID_NEM_BARRIER_INIT);

    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPID_nem_barrier
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
/* FIXME: this is not a scalable algorithm because everyone is polling on the same cacheline */
int MPID_nem_barrier(void)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPID_NEM_BARRIER);

    MPIDI_FUNC_ENTER(MPID_STATE_MPID_NEM_BARRIER);

    if (MPID_nem_mem_region.num_local == 1)
        goto fn_exit;

    MPIR_ERR_CHKINTERNAL(!barrier_init, mpi_errno, "barrier not initialized");

    if (OPA_fetch_and_incr_int(&MPID_nem_mem_region.barrier->val) == MPID_nem_mem_region.num_local - 1)
    {
	OPA_store_int(&MPID_nem_mem_region.barrier->val, 0);
	OPA_store_int(&MPID_nem_mem_region.barrier->wait, 1 - sense);
        OPA_write_barrier();
    }
    else
    {
	/* wait */
	while (OPA_load_int(&MPID_nem_mem_region.barrier->wait) == sense)
            MPIU_PW_Sched_yield(); /* skip */
    }
    sense = 1 - sense;

 fn_fail:
 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPID_NEM_BARRIER);
    return mpi_errno;
}

