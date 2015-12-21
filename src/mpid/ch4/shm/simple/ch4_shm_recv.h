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
#ifndef SHM_RECV_H_INCLUDED
#define SHM_RECV_H_INCLUDED

#include "ch4_shm_impl.h"
#include "ch4_impl.h"

/* ---------------------------------------------------- */
/* general queues                                       */
/* ---------------------------------------------------- */
extern MPIDI_shm_queue_t MPIDI_shm_recvq_posted;
extern MPIDI_shm_queue_t MPIDI_shm_recvq_unexpected;

/* ---------------------------------------------------- */
/* shm_do_irecv                                             */
/* ---------------------------------------------------- */
#undef FCNAME
#define FCNAME DECL_FUNC(shm_do_irecv)
static inline int shm_do_irecv(void *buf,
                               int count,
                               MPI_Datatype datatype,
                               int rank,
                               int tag,
                               MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS, dt_contig;
    MPIDI_msg_sz_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPID_Request *rreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_SHM_DO_IRECV);

    MPIDI_FUNC_ENTER(MPID_STATE_SHM_DO_IRECV);

    MPIDI_Request_create_rreq(rreq);
    if (unlikely(rank == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(rreq->status);
        *request = rreq;
        goto fn_exit;
    }

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    ENVELOPE_SET(REQ_SHM(rreq), rank, tag, comm->context_id + context_offset);
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    REQ_SHM(rreq)->user_buf = (char*)buf + dt_true_lb;
    REQ_SHM(rreq)->user_count = count;
    REQ_SHM(rreq)->datatype = datatype;
    REQ_SHM(rreq)->next = NULL;
    REQ_SHM(rreq)->segment_ptr = NULL;
    MPIU_CH4_REQUEST(rreq, anysource_partner_request) = NULL;
    MPIR_STATUS_SET_COUNT(rreq->status, 0);
    if( !dt_contig ) {
        REQ_SHM(rreq)->segment_ptr = MPID_Segment_alloc( );
        MPIR_ERR_CHKANDJUMP1((REQ_SHM(rreq)->segment_ptr == NULL), mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %s", "MPID_Segment_alloc");
        MPID_Segment_init((char*)buf, REQ_SHM(rreq)->user_count, REQ_SHM(rreq)->datatype, REQ_SHM(rreq)->segment_ptr, 0);
        REQ_SHM(rreq)->segment_first = 0;
        REQ_SHM(rreq)->segment_size = data_sz;
    }
    dtype_add_ref_if_not_builtin(datatype);
    /* enqueue rreq */
    REQ_SHM_ENQUEUE(rreq, MPIDI_shm_recvq_posted);
    MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
            (MPIU_DBG_FDEST, "Enqueued from grank %d to %d (comm_kind %d) in recv %d,%d,%d\n",
             MPIDI_CH4U_rank_to_lpid(rank, comm), MPID_nem_mem_region.rank, comm->comm_kind,
             rank, tag, comm->context_id + context_offset));
    *request = rreq;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_SHM_DO_IRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_recv)
static inline int MPIDI_shm_recv(void *buf,
                                 int count,
                                 MPI_Datatype datatype,
                                 int rank,
                                 int tag,
                                 MPID_Comm * comm,
                                 int context_offset, MPI_Status * status, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS, dt_contig __attribute__ ((__unused__));
    MPIDI_msg_sz_t data_sz __attribute__ ((__unused__));
    MPIDI_STATE_DECL(MPIDI_SHM_RECV);

    MPIDI_FUNC_ENTER(MPIDI_SHM_RECV);

    /* create a request */
    mpi_errno = shm_do_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);

    MPIDI_FUNC_EXIT(MPIDI_SHM_RECV);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_recv)
static inline int MPIDI_shm_recv_init(void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_RECV_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_RECV_INIT);

    MPIDI_Request_create_rreq(rreq);
    MPIU_Object_set_ref(rreq, 1);
    MPID_cc_set(&rreq->cc, 0);
    rreq->kind = MPID_PREQUEST_RECV;
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    ENVELOPE_SET(REQ_SHM(rreq), rank, tag, comm->context_id + context_offset);
    REQ_SHM(rreq)->user_buf = (char*)buf;
    REQ_SHM(rreq)->user_count = count;
    REQ_SHM(rreq)->datatype = datatype;
    *request = rreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_RECV_INIT);
    return mpi_errno;
}


static inline int MPIDI_shm_mrecv(void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  MPID_Request * message, MPI_Status * status)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_shm_imrecv(void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   MPID_Request * message, MPID_Request ** rreqp)
{
    int mpi_errno = MPI_SUCCESS;
    int dt_contig;
    MPIDI_msg_sz_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPID_Request *rreq = NULL, *sreq = NULL;
    int rank, tag, context_id;
    int context_offset = 0;

    MPIDI_STATE_DECL(MPIDI_SHM_IMRECV);
    MPIDI_FUNC_ENTER(MPIDI_SHM_IMRECV);

    if (message == NULL)
    {
        MPIDI_Request_create_null_rreq(rreq, mpi_errno, goto fn_fail);
        *rreqp = rreq;
        goto fn_exit;
    }

    MPIU_Assert(message != NULL);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIDI_Request_create_rreq(rreq);
    MPIU_Object_set_ref(rreq, 1);
    MPID_cc_set(&rreq->cc, 0);
    ENVELOPE_GET(REQ_SHM(message), rank, tag, context_id);
    ENVELOPE_SET(REQ_SHM(rreq), rank, tag, context_id);
    rreq->comm = message->comm;
    MPIR_Comm_add_ref(message->comm);
    REQ_SHM(rreq)->user_buf = (char*)buf + dt_true_lb;
    REQ_SHM(rreq)->user_count = count;
    REQ_SHM(rreq)->datatype = datatype;
    REQ_SHM(rreq)->next = NULL;
    REQ_SHM(rreq)->segment_ptr = NULL;
    MPIR_STATUS_SET_COUNT(rreq->status, 0);
    if (!dt_contig)
    {
        REQ_SHM(rreq)->segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1((REQ_SHM(rreq)->segment_ptr == NULL), mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %s", "MPID_Segment_alloc");
        MPID_Segment_init((char*)buf, REQ_SHM(rreq)->user_count, REQ_SHM(rreq)->datatype, REQ_SHM(rreq)->segment_ptr, 0);
        REQ_SHM(rreq)->segment_first = 0;
        REQ_SHM(rreq)->segment_size = data_sz;
    }

    if (REQ_SHM(message)->pending)
    {
        /* Sync send - we must send ACK */
        int srank = message->status.MPI_SOURCE;
        MPID_Request* req_ack = NULL;
        MPIDI_Request_create_sreq(req_ack);
        MPIU_Object_set_ref(req_ack, 1);
        req_ack->comm = message->comm;
        MPIR_Comm_add_ref(message->comm);
        ENVELOPE_SET(REQ_SHM(req_ack), message->comm->rank, tag, context_id);
        REQ_SHM(req_ack)->user_buf = NULL;
        REQ_SHM(req_ack)->user_count = 0;
        REQ_SHM(req_ack)->datatype = MPI_BYTE;
        REQ_SHM(req_ack)->data_sz = 0;
        REQ_SHM(req_ack)->type = TYPE_ACK;
        REQ_SHM(req_ack)->dest = srank;
        REQ_SHM(req_ack)->next = NULL;
        REQ_SHM(req_ack)->segment_ptr = NULL;
        REQ_SHM(req_ack)->pending = REQ_SHM(message)->pending;
        /* enqueue req_ack */
        REQ_SHM_ENQUEUE(req_ack, MPIDI_shm_sendq);
    }

    for (sreq = message; sreq; ) {
        MPID_Request* next_req = NULL;
        char *send_buffer = REQ_SHM(sreq)->user_buf;
        char *recv_buffer = (char *) REQ_SHM(rreq)->user_buf;
        if (REQ_SHM(sreq)->type == TYPE_EAGER)
        {
            /* eager message */
            data_sz = REQ_SHM(sreq)->data_sz;
            if (REQ_SHM(rreq)->segment_ptr)
            {
                /* non-contig */
                MPIDI_msg_sz_t last = REQ_SHM(rreq)->segment_first + data_sz;
                MPID_Segment_unpack(REQ_SHM(rreq)->segment_ptr, REQ_SHM(rreq)->segment_first, (MPI_Aint*)&last, send_buffer);
                MPID_Segment_free(REQ_SHM(rreq)->segment_ptr);
            }
            else
                /* contig */
                if (send_buffer)
                    MPIU_Memcpy(recv_buffer, (void *) send_buffer, data_sz);
            /* set status */
            rreq->status.MPI_SOURCE = sreq->status.MPI_SOURCE;
            rreq->status.MPI_TAG = sreq->status.MPI_TAG;
            count = MPIR_STATUS_GET_COUNT(rreq->status) + (MPI_Count)data_sz;
            MPIR_STATUS_SET_COUNT(rreq->status,count);
        }
        else if (REQ_SHM(sreq)->type == TYPE_LMT)
        {
            /* long message */
            if (REQ_SHM(rreq)->segment_ptr)
            {
                /* non-contig */
                MPIDI_msg_sz_t last = REQ_SHM(rreq)->segment_first + EAGER_THRESHOLD;
                MPID_Segment_unpack(REQ_SHM(rreq)->segment_ptr, REQ_SHM(rreq)->segment_first, (MPI_Aint*)&last, send_buffer);
                REQ_SHM(rreq)->segment_first = last;
            }
            else
                /* contig */
                if (send_buffer)
                    MPIU_Memcpy(recv_buffer, (void *) send_buffer, EAGER_THRESHOLD);
            REQ_SHM(rreq)->data_sz -= EAGER_THRESHOLD;
            REQ_SHM(rreq)->user_buf += EAGER_THRESHOLD;
            count = MPIR_STATUS_GET_COUNT(rreq->status) + (MPI_Count)EAGER_THRESHOLD;
            MPIR_STATUS_SET_COUNT(rreq->status,count);
        }
        /* destroy unexpected req */
        REQ_SHM(sreq)->pending = NULL;
        MPIU_Free(REQ_SHM(sreq)->user_buf);
        next_req = REQ_SHM(sreq)->next;
        REQ_SHM_COMPLETE(sreq);
        sreq = next_req;
    }
    *rreqp = rreq;

fn_exit:
    MPIDI_FUNC_EXIT(MPIDI_SHM_IMRECV);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_irecv)
static inline int MPIDI_shm_irecv(void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPIDI_SHM_IRECV);

    MPIDI_FUNC_ENTER(MPIDI_SHM_IRECV);

    mpi_errno = shm_do_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);

    MPIDI_FUNC_EXIT(MPIDI_SHM_IRECV);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_cancel_recv)
static inline int MPIDI_shm_cancel_recv(MPID_Request * rreq)
{
    MPID_Request *req = MPIDI_shm_recvq_posted.head;
    MPID_Request *prev_req = NULL;

    while (req) {

        if (req == rreq)
        {
            /* Remove request from shm posted receive queue */

            if (prev_req)
            {
                REQ_SHM(prev_req)->next = REQ_SHM(req)->next;
            }
            else
            {
                MPIDI_shm_recvq_posted.head = REQ_SHM(req)->next;
            }

            if (req == MPIDI_shm_recvq_posted.tail)
            {
                MPIDI_shm_recvq_posted.tail = prev_req;
            }

            MPIR_STATUS_SET_CANCEL_BIT(req->status, TRUE);
            MPIR_STATUS_SET_COUNT(req->status, 0);
            MPIDI_Request_complete(req);

            break;
        }

        prev_req = req;
        req = REQ_SHM(req)->next;
    }

    return MPI_SUCCESS;
}

#endif /* SHM_RECV_H_INCLUDED */
