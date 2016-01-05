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
#ifndef MPIDCH4U_INIT_H_INCLUDED
#define MPIDCH4U_INIT_H_INCLUDED

#include "ch4_impl.h"
#include "ch4u_util.h"
#include "ch4u_callbacks.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_init_comm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_init_comm(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS, comm_idx;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_INIT_COMM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_INIT_COMM);

    comm_idx = MPIDI_CH4R_get_context_index(comm->recvcontext_id);
    if (!MPIDI_CH4_Global.comm_req_lists[comm_idx].comm) {
        MPIDI_CH4R_COMM(comm, posted_list) = NULL;
        MPIDI_CH4R_COMM(comm, unexp_list) = NULL;
        MPIDI_CH4_Global.comm_req_lists[comm_idx].comm = comm;
        if (MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list) {
            MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
            MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list, 
                                curr, tmp) {
                MPL_DL_DELETE(MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list, curr);
                MPL_DL_APPEND(MPIDI_CH4R_COMM(comm, unexp_list), curr);
            }
            MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list = NULL;
        }
    }
    MPIDI_CH4R_COMM(comm, window_instance) = 0;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT_COMM);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_destroy_comm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_destroy_comm(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS, comm_idx;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_INIT_COMM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_INIT_COMM);

    comm_idx = MPIDI_CH4R_get_context_index(comm->recvcontext_id);
    if (MPIDI_CH4_Global.comm_req_lists[comm_idx].comm) {
        MPIU_Assert(MPIDI_CH4_Global.comm_req_lists[comm_idx].comm->dev.ch4.ch4u.posted_list == NULL);
        MPIU_Assert(MPIDI_CH4_Global.comm_req_lists[comm_idx].comm->dev.ch4.ch4u.unexp_list == NULL);
        MPIDI_CH4_Global.comm_req_lists[comm_idx].comm = NULL;
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT_COMM);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_init(MPID_Comm * comm_world, MPID_Comm * comm_self,
                                   int num_contexts, void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_INIT);

#ifndef MPIDI_CH4U_USE_PER_COMM_QUEUE
    MPIDI_CH4_Global.posted_list = NULL;
    MPIDI_CH4_Global.unexp_list = NULL;
#endif

    MPIDI_CH4_Global.cmpl_list = NULL;
    OPA_store_int(&MPIDI_CH4_Global.exp_seq_no, 0);
    OPA_store_int(&MPIDI_CH4_Global.nxt_seq_no, 0);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_SEND,
                                             &MPIDI_CH4I_am_send_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_send_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_SSEND_REQ,
                                             &MPIDI_CH4I_am_send_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_ssend_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_SSEND_ACK,
                                             &MPIDI_CH4I_am_ssend_ack_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_ssend_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_PUT_REQ,
                                             &MPIDI_CH4I_am_put_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_put_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_PUT_ACK,
                                             NULL,
                                             &MPIDI_CH4I_am_put_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_GET_REQ,
                                             &MPIDI_CH4I_am_get_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_get_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_GET_ACK,
                                             &MPIDI_CH4I_am_get_ack_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_get_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_CSWAP_REQ,
                                             &MPIDI_CH4I_am_cswap_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_cswap_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_CSWAP_ACK,
                                             &MPIDI_CH4I_am_cswap_ack_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_cswap_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_ACC_REQ,
                                             &MPIDI_CH4I_am_acc_origin_cmpl_handler,
                                             &MPIDI_CH4I_handle_acc_request);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_ACC_ACK,
                                             NULL,
                                             &MPIDI_CH4I_am_acc_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_GET_ACC_ACK,
                                             &MPIDI_CH4I_am_get_acc_ack_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_get_acc_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_WIN_CTRL,
                                             NULL,
                                             &MPIDI_CH4I_am_win_ctrl_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);


    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_PUT_IOV_REQ,
                                             &MPIDI_CH4I_am_put_iov_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_put_iov_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_PUT_IOV_ACK,
                                             NULL,
                                             &MPIDI_CH4I_am_put_iov_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_PUT_DAT_REQ,
                                             &MPIDI_CH4I_am_put_data_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_put_data_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_ACC_IOV_REQ,
                                             &MPIDI_CH4I_am_acc_iov_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_acc_iov_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_ACC_IOV_ACK,
                                             NULL,
                                             &MPIDI_CH4I_am_acc_iov_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_netmod_reg_hdr_handler(MPIDI_CH4U_AM_ACC_DAT_REQ,
                                             &MPIDI_CH4I_am_acc_data_origin_cmpl_handler,
                                             &MPIDI_CH4I_am_acc_data_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4R_init_comm(comm_world);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4R_init_comm(comm_self);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_CH4I_map_create(&MPIDI_CH4_Global.win_map);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4R_finalize()
{
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FINALIZE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FINALIZE);
    MPIDI_CH4I_map_destroy(MPIDI_CH4_Global.win_map);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FINALIZE);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_alloc_mem
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void *MPIDI_CH4R_alloc_mem(size_t size, MPID_Info * info_ptr)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ALLOC_MEM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ALLOC_MEM);
    void *p;
    p = MPIU_Malloc(size);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ALLOC_MEM);
    return p;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_free_mem
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_free_mem(void *ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FREE_MEM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FREE_MEM);
    MPIU_Free(ptr);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FREE_MEM);
    return mpi_errno;
}


#endif /* MPIDCH4_INIT_H_INCLUDED */
