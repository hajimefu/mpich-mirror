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

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    MPIDI_Request_create_rreq(rreq);
    ENVELOPE_SET(REQ_SHM(rreq), rank, tag, comm->context_id + context_offset);
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    REQ_SHM(rreq)->user_buf = (char*)buf + dt_true_lb;
    REQ_SHM(rreq)->user_count = count;
    REQ_SHM(rreq)->datatype = datatype;
    REQ_SHM(rreq)->next = NULL;
    REQ_SHM(rreq)->segment_ptr = NULL;
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
    MPIU_Assert(0);
    return MPI_SUCCESS;
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

static inline int MPIDI_shm_cancel_recv(MPID_Request * rreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_RECV_H_INCLUDED */
