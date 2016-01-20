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
#ifndef NETMOD_AM_OFI_EVENTS_H_INCLUDED
#define NETMOD_AM_OFI_EVENTS_H_INCLUDED

#include "am_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_short_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_short_am(MPIDI_AM_OFI_hdr_t        *msg_hdr,
                                               MPIDI_AM_OFI_reply_token_t reply_token)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    void *p_data;
    void *in_data;

    MPIDI_msg_sz_t data_sz, in_data_sz;
    MPIDI_CH4_NM_am_completion_handler_fn cmpl_handler_fn;
    struct iovec *iov;
    int i, is_contig, iov_len;
    size_t done, curr_len, rem;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SHORT_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SHORT_AM);

    p_data = in_data = (char *) msg_hdr->payload + msg_hdr->am_hdr_sz;
    in_data_sz = data_sz = msg_hdr->data_sz;

    MPIDI_Global.am_handlers[msg_hdr->handler_id] (msg_hdr->payload,
                                                   msg_hdr->am_hdr_sz,
                                                   reply_token.val,
                                                   &p_data, &data_sz,
                                                   &is_contig,
                                                   &cmpl_handler_fn,
                                                   &rreq);
    if (!rreq)
        goto fn_exit;

    if ((!p_data || !data_sz) && cmpl_handler_fn) {
        cmpl_handler_fn(rreq);
        goto fn_exit;
    }

    if (is_contig) {
        if (in_data_sz > data_sz) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        }
        else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }

        data_sz = MIN(data_sz, in_data_sz);
        MPIU_Memcpy(p_data, in_data, data_sz);
        MPIR_STATUS_SET_COUNT(rreq->status, data_sz);
    }
    else {
        done = 0;
        rem = in_data_sz;
        iov = (struct iovec *) p_data;
        iov_len = data_sz;
        for (i = 0; i < iov_len && rem > 0; i++) {
            curr_len = MIN(rem, iov[i].iov_len);
            MPIU_Memcpy(iov[i].iov_base, (char *) in_data + done, curr_len);
            rem -= curr_len;
            done += curr_len;
        }
        if (rem) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        }
        else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }
        MPIR_STATUS_SET_COUNT(rreq->status, done);
    }

    if (cmpl_handler_fn) {
        cmpl_handler_fn(rreq);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_SHORT_AM);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_short_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_short_am_hdr(MPIDI_AM_OFI_hdr_t         *msg_hdr,
                                                   void                       *am_hdr,
                                                   MPIDI_AM_OFI_reply_token_t  reply_token)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    MPIDI_CH4_NM_am_completion_handler_fn cmpl_handler_fn = NULL;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);

    MPIDI_Global.am_handlers[msg_hdr->handler_id] (am_hdr,
                                                   msg_hdr->am_hdr_sz,
                                                   reply_token.val,
                                                   NULL, NULL, NULL,
                                                   &cmpl_handler_fn, &rreq);
    if (!rreq)
        goto fn_exit;

    if (cmpl_handler_fn) {
        cmpl_handler_fn(rreq);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_do_rdma_read
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_do_rdma_read(void                       *dst,
                                            uint64_t                    src,
                                            size_t                      data_sz,
                                            MPIDI_AM_OFI_reply_token_t  reply_token,
                                            MPID_Request               *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    size_t done = 0, curr_len, rem = 0;
    MPIDI_CH4_NM_ofi_amrequest_t *am_req;
    MPID_Comm * comm;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_RDMA_READ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_RDMA_READ);

    rem = data_sz;
    while (done != data_sz) {
        curr_len = MIN(rem, MPIDI_Global.max_send);

        MPIU_Assert(sizeof(MPIDI_CH4_NM_ofi_amrequest_t) <= MPIDI_BUF_POOL_SZ);
        am_req = (MPIDI_CH4_NM_ofi_amrequest_t *)MPIDI_CH4R_get_buf(MPIDI_Global.buf_pool);
        MPIU_Assert(am_req);

        am_req->req_hdr  = AMREQ_OFI(rreq, req_hdr);
        am_req->event_id = MPIDI_EVENT_AM_READ;
        comm             = MPIDI_CH4R_context_id_to_comm(reply_token.data.context_id);
        MPIU_Assert(comm);
        FI_RC_RETRY_AM(fi_read(G_TXC_RMA(0),
                               (char *) dst + done,
                               curr_len, NULL,
                               _comm_to_phys(comm, reply_token.data.src_rank,MPIDI_API_TAG),
                               src + done,
                               AMREQ_OFI_HDR(rreq, lmt_info).rma_key,
                               &am_req->context), read);
        done += curr_len;
        rem -= curr_len;
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DO_RDMA_READ);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_long_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_long_am_hdr(MPIDI_AM_OFI_hdr_t         *msg_hdr,
                                                  MPIDI_AM_OFI_reply_token_t  reply_token)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_OFI_lmt_msg_pyld_t *lmt_msg;
    MPID_Request *rreq;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LONG_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_AM_HDR);

    rreq = MPIDI_AM_netmod_request_create();

    mpi_errno = MPIDI_netmod_am_ofi_init_req(NULL, 0, rreq);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

    lmt_msg = (MPIDI_OFI_lmt_msg_pyld_t *) msg_hdr->payload;
    AMREQ_OFI_HDR(rreq, lmt_info)    = *lmt_msg;
    AMREQ_OFI_HDR(rreq, msg_hdr)     = *msg_hdr;
    AMREQ_OFI_HDR(rreq, clientdata).reply_token = reply_token;
    AMREQ_OFI_HDR(rreq, rreq_ptr)    = (void *) rreq;
    AMREQ_OFI_HDR(rreq, am_hdr)      = MPIU_Malloc(msg_hdr->am_hdr_sz);
    AMREQ_OFI_HDR(rreq, lmt_cntr)    = ((msg_hdr->am_hdr_sz - 1) / MPIDI_Global.max_send) + 1;
    MPIDI_netmod_do_rdma_read(AMREQ_OFI_HDR(rreq, am_hdr),
                              lmt_msg->am_hdr_src,
                              msg_hdr->am_hdr_sz,
                              reply_token,
                              rreq);

fn_exit:
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_AM_HDR);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_long_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_long_hdr(MPIDI_AM_OFI_hdr_t         *msg_hdr,
                                               MPIDI_AM_OFI_reply_token_t  reply_token)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_OFI_lmt_msg_pyld_t *lmt_msg;
    MPID_Request *rreq;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LONG_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_HDR);

    rreq = MPIDI_AM_netmod_request_create();

    mpi_errno = MPIDI_netmod_am_ofi_init_req(NULL, 0, rreq);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

    lmt_msg = (MPIDI_OFI_lmt_msg_pyld_t *) msg_hdr->payload;
    AMREQ_OFI_HDR(rreq, lmt_info)    = *lmt_msg;
    AMREQ_OFI_HDR(rreq, msg_hdr)     = *msg_hdr;
    AMREQ_OFI_HDR(rreq, clientdata).reply_token = reply_token;
    AMREQ_OFI_HDR(rreq, rreq_ptr)    = (void *) rreq;

    AMREQ_OFI_HDR(rreq, am_hdr)      = MPIU_Malloc(msg_hdr->am_hdr_sz);
    AMREQ_OFI_HDR(rreq, lmt_cntr)    = ((msg_hdr->am_hdr_sz - 1) / MPIDI_Global.max_send) + 1;

    MPIDI_netmod_do_rdma_read(AMREQ_OFI_HDR(rreq, am_hdr), lmt_msg->am_hdr_src,
                              msg_hdr->am_hdr_sz, reply_token, rreq);

fn_exit:
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_HDR);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_do_handle_long_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_do_handle_long_am(MPIDI_AM_OFI_hdr_t         *msg_hdr,
                                                 MPIDI_OFI_lmt_msg_pyld_t   *lmt_msg,
                                                 void                       *am_hdr,
                                                 MPIDI_AM_OFI_reply_token_t  reply_token)
{
    int num_reads, i, iov_len, c, mpi_errno = MPI_SUCCESS, is_contig = 0;
    MPID_Request *rreq;
    void *p_data;
    size_t data_sz, rem, done, curr_len, in_data_sz;
    MPIDI_CH4_NM_am_completion_handler_fn cmpl_handler_fn;
    struct iovec *iov;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_HANDLE_LONG_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_HANDLE_LONG_AM);

    in_data_sz = data_sz = msg_hdr->data_sz;
    MPIDI_Global.am_handlers[msg_hdr->handler_id] (am_hdr,
                                                   msg_hdr->am_hdr_sz,
                                                   reply_token.val,
                                                   &p_data, &data_sz, &is_contig,
                                                   &cmpl_handler_fn, &rreq);
    if (!rreq)
        goto fn_exit;

    mpi_errno = MPIDI_netmod_am_ofi_init_req(NULL, 0, rreq);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);
    MPID_cc_incr(rreq->cc_ptr, &c);

    AMREQ_OFI_HDR(rreq, cmpl_handler_fn) = cmpl_handler_fn;
    if ((!p_data || !data_sz) && cmpl_handler_fn) {
        cmpl_handler_fn(rreq);
        MPIDI_AM_netmod_request_complete(rreq);
        goto fn_exit;
    }

    AMREQ_OFI_HDR(rreq, msg_hdr) = *msg_hdr;
    AMREQ_OFI_HDR(rreq, lmt_info) = *lmt_msg;
    AMREQ_OFI_HDR(rreq, rreq_ptr) = (void *) rreq;

    if (is_contig) {
        if (in_data_sz > data_sz) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        }
        else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }

        data_sz = MIN(data_sz, in_data_sz);
        AMREQ_OFI_HDR(rreq, lmt_cntr) = ((data_sz - 1) / MPIDI_Global.max_send) + 1;
        MPIDI_netmod_do_rdma_read(p_data, lmt_msg->src_offset, data_sz, reply_token, rreq);
        MPIR_STATUS_SET_COUNT(rreq->status, data_sz);
    } else {
        done = 0;
        rem = in_data_sz;
        iov = (struct iovec *) p_data;
        iov_len = data_sz;

        /* FIXME: optimize iov processing part */

        /* set lmt counter */
        AMREQ_OFI_HDR(rreq, lmt_cntr) = 0;
        for (i = 0; i < iov_len && rem > 0; i++) {
            curr_len = MIN(rem, iov[i].iov_len);
            num_reads = ((curr_len - 1) / MPIDI_Global.max_send) + 1;
            AMREQ_OFI_HDR(rreq, lmt_cntr) += num_reads;
            rem -= curr_len;
        }

        done = 0;
        rem = in_data_sz;
        for (i = 0; i < iov_len && rem > 0; i++) {
            curr_len = MIN(rem, iov[i].iov_len);
            MPIDI_netmod_do_rdma_read(iov[i].iov_base, lmt_msg->src_offset + done,
                                      curr_len, reply_token,rreq);
            rem -= curr_len;
            done += curr_len;
        }
        if (rem) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        }
        else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }
        MPIR_STATUS_SET_COUNT(rreq->status, done);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DO_HANDLE_LONG_AM);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_long_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_long_am(MPIDI_AM_OFI_hdr_t         *msg_hdr,
                                              MPIDI_AM_OFI_reply_token_t  reply_token)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_OFI_lmt_msg_pyld_t *lmt_msg;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LONG_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_AM);

    lmt_msg = (MPIDI_OFI_lmt_msg_pyld_t *) ((char *) msg_hdr->payload + msg_hdr->am_hdr_sz);
    mpi_errno = MPIDI_netmod_do_handle_long_am(msg_hdr, lmt_msg, msg_hdr->payload, reply_token);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_LONG_AM);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_lmt_ack
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_lmt_ack(MPIDI_AM_OFI_hdr_t         *msg_hdr,
                                              MPIDI_AM_OFI_reply_token_t  reply_token)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_OFI_Ack_msg_pyld_t *ack_msg;
    int handler_id;
    uint64_t index;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LMT_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LMT_ACK);

    ack_msg = (MPIDI_OFI_Ack_msg_pyld_t *) msg_hdr->payload;
    sreq    = (MPID_Request *)             ack_msg->sreq_ptr;

    index = fi_mr_key(AMREQ_OFI_HDR(sreq, lmt_mr)) >> MPIDI_Global.huge_rma_shift;
    MPIDI_OFI_Index_allocator_free(COMM_OFI(MPIR_Process.comm_world).rma_id_allocator, index);
    FI_RC(fi_close(&AMREQ_OFI_HDR(sreq, lmt_mr)->fid), mr_unreg);

    if (AMREQ_OFI_HDR(sreq, clientdata).pack_buffer) {
        MPIU_Free(AMREQ_OFI_HDR(sreq, clientdata).pack_buffer);
    }

    handler_id = AMREQ_OFI_HDR(sreq, msg_hdr).handler_id;
    MPIDI_AM_netmod_request_complete(sreq);
    mpi_errno = MPIDI_Global.send_cmpl_handlers[handler_id] (sreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_LMT_ACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_long_hdr_ack
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_long_hdr_ack(MPIDI_AM_OFI_hdr_t         *msg_hdr,
                                                   MPIDI_AM_OFI_reply_token_t  reply_token)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_OFI_Ack_msg_pyld_t *ack_msg;
    int handler_id;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LONG_HDR_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_HDR_ACK);

    ack_msg = (MPIDI_OFI_Ack_msg_pyld_t *) msg_hdr->payload;
    sreq    = (MPID_Request *)             ack_msg->sreq_ptr;

    handler_id = AMREQ_OFI_HDR(sreq, msg_hdr).handler_id;
    MPIDI_AM_netmod_request_complete(sreq);
    mpi_errno = MPIDI_Global.send_cmpl_handlers[handler_id] (sreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_LONG_HDR_ACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_dispatch_lmt_ack
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_dispatch_ack(int        rank,
                                            int        context_id,
                                            uint64_t   sreq_ptr,
                                            int        am_type,
                                            void      *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_OFI_Ack_msg_t msg;
    MPID_Comm *comm;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DISPATCH_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DISPATCH_ACK);

    comm =  MPIDI_CH4R_context_id_to_comm(context_id);

    msg.hdr.am_hdr_sz  = sizeof(msg.pyld);
    msg.hdr.data_sz    = 0;
    msg.hdr.am_type    = am_type;
    msg.hdr.context_id = comm->context_id;
    msg.hdr.src_rank   = comm->rank;
    msg.pyld.sreq_ptr  = sreq_ptr;
    FI_RC_RETRY_AM(fi_inject(G_TXC_MSG(0), &msg, sizeof(msg),
                             _comm_to_phys(comm, rank, MPIDI_API_TAG)), inject);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DISPATCH_ACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#endif /* NETMOD_AM_OFI_EVENTS_H_INCLUDED */
