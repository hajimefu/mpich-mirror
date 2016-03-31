/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef INIT_H_INCLUDED
#define INIT_H_INCLUDED

#include "impl.h"
#include "mpich_cvars.h"
#include "ucx_types.h"
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
                                    MPID_Comm * comm_world,
                                    MPID_Comm * comm_self,
                                    int spawned, int num_contexts, void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS, thr_err, pmi_errno;
    int str_errno = MPL_STR_SUCCESS;
    ucp_config_t *config;
    ucs_status_t ucx_status;
    uint64_t features = 0;
    int status;
    char valS[MPIDI_CH4_NMI_UCX_KVSAPPSTRLEN], *val;
    char keyS[MPIDI_CH4_NMI_UCX_KVSAPPSTRLEN];
    size_t maxlen = MPIDI_CH4_NMI_UCX_KVSAPPSTRLEN;
    char *table = NULL;
    int i;
    ucp_params_t ucp_params;

    size_t address_length = 0;

    MPIDI_STATE_DECL(MPID_STATE_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_INIT);

    ucx_status = ucp_config_read(NULL, NULL, &config);
    MPIDI_CH4_NMI_UCX_CHK_STATUS(ucx_status, read_config);

    /* For now use only the tag feature */
    features = UCP_FEATURE_TAG | UCP_FEATURE_RMA;
    ucp_params.features = features;
    ucp_params.request_size = sizeof(MPIDI_CH4_NMI_UCX_ucp_request_t);
    ucp_params.request_init = MPIDI_CH4_NMI_UCX_Request_init_callback;
    ucp_params.request_cleanup = NULL;
    ucx_status = ucp_init(&ucp_params, config, &MPIDI_CH4_NMI_UCX_global.context);
    MPIDI_CH4_NMI_UCX_CHK_STATUS(ucx_status, init);
    ucp_config_release(config);

    ucx_status  = ucp_worker_create(MPIDI_CH4_NMI_UCX_global.context, UCS_THREAD_MODE_SERIALIZED,
                             &MPIDI_CH4_NMI_UCX_global.worker);
    MPIDI_CH4_NMI_UCX_CHK_STATUS(ucx_status, worker_create);

    ucx_status = ucp_worker_get_address (MPIDI_CH4_NMI_UCX_global.worker, &MPIDI_CH4_NMI_UCX_global.if_address, &MPIDI_CH4_NMI_UCX_global.addrname_len);
    MPIDI_CH4_NMI_UCX_CHK_STATUS(ucx_status, get_worker_address);

    val = valS;
    str_errno = MPL_str_add_binary_arg(&val, (int *) &maxlen, "UCX", (char *) MPIDI_CH4_NMI_UCX_global.if_address,
                            (int) MPIDI_CH4_NMI_UCX_global.addrname_len);

    MPIDI_CH4_UCX_STR_ERRCHK(str_errno, buscard_len);
    pmi_errno = PMI_KVS_Get_my_name(MPIDI_CH4_NMI_UCX_global.kvsname, MPIDI_CH4_NMI_UCX_KVSAPPSTRLEN);

    val = valS;
    sprintf(keyS, "UCX-%d", rank);
    pmi_errno = PMI_KVS_Put(MPIDI_CH4_NMI_UCX_global.kvsname, keyS, val);
    MPIDI_CH4_NMI_UCX_PMI_ERROR(pmi_errno, pmi_put_name);
    pmi_errno = PMI_KVS_Commit(MPIDI_CH4_NMI_UCX_global.kvsname);
    MPIDI_CH4_NMI_UCX_PMI_ERROR(pmi_errno, pmi_commit);
    pmi_errno =  PMI_Barrier();
    MPIDI_CH4_NMI_UCX_PMI_ERROR(pmi_errno, pmi_barrier);

    table = MPL_malloc(size * MPIDI_CH4_NMI_UCX_global.addrname_len);
    MPIDI_CH4_NMI_UCX_eps = MPL_malloc(size * sizeof(ucp_ep_h));

    maxlen = MPIDI_CH4_NMI_UCX_KVSAPPSTRLEN;

    for (i = 0; i < size; i++) {
        sprintf(keyS, "UCX-%d", i);
        pmi_errno = PMI_KVS_Get(MPIDI_CH4_NMI_UCX_global.kvsname, keyS, valS, MPIDI_CH4_NMI_UCX_KVSAPPSTRLEN);
        MPIDI_CH4_NMI_UCX_PMI_ERROR(pmi_errno, pmi_commit);
        str_errno = MPL_str_get_binary_arg(valS, "UCX", &table[i * MPIDI_CH4_NMI_UCX_global.addrname_len],
                                (int) MPIDI_CH4_NMI_UCX_global.addrname_len, (int *) &maxlen);
        MPIDI_CH4_UCX_STR_ERRCHK(str_errno, buscard_len);
        ucx_status = ucp_ep_create(MPIDI_CH4_NMI_UCX_global.worker,
                                (ucp_address_t *) & table[i * MPIDI_CH4_NMI_UCX_global.addrname_len],
                                &MPIDI_CH4_NMI_UCX_eps[i]);
        MPIDI_CH4_NMI_UCX_CHK_STATUS(ucx_status, ep_create);
    }

    mpi_errno = MPIDI_CH4_NMI_UCX_VEPT_Create(comm_self->remote_size, &MPIDI_CH4_NMI_UCX_COMM(comm_self).vept);
    MPIDI_CH4_UCX_MPI_ERROR(mpi_errno);

    MPIDI_CH4_NMI_UCX_COMM(comm_self).vept->vep_table[0].addr_idx = rank;
    MPIDI_CH4_NMI_UCX_COMM(comm_self).vept->vep_table[0].is_local = 1;

    /* ---------------------------------- */
    /* Initialize MPI_COMM_WORLD and VEPT */
    /* ---------------------------------- */
    mpi_errno = MPIDI_CH4_NMI_UCX_VEPT_Create(comm_world->remote_size, &MPIDI_CH4_NMI_UCX_COMM(comm_world).vept);
    MPIDI_CH4_UCX_MPI_ERROR(mpi_errno);

    /* -------------------------------- */
    /* Setup CH4U Active Messages       */
    /* -------------------------------- */
    MPIDI_CH4U_init(comm_world, comm_self, num_contexts, netmod_contexts);
    for (i = 0; i < MPIDI_CH4_NMI_UCX_NUM_AM_BUFFERS; i++) {
        MPIDI_CH4_NMI_UCX_global.am_bufs[i] = MPL_malloc(MPIDI_CH4_NMI_UCX_MAX_AM_EAGER_SZ);
        MPIDI_CH4_NMI_UCX_global.ucp_am_requests[i] =
             (MPIDI_CH4_NMI_UCX_ucp_request_t *)ucp_tag_recv_nb(MPIDI_CH4_NMI_UCX_global.worker,
                                                                MPIDI_CH4_NMI_UCX_global.am_bufs[i],
                                                                MPIDI_CH4_NMI_UCX_MAX_AM_EAGER_SZ,
                                                                ucp_dt_make_contig(1),
                                                                MPIDI_CH4_NMI_UCX_AM_TAG,
                                                                ~MPIDI_CH4_NMI_UCX_AM_TAG,
                                                                &MPIDI_CH4_NMI_UCX_Handle_am_recv);
        MPIDI_CH4_UCX_REQUEST(MPIDI_CH4_NMI_UCX_global.ucp_am_requests[i], tag_recv_nb);
    }

    /* -------------------------------- */
    /* Calculate per-node map           */
    /* -------------------------------- */
    MPIDI_CH4_NMI_UCX_global.node_map = MPL_malloc(comm_world->local_size * sizeof(*MPIDI_CH4_NMI_UCX_global.node_map));

    MPIDI_CH4_NMI_UCX_global.max_node_id = 1024;
    mpi_errno = MPIDI_CH4U_build_nodemap(comm_world->rank,
                             comm_world,
                             comm_world->local_size,
                             MPIDI_CH4_NMI_UCX_global.node_map, &MPIDI_CH4_NMI_UCX_global.max_node_id);

    MPIDI_CH4_UCX_MPI_ERROR(mpi_errno);
    for (i = 0; i < comm_world->local_size; i++)
        MPIDI_CH4_NMI_UCX_COMM(comm_world).vept->vep_table[i].is_local =
            (MPIDI_CH4_NMI_UCX_global.node_map[i] == MPIDI_CH4_NMI_UCX_global.node_map[comm_world->rank]) ? 1 : 0;

    mpi_errno = MPIR_Datatype_init_names();
    MPIDI_CH4_UCX_MPI_ERROR(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_EXIT);
    return mpi_errno;
  fn_fail:
    if (MPIDI_CH4_NMI_UCX_eps != NULL) {
        for (i = 0; i < size; i++) {
            if (MPIDI_CH4_NMI_UCX_eps[i] != NULL)
                ucp_ep_destroy(MPIDI_CH4_NMI_UCX_eps[i]);
        }
    }
    if (MPIDI_CH4_NMI_UCX_global.worker != NULL)
        ucp_worker_destroy(MPIDI_CH4_NMI_UCX_global.worker);

    if (MPIDI_CH4_NMI_UCX_global.context != NULL)
        ucp_cleanup(MPIDI_CH4_NMI_UCX_global.context);

    goto fn_exit;

}

static inline int MPIDI_CH4_NM_finalize(void)
{
    int mpi_errno = MPI_SUCCESS, thr_err, pmi_errno;
    int i, size;

    MPIDI_CH4U_finalize();

    /* cancel and free active message buffers */
    for (i = 0; i < MPIDI_CH4_NMI_UCX_NUM_AM_BUFFERS; i++) {
        ucp_request_cancel(MPIDI_CH4_NMI_UCX_global.worker,
                           MPIDI_CH4_NMI_UCX_global.ucp_am_requests[i]);
        ucp_request_release(MPIDI_CH4_NMI_UCX_global.ucp_am_requests[i]);
        MPL_free(MPIDI_CH4_NMI_UCX_global.am_bufs[i]);
    }

    size = MPIR_Process.comm_world->local_size;
    if (MPIDI_CH4_NMI_UCX_eps != NULL) {
        for (i = 0; i < size; i++) {
            if (MPIDI_CH4_NMI_UCX_eps[i] != NULL)
                ucp_ep_destroy(MPIDI_CH4_NMI_UCX_eps[i]);
        }
    }

    if (MPIDI_CH4_NMI_UCX_global.worker != NULL)
        ucp_worker_destroy(MPIDI_CH4_NMI_UCX_global.worker);

    if (MPIDI_CH4_NMI_UCX_global.context != NULL)
        ucp_cleanup(MPIDI_CH4_NMI_UCX_global.context);

    MPL_free(MPIDI_CH4_NMI_UCX_global.node_map);

    PMI_Finalize();

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

static inline int MPIDI_CH4_NM_comm_get_lpid(MPID_Comm * comm_ptr,
                                             int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
   if(comm_ptr->comm_kind == MPID_INTRACOMM)
        *lpid_ptr = COMM_TO_INDEX(comm_ptr, idx);
    else if(is_remote)
        *lpid_ptr = COMM_TO_INDEX(comm_ptr, idx);
    else
        *lpid_ptr = 0;
    return MPI_SUCCESS;

}

static inline int MPIDI_CH4_NM_gpid_get(MPID_Comm * comm_ptr, int rank, MPIR_Gpid * gpid)
{
    int mpi_errno = MPI_SUCCESS;
  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    *id_p = MPIDI_CH4_NMI_UCX_global.node_map[COMM_TO_INDEX(comm, rank)];
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p)
{
    *max_id_p = MPIDI_CH4_NMI_UCX_global.max_node_id;
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_getallincomm(MPID_Comm * comm_ptr,
                                            int local_size, MPIR_Gpid local_gpids[], int *singlePG)
{
    int i;

    for (i = 0; i < comm_ptr->local_size; i++)
        MPIDI_GPID_Get(comm_ptr, i, &local_gpids[i]);

    *singlePG = 0;
    return 0;
}

static inline int MPIDI_CH4_NM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[])
{

    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_create_intercomm_from_lpids(MPID_Comm * newcomm_ptr,
                                                           int size, const int lpids[])
{
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_free_mem(void *ptr)
{
    return MPIDI_CH4U_free_mem(ptr);
}

static inline void *MPIDI_CH4_NM_alloc_mem(size_t size, MPID_Info * info_ptr)
{
    return  MPIDI_CH4U_alloc_mem(size, info_ptr);
}

#endif /* INIT_H_INCLUDED */
