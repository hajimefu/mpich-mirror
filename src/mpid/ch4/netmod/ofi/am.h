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
#ifndef NETMOD_OFI_AM_H_INCLUDED
#define NETMOD_OFI_AM_H_INCLUDED
#include "impl.h"
#include "am_impl.h"
#include "am_events.h"

static inline int MPIDI_OFI_progress_do_queue(void *netmod_context);

static inline void MPIDI_CH4_NM_am_request_init(MPIR_Request *req)
{
    MPIDI_OFI_AMREQUEST(req, req_hdr) = NULL;
}

static inline void MPIDI_CH4_NM_am_request_finalize(MPIR_Request *req)
{
    MPIDI_OFI_am_clear_request(req);
}

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

    if(handler_id > MPIDI_OFI_MAX_AM_HANDLERS) {
        mpi_errno = MPI_ERR_OTHER;
        goto fn_fail;
    }

    MPIDI_Global.am_handlers[handler_id] = target_handler_fn;
    MPIDI_Global.am_send_cmpl_handlers[handler_id] = origin_handler_fn;
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REG_HDR_HANDLER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_am_hdr(int           rank,
                                           MPIR_Comm    *comm,
                                           int           handler_id,
                                           const void   *am_hdr,
                                           size_t        am_hdr_sz,
                                           MPIR_Request *sreq,
                                           void         *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR);

    mpi_errno = MPIDI_OFI_do_send_am_header(rank,
                                                    comm,
                                                    handler_id,
                                                    am_hdr,
                                                    am_hdr_sz,
                                                    sreq,
                                                    FALSE);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_am(int           rank,
                                       MPIR_Comm    *comm,
                                       int           handler_id,
                                       const void   *am_hdr,
                                       size_t        am_hdr_sz,
                                       const void   *data,
                                       MPI_Count     count,
                                       MPI_Datatype  datatype,
                                       MPIR_Request *sreq,
                                       void         *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM);
    mpi_errno = MPIDI_OFI_do_send_am(rank, comm, handler_id,
                                             am_hdr, am_hdr_sz, data, count,
                                             datatype, sreq,FALSE);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_amv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_amv(int rank,
                                        MPIR_Comm *comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        const void *data,
                                        MPI_Count count,
                                        MPI_Datatype datatype,
                                        MPIR_Request *sreq,
                                        void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, is_allocated;
    size_t am_hdr_sz = 0, i;
    char *am_hdr_buf;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AMV);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AMV);

    for(i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    if(am_hdr_sz > MPIDI_OFI_BUF_POOL_SIZE) {
        am_hdr_buf = (char *) MPL_malloc(am_hdr_sz);
        is_allocated = 1;
    } else {
        am_hdr_buf = (char *)MPIDI_CH4R_get_buf(MPIDI_Global.am_buf_pool);
        is_allocated = 0;
    }

    MPIU_Assert(am_hdr_buf);
    am_hdr_sz = 0;

    for(i = 0; i < iov_len; i++) {
        MPIU_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_CH4_NM_send_am(rank, comm, handler_id, am_hdr_buf, am_hdr_sz,
                                     data, count, datatype, sreq, netmod_context);

    if(is_allocated)
        MPL_free(am_hdr_buf);
    else
        MPIDI_CH4R_release_buf(am_hdr_buf);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AMV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_amv_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_amv_hdr(int rank,
                                            MPIR_Comm *comm,
                                            int handler_id,
                                            struct iovec *am_hdr,
                                            size_t iov_len,
                                            MPIR_Request *sreq, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, is_allocated;
    size_t am_hdr_sz = 0, i;
    char *am_hdr_buf;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AMV_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AMV_HDR);

    for(i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    /* TODO: avoid the malloc here, use the am_hdr directly */
    if(am_hdr_sz > MPIDI_OFI_BUF_POOL_SIZE) {
        am_hdr_buf = (char *) MPL_malloc(am_hdr_sz);
        is_allocated = 1;
    } else {
        am_hdr_buf = (char *) MPIDI_CH4R_get_buf(MPIDI_Global.am_buf_pool);
        is_allocated = 0;
    }

    MPIU_Assert(am_hdr_buf);
    am_hdr_sz = 0;

    for(i = 0; i < iov_len; i++) {
        MPIU_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_CH4_NM_send_am_hdr(rank, comm, handler_id, am_hdr_buf, am_hdr_sz,
                                         sreq, netmod_context);

    if(is_allocated)
        MPL_free(am_hdr_buf);
    else
        MPIDI_CH4R_release_buf(am_hdr_buf);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AMV_HDR);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_am_hdr_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_am_hdr_reply(MPIU_Context_id_t context_id,
                                                 int           src_rank,
                                                 int           handler_id,
                                                 const void   *am_hdr,
                                                 size_t        am_hdr_sz,
                                                 MPIR_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_HDR_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM_HDR_REPLY);
    mpi_errno = MPIDI_OFI_do_send_am_header(src_rank,
                                                    MPIDI_CH4U_context_id_to_comm(context_id),
                                                    handler_id,
                                                    am_hdr,
                                                    am_hdr_sz,
                                                    sreq,
                                                    TRUE);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_HDR_REPLY);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_am_reply
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_am_reply(MPIU_Context_id_t context_id,
                                             int           src_rank,
                                             int           handler_id,
                                             const void   *am_hdr,
                                             size_t        am_hdr_sz,
                                             const void   *data,
                                             MPI_Count     count,
                                             MPI_Datatype  datatype,
                                             MPIR_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_SEND_AM_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_SEND_AM_REPLY);
    mpi_errno = MPIDI_OFI_do_send_am(src_rank,
                                             MPIDI_CH4U_context_id_to_comm(context_id),
                                             handler_id,
                                             am_hdr,
                                             am_hdr_sz,
                                             data,
                                             count,
                                             datatype,
                                             sreq,
                                             TRUE);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_SEND_AM_REPLY);
    return mpi_errno;
}

static inline int MPIDI_CH4_NM_send_amv_reply(MPIU_Context_id_t context_id,
                                              int           src_rank,
                                              int           handler_id,
                                              struct iovec *am_hdr,
                                              size_t        iov_len,
                                              const void   *data,
                                              MPI_Count     count,
                                              MPI_Datatype  datatype,
                                              MPIR_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS, is_allocated;
    size_t am_hdr_sz = 0, i;
    char *am_hdr_buf;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AMV_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AMV_REPLY);

    for(i = 0; i < iov_len; i++) {
        am_hdr_sz += am_hdr[i].iov_len;
    }

    /* TODO: avoid the malloc here, use the am_hdr directly */
    if(am_hdr_sz > MPIDI_OFI_BUF_POOL_SIZE) {
        am_hdr_buf = (char *) MPL_malloc(am_hdr_sz);
        is_allocated = 1;
    } else {
        am_hdr_buf = (char *) MPIDI_CH4R_get_buf(MPIDI_Global.am_buf_pool);
        is_allocated = 0;
    }

    MPIU_Assert(am_hdr_buf);
    am_hdr_sz = 0;

    for(i = 0; i < iov_len; i++) {
        MPIU_Memcpy(am_hdr_buf + am_hdr_sz, am_hdr[i].iov_base, am_hdr[i].iov_len);
        am_hdr_sz += am_hdr[i].iov_len;
    }

    mpi_errno = MPIDI_CH4_NM_send_am_reply(context_id, src_rank, handler_id, am_hdr_buf, am_hdr_sz,
                                           data, count, datatype, sreq);

    if(is_allocated)
        MPL_free(am_hdr_buf);
    else
        MPIDI_CH4R_release_buf(am_hdr_buf);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AMV_REPLY);
    return mpi_errno;
}

static inline size_t MPIDI_CH4_NM_am_hdr_max_sz(void)
{
    /* Maximum size that fits in short send */
    size_t max_shortsend = MPIDI_OFI_DEFAULT_SHORT_SEND_SIZE -
                           (sizeof(MPIDI_OFI_am_header_t) + sizeof(MPIDI_OFI_lmt_msg_payload_t));
    /* Maximum payload size representable by MPIDI_OFI_am_header_t::am_hdr_sz field */
    size_t max_representable = (1 << MPIDI_OFI_AM_HDR_SZ_BITS) - 1;

    return MPL_MIN(max_shortsend, max_representable);
}

static inline int MPIDI_CH4_NM_inject_am_hdr(int         rank,
                                             MPIR_Comm  *comm,
                                             int         handler_id,
                                             const void *am_hdr,
                                             size_t      am_hdr_sz,
                                             void       *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR);
    mpi_errno = MPIDI_OFI_do_inject(rank, comm,
                                            handler_id, am_hdr, am_hdr_sz,
                                            netmod_context, FALSE, TRUE, TRUE);

    if(mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_inject_am_hdr_reply(MPIU_Context_id_t context_id,
                                                   int         src_rank,
                                                   int         handler_id,
                                                   const void *am_hdr,
                                                   size_t      am_hdr_sz)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR_REPLY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR_REPLY);

    mpi_errno = MPIDI_OFI_do_inject(src_rank, MPIDI_CH4U_context_id_to_comm(context_id),
                                            handler_id, am_hdr,
                                            am_hdr_sz, NULL, TRUE, TRUE, FALSE);

    if(mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_INJECT_AM_HDR);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline size_t MPIDI_CH4_NM_am_inject_max_sz(void)
{
    if (unlikely(MPIDI_Global.max_buffered_send < sizeof(MPIDI_OFI_am_header_t)))
        return 0;
    return MPIDI_Global.max_buffered_send - sizeof(MPIDI_OFI_am_header_t);
}

#endif /* NETMOD_OFI_AM_H_INCLUDED */
