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
#ifndef SHM_SEND_H_INCLUDED
#define SHM_SEND_H_INCLUDED

#include "ch4_shm_impl.h"
#include "ch4_impl.h"
#include <mpibsend.h>
#include <../mpi/pt2pt/bsendutil.h>
/* ---------------------------------------------------- */
/* general queues                                       */
/* ---------------------------------------------------- */
extern MPIDI_shm_queue_t MPIDI_shm_sendq;

/* ---------------------------------------------------- */
/* from mpid/ch3/channels/nemesis/include/mpid_nem_impl.h */
/* ---------------------------------------------------- */
/* assumes value!=0 means the fbox is full.  Contains acquire barrier to
 * ensure that later operations that are dependent on this check don't
 * escape earlier than this check. */
#define MPID_nem_fbox_is_full(pbox_) (OPA_load_acquire_int(&(pbox_)->flag.value))

/* ---------------------------------------------------- */
/* shm_do_isend                                             */
/* ---------------------------------------------------- */
#undef FCNAME
#define FCNAME DECL_FUNC(shm_do_isend)
static inline int shm_do_isend(const void *buf,
                               int count,
                               MPI_Datatype datatype,
                               int rank,
                               int tag,
                               MPID_Comm * comm, int context_offset, MPID_Request ** request, int type)
{
    int dt_contig, mpi_errno = MPI_SUCCESS;
    MPI_Aint dt_true_lb;
    MPID_Request *sreq = NULL;
    MPIDI_msg_sz_t data_sz;
    MPID_Datatype *dt_ptr;
    MPIDI_STATE_DECL(MPID_STATE_SHM_DO_ISEND);

    MPIDI_FUNC_ENTER(MPID_STATE_SHM_DO_ISEND);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIDI_Request_create_sreq(sreq);
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    ENVELOPE_SET(REQ_SHM(sreq), comm->rank, tag, comm->context_id + context_offset);
    REQ_SHM(sreq)->user_buf = (char *) buf + dt_true_lb;
    REQ_SHM(sreq)->user_count = count;
    REQ_SHM(sreq)->datatype = datatype;
    REQ_SHM(sreq)->data_sz = data_sz;
    REQ_SHM(sreq)->type = type;
    REQ_SHM(sreq)->dest = rank;
    REQ_SHM(sreq)->next = NULL;
    REQ_SHM(sreq)->pending = NULL;
    REQ_SHM(sreq)->segment_ptr = NULL;
    if( !dt_contig ) {
        REQ_SHM(sreq)->segment_ptr = MPID_Segment_alloc( );
        MPIR_ERR_CHKANDJUMP1((REQ_SHM(sreq)->segment_ptr == NULL), mpi_errno, MPI_ERR_OTHER, "**nomem", "**nomem %s", "MPID_Segment_alloc");
        MPID_Segment_init((char*)buf, REQ_SHM(sreq)->user_count, REQ_SHM(sreq)->datatype, REQ_SHM(sreq)->segment_ptr, 0);
        REQ_SHM(sreq)->segment_first = 0;
        REQ_SHM(sreq)->segment_size = data_sz;
    }
    dtype_add_ref_if_not_builtin(datatype);
    /* enqueue sreq */
    REQ_SHM_ENQUEUE(sreq, MPIDI_shm_sendq);
    *request = sreq;
    MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
            (MPIU_DBG_FDEST, "Enqueued to grank %d from %d (comm_kind %d) in recv %d,%d,%d\n",
             MPIDI_CH4R_rank_to_lpid(rank, comm), MPID_nem_mem_region.rank, comm->comm_kind,
             comm->rank, tag, comm->context_id + context_offset));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_SHM_DO_ISEND);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_SEND)
static inline int MPIDI_shm_send(const void *buf,
                                 int count,
                                 MPI_Datatype datatype,
                                 int rank,
                                 int tag,
                                 MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int dt_contig __attribute__ ((__unused__)), mpi_errno = MPI_SUCCESS;
    MPI_Aint dt_true_lb;
    MPIDI_msg_sz_t data_sz;
    MPID_Datatype *dt_ptr;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_SEND);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_SEND);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    /* try to send immediately, contig, short message */
    if (dt_contig && data_sz <= EAGER_THRESHOLD) {
        /* eager message */
        int grank = MPIDI_CH4R_rank_to_lpid(rank, comm);

        /* Try freeQ */
        if (!MPID_nem_queue_empty(MPID_nem_mem_region.my_freeQ)) {
            MPID_nem_cell_ptr_t cell;
            MPID_nem_queue_dequeue(MPID_nem_mem_region.my_freeQ, &cell);
            ENVELOPE_SET(cell, comm->rank, tag, comm->context_id + context_offset);
            cell->pkt.mpich.datalen = data_sz;
            cell->pkt.mpich.type = TYPE_EAGER;
            MPIU_Memcpy((void *) cell->pkt.mpich.p.payload, (char*)buf+dt_true_lb, data_sz);
            cell->pending = NULL;
            MPID_nem_queue_enqueue(MPID_nem_mem_region.RecvQ[grank], cell);
            *request = NULL;
            MPIU_DBG_MSG_FMT(HANDLE, TYPICAL,
                    (MPIU_DBG_FDEST, "Sent to grank %d from %d in send %d,%d,%d\n", grank, cell->my_rank, cell->rank, cell->tag,
                     cell->context_id));
            goto fn_exit;
        }
    }
    /* Long message or */
    /* Failed to send immediately - create and return request */
    mpi_errno = shm_do_isend(buf, count, datatype, rank, tag, comm, context_offset, request, TYPE_STANDARD);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND);
    return mpi_errno;
}

static inline int MPIDI_shm_rsend(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}



static inline int MPIDI_shm_irsend(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_SSEND)
static inline int MPIDI_shm_ssend(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_ISSEND);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_ISSEND);
    mpi_errno = shm_do_isend(buf, count, datatype, rank, tag, comm, context_offset, request, TYPE_SYNC);

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_ISSEND);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_STARTALL)
static inline int MPIDI_shm_startall(int count, MPID_Request * requests[])
{
    int i, mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_STARTALL);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_STARTALL);
    for( i = 0; i < count; i++ ) {
        MPID_Request* preq = requests[i];
        if( preq->kind == MPID_PREQUEST_SEND ) {
            if( REQ_SHM(preq)->type != TYPE_BUFFERED ) {
                mpi_errno = shm_do_isend(REQ_SHM(preq)->user_buf, REQ_SHM(preq)->user_count,
                        REQ_SHM(preq)->datatype, REQ_SHM(preq)->dest, REQ_SHM(preq)->tag,
                        preq->comm, REQ_SHM(preq)->context_id - preq->comm->context_id,
                        &preq->partner_request, REQ_SHM(preq)->type);
            }
            else {
                MPI_Request sreq_handle;
                mpi_errno = MPIR_Ibsend_impl(REQ_SHM(preq)->user_buf, REQ_SHM(preq)->user_count,
                        REQ_SHM(preq)->datatype, REQ_SHM(preq)->dest, REQ_SHM(preq)->tag, preq->comm,
                        &sreq_handle);
                if (mpi_errno == MPI_SUCCESS)
                    MPID_Request_get_ptr(sreq_handle, preq->partner_request);
            }
        }
        else if( preq->kind == MPID_PREQUEST_RECV ) {
            mpi_errno = shm_do_irecv(REQ_SHM(preq)->user_buf, REQ_SHM(preq)->user_count,
                    REQ_SHM(preq)->datatype, REQ_SHM(preq)->rank, REQ_SHM(preq)->tag,
                    preq->comm, REQ_SHM(preq)->context_id - preq->comm->context_id,
                    &preq->partner_request);
        }
        else
        {
            MPIU_Assert(0);
        }
        if (mpi_errno == MPI_SUCCESS) {
            preq->status.MPI_ERROR = MPI_SUCCESS;

            if (REQ_SHM(preq)->type == TYPE_BUFFERED) {
                preq->cc_ptr = &preq->cc;
                MPID_cc_set(&preq->cc, 0);
            }
            else
                preq->cc_ptr = &preq->partner_request->cc;
        }
        else {
            preq->partner_request = NULL;
            preq->status.MPI_ERROR = mpi_errno;
            preq->cc_ptr = &preq->cc;
            MPID_cc_set(&preq->cc, 0);
        }
    }

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_STARTALL);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_SEND_INIT)
static inline int MPIDI_shm_send_init(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_SEND_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_SEND_INIT);
    MPIDI_Request_create_sreq(sreq);
    MPIU_Object_set_ref(sreq, 1);
    MPID_cc_set(&(sreq)->cc, 0);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    ENVELOPE_SET(REQ_SHM(sreq), comm->rank, tag, comm->context_id + context_offset);
    REQ_SHM(sreq)->user_buf = (char *) buf;
    REQ_SHM(sreq)->user_count = count;
    REQ_SHM(sreq)->dest = rank;
    REQ_SHM(sreq)->datatype = datatype;
    REQ_SHM(sreq)->type = TYPE_STANDARD;
    *request = sreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND_INIT);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_SSEND_INIT)
static inline int MPIDI_shm_ssend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_SEND_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_SEND_INIT);
    MPIDI_Request_create_sreq(sreq);
    MPIU_Object_set_ref(sreq, 1);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    ENVELOPE_SET(REQ_SHM(sreq), comm->rank, tag, comm->context_id + context_offset);
    REQ_SHM(sreq)->user_buf = (char *) buf;
    REQ_SHM(sreq)->user_count = count;
    REQ_SHM(sreq)->dest = rank;
    REQ_SHM(sreq)->datatype = datatype;
    REQ_SHM(sreq)->type = TYPE_SYNC;
    *request = sreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND_INIT);
    return mpi_errno;
}

static inline int MPIDI_shm_bsend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_SEND_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_SEND_INIT);
    MPIDI_Request_create_sreq(sreq);
    MPIU_Object_set_ref(sreq, 1);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    ENVELOPE_SET(REQ_SHM(sreq), comm->rank, tag, comm->context_id + context_offset);
    REQ_SHM(sreq)->user_buf = (char *) buf;
    REQ_SHM(sreq)->user_count = count;
    REQ_SHM(sreq)->dest = rank;
    REQ_SHM(sreq)->datatype = datatype;
    REQ_SHM(sreq)->type = TYPE_BUFFERED;
    *request = sreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND_INIT);
    return mpi_errno;
}

static inline int MPIDI_shm_rsend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_RSEND_INIT);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_RSEND_INIT);
    MPIDI_Request_create_sreq(sreq);
    MPIU_Object_set_ref(sreq, 1);
    MPID_cc_set(&(sreq)->cc, 0);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    ENVELOPE_SET(REQ_SHM(sreq), comm->rank, tag, comm->context_id + context_offset);
    REQ_SHM(sreq)->user_buf = (char *) buf;
    REQ_SHM(sreq)->user_count = count;
    REQ_SHM(sreq)->dest = rank;
    REQ_SHM(sreq)->datatype = datatype;
    REQ_SHM(sreq)->type = TYPE_READY;
    *request = sreq;

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_RSEND_INIT);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_isend)
static inline int MPIDI_shm_isend(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_ISEND);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_ISEND);
    mpi_errno = shm_do_isend(buf, count, datatype, rank, tag, comm, context_offset, request, TYPE_STANDARD);

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_ISEND);
    return mpi_errno;
}

static inline int MPIDI_shm_issend(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_ISSEND);

    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_ISSEND);
    mpi_errno = shm_do_isend(buf, count, datatype, rank, tag, comm, context_offset, request, TYPE_SYNC);

    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_ISSEND);
    return mpi_errno;
}

static inline int MPIDI_shm_cancel_send(MPID_Request * sreq)
{
    MPID_Request *req = MPIDI_shm_sendq.head;
    MPID_Request *prev_req = NULL;
    int mpi_errno = MPI_SUCCESS;

    while (req) {

        if (req == sreq) {
            MPIR_STATUS_SET_CANCEL_BIT(sreq->status, TRUE);
            MPIR_STATUS_SET_COUNT(sreq->status, 0);
            REQ_SHM_COMPLETE(sreq);
            REQ_SHM_DEQUEUE_AND_SET_ERROR(&sreq,prev_req,MPIDI_shm_sendq,mpi_errno);
            break;
        }

        prev_req = req;
        req = REQ_SHM(req)->next;
    }
    return mpi_errno;
}

#endif /* SHM_SEND_H_INCLUDED */
