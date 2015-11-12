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
#ifndef NETMOD_AM_OFI_PROGRESS_H_INCLUDED
#define NETMOD_AM_OFI_PROGRESS_H_INCLUDED

#include "impl.h"
#include "request.h"

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
    MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn;
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
static inline int MPIDI_netmod_handle_short_am_hdr(MPIDI_AM_OFI_hdr_t * msg_hdr, fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn = NULL;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);

    MPIDI_Global.am_handlers[msg_hdr->handler_id] (msg_hdr->payload, msg_hdr->am_hdr_sz,
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
    MPIDI_netmod_am_ofi_amrequest_t *am_req;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_RDMA_READ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_RDMA_READ);

    rem = data_sz;
    while (done != data_sz) {
        curr_len = MIN(rem, MPIDI_Global.max_send);

        /* TODO: get context from pool */
        am_req = (MPIDI_netmod_am_ofi_amrequest_t *)MPIU_Malloc(sizeof(*am_req));
        MPIU_Assert(am_req);
        
        am_req->req_hdr = AMREQ_OFI(rreq, req_hdr);
        FI_RC_RETRY(fi_read(MPIDI_Global.ep, (char *) dst + done,
                            curr_len, NULL, source, src + done,
                            MPIDI_Global.lkey, &am_req->context), read);
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

    rreq = MPIDI_netmod_request_create();
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
    MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn;
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
        MPIDI_netmod_am_ofi_req_complete(rreq);
        goto fn_exit;
    }

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
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LMT_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LMT_ACK);

    ack_msg = (MPIDI_OFI_Ack_msg_pyld_t *) msg_hdr->payload;
    sreq = (MPID_Request *) ack_msg->sreq_ptr;

    if (AMREQ_OFI_HDR(sreq, pack_buffer)) {
        MPIU_Free(AMREQ_OFI_HDR(sreq, pack_buffer));
    }

    handler_id = AMREQ_OFI_HDR(sreq, msg_hdr).handler_id;
    MPIDI_netmod_am_ofi_req_complete(sreq);
    mpi_errno = MPIDI_Global.send_cmpl_handlers[handler_id] (sreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_LMT_ACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_send_completion
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_send_completion(struct fi_cq_data_entry *cq_entry)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_netmod_am_ofi_amrequest_t *ofi_req;
    MPIDI_AM_OFI_hdr_t *msg_hdr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);

    ofi_req = container_of(cq_entry->op_context, MPIDI_netmod_am_ofi_amrequest_t, context);
    sreq = container_of(ofi_req, MPID_Request, dev.ch4.ch4u.netmod_am);
    msg_hdr = &ofi_req->req_hdr->msg_hdr;
    MPIDI_netmod_am_ofi_req_complete(sreq);

    switch (msg_hdr->am_type) {

    case MPIDI_AMTYPE_LMT_ACK:
    case MPIDI_AMTYPE_LMT_REQ:
    case MPIDI_AMTYPE_LMT_HDR_REQ:
        goto fn_exit;

    default:
        break;
    }

    if (AMREQ_OFI_HDR(sreq, pack_buffer)) {
        MPIU_Free(AMREQ_OFI_HDR(sreq, pack_buffer));
        AMREQ_OFI_HDR(sreq, pack_buffer) = NULL;
    }

    mpi_errno = MPIDI_Global.send_cmpl_handlers[msg_hdr->handler_id] (sreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_recv_completion
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_recv_completion(struct fi_cq_data_entry *cq_entry,
                                                      fi_addr_t source, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_AM_OFI_hdr_t *am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_RECV_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_RECV_COMPLETION);

    am_hdr = (MPIDI_AM_OFI_hdr_t *) cq_entry->buf;
    switch (am_hdr->am_type) {
    case MPIDI_AMTYPE_SHORT_HDR:
        mpi_errno = MPIDI_netmod_handle_short_am_hdr(am_hdr, source);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        break;

    case MPIDI_AMTYPE_SHORT:
        mpi_errno = MPIDI_netmod_handle_short_am(am_hdr, source);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        break;

    case MPIDI_AMTYPE_LMT_REQ:
        mpi_errno = MPIDI_netmod_handle_long_am(am_hdr, source);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        break;

    case MPIDI_AMTYPE_LMT_HDR_REQ:
        mpi_errno = MPIDI_netmod_handle_long_am_hdr(am_hdr, source);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        break;

    case MPIDI_AMTYPE_LMT_ACK:
        mpi_errno = MPIDI_netmod_handle_lmt_ack(am_hdr, source);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        break;

    default:
        MPIU_Assert(0);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_RECV_COMPLETION);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_dispatch_lmt_ack
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_dispatch_lmt_ack(fi_addr_t source,
                                                uint64_t sreq_ptr,
                                                void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_OFI_Ack_msg_t msg;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DISPATCH_LMT_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DISPATCH_LMT_ACK);

    msg.hdr.am_hdr_sz = sizeof(msg.pyld);
    msg.hdr.data_sz = 0;
    msg.hdr.am_type = MPIDI_AMTYPE_LMT_ACK;
    msg.pyld.sreq_ptr = sreq_ptr;

    FI_RC_RETRY(fi_inject(MPIDI_Global.ep, &msg, sizeof(msg), source), inject);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DISPATCH_LMT_ACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_read_completion
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_read_completion(struct fi_cq_data_entry *cq_entry,
                                                      fi_addr_t source, void *netmod_context)
{
    MPID_Request *rreq;
    int mpi_errno = MPI_SUCCESS;
    MPIDI_netmod_am_ofi_amrequest_t *ofi_req;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);

    ofi_req = container_of(cq_entry->op_context, MPIDI_netmod_am_ofi_amrequest_t, context);
    ofi_req->req_hdr->lmt_cntr--;
    if (ofi_req->req_hdr->lmt_cntr)
        goto fn_exit;

    if (ofi_req->req_hdr->msg_hdr.am_type == MPIDI_AMTYPE_LMT_HDR_REQ) {
        rreq = (MPID_Request *)ofi_req->req_hdr->rreq_ptr;
        AMREQ_OFI_HDR(rreq, msg_hdr).am_type = MPIDI_AMTYPE_LMT_REQ;
        mpi_errno = MPIDI_netmod_do_handle_long_am(&AMREQ_OFI_HDR(rreq, msg_hdr),
                                                   &AMREQ_OFI_HDR(rreq, lmt_info),
                                                   AMREQ_OFI_HDR(rreq, am_hdr),
                                                   (fi_addr_t) AMREQ_OFI_HDR(rreq, pack_buffer));
        if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);
        MPIDI_netmod_am_ofi_req_complete(rreq);
        goto fn_exit;
    }

    rreq = (MPID_Request *)ofi_req->req_hdr->rreq_ptr;
    mpi_errno = MPIDI_netmod_dispatch_lmt_ack(source, 
                                              AMREQ_OFI_HDR(rreq, lmt_info.sreq_ptr),
                                              netmod_context);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_netmod_am_ofi_req_complete(rreq);
    ofi_req->req_hdr->cmpl_handler_fn(rreq);
  fn_exit:
    MPIU_Free(ofi_req);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_repost_buffer
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_repost_buffer(void *buf, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REPOST_BUFFER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REPOST_BUFFER);

    FI_RC_RETRY(fi_recvmsg(MPIDI_Global.ep, (struct fi_msg *) buf,
                           FI_MULTI_RECV | FI_COMPLETION), repost);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REPOST_BUFFER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_progress
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_progress(void *netmod_context, int blocking)
{
    int mpi_errno = MPI_SUCCESS, found = 0, ret;
    struct fi_cq_data_entry cq_entry;
    struct fi_cq_err_entry cq_err_entry;
    fi_addr_t source;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_PROGRESS);

    do {
        ret = fi_cq_readfrom(MPIDI_Global.am_cq, &cq_entry, 1, &source);
        if (ret == -FI_EAGAIN)
            continue;

        if (ret < 0) {
            fi_cq_readerr(MPIDI_Global.am_cq, &cq_err_entry, 0);
            fprintf(stderr, "fi_cq_read failed with error: %s\n", fi_strerror(cq_err_entry.err));
            goto fn_fail;
        }

        found = 1;
        if (cq_entry.flags & FI_SEND) {
            mpi_errno = MPIDI_netmod_handle_send_completion(&cq_entry);
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);

        }
        else if (cq_entry.flags & FI_RECV) {
            mpi_errno = MPIDI_netmod_handle_recv_completion(&cq_entry, source, netmod_context);
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);

            if (cq_entry.flags & FI_MULTI_RECV) {
                mpi_errno = MPIDI_netmod_repost_buffer(cq_entry.op_context, netmod_context);
                if (mpi_errno) MPIR_ERR_POP(mpi_errno);
            }

        }
        else if (cq_entry.flags & FI_READ) {
            mpi_errno = MPIDI_netmod_handle_read_completion(&cq_entry, source, netmod_context);
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        }
        else {
            MPIU_Assert(0);
        }
    } while (blocking && !found);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_PROGRESS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_progress_test(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_poke(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline void MPIDI_netmod_progress_start(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return;
}

static inline void MPIDI_netmod_progress_end(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return;
}

static inline int MPIDI_netmod_progress_wait(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_register(int (*progress_fn) (int *), int *id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_deregister(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_activate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_deactivate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* NETMOD_AM_OFI_PROGRESS_H_INCLUDED */
