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

static inline int get_huge_event(cq_tagged_entry_t *wc, MPID_Request *req);


#undef FUNCNAME
#define FUNCNAME peek_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int peek_event(cq_tagged_entry_t * wc, MPID_Request * rreq)
{
    size_t count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EVENT);
    REQ_OFI(rreq, util_id)  = MPIDI_PEEK_FOUND;
    rreq->status.MPI_SOURCE = get_source(wc->tag);
    rreq->status.MPI_TAG    = get_tag(wc->tag);
    count                   = wc->len;
    rreq->status.MPI_ERROR  = MPI_SUCCESS;
    MPIR_STATUS_SET_COUNT(rreq->status, count);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME peek_empty_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int peek_empty_event(cq_tagged_entry_t * wc, MPID_Request * rreq)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EMPTY_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EMPTY_EVENT);
    MPIDI_Dynproc_req *ctrl;

    switch(REQ_OFI(rreq, event_id)){
    case MPIDI_EVENT_PEEK:
        REQ_OFI(rreq, util_id)  = MPIDI_PEEK_NOT_FOUND;
        rreq->status.MPI_ERROR  = MPI_SUCCESS;
        break;
    case MPIDI_EVENT_ACCEPT_PROBE:
        ctrl       = (MPIDI_Dynproc_req *)rreq;
        ctrl->done = MPIDI_PEEK_NOT_FOUND;
        break;
    default:
        MPIU_Assert(0);
        break;
    }
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_PEEK_EMPTY_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME recv_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int recv_event(cq_tagged_entry_t * wc, MPID_Request * rreq)
{
    int      mpi_errno = MPI_SUCCESS;
    MPI_Aint last;
    size_t   count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV_EVENT);

    rreq->status.MPI_ERROR  = MPI_SUCCESS;
    rreq->status.MPI_SOURCE = get_source(wc->tag);
    rreq->status.MPI_TAG    = get_tag(wc->tag);
    count                   = wc->len;
    MPIR_STATUS_SET_COUNT(rreq->status, count);

    if (REQ_OFI(rreq, pack_buffer)) {
        last = count;
        MPID_Segment_unpack(REQ_OFI(rreq, segment_ptr), 0, &last, REQ_OFI(rreq, pack_buffer));
        MPIU_Free(REQ_OFI(rreq, pack_buffer));
        MPID_Segment_free(REQ_OFI(rreq, segment_ptr));

        if (last != (MPI_Aint)count) {
            rreq->status.MPI_ERROR =
                MPIR_Err_create_code(MPI_SUCCESS,
                                     MPIR_ERR_RECOVERABLE,
                                     __FUNCTION__, __LINE__,
                                     MPI_ERR_TYPE, "**dtypemismatch", 0);
        }
    }

    dtype_release_if_not_builtin(REQ_OFI(rreq, datatype));

    /* If syncronous, ack and complete when the ack is done */
    if (unlikely(is_tag_sync(wc->tag))) {
        uint64_t ss_bits = init_sendtag(REQ_OFI(rreq, util_id),
                                        REQ_OFI(rreq, util_comm->rank),
                                        rreq->status.MPI_TAG,
                                        MPID_SYNC_SEND_ACK);
        MPID_Comm *c = REQ_OFI(rreq, util_comm);
        int r = rreq->status.MPI_SOURCE;
        FI_RC_RETRY_NOLOCK(fi_tinject(G_TXC_TAG(0), NULL, 0,
                                      _comm_to_phys(c, r, MPIDI_API_TAG),
                                      ss_bits), tsendsync);

    }

    MPIDI_Request_complete(rreq);

    /* Polling loop will check for truncation */
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV_EVENT);
    return mpi_errno;
  fn_fail:
    rreq->status.MPI_ERROR = mpi_errno;
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME recv_huge_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int recv_huge_event(cq_tagged_entry_t * wc, MPID_Request * rreq)
{
    MPIDI_Huge_recv_t *recv;
    MPIDI_Huge_chunk_t *hc;
    MPID_Comm *comm_ptr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RECV_HUGE_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RECV_HUGE_EVENT);

    /* Look up the receive sequence number and chunk queue */
    comm_ptr = REQ_OFI(rreq, util_comm);
    recv = (MPIDI_Huge_recv_t *) MPIDI_OFI_Map_lookup(COMM_OFI(comm_ptr).huge_recv_counters,
                                                      get_source(wc->tag));

    if (recv == MPIDI_MAP_NOT_FOUND) {
        recv = (MPIDI_Huge_recv_t *) MPIU_Malloc(sizeof(*recv));
        recv->seqno = 0;
        MPIDI_OFI_Map_create(&recv->chunk_q);
        MPIDI_OFI_Map_set(COMM_OFI(comm_ptr).huge_recv_counters, get_source(wc->tag), recv);
    }

    /* Look up the receive in the chunk queue */
    hc = (MPIDI_Huge_chunk_t *) MPIDI_OFI_Map_lookup(recv->chunk_q, recv->seqno);

    if (hc == MPIDI_MAP_NOT_FOUND) {
        hc = (MPIDI_Huge_chunk_t *) MPIU_Malloc(sizeof(*hc));
        memset(hc, 0, sizeof(*hc));
        hc->event_id = MPIDI_EVENT_GET_HUGE;
        MPIDI_OFI_Map_set(recv->chunk_q, recv->seqno, hc);
    }

    recv->seqno++;
    hc->localreq = rreq;
    hc->done_fn = recv_event;
    hc->wc = *wc;
    get_huge_event(NULL, (MPID_Request *) hc);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RECV_HUGE_EVENT);
    return MPI_SUCCESS;
}


#undef FUNCNAME
#define FUNCNAME send_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int send_event(cq_tagged_entry_t * wc, MPID_Request * sreq)
{
    int c;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_EVENT);

    MPID_cc_decr(sreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);

    if (c == 0) {
        if (REQ_OFI(sreq, pack_buffer))
            MPIU_Free(REQ_OFI(sreq, pack_buffer));

        dtype_release_if_not_builtin(REQ_OFI(sreq, datatype));
        MPIDI_Request_release(sreq);
    }   /* c != 0, ssend */

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_EVENT);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME send_event_huge
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int send_huge_event(cq_tagged_entry_t * wc, MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    int c;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_EVENT_HUGE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_EVENT_HUGE);

    MPID_cc_decr(sreq->cc_ptr, &c);
    MPIU_Assert(c >= 0);

    if (c == 0) {
        MPID_Comm *comm;
        void *ptr;
        MPIDI_Hugecntr *cntr;
        comm = REQ_OFI(sreq, util_comm);
        ptr = MPIDI_OFI_Map_lookup(COMM_OFI(comm).huge_send_counters, REQ_OFI(sreq, util_id));
        MPIU_Assert(ptr != MPIDI_MAP_NOT_FOUND);
        cntr = (MPIDI_Hugecntr *) ptr;
        cntr->outstanding--;

        if (cntr->outstanding == 0) {
            MPIDI_Send_control_t ctrl;
            MPIDI_OFI_Map_erase(COMM_OFI(comm).huge_send_counters, REQ_OFI(sreq, util_id));
            MPIU_Free(ptr);
            ctrl.type = MPIDI_CTRL_HUGE_CLEANUP;
            MPI_RC_POP(do_control_send(&ctrl, NULL, 0, REQ_OFI(sreq, util_id), comm, NULL));
        }

        if (REQ_OFI(sreq, pack_buffer))
            MPIU_Free(REQ_OFI(sreq, pack_buffer));

        dtype_release_if_not_builtin(REQ_OFI(sreq, datatype));
        MPIDI_Request_release(sreq);
    }   /* c != 0, ssend */

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_EVENT_HUGE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME ssend_ack_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int ssend_ack_event(cq_tagged_entry_t * wc, MPID_Request * sreq)
{
    int mpi_errno;
    MPIDI_Ssendack_request *req = (MPIDI_Ssendack_request *) sreq;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SSEND_ACK_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SSEND_ACK_EVENT);
    mpi_errno = send_event(NULL, req->signal_req);
    MPIDI_Ssendack_request_tls_free(req);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SSEND_ACK_EVENT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME control_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int control_event(cq_tagged_entry_t *wc,
                                MPID_Request      *req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CONTROL_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CONTROL_EVENT);

    MPIU_Assert(wc->len == 0 || wc->len == MPID_MIN_CTRL_MSG_SZ);

    if(wc->len == MPID_MIN_CTRL_MSG_SZ) {
        int16_t *buf;
        buf         = (int16_t *)wc->buf;
        MPIU_Assert(buf != NULL);
        mpi_errno   = MPIDI_OFI_control_dispatch(buf);
    }

    if(wc->flags & FI_MULTI_RECV) {
        FI_RC_RETRY_NOLOCK(fi_recvmsg(G_RXC_MSG(0),
                                      &MPIDI_Global.msg[MPIDI_Global.cur_ctrlblock],
                                      FI_MULTI_RECV|FI_COMPLETION),repost);
        MPIDI_Global.cur_ctrlblock++;

        if(MPIDI_Global.cur_ctrlblock == MPIDI_Global.num_ctrlblock)
            MPIDI_Global.cur_ctrlblock=0;
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CONTROL_EVENT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME get_huge_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int get_huge_event(cq_tagged_entry_t *wc,
                                 MPID_Request      *req)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_Huge_chunk_t *hc = (MPIDI_Huge_chunk_t *)req;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_GETHUGE_EVENT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_GETHUGE_EVENT);

    if(hc->localreq && hc->cur_offset!=0) {
        size_t bytesSent  = hc->cur_offset - MPIDI_Global.max_send;
        size_t bytesLeft  = hc->remote_info.msgsize - bytesSent - MPIDI_Global.max_send;
        size_t bytesToGet = (bytesLeft<=MPIDI_Global.max_send)?bytesLeft:MPIDI_Global.max_send;

        if(bytesToGet == 0ULL) {
            MPIDI_Send_control_t ctrl;
            hc->wc.len = hc->cur_offset;
            hc->done_fn(&hc->wc, hc->localreq);
            ctrl.type = MPIDI_CTRL_HUGEACK;
            MPI_RC_POP(do_control_send(&ctrl,NULL,0,hc->remote_info.origin_rank,
                                       hc->comm_ptr,hc->remote_info.ackreq));
            MPIU_Free(hc);
            goto fn_exit;
        }

        FI_RC_RETRY_NOLOCK(fi_read(G_TXC_RMA(0),                                           /* endpoint     */
                                   (void *)((uintptr_t)hc->wc.buf + hc->cur_offset),       /* local buffer */
                                   bytesToGet,                                             /* bytes        */
                                   NULL,                                                   /* descriptor   */
                                   _comm_to_phys(hc->comm_ptr,hc->remote_info.origin_rank,MPIDI_API_MSG), /* Destination  */
                                   (uint64_t)hc->remote_info.send_buf+hc->cur_offset,      /* remote maddr */
                                   MPIDI_Global.lkey,                                      /* Key          */
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
#define FUNCNAME chunk_done_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int chunk_done_event(cq_tagged_entry_t *wc,
                                   MPID_Request      *req)
{
  int c;
  MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CHUNK_DONE_EVENT);
  MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CHUNK_DONE_EVENT);

  MPIDI_Chunk_request *creq = (MPIDI_Chunk_request *)req;
  MPID_cc_decr(creq->parent->cc_ptr, &c);
  MPIU_Assert(c >= 0);
  if(c == 0)MPIDI_Request_release(creq->parent);
  MPIU_Free(creq);
  MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CHUNK_DONE_EVENT);
  return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME rma_done_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int rma_done_event(cq_tagged_entry_t *wc,
                                 MPID_Request      *in_req)
{
  MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_RMA_DONE_EVENT);
  MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_RMA_DONE_EVENT);

  MPIDI_Win_request *req = (MPIDI_Win_request *)in_req;
  MPIDI_Win_request_complete(req);

  MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_RMA_DONE_EVENT);
  return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME accept_probe_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int accept_probe_event(cq_tagged_entry_t *wc,
                                     MPID_Request      *rreq)
{
  MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_ACCEPT_PROBE_EVENT);
  MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_ACCEPT_PROBE_EVENT);
  MPIDI_Dynproc_req *ctrl = (MPIDI_Dynproc_req *)rreq;
  ctrl->source = get_source(wc->tag);
  ctrl->tag    = get_tag(wc->tag);
  ctrl->msglen = wc->len;
  ctrl->done   = MPIDI_PEEK_FOUND;
  MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_ACCEPT_PROBE_EVENT);
  return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME dynproc_done_event
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int dynproc_done_event(cq_tagged_entry_t *wc,
                                     MPID_Request      *rreq)
{
  MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_DYNPROC_DONE_EVENT);
  MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_DYNPROC_DONE_EVENT);
  MPIDI_Dynproc_req *ctrl = (MPIDI_Dynproc_req *)rreq;
  ctrl->done++;
  MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_DYNPROC_DONE_EVENT);
  return MPI_SUCCESS;
}

static inline MPID_Request *devreq_to_req(void *context)
{
    char *base = (char *) context;
    return (MPID_Request *) container_of(base, MPID_Request, dev.ch4.netmod);
}

static inline int dispatch_function(cq_tagged_entry_t * wc, MPID_Request *req)
{
    int mpi_errno;
    switch(REQ_OFI(req,event_id)) {
    case MPIDI_EVENT_PEEK:
        mpi_errno = peek_event(wc,req);
        break;
    case MPIDI_EVENT_RECV:
        mpi_errno = recv_event(wc,req);
        break;
    case MPIDI_EVENT_RECV_HUGE:
        mpi_errno = recv_huge_event(wc,req);
        break;
    case MPIDI_EVENT_SEND:
        mpi_errno = send_event(wc,req);
        break;
    case MPIDI_EVENT_SEND_HUGE:
        mpi_errno = send_huge_event(wc,req);
        break;
    case MPIDI_EVENT_SSEND_ACK:
        mpi_errno = ssend_ack_event(wc,req);
        break;
    case MPIDI_EVENT_GET_HUGE:
        mpi_errno = get_huge_event(wc,req);
        break;
    case MPIDI_EVENT_CONTROL:
        mpi_errno = control_event(wc,req);
        break;
    case MPIDI_EVENT_CHUNK_DONE:
        mpi_errno = chunk_done_event(wc,req);
        break;
    case MPIDI_EVENT_RMA_DONE:
        mpi_errno = rma_done_event(wc,req);
        break;
    case MPIDI_EVENT_DYNPROC_DONE:
        mpi_errno = dynproc_done_event(wc,req);
        break;
    case MPIDI_EVENT_ACCEPT_PROBE:
        mpi_errno = accept_probe_event(wc,req);
        break;
    case MPIDI_EVENT_ABORT:
    default:
        mpi_errno = MPI_SUCCESS;
        MPIU_Assert(0);
        break;
    }
    return mpi_errno;
}

#endif /* NETMOD_OFI_EVENTS_H_INCLUDED */
