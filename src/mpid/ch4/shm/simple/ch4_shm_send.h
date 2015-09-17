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
    /* enqueue sreq */
    REQ_SHM_ENQUEUE(sreq, MPIDI_shm_sendq);
    *request = sreq;

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
        /* Try fastbox */
        int grank = MPIDI_CH4U_rank_to_lpid(rank, comm);
#if 0
        int local_rank = MPID_nem_mem_region.local_ranks[grank];
        MPID_nem_fbox_mpich_t *fbox = &MPID_nem_mem_region.mailboxes.out[local_rank]->mpich;
        if (!MPID_nem_fbox_is_full((MPID_nem_fbox_common_ptr_t) fbox)) {
            ENVELOPE_SET(&fbox->cell, comm->rank, tag, comm->context_id + context_offset);
            fbox->cell.pkt.mpich.datalen = data_sz;
            MPIU_Memcpy((void *) fbox->cell.pkt.mpich.p.payload, (char*)buf+dt_true_lb, data_sz);
            OPA_store_release_int(&(fbox->flag.value), 1);
            *request = NULL;
            goto fn_exit;
        }
#endif
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
        MPID_Request* req = requests[i];
        if( req->kind == MPID_PREQUEST_SEND ) {
            mpi_errno = shm_do_isend(REQ_SHM(req)->user_buf, REQ_SHM(req)->user_count,
                    REQ_SHM(req)->datatype, REQ_SHM(req)->dest, REQ_SHM(req)->tag, 
                    req->comm, REQ_SHM(req)->context_id - req->comm->context_id, 
                    &req->partner_request, REQ_SHM(req)->type);
        }
        else if( req->kind == MPID_PREQUEST_RECV ) {
            mpi_errno = shm_do_irecv(REQ_SHM(req)->user_buf, REQ_SHM(req)->user_count,
                    REQ_SHM(req)->datatype, REQ_SHM(req)->rank, REQ_SHM(req)->tag, 
                    req->comm, REQ_SHM(req)->context_id - req->comm->context_id, 
                    &req->partner_request);
        }
        else
        {
            MPIU_Assert(0);
        }
        req->cc_ptr = &req->partner_request->cc;
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_STARTALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
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

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND_INIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
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

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_SEND_INIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_shm_bsend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_rsend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
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
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_SEND_H_INCLUDED */
