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
#ifndef CH4R_INIT_H_INCLUDED
#define CH4R_INIT_H_INCLUDED

#include "ch4_impl.h"
#include "ch4i_util.h"
#include "ch4r_buf.h"
#include "ch4r_callbacks.h"
#include "mpl_uthash.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_init_comm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_init_comm(MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS, comm_idx, subcomm_type,is_localcomm;
    MPIDI_CH4U_rreq_t **uelist;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_INIT_COMM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_INIT_COMM);

    /*
      Prevents double initialization of some special communicators.

      comm_world and comm_self may exhibit this function twice, first during MPIDI_CH4U_init
      and the second during MPIR_Comm_commit in MPIDI_Init.
      If there is an early arrival of an unexpected message before the second visit,
      the following code will wipe out the unexpected queue andthe message is lost forever.
    */
    if (unlikely(MPIDI_CH4_Global.is_ch4u_initialized &&
                 (comm == MPIR_Process.comm_world || comm == MPIR_Process.comm_self)))
        goto fn_exit;

    comm_idx     = MPIDI_CH4U_get_context_index(comm->recvcontext_id);
    subcomm_type = MPIR_CONTEXT_READ_FIELD(SUBCOMM, comm->recvcontext_id);
    is_localcomm = MPIR_CONTEXT_READ_FIELD(IS_LOCALCOMM, comm->recvcontext_id);

    MPIU_Assert(subcomm_type <= 3);
    MPIU_Assert(is_localcomm <= 1);
    MPIDI_CH4_Global.comm_req_lists[comm_idx].comm[is_localcomm][subcomm_type] = comm;
    MPIDI_CH4U_COMM(comm, posted_list) = NULL;
    MPIDI_CH4U_COMM(comm, unexp_list)  = NULL;

    uelist = MPIDI_CH4U_context_id_to_uelist(comm->context_id);
    if (*uelist) {
        MPIDI_CH4U_rreq_t *curr, *tmp;
        MPL_DL_FOREACH_SAFE(*uelist,
                            curr, tmp) {
            MPL_DL_DELETE(*uelist, curr);
            MPIR_Comm_add_ref(comm); /* +1 for each entry in unexp_list */
            MPL_DL_APPEND(MPIDI_CH4U_COMM(comm, unexp_list), curr);
        }
        *uelist = NULL;
    }

    MPIDI_CH4U_COMM(comm, window_instance) = 0;
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT_COMM);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_destroy_comm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_destroy_comm(MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS, comm_idx, subcomm_type, is_localcomm;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_DESTROY_COMM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_DESTROY_COMM);

    comm_idx     = MPIDI_CH4U_get_context_index(comm->recvcontext_id);
    subcomm_type = MPIR_CONTEXT_READ_FIELD(SUBCOMM, comm->recvcontext_id);
    is_localcomm = MPIR_CONTEXT_READ_FIELD(IS_LOCALCOMM, comm->recvcontext_id);

    MPIU_Assert(subcomm_type <=3);
    MPIU_Assert(is_localcomm <= 1);
    MPIU_Assert(MPIDI_CH4_Global.comm_req_lists[comm_idx].comm[is_localcomm][subcomm_type] != NULL);

    if (MPIDI_CH4_Global.comm_req_lists[comm_idx].comm[subcomm_type]) {
        MPIU_Assert(MPIDI_CH4_Global.comm_req_lists[comm_idx].comm[is_localcomm][subcomm_type]->dev.ch4.ch4u.posted_list == NULL);
        MPIU_Assert(MPIDI_CH4_Global.comm_req_lists[comm_idx].comm[is_localcomm][subcomm_type]->dev.ch4.ch4u.unexp_list == NULL);
    }
    MPIDI_CH4_Global.comm_req_lists[comm_idx].comm[is_localcomm][subcomm_type] = NULL;


    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_DESTROY_COMM);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_init(MPIR_Comm * comm_world, MPIR_Comm * comm_self,
                                   int num_contexts, void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_INIT);

    MPIDI_CH4_Global.is_ch4u_initialized = 0;

    MPIDI_CH4_Global.comm_req_lists = (MPIDI_CH4U_comm_req_list_t *)
        MPL_calloc(MPIR_MAX_CONTEXT_MASK*MPIR_CONTEXT_INT_BITS,sizeof(MPIDI_CH4U_comm_req_list_t));
#ifndef MPIDI_CH4U_USE_PER_COMM_QUEUE
    MPIDI_CH4_Global.posted_list = NULL;
    MPIDI_CH4_Global.unexp_list = NULL;
#endif

    MPIDI_CH4_Global.cmpl_list = NULL;
    OPA_store_int(&MPIDI_CH4_Global.exp_seq_no, 0);
    OPA_store_int(&MPIDI_CH4_Global.nxt_seq_no, 0);

    MPIDI_CH4_Global.buf_pool = MPIDI_CH4U_create_buf_pool(MPIDI_CH4I_BUF_POOL_NUM,
                                                           MPIDI_CH4I_BUF_POOL_SZ);
    MPIU_Assert(MPIDI_CH4_Global.buf_pool);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_SEND,
                                             &MPIDI_CH4U_send_origin_cmpl_handler,
                                             &MPIDI_CH4U_send_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_SEND_LONG_REQ,
                                             NULL /* Injection only */,
                                             &MPIDI_CH4U_send_long_req_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_SEND_LONG_ACK,
                                             NULL /* Injection only */,
                                             &MPIDI_CH4U_send_long_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_SEND_LONG_LMT,
                                             &MPIDI_CH4U_send_long_lmt_origin_cmpl_handler,
                                             &MPIDI_CH4U_send_long_lmt_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_SSEND_REQ,
                                             &MPIDI_CH4U_send_origin_cmpl_handler,
                                             &MPIDI_CH4U_ssend_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_SSEND_ACK,
                                             &MPIDI_CH4U_ssend_ack_origin_cmpl_handler,
                                             &MPIDI_CH4U_ssend_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_PUT_REQ,
                                             &MPIDI_CH4U_put_origin_cmpl_handler,
                                             &MPIDI_CH4U_put_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_PUT_ACK,
                                             NULL,
                                             &MPIDI_CH4U_put_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_GET_REQ,
                                             &MPIDI_CH4U_get_origin_cmpl_handler,
                                             &MPIDI_CH4U_get_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_GET_ACK,
                                             &MPIDI_CH4U_get_ack_origin_cmpl_handler,
                                             &MPIDI_CH4U_get_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_CSWAP_REQ,
                                             &MPIDI_CH4U_cswap_origin_cmpl_handler,
                                             &MPIDI_CH4U_cswap_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_CSWAP_ACK,
                                             &MPIDI_CH4U_cswap_ack_origin_cmpl_handler,
                                             &MPIDI_CH4U_cswap_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_ACC_REQ,
                                             &MPIDI_CH4U_acc_origin_cmpl_handler,
                                             &MPIDI_CH4U_handle_acc_request);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_ACC_ACK,
                                             NULL,
                                             &MPIDI_CH4U_acc_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_GET_ACC_ACK,
                                             &MPIDI_CH4U_get_acc_ack_origin_cmpl_handler,
                                             &MPIDI_CH4U_get_acc_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_WIN_CTRL,
                                             NULL,
                                             &MPIDI_CH4U_win_ctrl_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);


    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_PUT_IOV_REQ,
                                             &MPIDI_CH4U_put_iov_origin_cmpl_handler,
                                             &MPIDI_CH4U_put_iov_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_PUT_IOV_ACK,
                                             NULL,
                                             &MPIDI_CH4U_put_iov_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_PUT_DAT_REQ,
                                             &MPIDI_CH4U_put_data_origin_cmpl_handler,
                                             &MPIDI_CH4U_put_data_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_ACC_IOV_REQ,
                                             &MPIDI_CH4U_acc_iov_origin_cmpl_handler,
                                             &MPIDI_CH4U_acc_iov_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_ACC_IOV_ACK,
                                             NULL,
                                             &MPIDI_CH4U_acc_iov_ack_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4_NM_reg_hdr_handler(MPIDI_CH4U_ACC_DAT_REQ,
                                             &MPIDI_CH4U_acc_data_origin_cmpl_handler,
                                             &MPIDI_CH4U_acc_data_target_handler);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4U_init_comm(comm_world);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_CH4U_init_comm(comm_self);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_CH4_Global.win_hash = NULL;

    MPIDI_CH4_Global.is_ch4u_initialized = 1;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_INIT);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4U_finalize()
{
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FINALIZE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FINALIZE);
    MPIDI_CH4_Global.is_ch4u_initialized = 0;
    MPL_HASH_CLEAR(dev.ch4u.hash_handle, MPIDI_CH4_Global.win_hash);
    MPIDI_CH4R_destroy_buf_pool(MPIDI_CH4_Global.buf_pool);
    MPL_free(MPIDI_CH4_Global.comm_req_lists);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FINALIZE);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_alloc_mem
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void *MPIDI_CH4U_alloc_mem(size_t size, MPIR_Info * info_ptr)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ALLOC_MEM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ALLOC_MEM);
    void *p;
    p = MPL_malloc(size);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ALLOC_MEM);
    return p;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_free_mem
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_free_mem(void *ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FREE_MEM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FREE_MEM);
    MPL_free(ptr);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FREE_MEM);
    return mpi_errno;
}


#endif /* CH4R_INIT_H_INCLUDED */
