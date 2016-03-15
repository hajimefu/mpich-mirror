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
#ifndef NETMOD_PORTALS4_AM_H_INCLUDED
#define NETMOD_PORTALS4_AM_H_INCLUDED

#include "impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_reg_hdr_handler
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_reg_hdr_handler(int handler_id,
                                               MPIDI_CH4_NM_am_origin_handler_fn origin_handler_fn,
                                               MPIDI_CH4_NM_am_target_handler_fn target_handler_fn)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REG_HDR_HANDLER);

    MPIDI_CH4_NMI_PTL_global.am_handlers[handler_id] = target_handler_fn;
    MPIDI_CH4_NMI_PTL_global.send_cmpl_handlers[handler_id] = origin_handler_fn;
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_send_am_hdr(int rank,
                                           MPID_Comm * comm,
                                           int handler_id,
                                           const void *am_hdr,
                                           size_t am_hdr_sz,
                                           MPID_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, ret, c;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPID_Datatype  *dt_ptr;
    int             dt_contig;
    ptl_hdr_data_t   ptl_hdr;
    ptl_match_bits_t match_bits;
    char *send_buf = NULL;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    ptl_hdr = MPIDI_CH4_NMI_PTL_init_am_hdr(handler_id, comm->rank, 0);
    match_bits = MPIDI_CH4_NMI_PTL_init_tag(comm->context_id, MPIDI_CH4_NMI_PTL_AM_TAG);

    MPIR_cc_incr(sreq->cc_ptr, &c);

    if (dt_contig) {
        ret = PtlPut(MPIDI_CH4_NMI_PTL_global.md, (ptl_size_t)am_hdr, am_hdr_sz,
                     PTL_ACK_REQ, MPIDI_CH4_NMI_PTL_addr_table[rank].process,
                     MPIDI_CH4_NMI_PTL_addr_table[rank].pt, match_bits, 0, sreq, ptl_hdr);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_send_am(int rank,
                                       MPID_Comm * comm,
                                       int handler_id,
                                       const void *am_hdr,
                                       size_t am_hdr_sz,
                                       const void *data,
                                       MPI_Count count,
                                       MPI_Datatype datatype,
                                       MPID_Request * sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, ret, c;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPID_Datatype  *dt_ptr;
    int             dt_contig;
    ptl_hdr_data_t   ptl_hdr;
    ptl_match_bits_t match_bits;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    match_bits = MPIDI_CH4_NMI_PTL_init_tag(comm->context_id, MPIDI_CH4_NMI_PTL_AM_TAG);
    ptl_hdr = MPIDI_CH4_NMI_PTL_init_am_hdr(handler_id, comm->rank, data_sz);

    MPIR_cc_incr(sreq->cc_ptr, &c);

    if (dt_contig) {
        /* create a two element iovec and send */
        ptl_md_t md;
        ptl_iovec_t iovec[2];

        iovec[0].iov_base = (char *)am_hdr;
        iovec[0].iov_len = am_hdr_sz;
        iovec[1].iov_base = (char *)data + dt_true_lb;
        iovec[1].iov_len = data_sz;
        md.start = iovec;
        md.length = 2;
        md.options = PTL_IOVEC;
        md.eq_handle = MPIDI_CH4_NMI_PTL_global.eqs[0];
        md.ct_handle = PTL_CT_NONE;

        ret = PtlMDBind(MPIDI_CH4_NMI_PTL_global.ni, &md, &sreq->dev.ch4.ch4r.netmod_am.portals4.md);
        ret = PtlPut(sreq->dev.ch4.ch4r.netmod_am.portals4.md, 0, am_hdr_sz + data_sz,
                     PTL_ACK_REQ, MPIDI_CH4_NMI_PTL_addr_table[rank].process,
                     MPIDI_CH4_NMI_PTL_addr_table[rank].pt, match_bits, 0, sreq, ptl_hdr);
    } else {
        /* copy everything into pack_buffer */
        MPID_Segment *segment;
        MPI_Aint last;
        char *send_buf = NULL;

        send_buf = MPL_malloc(am_hdr_sz + data_sz);
        MPIU_Memcpy(send_buf, am_hdr, am_hdr_sz);
        segment = MPID_Segment_alloc();
        MPID_Segment_init(data, count, datatype, segment, 0);
        last = data_sz;
        MPID_Segment_pack(segment, 0, &last, send_buf + am_hdr_sz);
        MPIU_Assert(last == data_sz);
        sreq->dev.ch4.ch4r.netmod_am.portals4.pack_buffer = send_buf;

        ret = PtlPut(MPIDI_CH4_NMI_PTL_global.md, (ptl_size_t)send_buf, am_hdr_sz + data_sz,
                     PTL_ACK_REQ, MPIDI_CH4_NMI_PTL_addr_table[rank].process,
                     MPIDI_CH4_NMI_PTL_addr_table[rank].pt, match_bits, 0, sreq, ptl_hdr);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_send_amv(int rank,
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

static inline int MPIDI_CH4_NM_send_amv_hdr(int rank,
                                        MPID_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        MPID_Request * sreq, void *netmod_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_send_am_hdr_reply(uint64_t reply_token,
                                                 int handler_id,
                                                 const void *am_hdr,
                                                 size_t am_hdr_sz, MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_NM_send_am_reply(uint64_t reply_token,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz,
                                             const void *data,
                                             MPI_Count count,
                                             MPI_Datatype datatype, MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, ret, c;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPID_Datatype  *dt_ptr;
    int             dt_contig;
    ptl_hdr_data_t   ptl_hdr;
    ptl_match_bits_t match_bits;
    char *send_buf = NULL;
    MPIDI_CH4_NMI_PTL_am_reply_token_t use_token;
    MPID_Comm *use_comm;
    int use_rank;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    use_token.val = reply_token;
    use_comm = MPIDI_CH4R_context_id_to_comm(use_token.data.context_id);
    use_rank = use_token.data.src_rank;

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    match_bits = MPIDI_CH4_NMI_PTL_init_tag(use_comm->context_id, MPIDI_CH4_NMI_PTL_AM_TAG);
    ptl_hdr = MPIDI_CH4_NMI_PTL_init_am_hdr(handler_id, use_comm->rank, data_sz);

    MPIR_cc_incr(sreq->cc_ptr, &c);

    if (dt_contig) {
        /* create a two element iovec and send */
        ptl_md_t md;
        ptl_iovec_t iovec[2];

        iovec[0].iov_base = (char *)am_hdr;
        iovec[0].iov_len = am_hdr_sz;
        iovec[1].iov_base = (char *)data + dt_true_lb;
        iovec[1].iov_len = data_sz;
        md.start = iovec;
        md.length = 2;
        md.options = PTL_IOVEC;
        md.eq_handle = MPIDI_CH4_NMI_PTL_global.eqs[0];
        md.ct_handle = PTL_CT_NONE;

        ret = PtlMDBind(MPIDI_CH4_NMI_PTL_global.ni, &md, &sreq->dev.ch4.ch4r.netmod_am.portals4.md);
        ret = PtlPut(sreq->dev.ch4.ch4r.netmod_am.portals4.md, 0, am_hdr_sz + data_sz,
                     PTL_ACK_REQ, MPIDI_CH4_NMI_PTL_addr_table[use_rank].process,
                     MPIDI_CH4_NMI_PTL_addr_table[use_rank].pt, match_bits, 0, sreq, ptl_hdr);
    } else {
        /* copy everything into pack_buffer */
        MPID_Segment *segment;
        MPI_Aint last;
        char *send_buf = NULL;

        send_buf = MPL_malloc(am_hdr_sz + data_sz);
        MPIU_Memcpy(send_buf, am_hdr, am_hdr_sz);
        segment = MPID_Segment_alloc();
        MPID_Segment_init(data, count, datatype, segment, 0);
        last = data_sz;
        MPID_Segment_pack(segment, 0, &last, send_buf + am_hdr_sz);
        MPIU_Assert(last == data_sz);
        sreq->dev.ch4.ch4r.netmod_am.portals4.pack_buffer = send_buf;

        ret = PtlPut(MPIDI_CH4_NMI_PTL_global.md, (ptl_size_t)send_buf, am_hdr_sz + data_sz,
                     PTL_ACK_REQ, MPIDI_CH4_NMI_PTL_addr_table[use_rank].process,
                     MPIDI_CH4_NMI_PTL_addr_table[use_rank].pt, match_bits, 0, sreq, ptl_hdr);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_send_amv_reply(uint64_t reply_token,
                                              int handler_id,
                                              struct iovec *am_hdr,
                                              size_t iov_len,
                                              const void *data,
                                              MPI_Count count,
                                              MPI_Datatype datatype, MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_CH4_NM_am_hdr_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}

static inline int MPIDI_CH4_NM_inject_am_hdr(int rank,
                                             MPID_Comm * comm,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, ret, c;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPID_Datatype  *dt_ptr;
    int             dt_contig;
    ptl_hdr_data_t   ptl_hdr;
    ptl_match_bits_t match_bits;
    char *send_buf = NULL;
    int complete = 0;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    ptl_hdr = MPIDI_CH4_NMI_PTL_init_am_hdr(handler_id, comm->rank, 0);
    match_bits = MPIDI_CH4_NMI_PTL_init_tag(comm->context_id, MPIDI_CH4_NMI_PTL_AM_TAG);

    if (dt_contig) {
        ret = PtlPut(MPIDI_CH4_NMI_PTL_global.md, (ptl_size_t)am_hdr, am_hdr_sz,
                     PTL_ACK_REQ, MPIDI_CH4_NMI_PTL_addr_table[rank].process,
                     MPIDI_CH4_NMI_PTL_addr_table[rank].pt, match_bits, 0, &complete, ptl_hdr);
    }

    /* wait until request is complete */
    while (!complete) {
        MPIDI_CH4_NM_progress(NULL, FALSE);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_inject_am_hdr_reply(uint64_t reply_token,
                                                   int handler_id,
                                                   const void *am_hdr, size_t am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS, ret, c;
    size_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPID_Datatype  *dt_ptr;
    int             dt_contig;
    ptl_hdr_data_t   ptl_hdr;
    ptl_match_bits_t match_bits;
    char *send_buf = NULL;
    int complete = 0;
    MPIDI_CH4_NMI_PTL_am_reply_token_t use_token;
    MPID_Comm *use_comm;
    int use_rank;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);

    use_token.val = reply_token;
    use_comm = MPIDI_CH4R_context_id_to_comm(use_token.data.context_id);
    use_rank = use_token.data.src_rank;

    ptl_hdr = MPIDI_CH4_NMI_PTL_init_am_hdr(handler_id, use_comm->rank, 0);
    match_bits = MPIDI_CH4_NMI_PTL_init_tag(use_comm->context_id, MPIDI_CH4_NMI_PTL_AM_TAG);

    if (dt_contig) {
        ret = PtlPut(MPIDI_CH4_NMI_PTL_global.md, (ptl_size_t)am_hdr, am_hdr_sz,
                     PTL_ACK_REQ, MPIDI_CH4_NMI_PTL_addr_table[use_rank].process,
                     MPIDI_CH4_NMI_PTL_addr_table[use_rank].pt, match_bits, 0, &complete, ptl_hdr);
    }

    /* wait until request is complete */
    while (!complete) {
        MPIDI_CH4_NM_progress(NULL, FALSE);
    }

 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}

static inline size_t MPIDI_CH4_NM_am_inject_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}


#endif /* NETMOD_PORTALS4_AM_H_INCLUDED */
