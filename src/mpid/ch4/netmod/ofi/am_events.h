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

#include "impl.h"

static inline int MPIDI_netmod_progress_do_queue(void *netmod_context);

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_repost_buffer
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_repost_buffer(void* buf)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REPOST_BUFFER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REPOST_BUFFER);
    FI_RC_RETRY_AM(fi_recvmsg(MPIDI_Global.ep, (struct fi_msg *) buf,
                              FI_MULTI_RECV | FI_COMPLETION), repost);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REPOST_BUFFER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_short_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_short_am(MPIDI_AM_OFI_hdr_t * msg_hdr, fi_addr_t source)
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

    MPIDI_Global.am_handlers[msg_hdr->handler_id] (msg_hdr->payload, msg_hdr->am_hdr_sz,
                                                   (void *) source, &p_data, &data_sz, &is_contig,
                                                   &cmpl_handler_fn, &rreq);
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
static inline int MPIDI_netmod_handle_short_am_hdr(MPIDI_AM_OFI_hdr_t * msg_hdr,
                                                   void *am_hdr, fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    MPIDI_CH4_NM_am_completion_handler_fn cmpl_handler_fn = NULL;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);

    MPIDI_Global.am_handlers[msg_hdr->handler_id] (am_hdr, msg_hdr->am_hdr_sz,
                                                   (void *) source, NULL, NULL, NULL, 
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
static inline int MPIDI_netmod_do_rdma_read(void *dst, uint64_t src, size_t data_sz,
                                            fi_addr_t source, MPID_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    size_t done = 0, curr_len, rem = 0;
    MPIDI_CH4_NM_ofi_amrequest_t *am_req;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_RDMA_READ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_RDMA_READ);

    rem = data_sz;
    while (done != data_sz) {
        curr_len = MIN(rem, MPIDI_Global.max_send);

        MPIU_Assert(sizeof(MPIDI_CH4_NM_ofi_amrequest_t) <= MPIDI_BUF_POOL_SZ);
        am_req = (MPIDI_CH4_NM_ofi_amrequest_t *)MPIDI_CH4R_get_buf(MPIDI_Global.buf_pool);
        MPIU_Assert(am_req);

        am_req->req_hdr = AMREQ_OFI(rreq, req_hdr);
        FI_RC_RETRY_AM(fi_read(MPIDI_Global.ep, (char *) dst + done,
                            curr_len, NULL, source, src + done,
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
static inline int MPIDI_netmod_handle_long_am_hdr(MPIDI_AM_OFI_hdr_t * msg_hdr, fi_addr_t source)
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
    AMREQ_OFI_HDR(rreq, lmt_info) = *lmt_msg;
    AMREQ_OFI_HDR(rreq, msg_hdr) = *msg_hdr;
    AMREQ_OFI_HDR(rreq, pack_buffer) = (void *)source;
    AMREQ_OFI_HDR(rreq, rreq_ptr) = (void *) rreq;

    AMREQ_OFI_HDR(rreq, am_hdr) = MPIU_Malloc(msg_hdr->am_hdr_sz);
    AMREQ_OFI_HDR(rreq, lmt_cntr) = ((msg_hdr->am_hdr_sz - 1) / MPIDI_Global.max_send) + 1;
    MPIDI_netmod_do_rdma_read(AMREQ_OFI_HDR(rreq, am_hdr), lmt_msg->am_hdr_src,
                              msg_hdr->am_hdr_sz, source, rreq);

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
static inline int MPIDI_netmod_handle_long_hdr(MPIDI_AM_OFI_hdr_t * msg_hdr, fi_addr_t source)
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
    AMREQ_OFI_HDR(rreq, lmt_info) = *lmt_msg;
    AMREQ_OFI_HDR(rreq, msg_hdr) = *msg_hdr;
    AMREQ_OFI_HDR(rreq, pack_buffer) = (void *)source;
    AMREQ_OFI_HDR(rreq, rreq_ptr) = (void *) rreq;

    AMREQ_OFI_HDR(rreq, am_hdr) = MPIU_Malloc(msg_hdr->am_hdr_sz);
    AMREQ_OFI_HDR(rreq, lmt_cntr) = ((msg_hdr->am_hdr_sz - 1) / MPIDI_Global.max_send) + 1;
    MPIDI_netmod_do_rdma_read(AMREQ_OFI_HDR(rreq, am_hdr), lmt_msg->am_hdr_src,
                              msg_hdr->am_hdr_sz, source, rreq);

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
static inline int MPIDI_netmod_do_handle_long_am(MPIDI_AM_OFI_hdr_t *msg_hdr,
                                                 MPIDI_OFI_lmt_msg_pyld_t *lmt_msg,
                                                 void *am_hdr, fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS, is_contig = 0;
    MPID_Request *rreq;
    void *p_data;
    size_t data_sz, rem, done, curr_len, in_data_sz;
    MPIDI_CH4_NM_am_completion_handler_fn cmpl_handler_fn;
    int num_reads, i, iov_len, c;
    struct iovec *iov;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_HANDLE_LONG_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_HANDLE_LONG_AM);

    in_data_sz = data_sz = msg_hdr->data_sz;
    MPIDI_Global.am_handlers[msg_hdr->handler_id] (am_hdr, msg_hdr->am_hdr_sz,
                                                   (void *) source, &p_data, &data_sz, &is_contig,
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
        MPIDI_netmod_do_rdma_read(p_data, lmt_msg->src_offset, data_sz, source, rreq);
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
            MPIDI_netmod_do_rdma_read(iov[i].iov_base, lmt_msg->src_offset + done, curr_len, source,
                                      rreq);
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
static inline int MPIDI_netmod_handle_long_am(MPIDI_AM_OFI_hdr_t * msg_hdr, fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_OFI_lmt_msg_pyld_t *lmt_msg;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LONG_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_AM);

    lmt_msg = (MPIDI_OFI_lmt_msg_pyld_t *) ((char *) msg_hdr->payload + msg_hdr->am_hdr_sz);
    mpi_errno = MPIDI_netmod_do_handle_long_am(msg_hdr, lmt_msg, msg_hdr->payload, source);
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
static inline int MPIDI_netmod_handle_lmt_ack(MPIDI_AM_OFI_hdr_t * msg_hdr, fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_OFI_Ack_msg_pyld_t *ack_msg;
    int handler_id;
    uint64_t index;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LMT_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LMT_ACK);

    ack_msg = (MPIDI_OFI_Ack_msg_pyld_t *) msg_hdr->payload;
    sreq = (MPID_Request *) ack_msg->sreq_ptr;

    index = fi_mr_key(AMREQ_OFI_HDR(sreq, lmt_mr)) >> MPIDI_Global.huge_rma_shift;
    MPIDI_OFI_Index_allocator_free(COMM_OFI(MPIR_Process.comm_world).rma_id_allocator, index);
    FI_RC(fi_close(&AMREQ_OFI_HDR(sreq, lmt_mr)->fid), mr_unreg);

    if (AMREQ_OFI_HDR(sreq, pack_buffer)) {
        MPIU_Free(AMREQ_OFI_HDR(sreq, pack_buffer));
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
static inline int MPIDI_netmod_handle_long_hdr_ack(MPIDI_AM_OFI_hdr_t * msg_hdr, fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_OFI_Ack_msg_pyld_t *ack_msg;
    int handler_id;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LONG_HDR_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_HDR_ACK);

    ack_msg = (MPIDI_OFI_Ack_msg_pyld_t *) msg_hdr->payload;
    sreq = (MPID_Request *) ack_msg->sreq_ptr;

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
static inline int MPIDI_netmod_dispatch_ack(fi_addr_t source,
                                            uint64_t sreq_ptr,
                                            int am_type,
                                            void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_OFI_Ack_msg_t msg;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DISPATCH_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DISPATCH_ACK);

    msg.hdr.am_hdr_sz = sizeof(msg.pyld);
    msg.hdr.data_sz = 0;
    msg.hdr.am_type = am_type;
    msg.pyld.sreq_ptr = sreq_ptr;

    FI_RC_RETRY_AM(fi_inject(MPIDI_Global.ep, &msg, sizeof(msg), source), inject);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DISPATCH_ACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_progress_do_queue(void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, ret;
    struct fi_cq_data_entry cq_entry;
    struct fi_cq_err_entry cq_err_entry;
    fi_addr_t source;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_PROGRESS);

    ret = fi_cq_readfrom(MPIDI_Global.p2p_cq, &cq_entry, 1, &source);
    if (ret == -FI_EAGAIN)
        goto fn_exit;

    if (ret < 0) {
        fi_cq_readerr(MPIDI_Global.p2p_cq, &cq_err_entry, 0);
        fprintf(stderr, "fi_cq_read failed with error: %s\n", fi_strerror(cq_err_entry.err));
        goto fn_fail;
    }

    if (((MPIDI_Global.cq_buff_head + 1) %
         MPIDI_NUM_CQ_BUFFERED == MPIDI_Global.cq_buff_tail) ||
        !slist_empty(&MPIDI_Global.cq_buff_list)) {
        struct cq_list *list_entry = (struct cq_list *) MPIU_Malloc(sizeof(struct cq_list));
        MPIU_Assert(list_entry);
        list_entry->cq_entry = cq_entry;
        list_entry->source = source;
        slist_insert_tail(&list_entry->entry, &MPIDI_Global.cq_buff_list);
    } else {
        MPIDI_Global.cq_buffered[MPIDI_Global.cq_buff_head].cq_entry = cq_entry;
        MPIDI_Global.cq_buffered[MPIDI_Global.cq_buff_head].source = source;
        MPIDI_Global.cq_buff_head = (MPIDI_Global.cq_buff_head + 1) % MPIDI_NUM_CQ_BUFFERED;
    }

    if ((cq_entry.flags & FI_RECV) &&
        (cq_entry.flags & FI_MULTI_RECV)) {
        mpi_errno = MPIDI_netmod_repost_buffer(cq_entry.op_context);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_PROGRESS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* NETMOD_AM_OFI_EVENTS_H_INCLUDED */
