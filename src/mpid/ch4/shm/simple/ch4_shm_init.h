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
#ifndef SHM_INIT_H_INCLUDED
#define SHM_INIT_H_INCLUDED

#include "ch4_shm_impl.h"
#include "ch4_types.h"

/* ------------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/src/mpid_nem_init.c */
/* ------------------------------------------------------- */
extern MPID_nem_mem_region_t MPID_nem_mem_region;
extern char *MPID_nem_asymm_base_addr;

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_init)
static inline int MPIDI_CH4_SHM_init(int rank, int size)
{
    int mpi_errno = MPI_SUCCESS;
    int num_local = 0;
    int local_rank = -1;
    int *local_procs = NULL;
    int *local_ranks = NULL;
    int i;
    int grank;
    MPID_nem_fastbox_t *fastboxes_p = NULL;
    MPID_nem_cell_t(*cells_p)[MPID_NEM_NUM_CELLS];
    MPID_nem_queue_t *recv_queues_p = NULL;
    MPID_nem_queue_t *free_queues_p = NULL;
    MPIU_CHKPMEM_DECL(9);
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_INIT);

    MPID_nem_mem_region.num_seg = 1;
    MPIU_CHKPMEM_MALLOC(MPID_nem_mem_region.seg, MPID_nem_seg_info_ptr_t,
                        MPID_nem_mem_region.num_seg * sizeof(MPID_nem_seg_info_t), mpi_errno,
                        "mem_region segments");
    MPIU_CHKPMEM_MALLOC(local_procs, int *, size * sizeof(int), mpi_errno, "local process index array");
    MPIU_CHKPMEM_MALLOC(local_ranks, int *, size * sizeof(int), mpi_errno, "mem_region local ranks");
    for( i = 0; i < size; i++ )
    {
        if( MPIDI_CH4_rank_is_local(i, MPIR_Process.comm_world) )
        {
            if( i == rank ) {
                local_rank = num_local;
            }
            local_procs[num_local] = i;
            local_ranks[i] = num_local;
            num_local++;
        }
    }
    MPID_nem_mem_region.rank = rank;
    MPID_nem_mem_region.num_local = num_local;
    MPID_nem_mem_region.num_procs = size;
    MPID_nem_mem_region.local_procs    = local_procs;
    MPID_nem_mem_region.local_ranks    = local_ranks;
    MPID_nem_mem_region.local_rank = local_rank;
    MPID_nem_mem_region.next = NULL;

    /* Request fastboxes region */
    mpi_errno =
        MPIDU_Seg_alloc(MAX
                             ((num_local * ((num_local - 1) * sizeof(MPID_nem_fastbox_t))),
                              MPID_NEM_ASYMM_NULL_VAL), (void **) &fastboxes_p);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Request data cells region */
    mpi_errno =
        MPIDU_Seg_alloc(num_local * MPID_NEM_NUM_CELLS * sizeof(MPID_nem_cell_t),
                             (void **) &cells_p);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Request free q region */
    mpi_errno = MPIDU_Seg_alloc(num_local * sizeof(MPID_nem_queue_t), (void **) &free_queues_p);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Request recv q region */
    mpi_errno = MPIDU_Seg_alloc(num_local * sizeof(MPID_nem_queue_t), (void **) &recv_queues_p);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Request shared collectives barrier vars region */
    mpi_errno = MPIDU_Seg_alloc(MPID_NEM_NUM_BARRIER_VARS * sizeof(MPID_nem_barrier_vars_t),
                                     (void **) &MPID_nem_mem_region.barrier_vars);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Actually allocate the segment and assign regions to the pointers */
    mpi_errno = MPIDU_Seg_commit(&MPID_nem_mem_region.memory, num_local, local_rank);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* init shared collectives barrier region */
    mpi_errno = MPID_nem_barrier_vars_init(MPID_nem_mem_region.barrier_vars);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* local procs barrier */
    mpi_errno = MPID_nem_barrier();
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* find our cell region */
    MPID_nem_mem_region.Elements = cells_p[local_rank];

    /* Tables of pointers to shared memory Qs */
    MPIU_CHKPMEM_MALLOC(MPID_nem_mem_region.FreeQ, MPID_nem_queue_ptr_t *,
                        size * sizeof(MPID_nem_queue_ptr_t), mpi_errno, "FreeQ");
    MPIU_CHKPMEM_MALLOC(MPID_nem_mem_region.RecvQ, MPID_nem_queue_ptr_t *,
                        size * sizeof(MPID_nem_queue_ptr_t), mpi_errno, "RecvQ");

    /* Init table entry for our Qs */
    MPID_nem_mem_region.FreeQ[rank] = &free_queues_p[local_rank];
    MPID_nem_mem_region.RecvQ[rank] = &recv_queues_p[local_rank];

    /* Init our queues */
    MPID_nem_queue_init(MPID_nem_mem_region.RecvQ[rank]);
    MPID_nem_queue_init(MPID_nem_mem_region.FreeQ[rank]);

    /* Init and enqueue our free cells */
    for (i = 0; i < MPID_NEM_NUM_CELLS; ++i) {
        MPID_nem_cell_init(&(MPID_nem_mem_region.Elements[i]),rank);
        MPID_nem_queue_enqueue(MPID_nem_mem_region.FreeQ[rank],
                               &(MPID_nem_mem_region.Elements[i]));
    }

    /* set route for local procs through shmem */
    for (i = 0; i < num_local; i++) {
        grank = local_procs[i];
        MPID_nem_mem_region.FreeQ[grank] = &free_queues_p[i];
        MPID_nem_mem_region.RecvQ[grank] = &recv_queues_p[i];

        MPIU_Assert(MPID_NEM_ALIGNED(MPID_nem_mem_region.FreeQ[grank], MPID_NEM_CACHE_LINE_LEN));
        MPIU_Assert(MPID_NEM_ALIGNED(MPID_nem_mem_region.RecvQ[grank], MPID_NEM_CACHE_LINE_LEN));
    }

    /* make pointers to our queues global so we don't have to dereference the array */
    MPID_nem_mem_region.my_freeQ = MPID_nem_mem_region.FreeQ[rank];
    MPID_nem_mem_region.my_recvQ = MPID_nem_mem_region.RecvQ[rank];

    /* local barrier */
    mpi_errno = MPID_nem_barrier();
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Allocate table of pointers to fastboxes */
    MPIU_CHKPMEM_MALLOC(MPID_nem_mem_region.mailboxes.in, MPID_nem_fastbox_t **,
                        num_local * sizeof(MPID_nem_fastbox_t *), mpi_errno, "fastboxes");
    MPIU_CHKPMEM_MALLOC(MPID_nem_mem_region.mailboxes.out, MPID_nem_fastbox_t **,
                        num_local * sizeof(MPID_nem_fastbox_t *), mpi_errno, "fastboxes");

    MPIU_Assert(num_local > 0);

#define MAILBOX_INDEX(sender, receiver) (((sender) > (receiver)) ? ((num_local-1) * (sender) + (receiver)) :		\
                                          (((sender) < (receiver)) ? ((num_local-1) * (sender) + ((receiver)-1)) : 0))

    /* fill in tables */
    for (i = 0; i < num_local; ++i) {
        if (i == local_rank) {
            /* No fastboxs to myself */
            MPID_nem_mem_region.mailboxes.in[i] = NULL;
            MPID_nem_mem_region.mailboxes.out[i] = NULL;
        }
        else {
            MPID_nem_mem_region.mailboxes.in[i] = &fastboxes_p[MAILBOX_INDEX(i, local_rank)];
            MPID_nem_mem_region.mailboxes.out[i] = &fastboxes_p[MAILBOX_INDEX(local_rank, i)];
            OPA_store_int(&MPID_nem_mem_region.mailboxes.in[i]->common.flag.value, 0);
            OPA_store_int(&MPID_nem_mem_region.mailboxes.out[i]->common.flag.value, 0);
        }
    }
#undef MAILBOX_INDEX

    MPIU_CHKPMEM_COMMIT();
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_INIT);
    return mpi_errno;
  fn_fail:
    /* --BEGIN ERROR HANDLING-- */
    MPIU_CHKPMEM_REAP();
    goto fn_exit;
    /* --END ERROR HANDLING-- */
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_finalize)
static inline int MPIDI_CH4_SHM_finalize(void)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_FINALIZE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_FINALIZE);

    /* local barrier */
    mpi_errno = MPID_nem_barrier();
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* from MPID_nem_init */
    MPL_free(MPID_nem_mem_region.FreeQ);
    MPL_free(MPID_nem_mem_region.RecvQ);
    MPL_free(MPID_nem_mem_region.local_ranks);
    MPL_free(MPID_nem_mem_region.seg);
    MPL_free(MPID_nem_mem_region.mailboxes.out);
    MPL_free(MPID_nem_mem_region.mailboxes.in);
    MPL_free(MPID_nem_mem_region.local_procs);

    MPIDU_Seg_destroy();
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_FINALIZE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline void *MPIDI_CH4_SHM_alloc_mem(size_t size, MPID_Info * info_ptr)
{
    MPIU_Assert(0);
    return NULL;
}

static inline int MPIDI_CH4_SHM_free_mem(void *ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_comm_get_lpid(MPID_Comm * comm_ptr,
                                          int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_gpid_get(MPID_Comm * comm_ptr, int rank, MPID_Gpid * gpid)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    *id_p = (MPID_Node_id_t) 0;
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p)
{
    *max_id_p = (MPID_Node_id_t) 1;
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_getallincomm(MPID_Comm * comm_ptr,
                                         int local_size, MPID_Gpid local_gpids[], int *singlePG)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_gpid_tolpidarray(int size, MPID_Gpid gpid[], int lpid[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_create_intercomm_from_lpids(MPID_Comm * newcomm_ptr,
                                                        int size, const int lpids[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_INIT_H_INCLUDED */
