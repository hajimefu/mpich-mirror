/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2015 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */
#ifndef NETMOD_UCX_PROGRESS_H_INCLUDED
#define NETMOD_UCX_PROGRESS_H_INCLUDED

#include "impl.h"
//#include "events.h"

static inline int MPIDI_CH4_NMI_UCX_Am_handler(void *msg)
{
    int mpi_errno;
    MPID_Request *rreq;
    void *p_data;
    void *in_data;
    MPIDI_msg_sz_t data_sz, in_data_sz;
    MPIDI_CH4_NM_am_completion_handler_fn cmpl_handler_fn;
    struct iovec *iov;
    int i, is_contig, iov_len;
    size_t done, curr_len, rem;
    MPIDI_CH4_NMI_UCX_Am_reply_token_t reply_token;
    MPIDI_CH4_NMI_UCX_Am_header_t *msg_hdr = (MPIDI_CH4_NMI_UCX_Am_header_t *)msg;

    reply_token.data.context_id = msg_hdr->context_id;
    reply_token.data.src_rank   = msg_hdr->src_rank;

    p_data = in_data = (char *) msg_hdr->payload + msg_hdr->am_hdr_sz;
    in_data_sz = data_sz = msg_hdr->data_sz;

    MPIDI_CH4_NMI_UCX_Global.am_handlers[msg_hdr->handler_id](msg_hdr->payload,
                                                              reply_token.val,
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

static inline void MPIDI_CH4_NMI_UCX_Handle_am_recv(void *request, ucs_status_t status,
                                                    ucp_tag_recv_info_t * info)
{
    int mpi_errno = MPI_SUCCESS;
    static int am_recv_idx = 0;

    if (status == UCS_ERR_CANCELED) {
        goto fn_exit;
    }

    /* call the AM handler */
    MPIDI_CH4_NMI_UCX_Am_handler(MPIDI_CH4_NMI_UCX_Global.am_bufs[am_recv_idx]);

    /* update the idx */
    ++am_recv_idx;
    if (am_recv_idx == MPIDI_CH4_NMI_UCX_NUM_AM_BUFFERS) {
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

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_WORKER_MUTEX);
    ucp_worker_progress(MPIDI_CH4_NMI_UCX_Global.worker);

    while (ucp_request_is_completed(MPIDI_CH4_NMI_UCX_Global.ucp_am_requests[am_repost_idx])) {
        /* release the ucp request */
        ucp_request_release(MPIDI_CH4_NMI_UCX_Global.ucp_am_requests[am_repost_idx]);

        /* repost the buffer */
        MPIDI_CH4_NMI_UCX_Global.ucp_am_requests[am_repost_idx] =
            (MPIDI_CH4_NMI_UCX_Ucp_request_t*)ucp_tag_recv_nb(MPIDI_CH4_NMI_UCX_Global.worker,
                                                              MPIDI_CH4_NMI_UCX_Global.am_bufs[am_repost_idx],
                                                              MPIDI_CH4_NMI_UCX_MAX_AM_EAGER_SZ,
                                                              ucp_dt_make_contig(1),
                                                              MPIDI_CH4_NMI_UCX_AM_TAG,
                                                              ~MPIDI_CH4_NMI_UCX_AM_TAG,
                                                              &MPIDI_CH4_NMI_UCX_Handle_am_recv);
        MPIDI_CH4_UCX_REQUEST(MPIDI_CH4_NMI_UCX_Global.ucp_am_requests[am_repost_idx], tag_recv_nb);

        /* update the idx */
        ++am_repost_idx;
        if (am_repost_idx == MPIDI_CH4_NMI_UCX_NUM_AM_BUFFERS) {
            am_repost_idx = 0;
        }
    }
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_WORKER_MUTEX);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
#endif
