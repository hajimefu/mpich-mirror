/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef PROGRESS_H_INCLUDED
#define PROGRESS_H_INCLUDED

#include "impl.h"
//#include "events.h"

static inline int MPIDI_UCX_am_handler(void *msg, size_t msg_sz)
{
    int mpi_errno;
    MPIR_Request *rreq;
    void *p_data;
    void *in_data;
    size_t data_sz, in_data_sz;
    MPIDI_CH4_NM_am_completion_handler_fn cmpl_handler_fn;
    struct iovec *iov;
    int i, is_contig, iov_len;
    size_t done, curr_len, rem;
    MPIDI_UCX_am_header_t *msg_hdr = (MPIDI_UCX_am_header_t *)msg;

    p_data = in_data = (char *) msg_hdr->payload + (msg_sz - msg_hdr->data_sz - sizeof(*msg_hdr));
    in_data_sz = data_sz = msg_hdr->data_sz;

    MPIDI_UCX_global.am_handlers[msg_hdr->handler_id](msg_hdr->payload,
                                                              &p_data, &data_sz,
                                                              &is_contig,
                                                              &cmpl_handler_fn,
                                                              &rreq);

    if(!rreq)
        goto fn_exit;

    if((!p_data || !data_sz) && cmpl_handler_fn) {
        cmpl_handler_fn(rreq);
        goto fn_exit;
    }

    if(is_contig) {
        if(in_data_sz > data_sz) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        } else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }

        data_sz = MPL_MIN(data_sz, in_data_sz);
        MPIU_Memcpy(p_data, in_data, data_sz);
        MPIR_STATUS_SET_COUNT(rreq->status, data_sz);
    } else {
        done = 0;
        rem = in_data_sz;
        iov = (struct iovec *) p_data;
        iov_len = data_sz;

        for(i = 0; i < iov_len && rem > 0; i++) {
            curr_len = MPL_MIN(rem, iov[i].iov_len);
            MPIU_Memcpy(iov[i].iov_base, (char *) in_data + done, curr_len);
            rem -= curr_len;
            done += curr_len;
        }

        if(rem) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        } else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }

        MPIR_STATUS_SET_COUNT(rreq->status, done);
    }

    if(cmpl_handler_fn) {
        cmpl_handler_fn(rreq);
    }

 fn_exit:
    return mpi_errno;
}

static inline void MPIDI_UCX_Handle_am_recv(void *request, ucs_status_t status,
                                                    ucp_tag_recv_info_t * info)
{
    int mpi_errno = MPI_SUCCESS;
    static int am_recv_idx = 0;

    if (status == UCS_ERR_CANCELED) {
        goto fn_exit;
    }

    /* call the AM handler */
    MPIDI_UCX_am_handler(MPIDI_UCX_global.am_bufs[am_recv_idx], info->length);

    /* update the idx */
    ++am_recv_idx;
    if (am_recv_idx == MPIDI_UCX_NUM_AM_BUFFERS) {
        am_recv_idx = 0;
    }

 fn_exit:
    return;
 fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_progress
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_progress(void *netmod_context, int blocking)
{
    int mpi_errno = MPI_SUCCESS;
    static int am_repost_idx = 0;

    ucp_worker_progress(MPIDI_UCX_global.worker);

    while (ucp_request_is_completed(MPIDI_UCX_global.ucp_am_requests[am_repost_idx])) {
        /* release the ucp request */
        ucp_request_release(MPIDI_UCX_global.ucp_am_requests[am_repost_idx]);

        /* repost the buffer */
        MPIDI_UCX_global.ucp_am_requests[am_repost_idx] =
            (MPIDI_UCX_ucp_request_t*)ucp_tag_recv_nb(MPIDI_UCX_global.worker,
                                                              MPIDI_UCX_global.am_bufs[am_repost_idx],
                                                              MPIDI_UCX_MAX_AM_EAGER_SZ,
                                                              ucp_dt_make_contig(1),
                                                              MPIDI_UCX_AM_TAG,
                                                              ~MPIDI_UCX_AM_TAG,
                                                              &MPIDI_UCX_Handle_am_recv);
        MPIDI_CH4_UCX_REQUEST(MPIDI_UCX_global.ucp_am_requests[am_repost_idx], tag_recv_nb);

        /* update the idx */
        ++am_repost_idx;
        if (am_repost_idx == MPIDI_UCX_NUM_AM_BUFFERS) {
            am_repost_idx = 0;
        }
    }
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_WORKER_MUTEX);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* PROGRESS_H_INCLUDED */
