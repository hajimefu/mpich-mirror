/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef SHM_SIMPLE_INIT_H_INCLUDED
#define SHM_SIMPLE_INIT_H_INCLUDED

#include "impl.h"
#include "ch4_types.h"
#include "mpidu_shm.h"

/* ------------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/src/mpid_nem_init.c */
/* ------------------------------------------------------- */
extern MPIDI_SIMPLE_mem_region_t MPIDI_SIMPLE_mem_region;
extern char *MPIDI_SIMPLE_asym_base_addr;

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
    MPIDI_SIMPLE_fastbox_t *fastboxes_p = NULL;
    MPIDI_SIMPLE_cell_t(*cells_p)[MPIDI_SIMPLE_NUM_CELLS];
    MPIDI_SIMPLE_queue_t *recv_queues_p = NULL;
    MPIDI_SIMPLE_queue_t *free_queues_p = NULL;
    MPIU_CHKPMEM_DECL(9);
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_INIT);

    MPIDI_SIMPLE_mem_region.num_seg = 1;
    MPIU_CHKPMEM_MALLOC(MPIDI_SIMPLE_mem_region.seg, MPIDU_shm_seg_info_ptr_t,
                        MPIDI_SIMPLE_mem_region.num_seg * sizeof(MPIDU_shm_seg_info_t), mpi_errno,
                        "mem_region segments");
    MPIU_CHKPMEM_MALLOC(local_procs, int *, size * sizeof(int), mpi_errno, "local process index array");
    MPIU_CHKPMEM_MALLOC(local_ranks, int *, size * sizeof(int), mpi_errno, "mem_region local ranks");

    for(i = 0; i < size; i++) {
        if(MPIDI_CH4_rank_is_local(i, MPIR_Process.comm_world)) {
            if(i == rank) {
                local_rank = num_local;
            }

            local_procs[num_local] = i;
            local_ranks[i] = num_local;
            num_local++;
        }
    }

    MPIDI_SIMPLE_mem_region.rank = rank;
    MPIDI_SIMPLE_mem_region.num_local = num_local;
    MPIDI_SIMPLE_mem_region.num_procs = size;
    MPIDI_SIMPLE_mem_region.local_procs    = local_procs;
    MPIDI_SIMPLE_mem_region.local_ranks    = local_ranks;
    MPIDI_SIMPLE_mem_region.local_rank = local_rank;
    MPIDI_SIMPLE_mem_region.next = NULL;

    /* Request fastboxes region */
    mpi_errno =
        MPIDU_shm_seg_alloc(MAX
                            ((num_local * ((num_local - 1) * sizeof(MPIDI_SIMPLE_fastbox_t))),
                             MPIDI_SIMPLE_ASYMM_NULL_VAL), (void **) &fastboxes_p);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Request data cells region */
    mpi_errno =
        MPIDU_shm_seg_alloc(num_local * MPIDI_SIMPLE_NUM_CELLS * sizeof(MPIDI_SIMPLE_cell_t),
                            (void **) &cells_p);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Request free q region */
    mpi_errno = MPIDU_shm_seg_alloc(num_local * sizeof(MPIDI_SIMPLE_queue_t), (void **) &free_queues_p);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Request recv q region */
    mpi_errno = MPIDU_shm_seg_alloc(num_local * sizeof(MPIDI_SIMPLE_queue_t), (void **) &recv_queues_p);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Request shared collectives barrier vars region */
    mpi_errno = MPIDU_shm_seg_alloc(MPIDI_SIMPLE_NUM_BARRIER_VARS * sizeof(MPIDI_SIMPLE_barrier_vars_t),
                                    (void **) &MPIDI_SIMPLE_mem_region.barrier_vars);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Actually allocate the segment and assign regions to the pointers */
    mpi_errno = MPIDU_shm_seg_commit(&MPIDI_SIMPLE_mem_region.memory, &MPIDI_SIMPLE_mem_region.barrier,
                                     num_local, local_rank, MPIDI_SIMPLE_mem_region.local_procs[0],
                                     MPIDI_SIMPLE_mem_region.rank);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* post check_alloc steps */
    if(MPIDI_SIMPLE_mem_region.memory.symmetrical == 1) {
        MPIDI_SIMPLE_asym_base_addr = NULL;
    } else {
        MPIDI_SIMPLE_asym_base_addr = MPIDI_SIMPLE_mem_region.memory.base_addr;
#ifdef MPIDI_SIMPLE_SYMMETRIC_QUEUES
        MPIR_ERR_INTERNALANDJUMP(mpi_errno, "queues are not symmetrically allocated as expected");
#endif
    }

    /* init shared collectives barrier region */
    mpi_errno = MPIDI_SIMPLE_barrier_vars_init(MPIDI_SIMPLE_mem_region.barrier_vars);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* local procs barrier */
    mpi_errno = MPIDU_shm_barrier(MPIDI_SIMPLE_mem_region.barrier, num_local);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* find our cell region */
    MPIDI_SIMPLE_mem_region.Elements = cells_p[local_rank];

    /* Tables of pointers to shared memory Qs */
    MPIU_CHKPMEM_MALLOC(MPIDI_SIMPLE_mem_region.FreeQ, MPIDI_SIMPLE_queue_ptr_t *,
                        size * sizeof(MPIDI_SIMPLE_queue_ptr_t), mpi_errno, "FreeQ");
    MPIU_CHKPMEM_MALLOC(MPIDI_SIMPLE_mem_region.RecvQ, MPIDI_SIMPLE_queue_ptr_t *,
                        size * sizeof(MPIDI_SIMPLE_queue_ptr_t), mpi_errno, "RecvQ");

    /* Init table entry for our Qs */
    MPIDI_SIMPLE_mem_region.FreeQ[rank] = &free_queues_p[local_rank];
    MPIDI_SIMPLE_mem_region.RecvQ[rank] = &recv_queues_p[local_rank];

    /* Init our queues */
    MPIDI_SIMPLE_queue_init(MPIDI_SIMPLE_mem_region.RecvQ[rank]);
    MPIDI_SIMPLE_queue_init(MPIDI_SIMPLE_mem_region.FreeQ[rank]);

    /* Init and enqueue our free cells */
    for(i = 0; i < MPIDI_SIMPLE_NUM_CELLS; ++i) {
        MPIDI_SIMPLE_cell_init(&(MPIDI_SIMPLE_mem_region.Elements[i]),rank);
        MPIDI_SIMPLE_queue_enqueue(MPIDI_SIMPLE_mem_region.FreeQ[rank],
                                            &(MPIDI_SIMPLE_mem_region.Elements[i]));
    }

    /* set route for local procs through shmem */
    for(i = 0; i < num_local; i++) {
        grank = local_procs[i];
        MPIDI_SIMPLE_mem_region.FreeQ[grank] = &free_queues_p[i];
        MPIDI_SIMPLE_mem_region.RecvQ[grank] = &recv_queues_p[i];

        MPIU_Assert(MPIDI_SIMPLE_ALIGNED(MPIDI_SIMPLE_mem_region.FreeQ[grank], MPIDI_SIMPLE_CACHE_LINE_LEN));
        MPIU_Assert(MPIDI_SIMPLE_ALIGNED(MPIDI_SIMPLE_mem_region.RecvQ[grank], MPIDI_SIMPLE_CACHE_LINE_LEN));
    }

    /* make pointers to our queues global so we don't have to dereference the array */
    MPIDI_SIMPLE_mem_region.my_freeQ = MPIDI_SIMPLE_mem_region.FreeQ[rank];
    MPIDI_SIMPLE_mem_region.my_recvQ = MPIDI_SIMPLE_mem_region.RecvQ[rank];

    /* local barrier */
    mpi_errno = MPIDU_shm_barrier(MPIDI_SIMPLE_mem_region.barrier, num_local);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Allocate table of pointers to fastboxes */
    MPIU_CHKPMEM_MALLOC(MPIDI_SIMPLE_mem_region.mailboxes.in, MPIDI_SIMPLE_fastbox_t **,
                        num_local * sizeof(MPIDI_SIMPLE_fastbox_t *), mpi_errno, "fastboxes");
    MPIU_CHKPMEM_MALLOC(MPIDI_SIMPLE_mem_region.mailboxes.out, MPIDI_SIMPLE_fastbox_t **,
                        num_local * sizeof(MPIDI_SIMPLE_fastbox_t *), mpi_errno, "fastboxes");

    MPIU_Assert(num_local > 0);

#define MPIDI_SIMPLE_MAILBOX_INDEX(sender, receiver) (((sender) > (receiver)) ? ((num_local-1) * (sender) + (receiver)) :		\
                                          (((sender) < (receiver)) ? ((num_local-1) * (sender) + ((receiver)-1)) : 0))

    /* fill in tables */
    for(i = 0; i < num_local; ++i) {
        if(i == local_rank) {
            /* No fastboxs to myself */
            MPIDI_SIMPLE_mem_region.mailboxes.in[i] = NULL;
            MPIDI_SIMPLE_mem_region.mailboxes.out[i] = NULL;
        } else {
            MPIDI_SIMPLE_mem_region.mailboxes.in[i] = &fastboxes_p[MPIDI_SIMPLE_MAILBOX_INDEX(i, local_rank)];
            MPIDI_SIMPLE_mem_region.mailboxes.out[i] = &fastboxes_p[MPIDI_SIMPLE_MAILBOX_INDEX(local_rank, i)];
            OPA_store_int(&MPIDI_SIMPLE_mem_region.mailboxes.in[i]->common.flag.value, 0);
            OPA_store_int(&MPIDI_SIMPLE_mem_region.mailboxes.out[i]->common.flag.value, 0);
        }
    }

#undef MPIDI_SIMPLE_MAILBOX_INDEX

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
    mpi_errno = MPIDU_shm_barrier(MPIDI_SIMPLE_mem_region.barrier, MPIDI_SIMPLE_mem_region.num_local);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* from MPIDI_SIMPLE_init */
    MPL_free(MPIDI_SIMPLE_mem_region.FreeQ);
    MPL_free(MPIDI_SIMPLE_mem_region.RecvQ);
    MPL_free(MPIDI_SIMPLE_mem_region.local_ranks);
    MPL_free(MPIDI_SIMPLE_mem_region.seg);
    MPL_free(MPIDI_SIMPLE_mem_region.mailboxes.out);
    MPL_free(MPIDI_SIMPLE_mem_region.mailboxes.in);
    MPL_free(MPIDI_SIMPLE_mem_region.local_procs);

    mpi_errno = MPIDU_shm_seg_destroy(&MPIDI_SIMPLE_mem_region.memory, MPIDI_SIMPLE_mem_region.num_local);

    if(mpi_errno)
        MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_FINALIZE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline void *MPIDI_CH4_SHM_alloc_mem(size_t size, MPIR_Info *info_ptr)
{
    MPIU_Assert(0);
    return NULL;
}

static inline int MPIDI_CH4_SHM_free_mem(void *ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_comm_get_lpid(MPIR_Comm *comm_ptr,
                                              int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_gpid_get(MPIR_Comm *comm_ptr, int rank, MPIR_Gpid *gpid)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_get_node_id(MPIR_Comm *comm, int rank, MPID_Node_id_t *id_p)
{
    *id_p = (MPID_Node_id_t) 0;
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_get_max_node_id(MPIR_Comm *comm, MPID_Node_id_t *max_id_p)
{
    *max_id_p = (MPID_Node_id_t) 1;
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_getallincomm(MPIR_Comm *comm_ptr,
                                             int local_size, MPIR_Gpid local_gpids[], int *singleAVT)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_create_intercomm_from_lpids(MPIR_Comm *newcomm_ptr,
                                                            int size, const int lpids[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_SIMPLE_INIT_H_INCLUDED */
