/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef CH4_SHM_DEFS_H
#define CH4_SHM_DEFS_H

/* ************************************************************************** */
/* from mpid/ch3/channels/nemesis/include/mpid_nem_defs.h                     */
/* ************************************************************************** */

#include "mpidu_shm.h"
#define MPIDI_CH4_SHMI_SIMPLE_MAX_FNAME_LEN 256

/* FIXME: This definition should be gotten from mpidi_ch3_impl.h */
#ifndef MPIDI_CH4_SHMI_SIMPLE_MAX_HOSTNAME_LEN
#define MPIDI_CH4_SHMI_SIMPLE_MAX_HOSTNAME_LEN 256
#endif /* MPIDI_CH4_SHMI_SIMPLE_MAX_HOSTNAME_LEN */

extern char MPIDI_CH4_SHMI_SIMPLE_hostname[MPIDI_CH4_SHMI_SIMPLE_MAX_HOSTNAME_LEN];

#define MPIDI_CH4_SHMI_SIMPLE_RET_OK       1
#define MPIDI_CH4_SHMI_SIMPLE_RET_NG      -1
#define MPIDI_CH4_SHMI_SIMPLE_KEY          632236
#define MPIDI_CH4_SHMI_SIMPLE_ANY_SOURCE  -1
#define MPIDI_CH4_SHMI_SIMPLE_IN           1
#define MPIDI_CH4_SHMI_SIMPLE_OUT          0

#define MPIDI_CH4_SHMI_SIMPLE_POLL_IN      0
#define MPIDI_CH4_SHMI_SIMPLE_POLL_OUT     1

#define MPIDI_CH4_SHMI_SIMPLE_ASYMM_NULL_VAL    64
typedef MPI_Aint MPIDI_CH4_SHMI_SIMPLE_addr_t;
extern char *MPIDI_CH4_SHMI_SIMPLE_Asym_base_addr;

#define MPIDI_CH4_SHMI_SIMPLE_REL_NULL (0x0)
#define MPIDI_CH4_SHMI_SIMPLE_IS_REL_NULL(rel_ptr) (OPA_load_ptr(&(rel_ptr).p) == MPIDI_CH4_SHMI_SIMPLE_REL_NULL)
#define MPIDI_CH4_SHMI_SIMPLE_SET_REL_NULL(rel_ptr) (OPA_store_ptr(&((rel_ptr).p), MPIDI_CH4_SHMI_SIMPLE_REL_NULL))
#define MPIDI_CH4_SHMI_SIMPLE_REL_ARE_EQUAL(rel_ptr1, rel_ptr2) \
    (OPA_load_ptr(&(rel_ptr1).p) == OPA_load_ptr(&(rel_ptr2).p))

#ifndef MPIDI_CH4_SHMI_SIMPLE_SYMMETRIC_QUEUES

static inline MPIDI_CH4_SHMI_SIMPLE_Cell_ptr_t MPIDI_CH4_SHMI_SIMPLE_REL_TO_ABS(MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t r)
{
    return (MPIDI_CH4_SHMI_SIMPLE_Cell_ptr_t)((char *) OPA_load_ptr(&r.p) +
                                              (MPIDI_CH4_SHMI_SIMPLE_addr_t) MPIDI_CH4_SHMI_SIMPLE_Asym_base_addr);
}

static inline MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t MPIDI_CH4_SHMI_SIMPLE_ABS_TO_REL(MPIDI_CH4_SHMI_SIMPLE_Cell_ptr_t a)
{
    MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t ret;
    OPA_store_ptr(&ret.p, (char *) a - (MPIDI_CH4_SHMI_SIMPLE_addr_t) MPIDI_CH4_SHMI_SIMPLE_Asym_base_addr);
    return ret;
}

#else /*MPIDI_CH4_SHMI_SIMPLE_SYMMETRIC_QUEUES */
#define MPIDI_CH4_SHMI_SIMPLE_REL_TO_ABS(ptr) (ptr)
#define MPIDI_CH4_SHMI_SIMPLE_ABS_TO_REL(ptr) (ptr)
#endif /*MPIDI_CH4_SHMI_SIMPLE_SYMMETRIC_QUEUES */

/* NOTE: MPIDI_CH4_SHMI_SIMPLE_IS_LOCAL should only be used when the process is known to be
   in your comm_world (such as at init time).  This will generally not work for
   dynamic processes.  Check vc_ch->is_local instead.  If that is true, then
   it's safe to use MPIDI_CH4_SHMI_SIMPLE_LOCAL_RANK. */
#define MPIDI_CH4_SHMI_SIMPLE_NON_LOCAL -1
#define MPIDI_CH4_SHMI_SIMPLE_IS_LOCAL(grank) (MPIDI_CH4_SHMI_SIMPLE_mem_region.local_ranks[grank] != MPIDI_CH4_SHMI_SIMPLE_NON_LOCAL)
#define MPIDI_CH4_SHMI_SIMPLE_LOCAL_RANK(grank) (MPIDI_CH4_SHMI_SIMPLE_mem_region.local_ranks[grank])
#define MPIDI_CH4_SHMI_SIMPLE_NUM_BARRIER_VARS 16
#define MPIDI_CH4_SHMI_SIMPLE_SHM_MUTEX        MPID_shm_mutex
typedef struct MPIDI_CH4_SHMI_SIMPLE_Barrier_vars {
    OPA_int_t context_id;
    OPA_int_t usage_cnt;
    OPA_int_t cnt;
#if MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN != SIZEOF_INT
    char padding0[MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN - sizeof(int)];
#endif
    OPA_int_t sig0;
    OPA_int_t sig;
    char padding1[MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN - 2 * sizeof(int)];
} MPIDI_CH4_SHMI_SIMPLE_Barrier_vars_t;

typedef struct MPIDI_CH4_SHMI_SIMPLE_mem_region {
    MPIDU_shm_seg_t memory;
    MPIDU_shm_seg_info_t *seg;
    int num_seg;
    int map_lock;
    int num_local;
    int num_procs;
    int *local_procs;           /* local_procs[lrank] gives the global rank of proc with local rank lrank */
    int local_rank;
    int *local_ranks;           /* local_ranks[grank] gives the local rank of proc with global rank grank or MPIDI_CH4_SHMI_SIMPLE_NON_LOCAL */
    int ext_procs;              /* Number of non-local processes */
    int *ext_ranks;             /* Ranks of non-local processes */
    MPIDI_CH4_SHMI_SIMPLE_Fbox_arrays_t mailboxes;
    MPIDI_CH4_SHMI_SIMPLE_Cell_ptr_t Elements;
    MPIDI_CH4_SHMI_SIMPLE_Queue_ptr_t *FreeQ;
    MPIDI_CH4_SHMI_SIMPLE_Queue_ptr_t *RecvQ;
    MPIDU_shm_barrier_t *barrier;
    MPIDI_CH4_SHMI_SIMPLE_Queue_ptr_t my_freeQ;
    MPIDI_CH4_SHMI_SIMPLE_Queue_ptr_t my_recvQ;
    MPIDI_CH4_SHMI_SIMPLE_Barrier_vars_t *barrier_vars;
    int rank;
    struct MPIDI_CH4_SHMI_SIMPLE_mem_region *next;
} MPIDI_CH4_SHMI_SIMPLE_Mem_region_t, *MPIDI_CH4_SHMI_SIMPLE_mem_region_ptr_t;
extern MPIDI_CH4_SHMI_SIMPLE_Mem_region_t MPIDI_CH4_SHMI_SIMPLE_mem_region;
extern MPID_Thread_mutex_t MPID_shm_mutex;

#endif /* ifndef CH4_SHM_DEFS_H */
