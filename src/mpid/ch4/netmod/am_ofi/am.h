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
#ifndef NETMOD_AM_OFI_AM_H_INCLUDED
#define NETMOD_AM_OFI_AM_H_INCLUDED

#include "impl.h"
#include "request.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_reg_hdr_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_reg_hdr_handler(int handler_id,
                                               MPIDI_netmod_am_origin_handler_fn origin_handler_fn,
                                               MPIDI_netmod_am_target_handler_fn target_handler_fn)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REG_HDR_HANDLER);

    if (handler_id > MPIDI_MAX_AM_HANDLERS) {
        mpi_errno = MPI_ERR_OTHER;
        goto fn_fail;
    }
    MPIDI_Global.am_handlers[handler_id] = target_handler_fn;
    MPIDI_Global.send_cmpl_handlers[handler_id] = origin_handler_fn;
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_do_send_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_do_send_am_hdr(int64_t rank, int handler_id, const void *am_hdr,
                                                  size_t am_hdr_sz, MPID_Request * sreq,
                                                  int is_reply)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    struct iovec iov[2];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);

    mpi_errno = MPIDI_netmod_am_ofi_init_req(am_hdr, am_hdr_sz, sreq, is_reply);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    msg_hdr = &AMREQ_OFI_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = 0;
    msg_hdr->am_type = MPIDI_AMTYPE_SHORT_HDR;

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = AMREQ_OFI_HDR(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;
    AMREQ_OFI_HDR(sreq, pack_buffer) = NULL;

    MPID_cc_incr(sreq->cc_ptr, &c);
    FI_RC_RETRY(fi_sendv(MPIDI_Global.ep, iov, NULL, 2, rank, &AMREQ_OFI(sreq, context)), sendv);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_send_am_long
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_send_am_long(int64_t rank, int handler_id,
                                                const void *am_hdr, size_t am_hdr_sz,
                                                const void *data, size_t data_sz,
                                                MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    MPIDI_OFI_lmt_msg_pyld_t *lmt_info;
    struct iovec iov[3];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);

    msg_hdr = &AMREQ_OFI_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = data_sz;
    msg_hdr->am_type = MPIDI_AMTYPE_LMT_REQ;

    lmt_info = &AMREQ_OFI_HDR(sreq, lmt_info);
    lmt_info->src_offset = (uint64_t) data;
    lmt_info->sreq_ptr = (uint64_t) sreq;

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = AMREQ_OFI_HDR(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;

    iov[2].iov_base = lmt_info;
    iov[2].iov_len = sizeof(*lmt_info);

    MPID_cc_incr(sreq->cc_ptr, &c); /* send completion */
    MPID_cc_incr(sreq->cc_ptr, &c); /* lmt ack handler */
    FI_RC_RETRY(fi_sendv(MPIDI_Global.ep, iov, NULL, 3, rank, &AMREQ_OFI(sreq, context)), sendv);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_send_am_short
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_send_am_short(int64_t rank, int handler_id,
                                                 const void *am_hdr, size_t am_hdr_sz,
                                                 const void *data, MPI_Count count,
                                                 MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    struct iovec iov[3];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);

    msg_hdr = &AMREQ_OFI_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = count;
    msg_hdr->am_type = MPIDI_AMTYPE_SHORT;

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = AMREQ_OFI_HDR(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;

    iov[2].iov_base = (void *) data;
    iov[2].iov_len = count;

    MPID_cc_incr(sreq->cc_ptr, &c);
    FI_RC_RETRY(fi_sendv(MPIDI_Global.ep, iov, NULL, 3, rank, &AMREQ_OFI(sreq, context)), sendv);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_do_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_do_send_am(int64_t rank, int handler_id,
                                              const void *am_hdr, size_t am_hdr_sz,
                                              const void *buf, size_t count,
                                              MPI_Datatype datatype, MPID_Request * sreq,
                                              int is_reply)
{
    char *send_buf;
    int dt_contig, mpi_errno = MPI_SUCCESS;
    MPIDI_msg_sz_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPI_Aint last;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_SEND_AM);

    mpi_errno = MPIDI_netmod_am_ofi_init_req(am_hdr, am_hdr_sz, sreq, is_reply);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    send_buf = (char *) buf + dt_true_lb;

    if (!dt_contig) {
        MPIDI_msg_sz_t segment_first;
        struct MPID_Segment *segment_ptr;
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, segment_ptr, 0);
        segment_first = 0;
        last = data_sz;
        AMREQ_OFI_HDR(sreq, pack_buffer) = (char *) MPIU_Malloc(data_sz);
        MPIR_ERR_CHKANDJUMP1(AMREQ_OFI_HDR(sreq, pack_buffer) == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send Pack buffer alloc");
        MPID_Segment_pack(segment_ptr, segment_first, &last, AMREQ_OFI_HDR(sreq, pack_buffer));
        MPID_Segment_free(segment_ptr);
        send_buf = (char *) AMREQ_OFI_HDR(sreq, pack_buffer);
    }
    else {
        AMREQ_OFI_HDR(sreq, pack_buffer) = NULL;
    }

    mpi_errno = (data_sz <= MPIDI_MAX_SHORT_SEND_SZ) ?
        MPIDI_netmod_ofi_send_am_short(rank, handler_id,am_hdr, am_hdr_sz, send_buf, data_sz, sreq) :
        MPIDI_netmod_ofi_send_am_long(rank, handler_id, am_hdr, am_hdr_sz, send_buf, data_sz, sreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DO_SEND_AM);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_send_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_send_am_hdr(int rank,
                                           MPID_Comm * comm,
                                           int handler_id,
                                           const void *am_hdr,
                                           size_t am_hdr_sz,
                                           MPID_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR);
    mpi_errno = MPIDI_netmod_ofi_do_send_am_hdr(_comm_to_phys(comm, rank, MPIDI_API_TAG),
                                                handler_id, am_hdr, am_hdr_sz, sreq, 0);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_send_am(int rank,
                                       MPID_Comm * comm,
                                       int handler_id,
                                       const void *am_hdr,
                                       size_t am_hdr_sz,
                                       const void *data,
                                       MPI_Count count,
                                       MPI_Datatype datatype,
                                       MPID_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    mpi_errno = MPIDI_netmod_ofi_do_send_am(_comm_to_phys(comm, rank, MPIDI_API_TAG), handler_id,
                                            am_hdr, am_hdr_sz, data, count, datatype, sreq, 0);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_send_amv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_send_amv(int rank,
                                        MPID_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        const void *data,
                                        MPI_Count count,
                                        MPI_Datatype datatype,
                                        MPID_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, i, is_allocated;
    size_t am_hdr_sz = 0;
    char *am_hdr_buf;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AMV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AMV);

    for (i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    /* TODO: avoid the malloc here, use the am_hdr directly */
    if (am_hdr_sz > MPIDI_BUF_POOL_SZ) {
        am_hdr_buf = MPIU_Malloc(am_hdr_sz);
        is_allocated = 1;
    } else {
        am_hdr_buf = MPIU_CH4U_get_buf(MPIDI_Global.buf_pool);
        is_allocated = 0;
    }

    MPIU_Assert(am_hdr_buf);
    am_hdr_sz = 0;
    for (i = 0; i < iov_len; i++) {
        MPIU_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_netmod_send_am(rank, comm, handler_id, am_hdr_buf, am_hdr_sz,
                                     data, count, datatype, sreq, netmod_context);
    if (is_allocated)
        MPIU_Free(am_hdr_buf);
    else
        MPIU_CH4U_release_buf(am_hdr_buf);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AMV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_send_amv_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_send_amv_hdr(int rank,
                                            MPID_Comm * comm,
                                            int handler_id,
                                            struct iovec *am_hdr,
                                            size_t iov_len,
                                            MPID_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, i, is_allocated;
    size_t am_hdr_sz = 0;
    char *am_hdr_buf;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AMV_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AMV_HDR);

    for (i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    /* TODO: avoid the malloc here, use the am_hdr directly */
    if (am_hdr_sz > MPIDI_BUF_POOL_SZ) {
        am_hdr_buf = MPIU_Malloc(am_hdr_sz);
        is_allocated = 1;
    } else {
        am_hdr_buf = MPIU_CH4U_get_buf(MPIDI_Global.buf_pool);
        is_allocated = 0;
    }

    MPIU_Assert(am_hdr_buf);
    am_hdr_sz = 0;
    for (i = 0; i < iov_len; i++) {
        MPIU_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_netmod_send_am_hdr(rank, comm, handler_id, am_hdr_buf, am_hdr_sz,
                                         sreq, netmod_context);
    if (is_allocated)
        MPIU_Free(am_hdr_buf);
    else
        MPIU_CH4U_release_buf(am_hdr_buf);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AMV_HDR);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_send_am_hdr_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_send_am_hdr_reply(void *reply_token,
                                                 int handler_id,
                                                 const void *am_hdr,
                                                 size_t am_hdr_sz, MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR_REPLY);

    mpi_errno = MPIDI_netmod_ofi_do_send_am_hdr((int64_t) reply_token, handler_id,
                                                am_hdr, am_hdr_sz, sreq, 1);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR_REPLY);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_send_am_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_send_am_reply(void *reply_token,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz,
                                             const void *data, MPI_Count count,
                                             MPI_Datatype datatype, MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM_REPLY);
    mpi_errno = MPIDI_netmod_ofi_do_send_am((int64_t) reply_token, handler_id,
                                            am_hdr, am_hdr_sz, data, count, datatype, sreq, 1);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_REPLY);
    return mpi_errno;
}

static inline int MPIDI_netmod_send_amv_reply(void *reply_token,
                                              int handler_id,
                                              struct iovec *am_hdr,
                                              size_t iov_len,
                                              const void *data, MPI_Count count,
                                              MPI_Datatype datatype, MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, i, is_allocated;
    size_t am_hdr_sz = 0;
    char *am_hdr_buf;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AMV_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AMV_REPLY);

    for (i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    /* TODO: avoid the malloc here, use the am_hdr directly */
    if (am_hdr_sz > MPIDI_BUF_POOL_SZ) {
        am_hdr_buf = MPIU_Malloc(am_hdr_sz);
        is_allocated = 1;
    } else {
        am_hdr_buf = MPIU_CH4U_get_buf(MPIDI_Global.buf_pool);
        is_allocated = 0;
    }

    MPIU_Assert(am_hdr_buf);
    am_hdr_sz = 0;

    for (i = 0; i < iov_len; i++) {
        MPIU_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }
    mpi_errno = MPIDI_netmod_send_am_reply(reply_token, handler_id, am_hdr_buf, am_hdr_sz,
                                           data, count, datatype, sreq);
    if (is_allocated)
        MPIU_Free(am_hdr_buf);
    else
        MPIU_CH4U_release_buf(am_hdr_buf);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AMV_REPLY);
    return mpi_errno;
}

static inline size_t MPIDI_netmod_am_hdr_max_sz(void)
{
    return MPIDI_MAX_AM_HDR_SZ;
}

static inline int MPIDI_netmod_inject_am_hdr(int rank,
                                             MPID_Comm * comm,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, is_allocated;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    char *data_buf;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR);

    MPIU_Assert(am_hdr_sz + sizeof(*msg_hdr) < MPIDI_Global.max_buffered_send);

    if (am_hdr_sz + sizeof(*msg_hdr) > MPIDI_BUF_POOL_SZ) {
        data_buf = MPIU_Malloc(am_hdr_sz + sizeof(*msg_hdr));
        is_allocated = 1;
    } else {
        data_buf = MPIU_CH4U_get_buf(MPIDI_Global.buf_pool);
        is_allocated = 0;
    }

    MPIU_Assert(data_buf);

    msg_hdr = (MPIDI_AM_OFI_hdr_t *) data_buf;
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = 0;
    msg_hdr->am_type = MPIDI_AMTYPE_SHORT;

    MPIU_Memcpy(data_buf + sizeof(*msg_hdr), am_hdr, am_hdr_sz);
    
    FI_RC_RETRY(fi_inject(MPIDI_Global.ep, msg_hdr, sizeof(*msg_hdr) + am_hdr_sz,
                          _comm_to_phys(comm, rank, MPIDI_API_TAG)), inject);
    if (is_allocated)
        MPIU_Free(data_buf);
    else
        MPIU_CH4U_release_buf(data_buf);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_inject_am_hdr_reply(void *reply_token,
                                                   int handler_id,
                                                   const void *am_hdr, size_t am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS, is_allocated;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    char *data_buf;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR_REPLY);

    MPIU_Assert(am_hdr_sz + sizeof(*msg_hdr) < MPIDI_Global.max_buffered_send);
    if (am_hdr_sz + sizeof(*msg_hdr) > MPIDI_BUF_POOL_SZ) {
        data_buf = MPIU_Malloc(am_hdr_sz + sizeof(*msg_hdr));
        is_allocated = 1;
    } else {
        data_buf = MPIU_CH4U_get_buf(MPIDI_Global.buf_pool);
        is_allocated = 0;
    }

    MPIU_Assert(data_buf);

    msg_hdr = (MPIDI_AM_OFI_hdr_t *) data_buf;
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = 0;
    msg_hdr->am_type = MPIDI_AMTYPE_SHORT;

    MPIU_Memcpy(data_buf + sizeof(*msg_hdr), am_hdr, am_hdr_sz);
    FI_RC_RETRY(fi_inject(MPIDI_Global.ep, msg_hdr, sizeof(*msg_hdr) + am_hdr_sz,
                          (int64_t) reply_token), inject);

    if (is_allocated)
        MPIU_Free(data_buf);
    else
        MPIU_CH4U_release_buf(data_buf);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline size_t MPIDI_netmod_am_inject_max_sz(void)
{
    return MPIDI_Global.max_buffered_send - sizeof(MPIDI_AM_OFI_hdr_t);
}

#endif /* NETMOD_AM_OFI_AM_H_INCLUDED */
