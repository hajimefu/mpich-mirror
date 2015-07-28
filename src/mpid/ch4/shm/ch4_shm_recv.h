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
#include "../src/ch4_impl.h"

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
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq = NULL;
    MPIDI_STATE_DECL(MPID_STATE_DO_IRECV);

    MPIDI_FUNC_ENTER(MPID_STATE_DO_IRECV);

    MPIDI_Request_create_rreq(rreq);
    ENVELOPE_SET(REQ_SHM(rreq),rank,tag,comm->context_id+context_offset);
    rreq->comm = comm;
    MPIR_Comm_add_ref(comm);
    REQ_SHM(rreq)->user_buf = buf;
    REQ_SHM(rreq)->user_count = count;
    REQ_SHM(rreq)->datatype = datatype;
    REQ_SHM(rreq)->next = NULL;
    MPIR_STATUS_SET_COUNT(rreq->status, 0);
    /* enqueue rreq */
    REQ_SHM_ENQUEUE(rreq,MPIDI_shm_recvq_posted);
    *request = rreq;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_DO_IRECV);
    return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_shm_recv)
static inline int MPIDI_shm_recv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm,
                                    int context_offset,
                                    MPI_Status * status, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS, dt_contig, src;
    MPIDI_msg_sz_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    char *recv_buffer;
    MPID_Request *rreq = NULL;
    MPIDI_STATE_DECL(MPIDI_SHM_RECV);

    MPIDI_FUNC_ENTER(MPIDI_SHM_RECV);

    MPIDI_Datatype_get_info(count, datatype,
                            dt_contig, data_sz, dt_ptr, dt_true_lb);

    /* search in unexpected queue */
    MPID_Request* req = MPIDI_shm_recvq_unexpected.head;
    MPID_Request* prev_req = NULL;
    while( req ) {
        MPIU_DBG_MSG_FMT(HANDLE,TYPICAL,(MPIU_DBG_FDEST,"Matching to unexpected in irecv %d,%d,%d\n",rank,tag,comm->context_id+context_offset));
        if( ENVELOPE_MATCH(REQ_SHM(req),rank,tag,comm->context_id+context_offset) )
        {
            MPIU_DBG_MSG_FMT(HANDLE,TYPICAL,(MPIU_DBG_FDEST,"Matching done in irecv %d,%d,%d\n",rank,tag,comm->context_id+context_offset));
            recv_buffer = (char *)buf + dt_true_lb;
            MPIU_Memcpy( recv_buffer, (void*)REQ_SHM(req)->user_buf, REQ_SHM(req)->data_sz );
            /* set status */
            if( status != MPI_STATUS_IGNORE ) {
                status->MPI_SOURCE = REQ_SHM(req)->rank;
                status->MPI_TAG = REQ_SHM(req)->tag;
                MPIR_STATUS_SET_COUNT(*status, REQ_SHM(req)->data_sz);
            }
            /* dequeue unexpected req */
            MPIU_Free((void*)REQ_SHM(req)->user_buf);
            REQ_SHM_DEQUEUE_AND_SET_ERROR(&req,prev_req,MPIDI_shm_recvq_unexpected,mpi_errno);
            *request = NULL;
            goto fn_exit;
        }
        prev_req = req;
        req = REQ_SHM(req)->next;
    }

    /* try to receive immediately from fastbox */
    if( rank != MPI_ANY_SOURCE )
    {
        MPID_nem_fbox_mpich_t* fbox = &MPID_nem_mem_region.mailboxes.in[rank]->mpich;
        if( OPA_load_int(&fbox->flag.value) )
        {
            if( ENVELOPE_MATCH(&fbox->cell,rank,tag,comm->context_id+context_offset) )
            {
                MPIU_DBG_MSG_FMT(HANDLE,TYPICAL,(MPIU_DBG_FDEST,"Matching in irecv %d,%d,%d\n",rank,tag,comm->context_id+context_offset));
                recv_buffer = (char *)buf + dt_true_lb;
                MPIU_Memcpy( recv_buffer, (void*)fbox->cell.pkt.mpich.p.payload, fbox->cell.pkt.mpich.datalen );
                /* set status */
                if( status != MPI_STATUS_IGNORE ) {
                    status->MPI_SOURCE = fbox->cell.rank;
                    status->MPI_TAG = fbox->cell.tag;
                    MPIR_STATUS_SET_COUNT(*status, fbox->cell.pkt.mpich.datalen);
                }
                /* release fastbox */
                OPA_store_release_int( &(fbox->flag.value), 0 );
                *request = NULL;
                goto fn_exit;
            }
        }
    }

    /* failed to receive immediately */
    /* create a request */
    mpi_errno = shm_do_irecv( buf, count, datatype, rank, tag, comm, context_offset, request);

fn_exit:
    MPIDI_FUNC_EXIT(MPIDI_SHM_RECV);
    return mpi_errno;
}

static inline int MPIDI_shm_recv_init(void *buf,
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

    mpi_errno = shm_do_irecv( buf, count, datatype, rank, tag, comm, context_offset, request);

fn_exit:
    MPIDI_FUNC_EXIT(MPIDI_SHM_IRECV);
    return mpi_errno;
}

static inline int MPIDI_shm_cancel_recv(MPID_Request * rreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_RECV_H_INCLUDED */
