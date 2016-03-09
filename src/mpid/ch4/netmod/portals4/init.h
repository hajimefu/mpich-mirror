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
#ifndef NETMOD_PORTALS4_INIT_H_INCLUDED
#define NETMOD_PORTALS4_INIT_H_INCLUDED

#include "mpidch4r.h"
#include "types.h"
#include "impl.h"
#include "portals4.h"

static inline int MPIDI_CH4_NMI_PTL_append_overflow(int i)
{
    ptl_me_t me;
    ptl_process_t id_any;

    id_any.phys.pid = PTL_PID_ANY;
    id_any.phys.nid = PTL_NID_ANY;

    me.start = MPIDI_CH4_NMI_PTL_global.overflow_bufs[i];
    me.length = MPIDI_CH4_NMI_PTL_OVERFLOW_BUFFER_SZ;
    me.ct_handle = PTL_CT_NONE;
    me.uid = PTL_UID_ANY;
    me.options = ( PTL_ME_OP_PUT | PTL_ME_MANAGE_LOCAL | PTL_ME_NO_TRUNCATE | PTL_ME_MAY_ALIGN |
                   PTL_ME_IS_ACCESSIBLE | PTL_ME_EVENT_LINK_DISABLE );
    me.match_id = id_any;
    me.match_bits = 0;
    me.ignore_bits = ~((ptl_match_bits_t)0);
    me.min_free = MPIDI_CH4_NMI_PTL_MAX_AM_EAGER_SZ;

    return PtlMEAppend(MPIDI_CH4_NMI_PTL_global.ni, MPIDI_CH4_NMI_PTL_global.pt, &me, PTL_OVERFLOW_LIST, (void *)(size_t)i,
                       &MPIDI_CH4_NMI_PTL_global.overflow_me_handles[i]);
}

static inline int MPIDI_CH4_NM_init(int rank,
                                    int size,
                                    int appnum,
                                    int *tag_ub,
                                    MPID_Comm * comm_world,
                                    MPID_Comm * comm_self,
                                    int spawned,
                                    int num_contexts,
                                    void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS;
    int ret;
    ptl_md_t md;
    ptl_ni_limits_t desired;
    ptl_process_t my_ptl_id;

    /* Make sure our IOV is the same as portals4's IOV */
    MPIU_Assert(sizeof(ptl_iovec_t) == sizeof(MPL_IOV));
    MPIU_Assert(((void*)&(((ptl_iovec_t*)0)->iov_base)) == ((void*)&(((MPL_IOV*)0)->MPL_IOV_BUF)));
    MPIU_Assert(((void*)&(((ptl_iovec_t*)0)->iov_len))  == ((void*)&(((MPL_IOV*)0)->MPL_IOV_LEN)));
    MPIU_Assert(sizeof(((ptl_iovec_t*)0)->iov_len) == sizeof(((MPL_IOV*)0)->MPL_IOV_LEN));

    /* init portals */
    ret = PtlInit();
    /* MPIDI_CH4_NMI_PTL_CHK_STATUS(ret, PtlInit); */

    /* /\* do an interface pre-init to get the default limits struct *\/ */
    /* ret = PtlNIInit(PTL_IFACE_DEFAULT, PTL_NI_MATCHING | PTL_NI_PHYSICAL, */
    /*                 PTL_PID_ANY, NULL, &desired, &MPIDI_CH4_NMI_PTL_global.ni_handle); */
    /* MPIDI_CH4_NMI_PTL_CHK_STATUS(ret, PtlNIInit); */

    /* /\* finalize the interface so we can re-init with our desired maximums *\/ */
    /* ret = PtlNIFini(MPIDI_CH4_NMI_PTL_global.ni_handle); */
    /* MPIDI_CH4_NMI_PTL_CHK_STATUS(ret, PtlNIFini); */

    /* /\* set higher limits if they are determined to be too low *\/ */
    /* if (desired.max_unexpected_headers < UNEXPECTED_HDR_COUNT && getenv("PTL_LIM_MAX_UNEXPECTED_HEADERS") == NULL) */
    /*     desired.max_unexpected_headers = UNEXPECTED_HDR_COUNT; */
    /* if (desired.max_list_size < LIST_SIZE && getenv("PTL_LIM_MAX_LIST_SIZE") == NULL) */
    /*     desired.max_list_size = LIST_SIZE; */
    /* if (desired.max_entries < ENTRY_COUNT && getenv("PTL_LIM_MAX_ENTRIES") == NULL) */
    /*     desired.max_entries = ENTRY_COUNT; */

    /* do the real init */
    ret = PtlNIInit(PTL_IFACE_DEFAULT, PTL_NI_MATCHING | PTL_NI_PHYSICAL,
                    PTL_PID_ANY, NULL, &MPIDI_CH4_NMI_PTL_global.ni_limits, &MPIDI_CH4_NMI_PTL_global.ni);
    /* MPIDI_CH4_NMI_PTL_CHK_STATUS(ret, PtlNIInit); */

    /* allocate EQs */
    ret = PtlEQAlloc(MPIDI_CH4_NMI_PTL_global.ni, MPIDI_CH4_NMI_PTL_EVENT_COUNT, &MPIDI_CH4_NMI_PTL_global.eqs[0]);
    /* MPIDI_CH4_NMI_PTL_CHK_STATUS(ret, PtlEQAlloc); */
    ret = PtlEQAlloc(MPIDI_CH4_NMI_PTL_global.ni, MPIDI_CH4_NMI_PTL_EVENT_COUNT, &MPIDI_CH4_NMI_PTL_global.eqs[1]);
    /* MPIDI_CH4_NMI_PTL_CHK_STATUS(ret, PtlEQAlloc); */

    /* allocate portal */
    ret = PtlPTAlloc(MPIDI_CH4_NMI_PTL_global.ni, PTL_PT_ONLY_USE_ONCE | PTL_PT_ONLY_TRUNCATE | PTL_PT_FLOWCTRL,
                     MPIDI_CH4_NMI_PTL_global.eqs[1], PTL_PT_ANY, &MPIDI_CH4_NMI_PTL_global.pt);
    /* MPIDI_CH4_NMI_PTL_CHK_STATUS(ret, PtlPTAlloc); */

    /* create an MD that covers all of memory */
    md.start = NULL;
    md.length = PTL_SIZE_MAX;
    md.options = 0x0;
    md.eq_handle = MPIDI_CH4_NMI_PTL_global.eqs[0];
    md.ct_handle = PTL_CT_NONE;
    ret = PtlMDBind(MPIDI_CH4_NMI_PTL_global.ni, &md, &MPIDI_CH4_NMI_PTL_global.md);
    /* MPIDI_CH4_NMI_PTL_CHK_STATUS(ret, PtlMDBind); */

    /* create business card */
    int key_max_sz;
    int val_max_sz, val_sz_left;
    int name_max_sz;
    ret = PMI_KVS_Get_key_length_max(&key_max_sz);
    ret = PMI_KVS_Get_value_length_max(&val_max_sz);
    ret = PMI_KVS_Get_name_length_max(&name_max_sz);
    MPIDI_CH4_NMI_PTL_global.kvsname = MPL_malloc(name_max_sz);
    ret = PMI_KVS_Get_my_name(MPIDI_CH4_NMI_PTL_global.kvsname, name_max_sz);

    char keyS[key_max_sz], valS[val_max_sz], *buscard;
    buscard = valS;
    val_sz_left = val_max_sz;
    ret = PtlGetId(MPIDI_CH4_NMI_PTL_global.ni, &my_ptl_id);
    ret = MPL_str_add_binary_arg(&buscard, &val_sz_left, "NID", (char *)&my_ptl_id.phys.nid, sizeof(my_ptl_id.phys.nid));
    ret = MPL_str_add_binary_arg(&buscard, &val_sz_left, "PID", (char *)&my_ptl_id.phys.pid, sizeof(my_ptl_id.phys.pid));
    ret = MPL_str_add_binary_arg(&buscard, &val_sz_left, "PTI", (char *)&MPIDI_CH4_NMI_PTL_global.pt, sizeof(MPIDI_CH4_NMI_PTL_global.pt));

    sprintf(keyS, "PTL-%d", rank);
    buscard = valS;
    ret = PMI_KVS_Put(MPIDI_CH4_NMI_PTL_global.kvsname, keyS, buscard);
    ret = PMI_KVS_Commit(MPIDI_CH4_NMI_PTL_global.kvsname);
    ret = PMI_Barrier();

    /* get and store business cards in address table */
    MPIDI_CH4_NMI_PTL_addr_table = MPL_malloc(size * sizeof(MPIDI_CH4_NMI_PTL_addr_t));
    int len, i;
    for (i = 0; i < size; i++) {
        sprintf(keyS, "PTL-%d", i);
        ret = PMI_KVS_Get(MPIDI_CH4_NMI_PTL_global.kvsname, keyS, valS, val_max_sz);
        MPL_str_get_binary_arg(valS, "NID", (char *)&MPIDI_CH4_NMI_PTL_addr_table[i].process.phys.nid, sizeof(MPIDI_CH4_NMI_PTL_addr_table[i].process.phys.nid), &len);
        MPL_str_get_binary_arg(valS, "PID", (char *)&MPIDI_CH4_NMI_PTL_addr_table[i].process.phys.pid, sizeof(MPIDI_CH4_NMI_PTL_addr_table[i].process.phys.pid), &len);
        MPL_str_get_binary_arg(valS, "PTI", (char *)&MPIDI_CH4_NMI_PTL_addr_table[i].pt, sizeof(MPIDI_CH4_NMI_PTL_addr_table[i].pt), &len);
    }

    /* Setup CH4R Active Messages */
    MPIDI_CH4R_init(comm_world, comm_self, num_contexts, netmod_contexts);
    for (i = 0; i < MPIDI_CH4_NMI_PTL_NUM_OVERFLOW_BUFFERS; i++) {
        MPIDI_CH4_NMI_PTL_global.overflow_bufs[i] = MPL_malloc(MPIDI_CH4_NMI_PTL_OVERFLOW_BUFFER_SZ);
        MPIDI_CH4_NMI_PTL_append_overflow(i);
    }

    MPIDI_CH4_NMI_PTL_global.node_map = MPL_malloc(size * sizeof(*MPIDI_CH4_NMI_PTL_global.node_map));
    mpi_errno = MPIDI_CH4R_build_nodemap(rank, comm_world, size, MPIDI_CH4_NMI_PTL_global.node_map, &MPIDI_CH4_NMI_PTL_global.max_node_id);

 fn_exit:
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_finalize(void)
{
    int mpi_errno = MPI_SUCCESS;
    int ret, i;

    for (i = 0; i < MPIDI_CH4_NMI_PTL_NUM_OVERFLOW_BUFFERS; i++) {
        ret = PtlMEUnlink(MPIDI_CH4_NMI_PTL_global.overflow_me_handles[i]);
    }
    ret = PtlMDRelease(MPIDI_CH4_NMI_PTL_global.md);
    ret = PtlPTFree(MPIDI_CH4_NMI_PTL_global.ni, MPIDI_CH4_NMI_PTL_global.pt);
    ret = PtlEQFree(MPIDI_CH4_NMI_PTL_global.eqs[1]);
    ret = PtlEQFree(MPIDI_CH4_NMI_PTL_global.eqs[0]);
    ret = PtlNIFini(MPIDI_CH4_NMI_PTL_global.ni);
    PtlFini();

    return mpi_errno;
}


static inline int MPIDI_CH4_NM_comm_get_lpid(MPID_Comm * comm_ptr,
                                             int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_gpid_get(MPID_Comm * comm_ptr, int rank, MPIR_Gpid * gpid)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    *id_p = MPIDI_CH4_NMI_PTL_global.node_map[rank];
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p)
{
    *max_id_p = MPIDI_CH4_NMI_PTL_global.max_node_id;
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_getallincomm(MPID_Comm * comm_ptr,
                                            int local_size, MPIR_Gpid local_gpids[], int *singlePG)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_create_intercomm_from_lpids(MPID_Comm * newcomm_ptr,
                                                           int size, const int lpids[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_free_mem(void *ptr)
{
    return MPIDI_CH4R_free_mem(ptr);
}

static inline void *MPIDI_CH4_NM_alloc_mem(size_t size, MPID_Info * info_ptr)
{
    return MPIDI_CH4R_alloc_mem(size, info_ptr);
}


#endif /* NETMOD_PORTALS4_INIT_H_INCLUDED */
