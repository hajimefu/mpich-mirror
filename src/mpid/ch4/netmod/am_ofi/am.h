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
#define FUNCNAME MPIDI_netmod_ofi_send_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_send_am_hdr(int64_t rank, int handler_id, const void *am_hdr,
                                               size_t am_hdr_sz, MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    struct iovec iov[2];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_HDR);

    MPIU_Assert(am_hdr_sz <= MPIDI_MAX_AM_HDR_SZ);
    MPIU_Memcpy(AMREQ_OFI(sreq, am_hdr), am_hdr, am_hdr_sz);

    msg_hdr = &AMREQ_OFI(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = 0;
    msg_hdr->am_type = MPIDI_AMTYPE_SHORT_HDR;

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = AMREQ_OFI(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;

    FI_RC_RETRY(fi_sendv(MPIDI_Global.ep, iov, NULL, 2, rank, &AMREQ_OFI(sreq, context)), sendv);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AM_HDR);
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
    int mpi_errno = MPI_SUCCESS;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    MPIDI_OFI_lmt_msg_pyld_t *lmt_info;
    struct iovec iov[3];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);

    MPIU_Assert(am_hdr_sz <= MPIDI_MAX_AM_HDR_SZ);
    MPIU_Memcpy(AMREQ_OFI(sreq, am_hdr), am_hdr, am_hdr_sz);

    msg_hdr = &AMREQ_OFI(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = data_sz;
    msg_hdr->am_type = MPIDI_AMTYPE_LMT_REQ;

    lmt_info = &AMREQ_OFI(sreq, lmt_info);
    lmt_info->src_offset = (uint64_t) data;
    lmt_info->sreq_ptr = (uint64_t) sreq;

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = AMREQ_OFI(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;

    iov[2].iov_base = lmt_info;
    iov[2].iov_len = sizeof(*lmt_info);

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
    int mpi_errno = MPI_SUCCESS;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    struct iovec iov[3];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);

    MPIU_Assert(am_hdr_sz <= MPIDI_MAX_AM_HDR_SZ);
    MPIU_Memcpy(AMREQ_OFI(sreq, am_hdr), am_hdr, am_hdr_sz);

    msg_hdr = &AMREQ_OFI(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = count;
    msg_hdr->am_type = MPIDI_AMTYPE_SHORT;

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = AMREQ_OFI(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;

    iov[2].iov_base = (void *) data;
    iov[2].iov_len = count;
    FI_RC_RETRY(fi_sendv(MPIDI_Global.ep, iov, NULL, 3, rank, &AMREQ_OFI(sreq, context)), sendv);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_send_am(int64_t rank, int handler_id,
                                           const void *am_hdr, size_t am_hdr_sz,
                                           const void *buf, size_t count,
                                           MPI_Datatype datatype, MPID_Request * sreq)
{
    char *send_buf;
    int dt_contig, mpi_errno = MPI_SUCCESS;
    MPIDI_msg_sz_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPI_Aint last;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    send_buf = (char *) buf + dt_true_lb;

    if (!dt_contig) {
        MPIDI_msg_sz_t segment_first, segment_size;
        struct MPID_Segment *segment_ptr;
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, segment_ptr, 0);
        segment_first = 0;
        segment_size = data_sz;
        last = data_sz;
        AMREQ_OFI(sreq, pack_buffer) = (char *) MPIU_Malloc(data_sz);
        MPIR_ERR_CHKANDJUMP1(AMREQ_OFI(sreq, pack_buffer) == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send Pack buffer alloc");
        MPID_Segment_pack(segment_ptr, segment_first, &last, AMREQ_OFI(sreq, pack_buffer));
        MPID_Segment_free(segment_ptr);
        send_buf = (char *) AMREQ_OFI(sreq, pack_buffer);
    }
    else {
        AMREQ_OFI(sreq, pack_buffer) = NULL;
    }

    mpi_errno = (data_sz <= MPIDI_MAX_SHORT_SEND_SZ) ?
        MPIDI_netmod_ofi_send_am_short(rank, handler_id,
                                       am_hdr, am_hdr_sz, send_buf, data_sz,
                                       sreq) :
        MPIDI_netmod_ofi_send_am_long(rank, handler_id, am_hdr, am_hdr_sz, send_buf, data_sz, sreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
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
    mpi_errno = MPIDI_netmod_ofi_send_am_hdr(rank, handler_id, am_hdr, am_hdr_sz, sreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
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
    mpi_errno = MPIDI_netmod_ofi_send_am(rank, handler_id,
                                         am_hdr, am_hdr_sz, data, count, datatype, sreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

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
    MPIU_Assert(0);
    return MPI_SUCCESS;
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
    mpi_errno = MPIDI_netmod_ofi_send_am_hdr((int64_t) reply_token, handler_id,
                                             am_hdr, am_hdr_sz, sreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR_REPLY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
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
    mpi_errno = MPIDI_netmod_ofi_send_am((int64_t) reply_token, handler_id,
                                         am_hdr, am_hdr_sz, data, count, datatype, sreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_REPLY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_send_amv_reply(void *reply_token,
                                              int handler_id,
                                              struct iovec *am_hdr,
                                              size_t am_hdr_sz,
                                              const void *data, MPI_Count count,
                                              MPI_Datatype datatype, MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
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
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_netmod_inject_am_hdr_reply(void *reply_token,
                                                   int handler_id,
                                                   const void *am_hdr, size_t am_hdr_sz)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_netmod_am_inject_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}

#endif /* NETMOD_AM_OFI_AM_H_INCLUDED */