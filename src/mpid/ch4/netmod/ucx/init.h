/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef NETMOD_UCX_INIT_H_INCLUDED
#define NETMOD_UCX_INIT_H_INCLUDED

#include "impl.h"
#include "mpich_cvars.h"
#include "types.h"
#include "pmi.h"
#include <ucp/api/ucp.h>
#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_init(int rank,
                                    int size,
                                    int appnum,
                                    int *tag_ub,
                                    MPIR_Comm * comm_world,
                                    MPIR_Comm * comm_self,
                                    int spawned, int num_contexts, void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS, thr_err, pmi_errno;
    int str_errno = MPL_STR_SUCCESS;
    ucp_config_t *config;
    ucs_status_t ucx_status;
    uint64_t features = 0;
    int status;
    char valS[MPIDI_UCX_KVSAPPSTRLEN], *val;
    char keyS[MPIDI_UCX_KVSAPPSTRLEN];
    size_t maxlen = MPIDI_UCX_KVSAPPSTRLEN;
    char *table = NULL;
    int i;
    ucp_params_t ucp_params;
    int avtid = 0, max_n_avts;

    size_t address_length = 0;

    MPIDI_STATE_DECL(MPID_STATE_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_INIT);

    ucx_status = ucp_config_read(NULL, NULL, &config);
    MPIDI_UCX_CHK_STATUS(ucx_status, read_config);

    /* For now use only the tag feature */
    features = UCP_FEATURE_TAG | UCP_FEATURE_RMA;
    ucp_params.features = features;
    ucp_params.request_size = sizeof(MPIDI_UCX_ucp_request_t);
    ucp_params.request_init = MPIDI_UCX_Request_init_callback;
    ucp_params.request_cleanup = NULL;
    ucx_status = ucp_init(&ucp_params, config, &MPIDI_UCX_global.context);
    MPIDI_UCX_CHK_STATUS(ucx_status, init);
    ucp_config_release(config);

    ucx_status  = ucp_worker_create(MPIDI_UCX_global.context, UCS_THREAD_MODE_SERIALIZED,
                             &MPIDI_UCX_global.worker);
    MPIDI_UCX_CHK_STATUS(ucx_status, worker_create);

    ucx_status = ucp_worker_get_address (MPIDI_UCX_global.worker, &MPIDI_UCX_global.if_address, &MPIDI_UCX_global.addrname_len);
    MPIDI_UCX_CHK_STATUS(ucx_status, get_worker_address);

    val = valS;
    str_errno = MPL_str_add_binary_arg(&val, (int *) &maxlen, "UCX", (char *) MPIDI_UCX_global.if_address,
                            (int) MPIDI_UCX_global.addrname_len);

    /* MPIDI_CH4_UCX_STR_ERRCHK(str_errno, buscard_len); */
    pmi_errno = PMI_KVS_Get_my_name(MPIDI_UCX_global.kvsname, MPIDI_UCX_KVSAPPSTRLEN);

    val = valS;
    sprintf(keyS, "UCX-%d", rank);
    pmi_errno = PMI_KVS_Put(MPIDI_UCX_global.kvsname, keyS, val);
    MPIDI_UCX_PMI_ERROR(pmi_errno, pmi_put_name);
    pmi_errno = PMI_KVS_Commit(MPIDI_UCX_global.kvsname);
    MPIDI_UCX_PMI_ERROR(pmi_errno, pmi_commit);
    pmi_errno =  PMI_Barrier();
    MPIDI_UCX_PMI_ERROR(pmi_errno, pmi_barrier);

    table = MPL_malloc(size * MPIDI_UCX_global.addrname_len);
    MPIDI_UCX_global.pmi_addr_table = table;

    maxlen = MPIDI_UCX_KVSAPPSTRLEN;

    for (i = 0; i < size; i++) {
        sprintf(keyS, "UCX-%d", i);
        pmi_errno = PMI_KVS_Get(MPIDI_UCX_global.kvsname, keyS, valS, MPIDI_UCX_KVSAPPSTRLEN);
        MPIDI_UCX_PMI_ERROR(pmi_errno, pmi_commit);
        str_errno = MPL_str_get_binary_arg(valS, "UCX", &table[i * MPIDI_UCX_global.addrname_len],
                                (int) MPIDI_UCX_global.addrname_len, (int *) &maxlen);
        /* MPIDI_UCX_STR_ERRCHK(str_errno, buscard_len); */
        ucx_status = ucp_ep_create(MPIDI_UCX_global.worker,
                                (ucp_address_t *) & table[i * MPIDI_UCX_global.addrname_len],
                                &MPIDI_UCX_AV(&MPIDIU_get_av(0, i)).dest);
        MPIDI_UCX_CHK_STATUS(ucx_status, ep_create);
    }

    /* -------------------------------- */
    /* Setup CH4U Active Messages       */
    /* -------------------------------- */
    MPIDI_CH4U_init(comm_world, comm_self, num_contexts, netmod_contexts);
    for (i = 0; i < MPIDI_UCX_NUM_AM_BUFFERS; i++) {
        MPIDI_UCX_global.am_bufs[i] = MPL_malloc(MPIDI_UCX_MAX_AM_EAGER_SZ);
        MPIDI_UCX_global.ucp_am_requests[i] =
             (MPIDI_UCX_ucp_request_t *)ucp_tag_recv_nb(MPIDI_UCX_global.worker,
                                                                MPIDI_UCX_global.am_bufs[i],
                                                                MPIDI_UCX_MAX_AM_EAGER_SZ,
                                                                ucp_dt_make_contig(1),
                                                                MPIDI_UCX_AM_TAG,
                                                                ~MPIDI_UCX_AM_TAG,
                                                                &MPIDI_UCX_Handle_am_recv);
        MPIDI_CH4_UCX_REQUEST(MPIDI_UCX_global.ucp_am_requests[i], tag_recv_nb);
    }

    mpi_errno = MPIR_Datatype_init_names();
    MPIDI_CH4_UCX_MPI_ERROR(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_EXIT);
    return mpi_errno;
  fn_fail:
    if (MPIDI_UCX_global.worker != NULL)
        ucp_worker_destroy(MPIDI_UCX_global.worker);

    if (MPIDI_UCX_global.context != NULL)
        ucp_cleanup(MPIDI_UCX_global.context);

    goto fn_exit;

}

static inline int MPIDI_CH4_NM_finalize(void)
{
    int mpi_errno = MPI_SUCCESS, thr_err, pmi_errno;
    int i, max_n_avts;
    max_n_avts = MPIDIU_get_max_n_avts();

    MPIR_Comm_release(MPIR_Process.comm_world);
    MPIR_Comm_release(MPIR_Process.comm_self);

    MPIDI_CH4U_finalize();

    /* cancel and free active message buffers */
    for (i = 0; i < MPIDI_UCX_NUM_AM_BUFFERS; i++) {
        ucp_request_cancel(MPIDI_UCX_global.worker,
                           MPIDI_UCX_global.ucp_am_requests[i]);
        ucp_request_release(MPIDI_UCX_global.ucp_am_requests[i]);
        MPL_free(MPIDI_UCX_global.am_bufs[i]);
    }

    if (MPIDI_UCX_global.worker != NULL)
        ucp_worker_destroy(MPIDI_UCX_global.worker);

    if (MPIDI_UCX_global.context != NULL)
        ucp_cleanup(MPIDI_UCX_global.context);

    MPL_free(MPIDI_UCX_global.pmi_addr_table);

    PMI_Finalize();

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

static inline int MPIDI_CH4_NM_comm_get_lpid(MPIR_Comm * comm_ptr,
                                             int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
    int avtid = 0, lpid = 0;
   if(comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        MPIDIU_comm_rank_to_pid(comm_ptr, idx, &lpid, &avtid);
    } else if (is_remote) {
        MPIDIU_comm_rank_to_pid(comm_ptr, idx, &lpid, &avtid);
    } else {
        MPIDIU_comm_rank_to_pid_local(comm_ptr, idx, &lpid, &avtid);
    }

    *lpid_ptr = MPIDIU_LPID_CREATE(avtid, lpid);
    return MPI_SUCCESS;

}

static inline int MPIDI_CH4_NM_gpid_get(MPIR_Comm * comm_ptr, int rank, MPIR_Gpid * gpid)
{
    int mpi_errno = MPI_SUCCESS;
    int avtid = 0, lpid = 0;
    size_t sz;
    MPIDIU_comm_rank_to_pid(comm_ptr, rank, &lpid, &avtid);
    MPIU_Assert(rank < comm_ptr->local_size);
    sz = sizeof(MPIDI_UCX_GPID(gpid).addr);
    memset(MPIDI_UCX_GPID(gpid).addr, 0, sz);
    memcpy(MPIDI_UCX_GPID(gpid).addr,
           &MPIDI_UCX_global.pmi_addr_table[lpid * MPIDI_UCX_global.addrname_len], sz);
    MPIU_Assert(sz <= sizeof(MPIDI_UCX_GPID(gpid).addr));
  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_get_node_id(MPIR_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    MPIDI_CH4U_get_node_id(comm, rank, id_p);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_get_max_node_id(MPIR_Comm * comm, MPID_Node_id_t * max_id_p)
{
    MPIDI_CH4U_get_max_node_id(comm, max_id_p);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_getallincomm(MPIR_Comm * comm_ptr,
                                            int local_size, MPIR_Gpid local_gpids[], int *singleAVT)
{
    int i;

    for (i = 0; i < comm_ptr->local_size; i++)
        MPIDI_GPID_Get(comm_ptr, i, &local_gpids[i]);

    *singleAVT = 0;
    return 0;
}

static inline int MPIDI_CH4_NM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[])
{

    int i, mpi_errno = MPI_SUCCESS;
    int *new_avt_procs;
    int n_new_procs = 0;
    size_t sz;
    int max_n_avts;
    new_avt_procs = (int *) MPL_malloc(size * sizeof(int));
    max_n_avts = MPIDIU_get_max_n_avts();

    for(i = 0; i < size; i++) {
        int j, k;
        char tbladdr[128];
        int found = 0;

        for (k = 0; k < max_n_avts; k++) {
            if (MPIDIU_get_av_table(k) == NULL) { continue; }
            for(j = 0; j < MPIDIU_get_av_table(k)->size; j++) {
                sz = sizeof(MPIDI_UCX_GPID(&gpid[i]).addr);
                MPIU_Assert(sz <= sizeof(MPIDI_UCX_GPID(&gpid[i]).addr));

                if(!memcmp(&MPIDI_UCX_global.pmi_addr_table[j * MPIDI_UCX_global.addrname_len],
                           MPIDI_UCX_GPID(&gpid[i]).addr, sz)) {
                    lpid[i] = MPIDIU_LPID_CREATE(k, j);
                    found = 1;
                    break;
                }
            }
        }

        if (!found) {
            new_avt_procs[n_new_procs] = i;
            n_new_procs++;
        }
    }

    /* FIXME: add support for dynamic processes */
    if (n_new_procs > 0) {
        mpi_errno = -1;
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPL_free(new_avt_procs);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_create_intercomm_from_lpids(MPIR_Comm * newcomm_ptr,
                                                           int size, const int lpids[])
{
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_free_mem(void *ptr)
{
    return MPIDI_CH4U_free_mem(ptr);
}

static inline void *MPIDI_CH4_NM_alloc_mem(size_t size, MPIR_Info * info_ptr)
{
    return  MPIDI_CH4U_alloc_mem(size, info_ptr);
}

#endif /* NETMOD_UCX_INIT_H_INCLUDED */
