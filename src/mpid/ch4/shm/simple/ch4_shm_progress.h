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
#ifndef SHM_PROGRESS_H_INCLUDED
#define SHM_PROGRESS_H_INCLUDED

#include "ch4_shm_impl.h"

/* ----------------------------------------------------- */
/* MPIDI_shm_do_progress_recv                     */
/* ----------------------------------------------------- */
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_do_progress_recv)
static inline int MPIDI_shm_do_progress_recv(int blocking, int *completion_count)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_msg_sz_t data_sz;
    int in_cell = 0, in_fbox = 0;
    MPID_nem_fbox_mpich_t *fbox;
    MPID_nem_cell_ptr_t cell = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_DO_PROGRESS_RECV);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_DO_PROGRESS_RECV);
    /* try to match with unexpected */
    MPID_Request *sreq = MPIDI_shm_recvq_unexpected.head;
    MPID_Request *prev_sreq = NULL;
  unexpected_l:
    if (sreq != NULL) {
        MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                         (MPIU_DBG_FDEST, "Found unexpected in progress req %d,%d,%d\n",
                          REQ_SHM(sreq)->rank, REQ_SHM(sreq)->tag, REQ_SHM(sreq)->context_id));
        goto match_l;
    }
#if 0
    int local_rank;
    /* try to receive from fastbox */
    for (local_rank = 0; local_rank < MPID_nem_mem_region.num_local; local_rank++) {
        if (local_rank != MPID_nem_mem_region.local_rank) {
            fbox = &MPID_nem_mem_region.mailboxes.in[local_rank]->mpich;
            if (OPA_load_int(&fbox->flag.value)) {
                cell = &fbox->cell;
                MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                                 (MPIU_DBG_FDEST, "Found fastbox in progress %d,%d,%d\n",
                                  cell->rank, cell->tag, cell->context_id));
                in_cell = 1;
                in_fbox = 1;
                goto match_l;
            }
        }
    }
#endif
    /* try to receive from recvq */
    if (MPID_nem_mem_region.my_recvQ && !MPID_nem_queue_empty(MPID_nem_mem_region.my_recvQ)) {
        MPID_nem_queue_dequeue(MPID_nem_mem_region.my_recvQ, &cell);
        MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                         (MPIU_DBG_FDEST, "Found cell in progress %d,%d,%d\n", cell->rank,
                          cell->tag, cell->context_id));
        in_cell = 1;
        goto match_l;
    }
    goto fn_exit;
  match_l:
    {
        /* traverse posted receive queue */
        MPID_Request *req = MPIDI_shm_recvq_posted.head;
        MPID_Request *prev_req = NULL;
        char *send_buffer = in_cell ? (char *) cell->pkt.mpich.p.payload : (char *) REQ_SHM(sreq)->user_buf;
        int type = in_cell ? cell->pkt.mpich.type : REQ_SHM(sreq)->type;
        MPID_Request* pending = in_cell ? cell->pending : REQ_SHM(sreq)->pending;
        if (type == TYPE_ACK) {
            /* ACK message doesn't have a matching receive! */
            int c;
            MPIU_Assert(pending);
            MPID_cc_decr(pending->cc_ptr, &c);
            MPIDI_Request_release(pending);
            goto release_cell_l;
        }
        while (req) {
            int sender_rank, tag, context_id;
            ENVELOPE_GET(REQ_SHM(req), sender_rank, tag, context_id);
            MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                             (MPIU_DBG_FDEST, "Matching in progress req %d,%d,%d\n", sender_rank,
                              tag, context_id));
            if ((in_cell && ENVELOPE_MATCH(cell, sender_rank, tag, context_id)) ||
                (sreq && ENVELOPE_MATCH(REQ_SHM(sreq), sender_rank, tag, context_id))) {
                char *recv_buffer = (char *) REQ_SHM(req)->user_buf;
                MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                                 (MPIU_DBG_FDEST, "Matching in progress done %d,%d,%d, type %d, pending %p\n",
                                  sender_rank, tag, context_id, type, pending));
                if (pending) {
                    /* we must send ACK */
                    MPID_Request* req_ack = NULL;
                    MPIDI_Request_create_sreq(req_ack);
                    MPIU_Object_set_ref(req_ack, 1);
                    req_ack->comm = req->comm;
                    MPIR_Comm_add_ref(req->comm);
                    ENVELOPE_SET(REQ_SHM(req_ack), req->comm->rank, tag, context_id);
                    REQ_SHM(req_ack)->user_buf = NULL;
                    REQ_SHM(req_ack)->user_count = 0;
                    REQ_SHM(req_ack)->datatype = MPI_BYTE;
                    REQ_SHM(req_ack)->data_sz = 0;
                    REQ_SHM(req_ack)->type = TYPE_ACK;
                    REQ_SHM(req_ack)->dest = sender_rank;
                    REQ_SHM(req_ack)->next = NULL;
                    REQ_SHM(req_ack)->segment_ptr = NULL;
                    REQ_SHM(req_ack)->pending = pending;
                    /* enqueue req_ack */
                    REQ_SHM_ENQUEUE(req_ack, MPIDI_shm_sendq);
                }
                /*
                 * TODO: check for not overflowing recv buffer
                 */
                if (type == TYPE_EAGER) {
                    /* eager message */
                    data_sz = in_cell ? cell->pkt.mpich.datalen : REQ_SHM(sreq)->data_sz;
                    if( REQ_SHM(req)->segment_ptr ) {
                        /* non-contig */
                        MPIDI_msg_sz_t last = REQ_SHM(req)->segment_first + data_sz;
                        MPID_Segment_unpack(REQ_SHM(req)->segment_ptr, REQ_SHM(req)->segment_first, (MPI_Aint*)&last, send_buffer );
                        MPID_Segment_free(REQ_SHM(req)->segment_ptr);
                    }
                    else {
                        /* contig */
                        if( send_buffer ) MPIU_Memcpy(recv_buffer, (void *) send_buffer, data_sz);
                    }
                    REQ_SHM(req)->data_sz = data_sz;
                    /* set status */
                    req->status.MPI_SOURCE = sender_rank;
                    req->status.MPI_TAG = tag;
                    MPIR_STATUS_SET_COUNT(req->status,
                                          (MPIR_STATUS_GET_COUNT(req->status) + data_sz));
                    /* dequeue rreq */
                    REQ_SHM_DEQUEUE_AND_SET_ERROR(&req, prev_req, MPIDI_shm_recvq_posted,
                                                  mpi_errno);
                }
                else if (type == TYPE_LMT) {
                    /* long message */
                    if( REQ_SHM(req)->segment_ptr ) {
                        /* non-contig */
                        MPIDI_msg_sz_t last = REQ_SHM(req)->segment_first + EAGER_THRESHOLD;
                        MPID_Segment_unpack(REQ_SHM(req)->segment_ptr, REQ_SHM(req)->segment_first, (MPI_Aint*)&last, send_buffer );
                        REQ_SHM(req)->segment_first = last;
                    }
                    else {
                        /* contig */
                        if( send_buffer ) MPIU_Memcpy(recv_buffer, (void *) send_buffer, EAGER_THRESHOLD);
                    }
                    REQ_SHM(req)->data_sz -= EAGER_THRESHOLD;
                    REQ_SHM(req)->user_buf += EAGER_THRESHOLD;
                    MPIR_STATUS_SET_COUNT(req->status,
                                          (MPIR_STATUS_GET_COUNT(req->status) + EAGER_THRESHOLD));
                }
                else {
                    MPIU_Assert(0);
                }
                goto release_cell_l;
            }
            prev_req = req;
            req = REQ_SHM(req)->next;
        }
        /* unexpected message, no posted matching req */
        if (in_cell) {
            /* free the cell, move to unexpected queue */
            MPID_Request *rreq;
            MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                    (MPIU_DBG_FDEST, "Unexpected %d,%d,%d\n", cell->rank, cell->tag,
                     cell->context_id));
            MPIDI_Request_create_rreq(rreq);
            MPIU_Object_set_ref(rreq, 1);
            /* set status */
            rreq->status.MPI_SOURCE = cell->rank;
            rreq->status.MPI_TAG = cell->tag;
            MPIR_STATUS_SET_COUNT(rreq->status, cell->pkt.mpich.datalen);
            ENVELOPE_SET(REQ_SHM(rreq), cell->rank, cell->tag, cell->context_id);
            data_sz = cell->pkt.mpich.datalen;
            REQ_SHM(rreq)->data_sz = data_sz;
            REQ_SHM(rreq)->type = cell->pkt.mpich.type;
            if( data_sz > 0 ) {
                REQ_SHM(rreq)->user_buf = (char*)MPIU_Malloc(data_sz);
                MPIU_Memcpy(REQ_SHM(rreq)->user_buf, (void *) cell->pkt.mpich.p.payload, data_sz);
            }
            else {
                REQ_SHM(rreq)->user_buf = NULL;
            }
            REQ_SHM(rreq)->next = NULL;
            REQ_SHM(rreq)->pending = cell->pending;
            /* enqueue rreq */
            REQ_SHM_ENQUEUE(rreq, MPIDI_shm_recvq_unexpected);
            MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                    (MPIU_DBG_FDEST, "Unexpected enqueued %d,%d,%d\n", cell->rank, cell->tag,
                     cell->context_id));
        }
        else {
            /* examine another message in unexpected queue */
            prev_sreq = sreq;
            sreq = REQ_SHM(sreq)->next;
            MPIU_DBG_MSG_FMT(HANDLE, TYPICAL, (MPIU_DBG_FDEST, "Next unexpected %p\n", sreq));
            goto unexpected_l;
        }
    }
release_cell_l:
    if (in_cell) {
        /* release cell */
        MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                (MPIU_DBG_FDEST, "About to release cell %d,%d,%d in_fbox=%d\n", cell->rank, cell->tag,
                 cell->context_id, in_fbox));
        cell->pending = NULL;
#if 0
        if (in_fbox)
            OPA_store_release_int(&(fbox->flag.value), 0);
        else
#endif
        {
            MPID_nem_queue_enqueue(MPID_nem_mem_region.FreeQ[cell->my_rank], cell);
        }
        MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                (MPIU_DBG_FDEST, "Cell released\n"));
    }
    else {
        /* destroy unexpected req */
        MPIU_DBG_MSG_FMT(HANDLE, TYPICAL, (MPIU_DBG_FDEST, "About to release unexpected %p\n", sreq));
        REQ_SHM(sreq)->pending = NULL;
        MPIU_Free(REQ_SHM(sreq)->user_buf);
        REQ_SHM_DEQUEUE_AND_SET_ERROR(&sreq, prev_sreq, MPIDI_shm_recvq_unexpected, mpi_errno);
    }
    (*completion_count)++;
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_DO_PROGRESS_RECV);
    return mpi_errno;
}

/* ----------------------------------------------------- */
/* MPIDI_shm_do_progress_send                     */
/* ----------------------------------------------------- */
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_do_progress_send)
static inline int MPIDI_shm_do_progress_send(int blocking, int *completion_count)
{
    int mpi_errno = MPI_SUCCESS;
    int dest;
    MPID_nem_cell_ptr_t cell = NULL;
    MPID_Request *sreq = MPIDI_shm_sendq.head;
    MPID_Request *prev_sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_DO_PROGRESS_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_DO_PROGRESS_SEND);
    if (sreq == NULL)
        goto fn_exit;
    /* try to send via freeq */
    if (!MPID_nem_queue_empty(MPID_nem_mem_region.my_freeQ)) {
        MPID_nem_queue_dequeue(MPID_nem_mem_region.my_freeQ, &cell);
        ENVELOPE_GET(REQ_SHM(sreq), cell->rank, cell->tag, cell->context_id);
        dest = REQ_SHM(sreq)->dest;
        MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                (MPIU_DBG_FDEST, "Sending %d,%d,%d\n", cell->rank, cell->tag,
                 cell->context_id));
        char *recv_buffer = (char *) cell->pkt.mpich.p.payload;
        MPIDI_msg_sz_t data_sz = REQ_SHM(sreq)->data_sz;
        /*
         * TODO: make request field dest_lpid (or even recvQ[dest_lpid]) instead of dest - no need to do rank_to_lpid each time
         */
        int grank = MPIDI_CH4U_rank_to_lpid(dest, sreq->comm);
        cell->pending = NULL;
        if (REQ_SHM(sreq)->type == TYPE_SYNC ) {
            /* increase req cc in order to release req only after ACK, do it once per SYNC request */
            /* non-NULL pending req signal receiver about sending ACK back */
            /* the pending req should be sent back for sender to decrease cc, for it is dequeued already */
            int c;
            cell->pending = sreq;
            MPID_cc_incr(sreq->cc_ptr, &c);
            REQ_SHM(sreq)->type = TYPE_STANDARD;
        }
        if (data_sz <= EAGER_THRESHOLD) {
            cell->pkt.mpich.datalen = data_sz;
            if( REQ_SHM(sreq)->type == TYPE_ACK ) {
                cell->pkt.mpich.type = TYPE_ACK;
                cell->pending = REQ_SHM(sreq)->pending;
            }
            else {
                /* eager message */
                if( REQ_SHM(sreq)->segment_ptr ) {
                    /* non-contig */
                    MPID_Segment_pack(REQ_SHM(sreq)->segment_ptr, REQ_SHM(sreq)->segment_first, (MPI_Aint*)&REQ_SHM(sreq)->segment_size, recv_buffer );
                    MPID_Segment_free(REQ_SHM(sreq)->segment_ptr);
                }
                else {
                    /* contig */
                    MPIU_Memcpy((void *) recv_buffer, REQ_SHM(sreq)->user_buf, data_sz);
                }
                cell->pkt.mpich.type = TYPE_EAGER;
                /* set status */
                /*
                 * TODO: incorrect count for LMT - set to a last chunk of data
                 * is send status required?
                 */
                sreq->status.MPI_SOURCE = cell->rank;
                sreq->status.MPI_TAG = cell->tag;
                MPIR_STATUS_SET_COUNT(sreq->status, data_sz);
            }
            /* dequeue sreq */
            REQ_SHM_DEQUEUE_AND_SET_ERROR(&sreq, prev_sreq, MPIDI_shm_sendq, mpi_errno);
        }
        else {
            /* long message */
            if( REQ_SHM(sreq)->segment_ptr ) {
                /* non-contig */
                MPIDI_msg_sz_t last = REQ_SHM(sreq)->segment_first + EAGER_THRESHOLD;
                MPID_Segment_pack(REQ_SHM(sreq)->segment_ptr, REQ_SHM(sreq)->segment_first, (MPI_Aint*)&last, recv_buffer );
                REQ_SHM(sreq)->segment_first = last;
            }
            else {
                /* contig */
                MPIU_Memcpy((void *) recv_buffer, REQ_SHM(sreq)->user_buf, EAGER_THRESHOLD);
                REQ_SHM(sreq)->user_buf += EAGER_THRESHOLD;
            }
            cell->pkt.mpich.datalen = EAGER_THRESHOLD;
            REQ_SHM(sreq)->data_sz -= EAGER_THRESHOLD;
            cell->pkt.mpich.type = TYPE_LMT;
        }
        MPID_nem_queue_enqueue(MPID_nem_mem_region.RecvQ[grank], cell);
        (*completion_count)++;
    }
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_DO_PROGRESS_SEND);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_progress)
static inline int MPIDI_shm_progress(int blocking)
{
    int complete = 0;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_PROGRESS);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_PROGRESS);
    do {
        /* Receieve progress */
        MPIDI_shm_do_progress_recv(blocking, &complete);
        /* Send progress */
        MPIDI_shm_do_progress_send(blocking, &complete);
        if (complete > 0)
            break;
    } while (blocking);

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_PROGRESS);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_progress_test(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_progress_poke(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline void MPIDI_shm_progress_start(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return;
}

static inline void MPIDI_shm_progress_end(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return;
}

static inline int MPIDI_shm_progress_wait(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_progress_register(int (*progress_fn) (int *))
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_progress_deregister(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_progress_activate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_progress_deactivate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_PROGRESS_H_INCLUDED */
