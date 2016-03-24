/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef RECV_H_INCLUDED
#define RECV_H_INCLUDED

#include "impl.h"

__ALWAYS_INLINE__ int ucx_irecv_continous(void *buf,
        size_t data_sz,
        int rank,
        int tag,
        MPID_Comm * comm,
        int context_offset, MPID_Request ** request)
{

    int mpi_errno = MPI_SUCCESS;
    uint64_t ucp_tag, tag_mask;
    MPID_Request *req;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
//    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_WORKER_MUTEX);
    tag_mask = MPIDI_CH4_NMI_UCX_tag_mask(tag, rank);
    ucp_tag = MPIDI_CH4_NMI_UCX_recv_tag(tag, rank, comm->recvcontext_id + context_offset);

    ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_recv_nb(MPIDI_CH4_NMI_UCX_Global.worker,
                                                buf, data_sz, ucp_dt_make_contig(1),
                                                ucp_tag, tag_mask, &MPIDI_CH4_NMI_UCX_Handle_recv_callback);


    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);


    if (ucp_request->req == NULL) {
        req = MPIDI_CH4_NMI_UCX_Request_create();
        MPIDI_Request_init_rreq(req);
        ucp_request->req = req;
        ucp_request_release(ucp_request);
      }
     else {
        req  = ucp_request->req;
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    }
    (req)->kind = MPID_REQUEST_RECV;
fn_exit:
    *request = req;
    return mpi_errno;
fn_fail:
    goto fn_exit;
}
__ALWAYS_INLINE__ int ucx_irecv_non_continous(void *buf,
        size_t count,
        int rank,
        int tag,
        MPID_Comm * comm,
        int context_offset, MPID_Request ** request, MPID_Datatype* datatype)
{

    int mpi_errno = MPI_SUCCESS;
    uint64_t ucp_tag, tag_mask;
    MPID_Request *req;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
//    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_WORKER_MUTEX);
    tag_mask = MPIDI_CH4_NMI_UCX_tag_mask(tag, rank);
    ucp_tag = MPIDI_CH4_NMI_UCX_recv_tag(tag, rank, comm->recvcontext_id + context_offset);

    ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_recv_nb(MPIDI_CH4_NMI_UCX_Global.worker,
                                                buf, count, datatype->dev.netmod.ucx.ucp_datatype,
                                                ucp_tag, tag_mask, &MPIDI_CH4_NMI_UCX_Handle_recv_callback);


    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);


    if (ucp_request->req == NULL) {
        req = MPIDI_CH4_NMI_UCX_Request_create();
        MPIDI_Request_init_rreq(req);
        ucp_request->req = req;
        ucp_request_release(ucp_request);
      }
     else {
        req  = ucp_request->req;
        ucp_request->req = NULL;
        ucp_request_release(ucp_request);
    }
    (req)->kind = MPID_REQUEST_RECV;
fn_exit:
    *request = req;
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int do_irecv(void *buf,
                           int count,
                           MPI_Datatype datatype,
                           int rank,
                           int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    size_t data_sz;
    int dt_contig;
    MPID_Request *req;
    MPI_Aint dt_true_lb;

    MPID_Datatype *dt_ptr;

    if (unlikely(rank == MPI_PROC_NULL)) {
        req = MPIDI_CH4_NMI_UCX_Request_create();
        req->kind = MPID_REQUEST_RECV;
        req->status.MPI_ERROR = MPI_SUCCESS;
        req->status.MPI_SOURCE = rank;
        req->status.MPI_TAG = tag;
        MPIDI_CH4U_request_complete(req);
        *request = req;
        goto fn_exit;
    }

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    if (dt_contig)
        mpi_errno =
        ucx_irecv_continous(buf+dt_true_lb, data_sz, rank, tag, comm, context_offset, request);
    else
        mpi_errno =
        ucx_irecv_non_continous(buf, count, rank, tag, comm, context_offset, request, dt_ptr);
  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

__ALWAYS_INLINE__ int MPIDI_CH4_NM_recv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm,
                                    int context_offset,
                                    MPI_Status * status, MPID_Request ** request)
{

  return do_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);
}

__ALWAYS_INLINE__ int MPIDI_CH4_NM_recv_init(void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm,
                                         int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4U_recv_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

__ALWAYS_INLINE__ int MPIDI_CH4_NM_imrecv(void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      MPID_Request * message, MPID_Request ** rreqp)
{
    return MPIDI_CH4U_imrecv(buf, count, datatype, message, rreqp);
}
 __ALWAYS_INLINE__ int MPIDI_CH4_NM_irecv(void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{



 return   do_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);

}

static inline int MPIDI_CH4_NM_cancel_recv(MPID_Request * rreq)
{
    return MPIDI_CH4U_cancel_recv(rreq);
}

#endif /* RECV_H_INCLUDED */
