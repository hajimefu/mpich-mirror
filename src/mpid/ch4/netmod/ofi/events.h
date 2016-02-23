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
#ifndef NETMOD_OFI_EVENTS_H_INCLUDED
#define NETMOD_OFI_EVENTS_H_INCLUDED

#include "impl.h"
#include "am_impl.h"
#include "am_events.h"
#include "control.h"

static inline int MPIDI_CH4_NMI_OFI_Get_huge_event(struct fi_cq_tagged_entry *wc, MPID_Request *req);

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Peek_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Peek_event(struct fi_cq_tagged_entry *wc, MPID_Request *rreq)
{
    size_t count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EVENT);
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_id)  = MPIDI_CH4_NMI_OFI_PEEK_FOUND;
    rreq->status.MPI_SOURCE = MPIDI_CH4_NMI_OFI_Init_get_source(wc->tag);
    rreq->status.MPI_TAG    = MPIDI_CH4_NMI_OFI_Init_get_tag(wc->tag);
    count                   = wc->len;
    rreq->status.MPI_ERROR  = MPI_SUCCESS;
    MPIR_STATUS_SET_COUNT(rreq->status, count);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Peek_empty_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Peek_empty_event(struct fi_cq_tagged_entry *wc, MPID_Request *rreq)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EMPTY_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EMPTY_EVENT);
    MPIDI_CH4_NMI_OFI_Dynamic_process_request_t *ctrl;

    switch(MPIDI_CH4_NMI_OFI_REQUEST(rreq, event_id)) {
        case MPIDI_CH4_NMI_OFI_EVENT_PEEK:
            MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_id)  = MPIDI_CH4_NMI_OFI_PEEK_NOT_FOUND;
            rreq->status.MPI_ERROR  = MPI_SUCCESS;
            break;

        case MPIDI_CH4_NMI_OFI_EVENT_ACCEPT_PROBE:
            ctrl       = (MPIDI_CH4_NMI_OFI_Dynamic_process_request_t *)rreq;
            ctrl->done = MPIDI_CH4_NMI_OFI_PEEK_NOT_FOUND;
            break;

        default:
            MPIU_Assert(0);
            break;
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EMPTY_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Recv_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Recv_event(struct fi_cq_tagged_entry *wc, MPID_Request *rreq)
{
    int      mpi_errno = MPI_SUCCESS;
    MPI_Aint last;
    size_t   count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV_EVENT);

    rreq->status.MPI_ERROR  = MPI_SUCCESS;
    rreq->status.MPI_SOURCE = MPIDI_CH4_NMI_OFI_Init_get_source(wc->tag);
    rreq->status.MPI_TAG    = MPIDI_CH4_NMI_OFI_Init_get_tag(wc->tag);
    count                   = wc->len;
    MPIR_STATUS_SET_COUNT(rreq->status, count);

#ifdef MPIDI_BUILD_CH4_SHM

    if(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(rreq)) {
        int continue_matching = 1;

        MPIDI_CH4R_anysource_matched(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(rreq), MPIDI_CH4R_NETMOD, &continue_matching);

        /* It is always possible to cancel a request on shm side w/o an aux thread */

        /* Decouple requests */
        if(unlikely(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(rreq))) {
            MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(rreq)) = NULL;
            MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(rreq) = NULL;
        }

        if(!continue_matching) goto fn_exit;
    }

#endif

    if(MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig)) {
        last = count;
        MPID_Segment_unpack(&MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig->segment), 0, &last,
                            MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig->pack_buffer));
        MPL_free(MPIDI_CH4_NMI_OFI_REQUEST(rreq, noncontig));
        if(last != (MPI_Aint)count) {
            rreq->status.MPI_ERROR =
                MPIR_Err_create_code(MPI_SUCCESS,
                                     MPIR_ERR_RECOVERABLE,
                                     __FUNCTION__, __LINE__,
                                     MPI_ERR_TYPE, "**dtypemismatch", 0);
        }
    }

    dtype_release_if_not_builtin(MPIDI_CH4_NMI_OFI_REQUEST(rreq, datatype));

    /* If syncronous, ack and complete when the ack is done */
    if(unlikely(MPIDI_CH4_NMI_OFI_Is_tag_sync(wc->tag))) {
        uint64_t ss_bits = MPIDI_CH4_NMI_OFI_Init_sendtag(MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_id),
                                                          MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_comm->rank),
                                                          rreq->status.MPI_TAG,
                                                          MPIDI_CH4_NMI_OFI_SYNC_SEND_ACK);
        MPID_Comm *c = MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_comm);
        int r = rreq->status.MPI_SOURCE;
        MPIDI_CH4_NMI_OFI_CALL_RETRY_NOLOCK(fi_tinject(MPIDI_CH4_NMI_OFI_EP_TX_TAG(0), NULL, 0,
                                                       MPIDI_CH4_NMI_OFI_Comm_to_phys(c, r, MPIDI_CH4_NMI_OFI_API_TAG),
                                                       ss_bits), tsendsync);

    }

    MPIDI_CH4R_request_complete(rreq);

    /* Polling loop will check for truncation */
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV_EVENT);
    return mpi_errno;
fn_fail:
    rreq->status.MPI_ERROR = mpi_errno;
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Recv_huge_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Recv_huge_event(struct fi_cq_tagged_entry *wc, MPID_Request *rreq)
{
    MPIDI_CH4_NMI_OFI_Huge_recv_t *recv;
    MPIDI_CH4_NMI_OFI_Huge_chunk_t *hc;
    MPID_Comm *comm_ptr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV_HUGE_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV_HUGE_EVENT);

    /* Look up the receive sequence number and chunk queue */
    comm_ptr = MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_comm);
    recv = (MPIDI_CH4_NMI_OFI_Huge_recv_t *) MPIDI_CH4_NMI_OFI_Map_lookup(MPIDI_CH4_NMI_OFI_COMM(comm_ptr).huge_recv_counters,
                                                                          MPIDI_CH4_NMI_OFI_Init_get_source(wc->tag));

    if(recv == MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND) {
        recv = (MPIDI_CH4_NMI_OFI_Huge_recv_t *) MPL_malloc(sizeof(*recv));
        recv->seqno = 0;
        MPIDI_CH4_NMI_OFI_Map_create(&recv->chunk_q);
        MPIDI_CH4_NMI_OFI_Map_set(MPIDI_CH4_NMI_OFI_COMM(comm_ptr).huge_recv_counters, MPIDI_CH4_NMI_OFI_Init_get_source(wc->tag), recv);
    }

    /* Look up the receive in the chunk queue */
    hc = (MPIDI_CH4_NMI_OFI_Huge_chunk_t *) MPIDI_CH4_NMI_OFI_Map_lookup(recv->chunk_q, recv->seqno);

    if(hc == MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND) {
        hc = (MPIDI_CH4_NMI_OFI_Huge_chunk_t *) MPL_malloc(sizeof(*hc));
        memset(hc, 0, sizeof(*hc));
        hc->event_id = MPIDI_CH4_NMI_OFI_EVENT_GET_HUGE;
        MPIDI_CH4_NMI_OFI_Map_set(recv->chunk_q, recv->seqno, hc);
    }

    recv->seqno++;
    hc->localreq = rreq;
    hc->done_fn  = MPIDI_CH4_NMI_OFI_Recv_event;
    hc->wc       = *wc;
    MPIDI_CH4_NMI_OFI_Get_huge_event(NULL, (MPID_Request *) hc);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV_HUGE_EVENT);
    return MPI_SUCCESS;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Send_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Send_event(struct fi_cq_tagged_entry *wc, MPID_Request *sreq)
{
    int c;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_EVENT);

    MPIR_cc_decr(sreq->cc_ptr, &c);

    if(c == 0) {
        if(MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig))
            MPL_free(MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig));

        dtype_release_if_not_builtin(MPIDI_CH4_NMI_OFI_REQUEST(sreq, datatype));
        MPIDI_CH4R_Request_release(sreq);
    }   /* c != 0, ssend */

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Send_huge_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Send_huge_event(struct fi_cq_tagged_entry *wc, MPID_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    int c;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_EVENT_HUGE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_EVENT_HUGE);

    MPIR_cc_decr(sreq->cc_ptr, &c);

    if(c == 0) {
        MPID_Comm *comm;
        void *ptr;
        MPIDI_CH4_NMI_OFI_Huge_counter_t *cntr;
        comm = MPIDI_CH4_NMI_OFI_REQUEST(sreq, util_comm);
        ptr = MPIDI_CH4_NMI_OFI_Map_lookup(MPIDI_CH4_NMI_OFI_COMM(comm).huge_send_counters, MPIDI_CH4_NMI_OFI_REQUEST(sreq, util_id));
        MPIU_Assert(ptr != MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND);
        cntr = (MPIDI_CH4_NMI_OFI_Huge_counter_t *) ptr;
        cntr->outstanding--;

        if(cntr->outstanding == 0) {
            MPIDI_CH4_NMI_OFI_Send_control_t ctrl;
            uint64_t key;
            int      key_back;
            MPIDI_CH4_NMI_OFI_Map_erase(MPIDI_CH4_NMI_OFI_COMM(comm).huge_send_counters, MPIDI_CH4_NMI_OFI_REQUEST(sreq, util_id));
            key          = fi_mr_key(cntr->mr);
            key_back     = (key >> MPIDI_Global.huge_rma_shift);
            MPIDI_CH4_NMI_OFI_Index_allocator_free(MPIDI_CH4_NMI_OFI_COMM(comm).rma_id_allocator,key_back);
            MPIDI_CH4_NMI_OFI_CALL_NOLOCK(fi_close(&cntr->mr->fid), mr_unreg);
            MPL_free(ptr);
            ctrl.type = MPIDI_CH4_NMI_OFI_CTRL_HUGE_CLEANUP;
            MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Do_control_send(&ctrl, NULL, 0, MPIDI_CH4_NMI_OFI_REQUEST(sreq, util_id), comm, NULL));
        }

        if(MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig))
            MPL_free(MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig));

        dtype_release_if_not_builtin(MPIDI_CH4_NMI_OFI_REQUEST(sreq, datatype));
        MPIDI_CH4R_Request_release(sreq);
    }   /* c != 0, ssend */

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_EVENT_HUGE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Ssend_ack_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Ssend_ack_event(struct fi_cq_tagged_entry *wc, MPID_Request *sreq)
{
    int mpi_errno;
    MPIDI_CH4_NMI_OFI_Ssendack_request_t *req = (MPIDI_CH4_NMI_OFI_Ssendack_request_t *) sreq;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SSEND_ACK_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SSEND_ACK_EVENT);
    mpi_errno = MPIDI_CH4_NMI_OFI_Send_event(NULL, req->signal_req);
    MPIDI_CH4_NMI_OFI_Ssendack_request_t_tls_free(req);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SSEND_ACK_EVENT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Get_huge_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Get_huge_event(struct fi_cq_tagged_entry *wc,
                                                   MPID_Request      *req)
{
    int                 mpi_errno = MPI_SUCCESS;
    MPIDI_CH4_NMI_OFI_Huge_chunk_t *hc = (MPIDI_CH4_NMI_OFI_Huge_chunk_t *)req;
    uint64_t            remote_key;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_GETHUGE_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_GETHUGE_EVENT);

    if(hc->localreq && hc->cur_offset!=0) {
        size_t bytesSent  = hc->cur_offset - MPIDI_Global.max_send;
        size_t bytesLeft  = hc->remote_info.msgsize - bytesSent - MPIDI_Global.max_send;
        size_t bytesToGet = (bytesLeft<=MPIDI_Global.max_send)?bytesLeft:MPIDI_Global.max_send;

        if(bytesToGet == 0ULL) {
            MPIDI_CH4_NMI_OFI_Send_control_t ctrl;
            hc->wc.len = hc->cur_offset;
            hc->done_fn(&hc->wc, hc->localreq);
            ctrl.type = MPIDI_CH4_NMI_OFI_CTRL_HUGEACK;
            MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Do_control_send(&ctrl,NULL,0,hc->remote_info.origin_rank,
                                                                             hc->comm_ptr,hc->remote_info.ackreq));
            MPL_free(hc);
            goto fn_exit;
        }

        remote_key = hc->remote_info.rma_key << MPIDI_Global.huge_rma_shift;
        MPIDI_CH4_NMI_OFI_CALL_RETRY_NOLOCK(fi_read(MPIDI_CH4_NMI_OFI_EP_TX_RMA(0),                                           /* endpoint     */
                                                    (void *)((uintptr_t)hc->wc.buf + hc->cur_offset),       /* local buffer */
                                                    bytesToGet,                                             /* bytes        */
                                                    NULL,                                                   /* descriptor   */
                                                    MPIDI_CH4_NMI_OFI_Comm_to_phys(hc->comm_ptr,hc->remote_info.origin_rank,MPIDI_CH4_NMI_OFI_API_MSG), /* Destination  */
                                                    hc->cur_offset,                                         /* remote maddr */
                                                    remote_key,                                             /* Key          */
                                                    (void *)&hc->context),rdma_readfrom);                   /* Context      */
        hc->cur_offset+=bytesToGet;
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_GETHUGE_EVENT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Chunk_done_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Chunk_done_event(struct fi_cq_tagged_entry *wc,
                                                     MPID_Request      *req)
{
    int c;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CHUNK_DONE_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CHUNK_DONE_EVENT);

    MPIDI_CH4_NMI_OFI_Chunk_request *creq = (MPIDI_CH4_NMI_OFI_Chunk_request *)req;
    MPIR_cc_decr(creq->parent->cc_ptr, &c);

    if(c == 0)MPIDI_CH4R_Request_release(creq->parent);

    MPL_free(creq);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CHUNK_DONE_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Rma_done_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Rma_done_event(struct fi_cq_tagged_entry *wc,
                                                   MPID_Request      *in_req)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_RMA_DONE_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_RMA_DONE_EVENT);

    MPIDI_CH4_NMI_OFI_Win_request_t *req = (MPIDI_CH4_NMI_OFI_Win_request_t *)in_req;
    MPIDI_CH4_NMI_OFI_Win_request_t_complete(req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_RMA_DONE_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Accept_probe_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Accept_probe_event(struct fi_cq_tagged_entry *wc,
                                                       MPID_Request      *rreq)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_ACCEPT_PROBE_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_ACCEPT_PROBE_EVENT);
    MPIDI_CH4_NMI_OFI_Dynamic_process_request_t *ctrl = (MPIDI_CH4_NMI_OFI_Dynamic_process_request_t *)rreq;
    ctrl->source = MPIDI_CH4_NMI_OFI_Init_get_source(wc->tag);
    ctrl->tag    = MPIDI_CH4_NMI_OFI_Init_get_tag(wc->tag);
    ctrl->msglen = wc->len;
    ctrl->done   = MPIDI_CH4_NMI_OFI_PEEK_FOUND;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_ACCEPT_PROBE_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Dynproc_done_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Dynproc_done_event(struct fi_cq_tagged_entry *wc,
                                                       MPID_Request      *rreq)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_DYNPROC_DONE_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_DYNPROC_DONE_EVENT);
    MPIDI_CH4_NMI_OFI_Dynamic_process_request_t *ctrl = (MPIDI_CH4_NMI_OFI_Dynamic_process_request_t *)rreq;
    ctrl->done++;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_DYNPROC_DONE_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Am_send_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Am_send_event(struct fi_cq_tagged_entry *wc,
                                                  MPID_Request      *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4_NMI_OFI_Am_header_t           *msg_hdr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);

    msg_hdr  = &MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq,msg_hdr);
    MPIDI_CH4_NMI_OFI_Am_request_complete(sreq);

    switch(msg_hdr->am_type) {
        case MPIDI_AMTYPE_LMT_ACK:
        case MPIDI_AMTYPE_LMT_REQ:
        case MPIDI_AMTYPE_LMT_HDR_REQ:
        case MPIDI_AMTYPE_LONG_HDR_REQ:
        case MPIDI_AMTYPE_LONG_HDR_ACK:
            goto fn_exit;

        default:
            break;
    }

    if(MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer) {
        MPL_free(MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer);
        MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer = NULL;
    }

    mpi_errno = MPIDI_Global.am_send_cmpl_handlers[msg_hdr->handler_id](sreq);

    if(mpi_errno) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Am_recv_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Am_recv_event(struct fi_cq_tagged_entry *wc,
                                                  MPID_Request      *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4_NMI_OFI_Am_header_t         *am_hdr;
    MPIDI_CH4_NMI_OFI_Am_reply_token_t  reply_token;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_RECV_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_RECV_COMPLETION);

    am_hdr                      = (MPIDI_CH4_NMI_OFI_Am_header_t *) wc->buf;
    reply_token.data.context_id = am_hdr->context_id;
    reply_token.data.src_rank   = am_hdr->src_rank;

    switch(am_hdr->am_type) {
        case MPIDI_AMTYPE_SHORT_HDR:
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_short_am_hdr(am_hdr,
                                                              am_hdr->payload,
                                                              reply_token);

            if(mpi_errno) MPIR_ERR_POP(mpi_errno);

            break;

        case MPIDI_AMTYPE_SHORT:
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_short_am(am_hdr, reply_token);

            if(mpi_errno) MPIR_ERR_POP(mpi_errno);

            break;

        case MPIDI_AMTYPE_LMT_REQ:
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_long_am(am_hdr, reply_token);

            if(mpi_errno) MPIR_ERR_POP(mpi_errno);

            break;

        case MPIDI_AMTYPE_LMT_HDR_REQ:
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_long_am_hdr(am_hdr, reply_token);

            if(mpi_errno) MPIR_ERR_POP(mpi_errno);

            break;

        case MPIDI_AMTYPE_LMT_ACK:
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_lmt_ack(am_hdr, reply_token);

            if(mpi_errno) MPIR_ERR_POP(mpi_errno);

            break;

        case MPIDI_AMTYPE_LONG_HDR_REQ:
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_long_hdr(am_hdr, reply_token);

            if(mpi_errno) MPIR_ERR_POP(mpi_errno);

            break;

        case MPIDI_AMTYPE_LONG_HDR_ACK:
            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_long_hdr_ack(am_hdr, reply_token);

            if(mpi_errno) MPIR_ERR_POP(mpi_errno);

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
#define FUNCNAME MPIDI_CH4_NMI_OFI_Am_read_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Am_read_event(struct fi_cq_tagged_entry *wc,
                                                  MPID_Request      *dont_use_me)
{
    int                           mpi_errno      = MPI_SUCCESS;
    void                         *netmod_context = NULL;
    MPID_Request                 *rreq;
    MPIDI_CH4_NMI_OFI_Am_request_t *ofi_req;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);

    ofi_req = container_of(wc->op_context, MPIDI_CH4_NMI_OFI_Am_request_t, context);
    ofi_req->req_hdr->lmt_cntr--;

    if(ofi_req->req_hdr->lmt_cntr)
        goto fn_exit;

    switch(ofi_req->req_hdr->msg_hdr.am_type) {
        case MPIDI_AMTYPE_LMT_HDR_REQ:
            rreq = (MPID_Request *)ofi_req->req_hdr->rreq_ptr;
            MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, msg_hdr).am_type = MPIDI_AMTYPE_LMT_REQ;
            mpi_errno = MPIDI_CH4_NMI_OFI_Do_handle_long_am(&MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, msg_hdr),
                                                            &MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, lmt_info),
                                                            MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, am_hdr),
                                                            MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, clientdata).reply_token);

            if(mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

            MPIDI_CH4_NMI_OFI_Am_request_complete(rreq);
            goto fn_exit;

        case MPIDI_AMTYPE_LONG_HDR_REQ:
            rreq = (MPID_Request *)ofi_req->req_hdr->rreq_ptr;
            mpi_errno = MPIDI_CH4_NMI_OFI_Dispatch_ack(ofi_req->req_hdr->msg_hdr.src_rank,
                                                       ofi_req->req_hdr->msg_hdr.context_id,
                                                       MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, lmt_info.sreq_ptr),
                                                       MPIDI_AMTYPE_LONG_HDR_ACK, netmod_context);

            if(mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

            mpi_errno = MPIDI_CH4_NMI_OFI_Handle_short_am_hdr(&MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, msg_hdr),
                                                              MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, am_hdr),
                                                              MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, clientdata).reply_token);

            if(mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

            MPIDI_CH4_NMI_OFI_Am_request_complete(rreq);
            goto fn_exit;

        default:
            break;
    }

    rreq = (MPID_Request *)ofi_req->req_hdr->rreq_ptr;
    mpi_errno = MPIDI_CH4_NMI_OFI_Dispatch_ack(ofi_req->req_hdr->msg_hdr.src_rank,
                                               ofi_req->req_hdr->msg_hdr.context_id,
                                               MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(rreq, lmt_info).sreq_ptr,
                                               MPIDI_AMTYPE_LMT_ACK,netmod_context);

    if(mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_CH4_NMI_OFI_Am_request_complete(rreq);
    ofi_req->req_hdr->cmpl_handler_fn(rreq);
fn_exit:
    MPIDI_CH4R_release_buf((void *)ofi_req);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Am_repost_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Am_repost_event(struct fi_cq_tagged_entry *wc,
                                                    MPID_Request      *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REPOST_BUFFER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REPOST_BUFFER);

    mpi_errno = MPIDI_CH4_NMI_OFI_Repost_buffer(wc->op_context, rreq);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REPOST_BUFFER);
    return mpi_errno;
}

static inline int MPIDI_CH4_NMI_OFI_Dispatch_function(struct fi_cq_tagged_entry *wc,
                                                      MPID_Request      *req,
                                                      int                buffered)
{
    int mpi_errno;

    if(likely(MPIDI_CH4_NMI_OFI_REQUEST(req,event_id) == MPIDI_CH4_NMI_OFI_EVENT_SEND)) {
        mpi_errno = MPIDI_CH4_NMI_OFI_Send_event(wc,req);
        goto fn_exit;
    } else if(likely(MPIDI_CH4_NMI_OFI_REQUEST(req,event_id) == MPIDI_CH4_NMI_OFI_EVENT_RECV)) {
        mpi_errno = MPIDI_CH4_NMI_OFI_Recv_event(wc,req);
        goto fn_exit;
    } else if(likely(MPIDI_CH4_NMI_OFI_REQUEST(req,event_id) == MPIDI_CH4_NMI_OFI_EVENT_RMA_DONE)) {
        mpi_errno = MPIDI_CH4_NMI_OFI_Rma_done_event(wc,req);
        goto fn_exit;
    } else if(likely(MPIDI_CH4_NMI_OFI_REQUEST(req,event_id) == MPIDI_CH4_NMI_OFI_EVENT_AM_SEND)) {
        mpi_errno = MPIDI_CH4_NMI_OFI_Am_send_event(wc,req);
        goto fn_exit;
    } else if(likely(MPIDI_CH4_NMI_OFI_REQUEST(req,event_id) == MPIDI_CH4_NMI_OFI_EVENT_AM_RECV)) {
        mpi_errno = MPIDI_CH4_NMI_OFI_Am_recv_event(wc,req);

        if(unlikely((wc->flags & FI_MULTI_RECV) && !buffered))
            MPIDI_CH4_NMI_OFI_Am_repost_event(wc,req);

        goto fn_exit;
    } else if(likely(MPIDI_CH4_NMI_OFI_REQUEST(req,event_id) == MPIDI_CH4_NMI_OFI_EVENT_AM_READ)) {
        mpi_errno = MPIDI_CH4_NMI_OFI_Am_read_event(wc,req);
        goto fn_exit;
    } else if(unlikely(1)) {
        switch(MPIDI_CH4_NMI_OFI_REQUEST(req,event_id)) {
            case MPIDI_CH4_NMI_OFI_EVENT_AM_MULTI:
                mpi_errno = MPIDI_CH4_NMI_OFI_Am_repost_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_PEEK:
                mpi_errno = MPIDI_CH4_NMI_OFI_Peek_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_RECV_HUGE:
                mpi_errno = MPIDI_CH4_NMI_OFI_Recv_huge_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_SEND_HUGE:
                mpi_errno = MPIDI_CH4_NMI_OFI_Send_huge_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_SSEND_ACK:
                mpi_errno = MPIDI_CH4_NMI_OFI_Ssend_ack_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_GET_HUGE:
                mpi_errno = MPIDI_CH4_NMI_OFI_Get_huge_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_CHUNK_DONE:
                mpi_errno = MPIDI_CH4_NMI_OFI_Chunk_done_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_DYNPROC_DONE:
                mpi_errno = MPIDI_CH4_NMI_OFI_Dynproc_done_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_ACCEPT_PROBE:
                mpi_errno = MPIDI_CH4_NMI_OFI_Accept_probe_event(wc,req);
                break;

            case MPIDI_CH4_NMI_OFI_EVENT_ABORT:
            default:
                mpi_errno = MPI_SUCCESS;
                MPIU_Assert(0);
                break;
        }
    }

fn_exit:
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Get_buffered
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Get_buffered(struct fi_cq_tagged_entry *wc, ssize_t num)
{
    int rc = 0;

    if((MPIDI_Global.cq_buff_head != MPIDI_Global.cq_buff_tail) ||
       !slist_empty(&MPIDI_Global.cq_buff_list)) {
        if(MPIDI_Global.cq_buff_head != MPIDI_Global.cq_buff_tail) {
            wc[0]                     = MPIDI_Global.cq_buffered[MPIDI_Global.cq_buff_tail].cq_entry;
            MPIDI_Global.cq_buff_tail = (MPIDI_Global.cq_buff_tail + 1) % MPIDI_CH4_NMI_OFI_NUM_CQ_BUFFERED;
        } else {
            MPIDI_CH4_NMI_OFI_Cq_list_t     *MPIDI_CH4_NMI_OFI_Cq_list_entry;
            struct slist_entry *entry = slist_remove_head(&MPIDI_Global.cq_buff_list);
            MPIDI_CH4_NMI_OFI_Cq_list_entry             = container_of(entry, MPIDI_CH4_NMI_OFI_Cq_list_t, entry);
            wc[0]                     = MPIDI_CH4_NMI_OFI_Cq_list_entry->cq_entry;
            MPL_free((void *)MPIDI_CH4_NMI_OFI_Cq_list_entry);
        }

        rc = 1;
    }

    return rc;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Handle_cq_entries
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Handle_cq_entries(struct fi_cq_tagged_entry *wc,
                                                      ssize_t            num,
                                                      int                buffered)
{
    int i, mpi_errno;
    MPID_Request *req;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_CQ_ENTRIES);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_CQ_ENTRIES);

    for(i = 0; i < num; i++) {
        req = MPIDI_CH4_NMI_OFI_Context_to_request(wc[i].op_context);
        MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dispatch_function(&wc[i],req,buffered));
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_CQ_ENTRIES);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Handle_cq_error
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Handle_cq_error(ssize_t ret)
{
    int mpi_errno = MPI_SUCCESS;
    struct fi_cq_err_entry e;
    MPID_Request *req;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_CQ_ERROR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_CQ_ERROR);

    switch(ret) {
        case -FI_EAVAIL:
            fi_cq_readerr(MPIDI_Global.p2p_cq, &e, 0);

            switch(e.err) {
                case FI_ETRUNC:
                    req = MPIDI_CH4_NMI_OFI_Context_to_request(e.op_context);

                    switch(req->kind) {
                        case MPID_REQUEST_SEND:
                            mpi_errno = MPIDI_CH4_NMI_OFI_Dispatch_function(NULL,req,0);
                            break;

                        case MPID_REQUEST_RECV:
                            mpi_errno = MPIDI_CH4_NMI_OFI_Dispatch_function((struct fi_cq_tagged_entry *) &e, req, 0);
                            req->status.MPI_ERROR = MPI_ERR_TRUNCATE;
                            break;

                        default:
                            MPIR_ERR_SETFATALANDJUMP4(mpi_errno, MPI_ERR_OTHER, "**ofid_poll",
                                                      "**ofid_poll %s %d %s %s", __SHORT_FILE__,
                                                      __LINE__, FCNAME, fi_strerror(e.err));
                    }

                    break;

                case FI_ECANCELED:
                    req = MPIDI_CH4_NMI_OFI_Context_to_request(e.op_context);
                    MPIR_STATUS_SET_CANCEL_BIT(req->status, TRUE);
                    break;

                case FI_ENOMSG:
                    req = MPIDI_CH4_NMI_OFI_Context_to_request(e.op_context);
                    MPIDI_CH4_NMI_OFI_Peek_empty_event(NULL, req);
                    break;
            }

            break;

        default:
            MPIR_ERR_SETFATALANDJUMP4(mpi_errno, MPI_ERR_OTHER, "**ofid_poll",
                                      "**ofid_poll %s %d %s %s", __SHORT_FILE__, __LINE__,
                                      FCNAME, fi_strerror(errno));
            break;
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_CQ_ERROR);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#endif /* NETMOD_OFI_EVENTS_H_INCLUDED */
