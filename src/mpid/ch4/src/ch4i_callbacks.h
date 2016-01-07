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
#ifndef MPIDCH4U_CALLBACKS_H_INCLUDED
#define MPIDCH4U_CALLBACKS_H_INCLUDED

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_progress_cmpl_list
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_CH4I_progress_cmpl_list(void)
{
    MPID_Request *req;
    MPIDI_CH4U_Devreq_t *curr, *tmp;
    MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_PROGRESS_CMPL_LIST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_PROGRESS_CMPL_LIST);

    /* MPIDI_CS_ENTER(); */
do_check_again:
    MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.cmpl_list, curr, tmp) {
        if (curr->seq_no == OPA_load_int(&MPIDI_CH4_Global.exp_seq_no)) {
            MPL_DL_DELETE(MPIDI_CH4_Global.cmpl_list, curr);
            req = container_of(curr, MPID_Request, dev.ch4.ch4u);
            cmpl_handler_fn = (MPIDI_netmod_am_completion_handler_fn) curr->cmpl_handler_fn;
            cmpl_handler_fn(req);
            goto do_check_again;
        }
    }
    /* MPIDI_CS_EXIT(); */
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_PROGRESS_CMPL_LIST);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_check_cmpl_order
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_check_cmpl_order(MPID_Request *req,
                                              MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4U_CHECK_CMPL_ORDER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_CHECK_CMPL_ORDER);

    if (MPIDI_CH4R_REQUEST(req, seq_no) == OPA_load_int(&MPIDI_CH4_Global.exp_seq_no)) {
        OPA_incr_int(&MPIDI_CH4_Global.exp_seq_no);
        return 1;
    }

    MPIDI_CH4R_REQUEST(req, cmpl_handler_fn) = (void *)cmpl_handler_fn;
    /* MPIDI_CS_ENTER(); */
    MPL_DL_APPEND(MPIDI_CH4_Global.cmpl_list, &req->dev.ch4.ch4u);
    /* MPIDI_CS_EXIT(); */

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_CHECK_CMPL_ORDER);
    return 0;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_send_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_send_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_SEND_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_SEND_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_SEND_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_ssend_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_ssend_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_SSEND_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_SSEND_ACK_TX_HANDLER);
    MPIDI_CH4R_complete_req(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_SSEND_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_ACK_TX_HANDLER);
    MPIDI_CH4R_complete_req(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS, i, c;
    size_t data_sz, offset;
    MPIDI_CH4U_get_ack_msg_t get_ack;
    struct iovec *iov;
    char *p_data;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_CMPL_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(req, MPIDI_CH4I_am_get_cmpl_handler))
        return mpi_errno;

    MPID_cc_incr(req->cc_ptr, &c);
    get_ack.greq_ptr = MPIDI_CH4R_REQUEST(req, greq.greq_ptr);
    if (MPIDI_CH4R_REQUEST(req, greq.n_iov) == 0) {
        mpi_errno = MPIDI_netmod_send_am_reply(MPIDI_CH4R_REQUEST(req, greq.reply_token),
                                               MPIDI_CH4R_GET_ACK,
                                               &get_ack, sizeof(get_ack),
                                               (void *)MPIDI_CH4R_REQUEST(req, greq.addr),
                                               MPIDI_CH4R_REQUEST(req, greq.count),
                                               MPIDI_CH4R_REQUEST(req, greq.datatype),
                                               req);
        MPIDI_CH4R_complete_req(req);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        goto fn_exit;
    }

    iov = (struct iovec *)MPIDI_CH4R_REQUEST(req, greq.dt_iov);

    data_sz = 0;
    for(i = 0; i < MPIDI_CH4R_REQUEST(req, greq.n_iov); i++) {
        data_sz += iov[i].iov_len;
    }

    p_data = (char *) MPIU_Malloc(data_sz);
    MPIU_Assert(p_data);

    offset = 0;
    for(i = 0; i < MPIDI_CH4R_REQUEST(req, greq.n_iov); i++) {
        MPIU_Memcpy(p_data + offset, iov[i].iov_base, iov[i].iov_len);
        offset += iov[i].iov_len;
    }

    MPIU_Free(MPIDI_CH4R_REQUEST(req, greq.dt_iov));
    MPIDI_CH4R_REQUEST(req, greq.dt_iov) = (void *)p_data;

    mpi_errno = MPIDI_netmod_send_am_reply(MPIDI_CH4R_REQUEST(req, greq.reply_token),
                                           MPIDI_CH4R_GET_ACK,
                                           &get_ack, sizeof(get_ack),
                                           p_data, data_sz, MPI_BYTE, req);
    MPIDI_CH4R_complete_req(req);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    MPIDI_CH4I_progress_cmpl_list();
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_CMPL_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_ACK_TX_HANDLER);
    MPIDI_CH4R_complete_req(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_acc_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_ACC_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_ACC_ACK_TX_HANDLER);
    MPIU_Free(MPIDI_CH4R_REQUEST(req, areq.data));

    win = (MPID_Win *)MPIDI_CH4R_REQUEST(req, areq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_complete_req(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_ACC_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_CSWAP_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_CSWAP_ACK_TX_HANDLER);

    MPIU_Free(MPIDI_CH4R_REQUEST(req, creq.data));
    win = (MPID_Win *)MPIDI_CH4R_REQUEST(req, creq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_complete_req(req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_CSWAP_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_ack_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_ack_origin_cmpl_handler(MPID_Request * req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_ACK_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_ACK_TX_HANDLER);

    if (MPIDI_CH4R_REQUEST(req, greq.dt_iov)) {
        MPIU_Free(MPIDI_CH4R_REQUEST(req, greq.dt_iov));
    }

    win = (MPID_Win *)MPIDI_CH4R_REQUEST(req, greq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_complete_req(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_ACK_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_CSWAP_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_CSWAP_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_CSWAP_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_data_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_data_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_DATA_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_DATA_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_DATA_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_data_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_data_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_DATA_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_DATA_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_DATA_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_iov_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_iov_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_IOV_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_IOV_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_IOV_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_iov_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_iov_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_IOV_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_IOV_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_IOV_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_origin_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_origin_cmpl_handler(MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_TX_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_TX_HANDLER);
    MPIDI_CH4R_complete_req(sreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_TX_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_reply_ssend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_reply_ssend(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_Ssend_ack_msg_t ack_msg;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_REPLY_SSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_REPLY_SSEND);
    MPID_cc_incr(rreq->cc_ptr, &c);
    ack_msg.sreq_ptr = MPIDI_CH4R_REQUEST(rreq, rreq.peer_req_ptr);

    mpi_errno = MPIDI_netmod_send_am_hdr_reply(MPIDI_CH4R_REQUEST(rreq, rreq.reply_token),
                                               MPIDI_CH4R_SSEND_ACK, &ack_msg, sizeof(ack_msg),
                                               rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_REPLY_SSEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_ack_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_ack_put(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_put_ack_msg_t ack_msg;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACK_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACK_PUT);

    ack_msg.preq_ptr = MPIDI_CH4R_REQUEST(rreq, preq.preq_ptr);
    mpi_errno = MPIDI_netmod_inject_am_hdr_reply(MPIDI_CH4R_REQUEST(rreq, preq.reply_token),
                                                 MPIDI_CH4R_PUT_ACK, &ack_msg, sizeof(ack_msg));
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACK_PUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_ack_cswap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_ack_cswap(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_cswap_ack_msg_t ack_msg;
    void *result_addr;
    size_t data_sz;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACK_CSWAP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACK_CSWAP);

    MPIDI_Datatype_check_size(MPIDI_CH4R_REQUEST(rreq, creq.datatype), 1, data_sz);
    result_addr = ((char *)MPIDI_CH4R_REQUEST(rreq, creq.data)) + data_sz;

    MPID_cc_incr(rreq->cc_ptr, &c);
    ack_msg.req_ptr = MPIDI_CH4R_REQUEST(rreq, creq.creq_ptr);

    mpi_errno = MPIDI_netmod_send_am_reply(MPIDI_CH4R_REQUEST(rreq, creq.reply_token),
                                           MPIDI_CH4R_CSWAP_ACK,
                                           &ack_msg, sizeof(ack_msg),
                                           result_addr, 1,
                                           MPIDI_CH4R_REQUEST(rreq, creq.datatype),
                                           rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACK_CSWAP);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_ack_acc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_ack_acc(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_acc_ack_msg_t ack_msg;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACK_ACC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACK_ACC);

    ack_msg.req_ptr = MPIDI_CH4R_REQUEST(rreq, areq.req_ptr);
    mpi_errno = MPIDI_netmod_inject_am_hdr_reply(MPIDI_CH4R_REQUEST(rreq, areq.reply_token),
                                                 MPIDI_CH4R_ACC_ACK, &ack_msg, sizeof(ack_msg));

    win = (MPID_Win *)MPIDI_CH4R_REQUEST(rreq, areq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACK_ACC);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_ack_get_acc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_ack_get_acc(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4U_acc_ack_msg_t ack_msg;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACK_GET_ACC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACK_GET_ACC);

    MPID_cc_incr(rreq->cc_ptr, &c);
    ack_msg.req_ptr = MPIDI_CH4R_REQUEST(rreq, areq.req_ptr);

    mpi_errno = MPIDI_netmod_send_am_reply(MPIDI_CH4R_REQUEST(rreq, areq.reply_token),
                                           MPIDI_CH4R_GET_ACC_ACK,
                                           &ack_msg, sizeof(ack_msg),
                                           MPIDI_CH4R_REQUEST(rreq, areq.data),
                                           MPIDI_CH4R_REQUEST(rreq, areq.data_sz),
                                           MPI_BYTE, rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACK_GET_ACC);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_unexp_mrecv_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_unexp_mrecv_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    uint64_t msg_tag;
    size_t message_sz;
    MPI_Aint last;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPIDI_msg_sz_t data_sz, dt_sz;
    MPID_Segment *segment_ptr;
    void *buf;
    int count;
    MPI_Datatype datatype;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_UNEXP_MRECV_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_UNEXP_MRECV_CMPL_HANDLER);

    msg_tag = MPIDI_CH4R_REQUEST(rreq, tag);
    rreq->status.MPI_SOURCE = MPIDI_CH4R_get_source(msg_tag);
    rreq->status.MPI_TAG = MPIDI_CH4R_get_tag(msg_tag);

    buf = MPIDI_CH4R_REQUEST(rreq, rreq.mrcv_buffer);
    count = MPIDI_CH4R_REQUEST(rreq, rreq.mrcv_count);
    datatype = MPIDI_CH4R_REQUEST(rreq, rreq.mrcv_datatype);

    message_sz = MPIDI_CH4R_REQUEST(rreq, count);
    MPID_Datatype_get_size_macro(datatype, dt_sz);

    if (message_sz > count * dt_sz) {
        rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
    }
    else {
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        count = message_sz / dt_sz;
    }

    MPIR_STATUS_SET_COUNT(rreq->status, count * dt_sz);
    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    if (!dt_contig) {
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, segment_ptr, 0);

        last = count * dt_sz;
        MPID_Segment_unpack(segment_ptr, 0, &last, MPIDI_CH4R_REQUEST(rreq, buffer));
        MPID_Segment_free(segment_ptr);
        if (last != (MPI_Aint)(count * dt_sz)) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __FUNCTION__, __LINE__,
                                             MPI_ERR_TYPE, "**dtypemismatch", 0);
            rreq->status.MPI_ERROR = mpi_errno;
        }
    }
    else {
        MPIU_Memcpy((char *) buf + dt_true_lb, MPIDI_CH4R_REQUEST(rreq, buffer), data_sz);
    }

    MPIU_Free(MPIDI_CH4R_REQUEST(rreq, buffer));
    rreq->kind = MPID_REQUEST_RECV;

    if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_PEER_SSEND) {
        mpi_errno = MPIDI_CH4I_reply_ssend(rreq);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }
    MPIDI_CH4R_complete_req(rreq);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_UNEXP_MRECV_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_unexp_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_unexp_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS, comm_idx, c;
    MPID_Comm *root_comm;
    MPID_Request *match_req = NULL;
    uint64_t msg_tag;
    size_t count;
    MPI_Aint last;
    int dt_contig;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPIDI_msg_sz_t dt_sz;
    MPID_Segment *segment_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_UNEXP_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_UNEXP_CMPL_HANDLER);

    /* MPIDI_CS_ENTER(); */
    if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_UNEXP_DQUED) {
        if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_UNEXP_CLAIMED) {
            MPIDI_CH4I_unexp_mrecv_cmpl_handler(rreq);
        }
        /* MPIDI_CS_EXIT(); */
        goto fn_exit;
    }
    /* MPIDI_CS_EXIT(); */

    msg_tag = MPIDI_CH4R_REQUEST(rreq, tag);
    comm_idx = MPIDI_CH4R_get_context_index(MPIDI_CH4R_get_context(msg_tag));
    root_comm = MPIDI_CH4_Global.comm_req_lists[comm_idx].comm;

    if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_MATCHED) {
        match_req = (MPID_Request *) MPIDI_CH4R_REQUEST(rreq, rreq.match_req);
    } else {
        /* MPIDI_CS_ENTER(); */
        if (root_comm)
            match_req = MPIDI_CH4I_dequeue_posted(msg_tag, &MPIDI_CH4R_COMM(root_comm, posted_list));

        if (match_req)
            MPIDI_CH4I_delete_unexp(rreq, &MPIDI_CH4R_COMM(root_comm, unexp_list));
        /* MPIDI_CS_EXIT(); */
    }

    if (!match_req) {
        MPIDI_CH4R_REQUEST(rreq, status) &= ~MPIDI_CH4U_REQ_BUSY;
        goto fn_exit;
    }

    match_req->status.MPI_SOURCE = MPIDI_CH4R_get_source(msg_tag);
    match_req->status.MPI_TAG = MPIDI_CH4R_get_tag(msg_tag);

    MPIDI_Datatype_get_info(MPIDI_CH4R_REQUEST(match_req, count),
                            MPIDI_CH4R_REQUEST(match_req, datatype),
                            dt_contig, dt_sz, dt_ptr, dt_true_lb);
    MPID_Datatype_get_size_macro(MPIDI_CH4R_REQUEST(match_req, datatype), dt_sz);

    if (MPIDI_CH4R_REQUEST(rreq, count) > dt_sz * MPIDI_CH4R_REQUEST(match_req, count)) {
        rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        count = MPIDI_CH4R_REQUEST(match_req, count);
    }
    else {
        rreq->status.MPI_ERROR = MPI_SUCCESS;
        count = MPIDI_CH4R_REQUEST(rreq, count) / dt_sz;
    }

    MPIR_STATUS_SET_COUNT(match_req->status, count * dt_sz);
    MPIDI_CH4R_REQUEST(rreq, count) = count;

    if (!dt_contig) {
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Recv MPID_Segment_alloc");
        MPID_Segment_init(MPIDI_CH4R_REQUEST(match_req, buffer), count,
                          MPIDI_CH4R_REQUEST(match_req, datatype), segment_ptr, 0);

        last = count * dt_sz;
        MPID_Segment_unpack(segment_ptr, 0, &last, MPIDI_CH4R_REQUEST(rreq, buffer));
        MPID_Segment_free(segment_ptr);
        if (last != (MPI_Aint)(count * dt_sz)) {
            mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                             __FUNCTION__, __LINE__,
                                             MPI_ERR_TYPE, "**dtypemismatch", 0);
            match_req->status.MPI_ERROR = mpi_errno;
        }
    }
    else {
        MPIU_Memcpy((char *) MPIDI_CH4R_REQUEST(match_req, buffer) + dt_true_lb,
                    MPIDI_CH4R_REQUEST(rreq, buffer), count * dt_sz);
    }

    MPIDI_CH4R_REQUEST(rreq, status) &= ~MPIDI_CH4U_REQ_UNEXPECTED;
    if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_PEER_SSEND) {
        mpi_errno = MPIDI_CH4I_reply_ssend(rreq);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }

    dtype_release_if_not_builtin(MPIDI_CH4R_REQUEST(match_req, datatype));
    MPIU_Free(MPIDI_CH4R_REQUEST(rreq, buffer));
    MPIU_Object_release_ref(rreq, &c);
    MPIDI_CH4R_complete_req(rreq);
    MPIDI_CH4R_complete_req(match_req);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_UNEXP_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_recv_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_recv_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_RECV_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_RECV_CMPL_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(rreq, MPIDI_CH4I_am_recv_cmpl_handler))
        return mpi_errno;

    if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_RCV_NON_CONTIG) {
        MPIU_Free(MPIDI_CH4R_REQUEST(rreq, iov));
    }

    if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_UNEXPECTED) {
        mpi_errno = MPIDI_CH4I_unexp_cmpl_handler(rreq);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        goto fn_exit;
    }

    rreq->status.MPI_SOURCE = MPIDI_CH4R_get_source(MPIDI_CH4R_REQUEST(rreq, tag));
    rreq->status.MPI_TAG = MPIDI_CH4R_get_tag(MPIDI_CH4R_REQUEST(rreq, tag));

    if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_PEER_SSEND) {
        mpi_errno = MPIDI_CH4I_reply_ssend(rreq);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }

    dtype_release_if_not_builtin(MPIDI_CH4R_REQUEST(rreq, datatype));
    MPIDI_CH4R_complete_req(rreq);
  fn_exit:
    MPIDI_CH4I_progress_cmpl_list();
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_RECV_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_acc_ack_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_ack_cmpl_handler(MPID_Request * areq)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_ACC_ACK_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_ACC_ACK_CMPL_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(areq, MPIDI_CH4I_am_get_acc_ack_cmpl_handler))
        return mpi_errno;

    if (MPIDI_CH4R_REQUEST(areq, status) & MPIDI_CH4U_REQ_RCV_NON_CONTIG) {
        MPIU_Free(MPIDI_CH4R_REQUEST(areq, iov));
    }

    win = (MPID_Win *) MPIDI_CH4R_REQUEST(areq, areq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    dtype_release_if_not_builtin(MPIDI_CH4R_REQUEST(areq, areq.result_datatype));
    MPIDI_CH4R_complete_req(areq);

    MPIDI_CH4I_progress_cmpl_list();
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_ACC_ACK_CMPL_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_CMPL_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(rreq, MPIDI_CH4I_am_put_cmpl_handler))
        return mpi_errno;

    if (MPIDI_CH4R_REQUEST(rreq, status) & MPIDI_CH4U_REQ_RCV_NON_CONTIG) {
        MPIU_Free(MPIDI_CH4R_REQUEST(rreq, iov));
    }

    if (MPIDI_CH4R_REQUEST(rreq, preq.dt_iov)) {
        MPIU_Free(MPIDI_CH4R_REQUEST(rreq, preq.dt_iov));
    }

    mpi_errno = MPIDI_CH4I_ack_put(rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    win = (MPID_Win *) MPIDI_CH4R_REQUEST(rreq, preq.win_ptr);

    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_complete_req(rreq);
    MPIDI_CH4I_progress_cmpl_list();
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_iov_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_iov_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_put_iov_ack_msg_t ack_msg;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_IOV_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_IOV_CMPL_HANDLER);

    ack_msg.origin_preq_ptr = (uint64_t) MPIDI_CH4R_REQUEST(rreq, preq.preq_ptr);
    ack_msg.target_preq_ptr = (uint64_t) rreq;

    mpi_errno = MPIDI_netmod_inject_am_hdr_reply(MPIDI_CH4R_REQUEST(rreq, preq.reply_token),
                                                 MPIDI_CH4R_PUT_IOV_ACK,
                                                 &ack_msg, sizeof(ack_msg));
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_IOV_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_iov_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_iov_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_acc_iov_ack_msg_t ack_msg;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_IOV_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_IOV_CMPL_HANDLER);

    ack_msg.origin_preq_ptr = (uint64_t) MPIDI_CH4R_REQUEST(rreq, areq.req_ptr);
    ack_msg.target_preq_ptr = (uint64_t) rreq;

    mpi_errno = MPIDI_netmod_inject_am_hdr_reply(MPIDI_CH4R_REQUEST(rreq, areq.reply_token),
                                                 MPIDI_CH4R_ACC_IOV_ACK,
                                                 &ack_msg, sizeof(ack_msg));
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_IOV_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    void *compare_addr;
    void *origin_addr;
    size_t data_sz;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_CSWAP_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_CSWAP_CMPL_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(rreq, MPIDI_CH4I_am_cswap_cmpl_handler))
        return mpi_errno;

    MPIDI_Datatype_check_size(MPIDI_CH4R_REQUEST(rreq, creq.datatype), 1, data_sz);
    origin_addr = MPIDI_CH4R_REQUEST(rreq, creq.data);
    compare_addr = ((char *)MPIDI_CH4R_REQUEST(rreq, creq.data)) + data_sz;

    /* MPIDI_CS_ENTER(); */

    if (MPIR_Compare_equal((void *)MPIDI_CH4R_REQUEST(rreq, creq.addr), compare_addr,
                           MPIDI_CH4R_REQUEST(rreq, creq.datatype))) {
        MPIU_Memcpy(compare_addr, (void *)MPIDI_CH4R_REQUEST(rreq, creq.addr), data_sz);
        MPIU_Memcpy((void *)MPIDI_CH4R_REQUEST(rreq, creq.addr), origin_addr, data_sz);
    } else {
        MPIU_Memcpy(compare_addr, (void *)MPIDI_CH4R_REQUEST(rreq, creq.addr), data_sz);
    }

    /* MPIDI_CS_EXIT(); */

    mpi_errno = MPIDI_CH4I_ack_cswap(rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    MPIDI_CH4R_complete_req(rreq);
    MPIDI_CH4I_progress_cmpl_list();
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_CSWAP_CMPL_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_do_accumulate_op
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_do_accumulate_op(void *source_buf, int source_count, MPI_Datatype source_dtp,
                                   void *target_buf, int target_count, MPI_Datatype target_dtp,
                                   MPI_Aint stream_offset, MPI_Op acc_op)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_User_function *uop = NULL;
    MPI_Aint source_dtp_size = 0, source_dtp_extent = 0;
    int is_empty_source = FALSE;
    MPIDI_STATE_DECL(MPID_STATE_DO_ACCUMULATE_OP);

    MPIDI_FUNC_ENTER(MPID_STATE_DO_ACCUMULATE_OP);

    /* first Judge if source buffer is empty */
    if (acc_op == MPI_NO_OP)
        is_empty_source = TRUE;

    if (is_empty_source == FALSE) {
        MPIU_Assert(MPIR_DATATYPE_IS_PREDEFINED(source_dtp));
        MPID_Datatype_get_size_macro(source_dtp, source_dtp_size);
        MPID_Datatype_get_extent_macro(source_dtp, source_dtp_extent);
    }

    if (HANDLE_GET_KIND(acc_op) == HANDLE_KIND_BUILTIN) {
        /* get the function by indexing into the op table */
        uop = MPIR_OP_HDL_TO_FN(acc_op);
    }
    else {
        /* --BEGIN ERROR HANDLING-- */
        mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                                         FCNAME, __LINE__, MPI_ERR_OP,
                                         "**opnotpredefined", "**opnotpredefined %d", acc_op);
        return mpi_errno;
        /* --END ERROR HANDLING-- */
    }


    if (is_empty_source == TRUE || MPIR_DATATYPE_IS_PREDEFINED(target_dtp)) {
        /* directly apply op if target dtp is predefined dtp OR source buffer is empty */
        MPI_Aint real_stream_offset;
        void *curr_target_buf;

        if (is_empty_source == FALSE) {
            MPIU_Assert(source_dtp == target_dtp);
            real_stream_offset = (stream_offset / source_dtp_size) * source_dtp_extent;
            curr_target_buf = (void *) ((char *) target_buf + real_stream_offset);
        }
        else {
            curr_target_buf = target_buf;
        }

        (*uop) (source_buf, curr_target_buf, &source_count, &source_dtp);
    }
    else {
        /* derived datatype */
        MPID_Segment *segp;
        DLOOP_VECTOR *dloop_vec;
        MPI_Aint first, last;
        int vec_len, i, count;
        MPI_Aint type_extent, type_size;
        MPI_Datatype type;
        MPID_Datatype *dtp;
        MPI_Aint curr_len;
        void *curr_loc;
        int accumulated_count;

        segp = MPID_Segment_alloc();
        /* --BEGIN ERROR HANDLING-- */
        if (!segp) {
            mpi_errno =
                MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, FCNAME, __LINE__,
                                     MPI_ERR_OTHER, "**nomem", 0);
            MPIDI_FUNC_EXIT(MPID_STATE_DO_ACCUMULATE_OP);
            return mpi_errno;
        }
        /* --END ERROR HANDLING-- */
        MPID_Segment_init(NULL, target_count, target_dtp, segp, 0);
        first = stream_offset;
        last = first + source_count * source_dtp_size;

        MPID_Datatype_get_ptr(target_dtp, dtp);
        vec_len = dtp->max_contig_blocks * target_count + 1;
        /* +1 needed because Rob says so */
        dloop_vec = (DLOOP_VECTOR *)
            MPIU_Malloc(vec_len * sizeof(DLOOP_VECTOR));
        /* --BEGIN ERROR HANDLING-- */
        if (!dloop_vec) {
            mpi_errno =
                MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, FCNAME, __LINE__,
                                     MPI_ERR_OTHER, "**nomem", 0);
            MPIDI_FUNC_EXIT(MPID_STATE_DO_ACCUMULATE_OP);
            return mpi_errno;
        }
        /* --END ERROR HANDLING-- */

        MPID_Segment_pack_vector(segp, first, &last, dloop_vec, &vec_len);

        type = dtp->basic_type;
        MPIU_Assert(type != MPI_DATATYPE_NULL);

        MPIU_Assert(type == source_dtp);
        type_size = source_dtp_size;
        type_extent = source_dtp_extent;

        i = 0;
        curr_loc = dloop_vec[0].DLOOP_VECTOR_BUF;
        curr_len = dloop_vec[0].DLOOP_VECTOR_LEN;
        accumulated_count = 0;
        while (i != vec_len) {
            if (curr_len < type_size) {
                MPIU_Assert(i != vec_len);
                i++;
                curr_len += dloop_vec[i].DLOOP_VECTOR_LEN;
                continue;
            }

            MPIU_Assign_trunc(count, curr_len / type_size, int);

            (*uop) ((char *) source_buf + type_extent * accumulated_count,
                    (char *) target_buf + MPIU_PtrToAint(curr_loc), &count, &type);

            if (curr_len % type_size == 0) {
                i++;
                if (i != vec_len) {
                    curr_loc = dloop_vec[i].DLOOP_VECTOR_BUF;
                    curr_len = dloop_vec[i].DLOOP_VECTOR_LEN;
                }
            }
            else {
                curr_loc = (void *) ((char *) curr_loc + type_extent * count);
                curr_len -= type_size * count;
            }

            accumulated_count += count;
        }

        MPID_Segment_free(segp);
        MPIU_Free(dloop_vec);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_DO_ACCUMULATE_OP);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_handle_acc_cmpl
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_handle_acc_cmpl(MPID_Request * rreq, int do_get)
{
    int mpi_errno = MPI_SUCCESS, i;
    MPI_Aint basic_sz, count, offset = 0;
    struct iovec *iov;
    char *src_ptr, *original = NULL;
    size_t data_sz;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_HANDLE_ACC_CMPL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_HANDLE_ACC_CMPL);

    MPID_Datatype_get_size_macro(MPIDI_CH4R_REQUEST(rreq, areq.target_datatype), basic_sz);
    data_sz = MPIDI_CH4R_REQUEST(rreq, areq.data_sz);

    /* MPIDI_CS_ENTER(); */

    if (do_get) {
        original = (char *) MPIU_Malloc(data_sz);
        MPIU_Assert(original);
    }

    if (MPIDI_CH4R_REQUEST(rreq, areq.op) == MPI_NO_OP) {
        MPIDI_CH4R_REQUEST(rreq, areq.origin_count) =
            MPIDI_CH4R_REQUEST(rreq, areq.target_count);
        MPIDI_CH4R_REQUEST(rreq, areq.data_sz) = data_sz;
    }

    if (MPIDI_CH4R_REQUEST(rreq, areq.dt_iov) == NULL) {

        if (original) {
            MPIU_Memcpy(original, MPIDI_CH4R_REQUEST(rreq, areq.target_addr),
                        basic_sz * MPIDI_CH4R_REQUEST(rreq, areq.target_count));
        }

        mpi_errno = MPIDI_CH4I_do_accumulate_op(MPIDI_CH4R_REQUEST(rreq, areq.data),
                                                MPIDI_CH4R_REQUEST(rreq, areq.origin_count),
                                                MPIDI_CH4R_REQUEST(rreq, areq.origin_datatype),
                                                MPIDI_CH4R_REQUEST(rreq, areq.target_addr),
                                                MPIDI_CH4R_REQUEST(rreq, areq.target_count),
                                                MPIDI_CH4R_REQUEST(rreq, areq.target_datatype), 0,
                                                MPIDI_CH4R_REQUEST(rreq, areq.op));
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    } else {
        iov = (struct iovec *)MPIDI_CH4R_REQUEST(rreq, areq.dt_iov);
        src_ptr = (char *)MPIDI_CH4R_REQUEST(rreq, areq.data);
        for (i = 0; i < MPIDI_CH4R_REQUEST(rreq, areq.n_iov); i++) {
            count = iov[i].iov_len / basic_sz;
            MPIU_Assert(count > 0);

            if (original) {
                MPIU_Memcpy(original + offset, iov[i].iov_base, count * basic_sz);
                offset += count * basic_sz;
            }

            mpi_errno = MPIDI_CH4I_do_accumulate_op(src_ptr, count,
                                                    MPIDI_CH4R_REQUEST(rreq, areq.origin_datatype),
                                                    iov[i].iov_base, count,
                                                    MPIDI_CH4R_REQUEST(rreq, areq.target_datatype), 0,
                                                    MPIDI_CH4R_REQUEST(rreq, areq.op));
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);
            src_ptr += count * basic_sz;
        }
        MPIU_Free(iov);
    }

    /* MPIDI_CS_EXIT(); */
    if (MPIDI_CH4R_REQUEST(rreq, areq.data))
        MPIU_Free(MPIDI_CH4R_REQUEST(rreq, areq.data));

    if (original) {
        MPIDI_CH4R_REQUEST(rreq, areq.data) = original;
        mpi_errno = MPIDI_CH4I_ack_get_acc(rreq);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    } else {
        MPIDI_CH4R_REQUEST(rreq, areq.data) = NULL;
        mpi_errno = MPIDI_CH4I_ack_acc(rreq);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }
    MPIDI_CH4R_complete_req(rreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_HANDLE_ACC_CMPL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(rreq, MPIDI_CH4I_am_acc_cmpl_handler))
        return mpi_errno;

    mpi_errno = MPIDI_CH4I_handle_acc_cmpl(rreq, 0);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_CH4I_progress_cmpl_list();
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_acc_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_ACC_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_ACC_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(rreq, MPIDI_CH4I_am_get_acc_cmpl_handler))
        return mpi_errno;

    mpi_errno = MPIDI_CH4I_handle_acc_cmpl(rreq, 1);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_CH4I_progress_cmpl_list();
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_ACC_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_ack_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_ack_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *greq;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_ACK_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_ACK_CMPL_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(rreq, MPIDI_CH4I_am_get_ack_cmpl_handler))
        return mpi_errno;

    greq = (MPID_Request *) MPIDI_CH4R_REQUEST(rreq, greq.greq_ptr);
    if (MPIDI_CH4R_REQUEST(greq, status) & MPIDI_CH4U_REQ_RCV_NON_CONTIG) {
        MPIU_Free(MPIDI_CH4R_REQUEST(greq, iov));
    }

    win = (MPID_Win *) MPIDI_CH4R_REQUEST(greq, greq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_complete_req(greq);
    MPIDI_CH4R_complete_req(rreq);
    MPIDI_CH4I_progress_cmpl_list();
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_ACK_CMPL_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_ack_cmpl_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_ack_cmpl_handler(MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_CSWAP_ACK_CMPL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_CSWAP_ACK_CMPL_HANDLER);

    if (!MPIDI_CH4I_check_cmpl_order(rreq, MPIDI_CH4I_am_cswap_ack_cmpl_handler))
        return mpi_errno;

    win = (MPID_Win *) MPIDI_CH4R_REQUEST(rreq, creq.win_ptr);
    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIU_Free(MPIDI_CH4R_REQUEST(rreq, creq.data));
    MPIDI_CH4R_complete_req(rreq);

    MPIDI_CH4I_progress_cmpl_list();
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_CSWAP_ACK_CMPL_HANDLER);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_send_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_send_target_handler(void *am_hdr,
                                                    size_t am_hdr_sz,
                                                    void *reply_token,
                                                    void **data,
                                                    size_t * p_data_sz,
                                                    int *is_contig,
                                                    MPIDI_netmod_am_completion_handler_fn *
                                                    cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, comm_idx;
    MPID_Request *rreq = NULL;
    MPID_Comm *root_comm;
    size_t data_sz;

    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last, num_iov;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;

    MPIDI_CH4R_Hdr_t *hdr = (MPIDI_CH4R_Hdr_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_SEND_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_SEND_HANDLER);

    comm_idx = MPIDI_CH4R_get_context_index(MPIDI_CH4R_get_context(hdr->msg_tag));
    root_comm = MPIDI_CH4_Global.comm_req_lists[comm_idx].comm;

    if (root_comm) {
        /* MPIDI_CS_ENTER(); */
        rreq = MPIDI_CH4I_dequeue_posted(hdr->msg_tag, &MPIDI_CH4R_COMM(root_comm, posted_list));
        /* MPIDI_CS_EXIT(); */
    }

    if (rreq == NULL) {
        rreq = MPIDI_CH4R_create_req();
        rreq->kind = MPID_REQUEST_RECV;
        MPIDI_CH4R_REQUEST(rreq, buffer) = (char *) MPIU_Malloc(*p_data_sz);
        MPIDI_CH4R_REQUEST(rreq, datatype) = MPI_BYTE;
        MPIDI_CH4R_REQUEST(rreq, count) = *p_data_sz;
        MPIDI_CH4R_REQUEST(rreq, tag) = hdr->msg_tag;
        MPIDI_CH4R_REQUEST(rreq, status) |= MPIDI_CH4U_REQ_BUSY;
        MPIDI_CH4R_REQUEST(rreq, status) |= MPIDI_CH4U_REQ_UNEXPECTED;
        /* MPIDI_CS_ENTER(); */
        if (root_comm)
            MPIDI_CH4I_enqueue_unexp(rreq, &MPIDI_CH4R_COMM(root_comm, unexp_list));
        else
            MPIDI_CH4I_enqueue_unexp(rreq, &MPIDI_CH4_Global.comm_req_lists[comm_idx].unexp_list);
        /* MPIDI_CS_EXIT(); */
    }

    MPIDI_CH4R_REQUEST(rreq, tag) = hdr->msg_tag;
    *req = rreq;
    *cmpl_handler_fn = MPIDI_CH4I_am_recv_cmpl_handler;
    MPIDI_CH4R_REQUEST(rreq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    MPIDI_Datatype_get_info(MPIDI_CH4R_REQUEST(rreq, count),
                            MPIDI_CH4R_REQUEST(rreq, datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);
    *is_contig = dt_contig;

    if (dt_contig) {
        *p_data_sz = data_sz;
        *data = (char *) MPIDI_CH4R_REQUEST(rreq, buffer) + dt_true_lb;
    }
    else {
        segment_ptr = MPID_Segment_alloc();
        MPIU_Assert(segment_ptr);

        MPID_Segment_init(MPIDI_CH4R_REQUEST(rreq, buffer),
                          MPIDI_CH4R_REQUEST(rreq, count),
                          MPIDI_CH4R_REQUEST(rreq, datatype), segment_ptr, 0);

        last = data_sz;
        MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, &num_iov);
        n_iov = (int) num_iov;
        MPIU_Assert(n_iov > 0);
        MPIDI_CH4R_REQUEST(rreq, iov) = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
        MPIU_Assert(MPIDI_CH4R_REQUEST(rreq, iov));

        last = data_sz;
        MPID_Segment_pack_vector(segment_ptr, 0, &last, MPIDI_CH4R_REQUEST(rreq, iov), &n_iov);
        MPIU_Assert(last == (MPI_Aint)data_sz);
        *data = MPIDI_CH4R_REQUEST(rreq, iov);
        *p_data_sz = n_iov;
        MPIDI_CH4R_REQUEST(rreq, status) |= MPIDI_CH4U_REQ_RCV_NON_CONTIG;
        MPIU_Free(segment_ptr);
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_SEND_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_ssend_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_ssend_target_handler(void *am_hdr,
                                                     size_t am_hdr_sz,
                                                     void *reply_token,
                                                     void **data,
                                                     size_t * p_data_sz,
                                                     int *is_contig,
                                                     MPIDI_netmod_am_completion_handler_fn *
                                                     cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_CH4U_Ssend_req_msg_t *msg_hdr = (MPIDI_CH4U_Ssend_req_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_SSEND_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_SSEND_HANDLER);

    mpi_errno = MPIDI_CH4I_am_send_target_handler(am_hdr, sizeof(MPIDI_CH4R_Hdr_t),
                                                  reply_token, data, p_data_sz,
                                                  is_contig, cmpl_handler_fn, req);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIU_Assert(req);
    MPIDI_CH4R_REQUEST(*req, rreq.peer_req_ptr) = msg_hdr->sreq_ptr;
    MPIDI_CH4R_REQUEST(*req, rreq.reply_token) = reply_token;
    MPIDI_CH4R_REQUEST(*req, status) |= MPIDI_CH4U_REQ_PEER_SSEND;
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_SSEND_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_ssend_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_ssend_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                         void *reply_token, void **data,
                                                         size_t * p_data_sz, int *is_contig,
                                                         MPIDI_netmod_am_completion_handler_fn *
                                                         cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_CH4U_Ssend_ack_msg_t *msg_hdr = (MPIDI_CH4U_Ssend_ack_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_SSEND_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_SSEND_ACK_HANDLER);

    sreq = (MPID_Request *)msg_hdr->sreq_ptr;
    MPIDI_CH4R_complete_req(sreq);

    if (req)
        *req = NULL;
    if (cmpl_handler_fn)
        *cmpl_handler_fn = NULL;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_SSEND_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                       void *reply_token, void **data,
                                                       size_t * p_data_sz, int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_put_ack_msg_t *msg_hdr = (MPIDI_CH4U_put_ack_msg_t *) am_hdr;
    MPID_Win *win;
    MPID_Request *preq;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_ACK_HANDLER);

    preq = (MPID_Request *)msg_hdr->preq_ptr;
    win = (MPID_Win *) MPIDI_CH4R_REQUEST(preq, preq.win_ptr);

    if (MPIDI_CH4R_REQUEST(preq, preq.dt_iov)) {
        MPIU_Free(MPIDI_CH4R_REQUEST(preq, preq.dt_iov));
    }

    MPIDI_CH4R_complete_req(preq);

    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    if (req)
        *req = NULL;
    if (cmpl_handler_fn)
        *cmpl_handler_fn = NULL;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                       void *reply_token, void **data,
                                                       size_t * p_data_sz, int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_acc_ack_msg_t *msg_hdr = (MPIDI_CH4U_acc_ack_msg_t *) am_hdr;
    MPID_Win *win;
    MPID_Request *areq;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_ACK_HANDLER);

    areq = (MPID_Request *)msg_hdr->req_ptr;
    win = (MPID_Win *) MPIDI_CH4R_REQUEST(areq, areq.win_ptr);

    if (MPIDI_CH4R_REQUEST(areq, areq.dt_iov)) {
        MPIU_Free(MPIDI_CH4R_REQUEST(areq, areq.dt_iov));
    }

    MPIDI_CH4R_complete_req(areq);

    /* MPIDI_CS_ENTER(); */
    OPA_decr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    if (req)
        *req = NULL;
    if (cmpl_handler_fn)
        *cmpl_handler_fn = NULL;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_acc_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_acc_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                       void *reply_token, void **data,
                                                       size_t * p_data_sz, int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_acc_ack_msg_t *msg_hdr = (MPIDI_CH4U_acc_ack_msg_t *) am_hdr;
    MPID_Request *areq;

    size_t data_sz;
    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last, num_iov;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_ACK_HANDLER);

    areq = (MPID_Request *)msg_hdr->req_ptr;

    if (MPIDI_CH4R_REQUEST(areq, areq.dt_iov)) {
        MPIU_Free(MPIDI_CH4R_REQUEST(areq, areq.dt_iov));
    }

    MPIDI_Datatype_get_info(MPIDI_CH4R_REQUEST(areq, areq.result_count),
                            MPIDI_CH4R_REQUEST(areq, areq.result_datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);
    *is_contig = dt_contig;

    if (dt_contig) {
        *p_data_sz = data_sz;
        *data = (char *) MPIDI_CH4R_REQUEST(areq, areq.result_addr) + dt_true_lb;
    }
    else {
        segment_ptr = MPID_Segment_alloc();
        MPIU_Assert(segment_ptr);

        MPID_Segment_init(MPIDI_CH4R_REQUEST(areq, areq.result_addr),
                          MPIDI_CH4R_REQUEST(areq, areq.result_count),
                          MPIDI_CH4R_REQUEST(areq, areq.result_datatype),
                          segment_ptr, 0);

        last = data_sz;
        MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, &num_iov);
        n_iov = (int) num_iov;
        MPIU_Assert(n_iov > 0);
        MPIDI_CH4R_REQUEST(areq, iov) = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
        MPIU_Assert(MPIDI_CH4R_REQUEST(areq, iov));

        last = data_sz;
        MPID_Segment_pack_vector(segment_ptr, 0, &last, MPIDI_CH4R_REQUEST(areq, iov), &n_iov);
        MPIU_Assert(last == (MPI_Aint)data_sz);
        *data = MPIDI_CH4R_REQUEST(areq, iov);
        *p_data_sz = n_iov;
        MPIDI_CH4R_REQUEST(areq, status) |= MPIDI_CH4U_REQ_RCV_NON_CONTIG;
        MPIU_Free(segment_ptr);
    }

    *req = areq;
    *cmpl_handler_fn = MPIDI_CH4I_am_get_acc_ack_cmpl_handler;
    MPIDI_CH4R_REQUEST(areq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_ack_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                       void *reply_token, void **data,
                                                       size_t * p_data_sz, int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_cswap_ack_msg_t *msg_hdr = (MPIDI_CH4U_cswap_ack_msg_t *) am_hdr;
    MPID_Request *creq;
    uint64_t data_sz;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_CSWAP_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_CSWAP_ACK_HANDLER);

    creq = (MPID_Request *)msg_hdr->req_ptr;
    MPIDI_Datatype_check_size(MPIDI_CH4R_REQUEST(creq, creq.datatype), 1, data_sz);
    *data = MPIDI_CH4R_REQUEST(creq, creq.result_addr);
    *p_data_sz = data_sz;
    *is_contig = 1;

    *req = creq;
    *cmpl_handler_fn = MPIDI_CH4I_am_cswap_ack_cmpl_handler;
    MPIDI_CH4R_REQUEST(creq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_CSWAP_ACK_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_lock_advance
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_win_lock_advance(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    struct MPIDI_CH4I_win_sync_lock *slock = &MPIDI_CH4R_WIN(win, sync).lock;
    struct MPIDI_CH4I_win_queue *q     = &slock->local.requested;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_LOCK_ADVANCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_LOCK_ADVANCE);

    if( (q->head != NULL) && ((slock->local.count == 0) ||
                              ((slock->local.type == MPI_LOCK_SHARED) &&
                               (q->head->type     == MPI_LOCK_SHARED)))) {
        struct MPIDI_CH4I_win_lock *lock = q->head;
        q->head = lock->next;

        if(q->head == NULL)
            q->tail = NULL;

        ++slock->local.count;
        slock->local.type = lock->type;

        MPIDI_CH4R_win_cntrl_msg_t msg;
        msg.win_id = MPIDI_CH4R_WIN(win, win_id);
        msg.origin_rank = win->comm_ptr->rank;

        if(lock->mtype == MPIDI_CH4U_WIN_LOCK)
            msg.type = MPIDI_CH4U_WIN_LOCK_ACK;
        else if(lock->mtype == MPIDI_CH4U_WIN_LOCKALL)
            msg.type = MPIDI_CH4U_WIN_LOCKALL_ACK;
        else
            MPIU_Assert(0);

        mpi_errno = MPIDI_netmod_inject_am_hdr(lock->rank, win->comm_ptr,
                                               MPIDI_CH4R_WIN_CTRL,
                                               &msg, sizeof (msg), NULL);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        MPIU_Free(lock);

        mpi_errno = MPIDI_CH4I_win_lock_advance(win);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_LOCK_ADVANCE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_lock_req_proc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_CH4I_win_lock_req_proc(const MPIDI_CH4R_win_cntrl_msg_t  *info,
                                                MPID_Win                   *win,
                                                unsigned                    peer)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_LOCK_REQ_PROC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_LOCK_REQ_PROC);

    struct MPIDI_CH4I_win_lock *lock = (struct MPIDI_CH4I_win_lock *)
        MPIU_Calloc(1, sizeof(struct MPIDI_CH4I_win_lock));

    if(info->type == MPIDI_CH4U_WIN_LOCK)
        lock->mtype = MPIDI_CH4U_WIN_LOCK;
    else if(info->type == MPIDI_CH4U_WIN_LOCKALL)
        lock->mtype = MPIDI_CH4U_WIN_LOCKALL;

    lock->rank                = info->origin_rank;
    lock->type                = info->lock_type;
    struct MPIDI_CH4I_win_queue *q = &MPIDI_CH4R_WIN(win, sync).lock.local.requested;
    MPIU_Assert((q->head != NULL) ^ (q->tail == NULL));

    if(q->tail == NULL)
        q->head = lock;
    else
        q->tail->next = lock;

    q->tail = lock;

    MPIDI_CH4I_win_lock_advance(win);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_LOCK_REQ_PROC);
    return;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_lock_ack_proc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_CH4I_win_lock_ack_proc(const MPIDI_CH4R_win_cntrl_msg_t *info,
                                                MPID_Win                   *win,
                                                unsigned                    peer)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_LOCK_ACK_PROC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_LOCK_ACK_PROC);

    if(info->type == MPIDI_CH4U_WIN_LOCK_ACK)
        MPIDI_CH4R_WIN(win, sync).lock.remote.locked += 1;
    else  if(info->type == MPIDI_CH4U_WIN_LOCKALL_ACK)
        MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked += 1;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_LOCK_ACK_PROC);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_unlock_ack_proc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_CH4I_win_unlock_proc(const MPIDI_CH4R_win_cntrl_msg_t *info,
                                              MPID_Win                   *win,
                                              unsigned                    peer)
{

    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_UNLOCK_ACK_PROC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_UNLOCK_ACK_PROC);

    --MPIDI_CH4R_WIN(win, sync).lock.local.count;
    MPIU_Assert((int)MPIDI_CH4R_WIN(win, sync).lock.local.count >= 0);
    MPIDI_CH4I_win_lock_advance(win);

    MPIDI_CH4R_win_cntrl_msg_t msg;
    msg.win_id = MPIDI_CH4R_WIN(win, win_id);
    msg.origin_rank = win->comm_ptr->rank;
    msg.type = MPIDI_CH4U_WIN_UNLOCK_ACK;

    mpi_errno = MPIDI_netmod_inject_am_hdr(peer, win->comm_ptr,
                                           MPIDI_CH4R_WIN_CTRL,
                                           &msg, sizeof (msg), NULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_UNLOCK_ACK_PROC);
    return;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_complete_proc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_CH4I_win_complete_proc(const MPIDI_CH4R_win_cntrl_msg_t *info,
                                                MPID_Win                   *win,
                                                unsigned                    peer)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_COMPLETE_PROC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_COMPLETE_PROC);

    ++MPIDI_CH4R_WIN(win, sync).sc.count;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_COMPLETE_PROC);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_post_proc
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_CH4I_win_post_proc(const MPIDI_CH4R_win_cntrl_msg_t *info,
                                            MPID_Win                   *win,
                                            unsigned                    peer)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_POST_PROC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_POST_PROC);

    ++MPIDI_CH4R_WIN(win, sync).pw.count;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_POST_PROC);
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_unlock_done_cb
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_CH4I_win_unlock_done_cb(const MPIDI_CH4R_win_cntrl_msg_t *info,
                                                 MPID_Win                   *win,
                                                 unsigned                    peer)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_UNLOCK_DONE_CB);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_UNLOCK_DONE_CB);

    if (MPIDI_CH4R_WIN(win, sync).origin_epoch_type == MPIDI_CH4R_EPOTYPE_LOCK) {
        MPIDI_CH4R_WIN(win, sync).lock.remote.locked--;
    } else if (MPIDI_CH4R_WIN(win, sync).origin_epoch_type == MPIDI_CH4R_EPOTYPE_LOCK_ALL) {
        MPIU_Assert((int)MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked > 0);
        MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked -= 1;
    } else {
        MPIU_Assert(0);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_UNLOCK_DONE_CB);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_win_ctrl_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_win_ctrl_target_handler(void *am_hdr, size_t am_hdr_sz,
                                                        void *reply_token, void **data,
                                                        size_t * p_data_sz, int *is_contig,
                                                        MPIDI_netmod_am_completion_handler_fn *
                                                        cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS, sender_rank;
    MPIDI_CH4R_win_cntrl_msg_t *msg_hdr = (MPIDI_CH4R_win_cntrl_msg_t *) am_hdr;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_WIN_CTRL_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_WIN_CTRL_HANDLER);

    win = (MPID_Win *)MPIDI_CH4I_map_lookup(MPIDI_CH4_Global.win_map, msg_hdr->win_id);
    sender_rank = msg_hdr->origin_rank;

    switch (msg_hdr->type) {
        char buff[32];

    case MPIDI_CH4U_WIN_LOCK:
    case MPIDI_CH4U_WIN_LOCKALL:
        MPIDI_CH4I_win_lock_req_proc(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_LOCK_ACK:
    case MPIDI_CH4U_WIN_LOCKALL_ACK:
        MPIDI_CH4I_win_lock_ack_proc(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_UNLOCK:
    case MPIDI_CH4U_WIN_UNLOCKALL:
        MPIDI_CH4I_win_unlock_proc(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_UNLOCK_ACK:
    case MPIDI_CH4U_WIN_UNLOCKALL_ACK:
        MPIDI_CH4I_win_unlock_done_cb(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_COMPLETE:
        MPIDI_CH4I_win_complete_proc(msg_hdr, win, sender_rank);
        break;

    case MPIDI_CH4U_WIN_POST:
        MPIDI_CH4I_win_post_proc(msg_hdr, win, sender_rank);
        break;

    default:
        MPL_snprintf(buff, sizeof(buff), "Invalid message type: %d\n", msg_hdr->type);
        MPID_Abort(NULL, MPI_ERR_INTERN, 1, buff);
    }

    if (req)
        *req = NULL;
    if (cmpl_handler_fn)
        *cmpl_handler_fn = NULL;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_WIN_CTRL_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_target_handler(void *am_hdr,
                                                   size_t am_hdr_sz,
                                                   void *reply_token,
                                                   void **data,
                                                   size_t * p_data_sz,
                                                   int *is_contig,
                                                    MPIDI_netmod_am_completion_handler_fn *
                                                   cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    size_t data_sz;
    struct iovec *iov, *dt_iov;

    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last, num_iov;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;
    MPID_Win *win;
    MPIDI_CH4U_put_msg_t *msg_hdr = (MPIDI_CH4U_put_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_HANDLER);

    rreq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    *req = rreq;

    MPIDI_CH4R_REQUEST(*req, preq.preq_ptr) = msg_hdr->preq_ptr;
    MPIDI_CH4R_REQUEST(*req, preq.reply_token) = reply_token;

    win = (MPID_Win *)MPIDI_CH4I_map_lookup(MPIDI_CH4_Global.win_map, msg_hdr->win_id);
    MPIU_Assert(win);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */
    MPIDI_CH4R_REQUEST(rreq, preq.win_ptr) = (uint64_t) win;

    *cmpl_handler_fn = MPIDI_CH4I_am_put_cmpl_handler;
    MPIDI_CH4R_REQUEST(rreq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    if (msg_hdr->n_iov) {
        dt_iov = (struct iovec *)MPIU_Malloc(sizeof(struct iovec) * msg_hdr->n_iov);
        MPIU_Assert(dt_iov);

        iov = (struct iovec *)((char *)am_hdr + sizeof(*msg_hdr));
        MPIU_Memcpy(dt_iov, iov, sizeof(struct iovec) * msg_hdr->n_iov);
        MPIDI_CH4R_REQUEST(rreq, preq.dt_iov) = dt_iov;
        MPIDI_CH4R_REQUEST(rreq, preq.n_iov) = msg_hdr->n_iov;
        *is_contig = 0;
        *data = iov;
        *p_data_sz = msg_hdr->n_iov;
        goto fn_exit;
    }

    MPIDI_CH4R_REQUEST(rreq, preq.dt_iov) = NULL;
    MPIDI_Datatype_get_info(msg_hdr->count, msg_hdr->datatype,
                            dt_contig, data_sz, dt_ptr, dt_true_lb);
    *is_contig = dt_contig;

    if (dt_contig) {
        *p_data_sz = data_sz;
        *data = (char *) (msg_hdr->addr + dt_true_lb);
    }
    else {
        segment_ptr = MPID_Segment_alloc();
        MPIU_Assert(segment_ptr);

        MPID_Segment_init((void *)msg_hdr->addr, msg_hdr->count, msg_hdr->datatype,
                          segment_ptr, 0);
        last = data_sz;
        MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, &num_iov);
        n_iov = (int) num_iov;
        MPIU_Assert(n_iov > 0);
        MPIDI_CH4R_REQUEST(rreq, iov) = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
        MPIU_Assert(MPIDI_CH4R_REQUEST(rreq, iov));

        last = data_sz;
        MPID_Segment_pack_vector(segment_ptr, 0, &last, MPIDI_CH4R_REQUEST(rreq, iov), &n_iov);
        MPIU_Assert(last == (MPI_Aint)data_sz);
        *data = MPIDI_CH4R_REQUEST(rreq, iov);
        *p_data_sz = n_iov;
        MPIDI_CH4R_REQUEST(rreq, status) |= MPIDI_CH4U_REQ_RCV_NON_CONTIG;
        MPIU_Free(segment_ptr);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_iov_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_iov_target_handler(void *am_hdr,
                                                       size_t am_hdr_sz,
                                                       void *reply_token,
                                                       void **data,
                                                       size_t * p_data_sz,
                                                       int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    struct iovec *dt_iov;

    MPID_Win *win;
    MPIDI_CH4U_put_msg_t *msg_hdr = (MPIDI_CH4U_put_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_IOV_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_IOV_HANDLER);

    rreq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    *req = rreq;

    MPIDI_CH4R_REQUEST(*req, preq.preq_ptr) = msg_hdr->preq_ptr;
    MPIDI_CH4R_REQUEST(*req, preq.reply_token) = reply_token;

    win = (MPID_Win *)MPIDI_CH4I_map_lookup(MPIDI_CH4_Global.win_map, msg_hdr->win_id);
    MPIU_Assert(win);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */
    MPIDI_CH4R_REQUEST(rreq, preq.win_ptr) = (uint64_t) win;

    *cmpl_handler_fn = MPIDI_CH4I_am_put_iov_cmpl_handler;
    MPIDI_CH4R_REQUEST(rreq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    MPIU_Assert(msg_hdr->n_iov);
    dt_iov = (struct iovec *)MPIU_Malloc(sizeof(struct iovec) * msg_hdr->n_iov);
    MPIU_Assert(dt_iov);

    MPIDI_CH4R_REQUEST(rreq, preq.dt_iov) = dt_iov;
    MPIDI_CH4R_REQUEST(rreq, preq.n_iov) = msg_hdr->n_iov;
    *is_contig = 1;
    *data = dt_iov;
    *p_data_sz = msg_hdr->n_iov * sizeof(struct iovec);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_IOV_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_iov_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_iov_ack_target_handler(void *am_hdr,
                                                       size_t am_hdr_sz,
                                                       void *reply_token,
                                                       void **data,
                                                       size_t * p_data_sz,
                                                       int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq, *origin_req;
    MPIDI_CH4U_put_iov_ack_msg_t *msg_hdr = (MPIDI_CH4U_put_iov_ack_msg_t *) am_hdr;
    MPIDI_CH4U_put_dat_msg_t dat_msg;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_IOV_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_IOV_ACK_HANDLER);

    rreq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;

    origin_req = (MPID_Request *)msg_hdr->origin_preq_ptr;
    dat_msg.preq_ptr = msg_hdr->target_preq_ptr;
    mpi_errno = MPIDI_netmod_send_am_reply(reply_token,
                                           MPIDI_CH4R_PUT_DAT_REQ,
                                           &dat_msg, sizeof(dat_msg),
                                           MPIDI_CH4R_REQUEST(origin_req, preq.origin_addr),
                                           MPIDI_CH4R_REQUEST(origin_req, preq.origin_count),
                                           MPIDI_CH4R_REQUEST(origin_req, preq.origin_datatype),
                                           rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    dtype_release_if_not_builtin(MPIDI_CH4R_REQUEST(origin_req, preq.origin_datatype));

    *cmpl_handler_fn = NULL;
    *req = NULL;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_IOV_ACK_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_iov_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_iov_ack_target_handler(void *am_hdr,
                                                       size_t am_hdr_sz,
                                                       void *reply_token,
                                                       void **data,
                                                       size_t * p_data_sz,
                                                       int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq, *origin_req;
    MPIDI_CH4U_acc_iov_ack_msg_t *msg_hdr = (MPIDI_CH4U_acc_iov_ack_msg_t *) am_hdr;
    MPIDI_CH4U_acc_dat_msg_t dat_msg;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_IOV_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_IOV_ACK_HANDLER);

    rreq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;

    origin_req = (MPID_Request *)msg_hdr->origin_preq_ptr;
    dat_msg.preq_ptr = msg_hdr->target_preq_ptr;
    mpi_errno = MPIDI_netmod_send_am_reply(reply_token,
                                           MPIDI_CH4R_ACC_DAT_REQ,
                                           &dat_msg, sizeof(dat_msg),
                                           MPIDI_CH4R_REQUEST(origin_req, areq.origin_addr),
                                           MPIDI_CH4R_REQUEST(origin_req, areq.origin_count),
                                           MPIDI_CH4R_REQUEST(origin_req, areq.origin_datatype),
                                           rreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    dtype_release_if_not_builtin(MPIDI_CH4R_REQUEST(origin_req, areq.origin_datatype));

    *cmpl_handler_fn = NULL;
    *req = NULL;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_IOV_ACK_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_put_data_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_put_data_target_handler(void *am_hdr,
                                                        size_t am_hdr_sz,
                                                        void *reply_token,
                                                        void **data,
                                                        size_t * p_data_sz,
                                                        int *is_contig,
                                                        MPIDI_netmod_am_completion_handler_fn *
                                                        cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    MPIDI_CH4U_put_dat_msg_t *msg_hdr = (MPIDI_CH4U_put_dat_msg_t *) am_hdr;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_PUT_DATA_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_PUT_DATA_HANDLER);

    rreq = (MPID_Request *) msg_hdr->preq_ptr;

    *data = MPIDI_CH4R_REQUEST(rreq, preq.dt_iov);
    *is_contig = 0;
    *p_data_sz = MPIDI_CH4R_REQUEST(rreq, preq.n_iov);
    *req = rreq;
    *cmpl_handler_fn = MPIDI_CH4I_am_put_cmpl_handler;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_PUT_DATA_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_data_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_data_target_handler(void *am_hdr,
                                                        size_t am_hdr_sz,
                                                        void *reply_token,
                                                        void **data,
                                                        size_t * p_data_sz,
                                                        int *is_contig,
                                                        MPIDI_netmod_am_completion_handler_fn *
                                                        cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    size_t data_sz;
    void *p_data = NULL;
    MPIDI_CH4U_acc_dat_msg_t *msg_hdr = (MPIDI_CH4U_acc_dat_msg_t *) am_hdr;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_ACC_DATA_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_ACC_DATA_HANDLER);

    rreq = (MPID_Request *) msg_hdr->preq_ptr;
    MPIDI_Datatype_check_size(MPIDI_CH4R_REQUEST(rreq, areq.origin_datatype),
                              MPIDI_CH4R_REQUEST(rreq, areq.origin_count), data_sz);
    if (data_sz) {
        p_data = MPIU_Malloc(data_sz);
        MPIU_Assert(p_data);
    }

    MPIDI_CH4R_REQUEST(rreq, areq.data) = p_data;

    *data = p_data;
    *is_contig = 1;
    *p_data_sz = data_sz;
    *req = rreq;
    *cmpl_handler_fn = MPIDI_CH4R_REQUEST(rreq, areq.do_get) ?
        MPIDI_CH4I_am_get_acc_cmpl_handler : MPIDI_CH4I_am_acc_cmpl_handler;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_ACC_DATA_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_cswap_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_cswap_target_handler(void *am_hdr,
                                                     size_t am_hdr_sz,
                                                     void *reply_token,
                                                     void **data,
                                                     size_t * p_data_sz,
                                                     int *is_contig,
                                                     MPIDI_netmod_am_completion_handler_fn *
                                                     cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    size_t data_sz;
    MPID_Win *win;

    int dt_contig;
    void *p_data;

    MPIDI_CH4U_cswap_req_msg_t *msg_hdr = (MPIDI_CH4U_cswap_req_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_CSWAP_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_CSWAP_HANDLER);

    rreq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    *req = rreq;

    *cmpl_handler_fn = MPIDI_CH4I_am_cswap_cmpl_handler;
    MPIDI_CH4R_REQUEST(rreq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    MPIDI_Datatype_check_contig_size(msg_hdr->datatype, 1, dt_contig, data_sz);
    *is_contig = dt_contig;

    win = (MPID_Win *)MPIDI_CH4I_map_lookup(MPIDI_CH4_Global.win_map, msg_hdr->win_id);
    MPIU_Assert(win);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_REQUEST(*req, creq.win_ptr) = (uint64_t) win;
    MPIDI_CH4R_REQUEST(*req, creq.creq_ptr) = msg_hdr->req_ptr;
    MPIDI_CH4R_REQUEST(*req, creq.reply_token) = reply_token;
    MPIDI_CH4R_REQUEST(*req, creq.datatype) = msg_hdr->datatype;
    MPIDI_CH4R_REQUEST(*req, creq.addr) = msg_hdr->addr;

    MPIU_Assert(dt_contig == 1);
    p_data = MPIU_Malloc(data_sz * 2);
    MPIU_Assert(p_data);

    *p_data_sz = data_sz * 2;
    *data = p_data;
    MPIDI_CH4R_REQUEST(*req, creq.data) = p_data;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_CSWAP_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_handle_acc_request
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_handle_acc_request(void *am_hdr,
                                                size_t am_hdr_sz,
                                                void *reply_token,
                                                void **data,
                                                size_t * p_data_sz,
                                                int *is_contig,
                                                MPIDI_netmod_am_completion_handler_fn *
                                                cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    size_t data_sz;
    void *p_data = NULL;
    struct iovec *iov, *dt_iov;
    MPID_Win *win;

    MPIDI_CH4U_acc_req_msg_t *msg_hdr = (MPIDI_CH4U_acc_req_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_HANDLE_ACC_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_HANDLE_ACC_REQ);

    rreq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    *req = rreq;

    MPIDI_Datatype_check_size(msg_hdr->origin_datatype, msg_hdr->origin_count,
                              data_sz);
    if (data_sz) {
        p_data = MPIU_Malloc(data_sz);
        MPIU_Assert(p_data);
    }

    *cmpl_handler_fn = (msg_hdr->do_get) ? MPIDI_CH4I_am_get_acc_cmpl_handler :
        MPIDI_CH4I_am_acc_cmpl_handler;
    MPIDI_CH4R_REQUEST(rreq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    *is_contig = 1;
    *p_data_sz = data_sz;
    *data = p_data;

    win = (MPID_Win *)MPIDI_CH4I_map_lookup(MPIDI_CH4_Global.win_map, msg_hdr->win_id);
    MPIU_Assert(win);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_REQUEST(*req, areq.win_ptr) = (uint64_t) win;
    MPIDI_CH4R_REQUEST(*req, areq.req_ptr) = msg_hdr->req_ptr;
    MPIDI_CH4R_REQUEST(*req, areq.reply_token) = reply_token;
    MPIDI_CH4R_REQUEST(*req, areq.origin_datatype) = msg_hdr->origin_datatype;
    MPIDI_CH4R_REQUEST(*req, areq.target_datatype) = msg_hdr->target_datatype;
    MPIDI_CH4R_REQUEST(*req, areq.origin_count) = msg_hdr->origin_count;
    MPIDI_CH4R_REQUEST(*req, areq.target_count) = msg_hdr->target_count;
    MPIDI_CH4R_REQUEST(*req, areq.target_addr) = (void *)msg_hdr->target_addr;
    MPIDI_CH4R_REQUEST(*req, areq.op) = msg_hdr->op;
    MPIDI_CH4R_REQUEST(*req, areq.data) = p_data;
    MPIDI_CH4R_REQUEST(*req, areq.n_iov) = msg_hdr->n_iov;
    MPIDI_CH4R_REQUEST(*req, areq.data_sz) = msg_hdr->result_data_sz;

    if (!msg_hdr->n_iov) {
        MPIDI_CH4R_REQUEST(rreq, areq.dt_iov) = NULL;
        goto fn_exit;
    }

    dt_iov = (struct iovec *) MPIU_Malloc(sizeof(struct iovec) * msg_hdr->n_iov);
    MPIU_Assert(dt_iov);

    iov = (struct iovec *) ((char *) msg_hdr + sizeof(*msg_hdr));
    MPIU_Memcpy(dt_iov, iov, sizeof(struct iovec) * msg_hdr->n_iov);
    MPIDI_CH4R_REQUEST(rreq, areq.dt_iov) = dt_iov;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_HANDLE_ACC_REQ);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_acc_iov_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_acc_iov_target_handler(void *am_hdr,
                                                       size_t am_hdr_sz,
                                                       void *reply_token,
                                                       void **data,
                                                       size_t * p_data_sz,
                                                       int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    struct iovec *dt_iov;
    MPID_Win *win;

    MPIDI_CH4U_acc_req_msg_t *msg_hdr = (MPIDI_CH4U_acc_req_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_HANDLE_ACC_IOV_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_HANDLE_ACC_IOV_REQ);

    rreq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;
    *req = rreq;

    win = (MPID_Win *)MPIDI_CH4I_map_lookup(MPIDI_CH4_Global.win_map, msg_hdr->win_id);
    MPIU_Assert(win);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_REQUEST(*req, areq.win_ptr) = (uint64_t) win;
    MPIDI_CH4R_REQUEST(*req, areq.req_ptr) = msg_hdr->req_ptr;
    MPIDI_CH4R_REQUEST(*req, areq.reply_token) = reply_token;
    MPIDI_CH4R_REQUEST(*req, areq.origin_datatype) = msg_hdr->origin_datatype;
    MPIDI_CH4R_REQUEST(*req, areq.target_datatype) = msg_hdr->target_datatype;
    MPIDI_CH4R_REQUEST(*req, areq.origin_count) = msg_hdr->origin_count;
    MPIDI_CH4R_REQUEST(*req, areq.target_count) = msg_hdr->target_count;
    MPIDI_CH4R_REQUEST(*req, areq.target_addr) = (void *)msg_hdr->target_addr;
    MPIDI_CH4R_REQUEST(*req, areq.op) = msg_hdr->op;
    MPIDI_CH4R_REQUEST(*req, areq.n_iov) = msg_hdr->n_iov;
    MPIDI_CH4R_REQUEST(*req, areq.data_sz) = msg_hdr->result_data_sz;
    MPIDI_CH4R_REQUEST(*req, areq.do_get) = msg_hdr->do_get;

    dt_iov = (struct iovec *) MPIU_Malloc(sizeof(struct iovec) * msg_hdr->n_iov);
    MPIDI_CH4R_REQUEST(rreq, areq.dt_iov) = dt_iov;
    MPIU_Assert(dt_iov);

    *is_contig = 1;
    *p_data_sz = sizeof(struct iovec) * msg_hdr->n_iov;
    *data = (void *) dt_iov;

    *cmpl_handler_fn = MPIDI_CH4I_am_acc_iov_cmpl_handler;
    MPIDI_CH4R_REQUEST(rreq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_HANDLE_ACC_IOV_REQ);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_target_handler(void *am_hdr,
                                                   size_t am_hdr_sz,
                                                   void *reply_token,
                                                   void **data,
                                                   size_t * p_data_sz,
                                                   int *is_contig,
                                                   MPIDI_netmod_am_completion_handler_fn *
                                                   cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    MPIDI_CH4U_get_req_msg_t *msg_hdr = (MPIDI_CH4U_get_req_msg_t *) am_hdr;
    struct iovec *iov;
    MPID_Win *win;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_HANDLER);

    rreq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(rreq);
    rreq->kind = MPID_WIN_REQUEST;

    *req = rreq;
    *cmpl_handler_fn = MPIDI_CH4I_am_get_cmpl_handler;
    MPIDI_CH4R_REQUEST(rreq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    win = (MPID_Win *)MPIDI_CH4I_map_lookup(MPIDI_CH4_Global.win_map, msg_hdr->win_id);
    MPIU_Assert(win);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_REQUEST(rreq, greq.win_ptr) = (uint64_t) win;
    MPIDI_CH4R_REQUEST(rreq, greq.n_iov) = msg_hdr->n_iov;
    MPIDI_CH4R_REQUEST(rreq, greq.addr) = msg_hdr->addr;
    MPIDI_CH4R_REQUEST(rreq, greq.count) = msg_hdr->count;
    MPIDI_CH4R_REQUEST(rreq, greq.datatype) = msg_hdr->datatype;
    MPIDI_CH4R_REQUEST(rreq, greq.dt_iov) = NULL;
    MPIDI_CH4R_REQUEST(rreq, greq.greq_ptr) = msg_hdr->greq_ptr;
    MPIDI_CH4R_REQUEST(rreq, greq.reply_token) = reply_token;

    if (msg_hdr->n_iov) {
        iov = (struct iovec *) MPIU_Malloc(msg_hdr->n_iov * sizeof(*iov));
        MPIU_Assert(iov);

        *data = (void *)iov;
        *is_contig = 1;
        *p_data_sz = msg_hdr->n_iov * sizeof(*iov);
        MPIDI_CH4R_REQUEST(rreq, greq.dt_iov) = iov;
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_HANDLER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_get_ack_target_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_am_get_ack_target_handler(void *am_hdr,
                                                       size_t am_hdr_sz,
                                                       void *reply_token,
                                                       void **data,
                                                       size_t * p_data_sz,
                                                       int *is_contig,
                                                       MPIDI_netmod_am_completion_handler_fn *
                                                       cmpl_handler_fn, MPID_Request ** req)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL, *greq;
    size_t data_sz;

    int dt_contig, n_iov;
    MPI_Aint dt_true_lb, last, num_iov;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;

    MPIDI_CH4U_get_ack_msg_t *msg_hdr = (MPIDI_CH4U_get_ack_msg_t *) am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_ACK_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_ACK_HANDLER);

    greq = MPIDI_CH4R_create_win_req();
    MPIU_Assert(greq);
    greq->kind = MPID_WIN_REQUEST;
    *req = greq;

    rreq = (MPID_Request *) msg_hdr->greq_ptr;
    MPIU_Assert(rreq->kind == MPID_WIN_REQUEST);
    MPIDI_CH4R_REQUEST(greq, greq.greq_ptr) = (uint64_t) rreq;

    if (MPIDI_CH4R_REQUEST(rreq, greq.dt_iov)) {
        MPIU_Free(MPIDI_CH4R_REQUEST(rreq, greq.dt_iov));
    }

    *cmpl_handler_fn = MPIDI_CH4I_am_get_ack_cmpl_handler;
    MPIDI_CH4R_REQUEST(greq, seq_no) = OPA_fetch_and_add_int(&MPIDI_CH4_Global.nxt_seq_no, 1);

    MPIDI_Datatype_get_info(MPIDI_CH4R_REQUEST(rreq, greq.count),
                            MPIDI_CH4R_REQUEST(rreq, greq.datatype),
                            dt_contig, data_sz, dt_ptr, dt_true_lb);

    *is_contig = dt_contig;

    if (dt_contig) {
        *p_data_sz = data_sz;
        *data = (char *) (MPIDI_CH4R_REQUEST(rreq, greq.addr) + dt_true_lb);
    }
    else {
        segment_ptr = MPID_Segment_alloc();
        MPIU_Assert(segment_ptr);

        MPID_Segment_init((void *)MPIDI_CH4R_REQUEST(rreq, greq.addr),
                          MPIDI_CH4R_REQUEST(rreq, greq.count),
                          MPIDI_CH4R_REQUEST(rreq, greq.datatype),
                          segment_ptr, 0);
        last = data_sz;
        MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, &num_iov);
        n_iov = (int) num_iov;
        MPIU_Assert(n_iov > 0);
        MPIDI_CH4R_REQUEST(rreq, iov) = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
        MPIU_Assert(MPIDI_CH4R_REQUEST(rreq, iov));

        last = data_sz;
        MPID_Segment_pack_vector(segment_ptr, 0, &last, MPIDI_CH4R_REQUEST(rreq, iov), &n_iov);
        MPIU_Assert(last == (MPI_Aint)data_sz);
        *data = MPIDI_CH4R_REQUEST(rreq, iov);
        *p_data_sz = n_iov;
        MPIDI_CH4R_REQUEST(rreq, status) |= MPIDI_CH4U_REQ_RCV_NON_CONTIG;
        MPIU_Free(segment_ptr);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_ACK_HANDLER);
    return mpi_errno;
}

#endif
