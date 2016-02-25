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
#ifndef NETMOD_UCX_SEND_H_INCLUDED
#define NETMOD_UCX_SEND_H_INCLUDED
#include <ucp/api/ucp.h>
#include "impl.h"
#include "ucx_types.h"

#undef FUNCNAME
#define FUNCNAME ucx_send_continous
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int ucx_send_continous(const void *buf,
                                         size_t data_sz,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm, int context_offset,
                                         MPID_Request ** request, int have_request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *req;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
    MPIDI_CH4_NMI_UCX_EP_t ep;
    uint64_t ucx_tag;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_SEND_CONTINOUS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_SEND_CONTINOUS);

    ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_CH4_NMI_UCX_init_tag(comm->context_id + context_offset, comm->rank, tag);

    ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_send_nb(ep, buf, data_sz, ucp_dt_make_contig(1),
                                     ucx_tag, &MPIDI_CH4_NMI_UCX_Handle_send_callback);

    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    if (ucp_request == NULL) {
        req = MPIDI_CH4_NMI_UCX_Alloc_send_request_done();
        goto fn_exit;
   }

   if(ucp_request->req){
       req = ucp_request->req;
       ucp_request->req = NULL;
       ucp_request_release(ucp_request);
   } else {
       req = MPIDI_CH4_NMI_UCX_Request_create();
       (req)->kind = MPID_REQUEST_SEND;
       ucp_request->req = req;
       ucp_request_release(ucp_request);
   }


fn_exit:
    *request = req;

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_SEND_CONTINOUS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME ucx_send_continous
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)

__ALWAYS_INLINE__ int ucx_send_non_continous(const void *buf,
                                         size_t count,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm, int context_offset,
                                         MPID_Request ** request, int have_request,
                                         MPID_Datatype *datatype)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *req;
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_request;
    MPIDI_CH4_NMI_UCX_EP_t ep;
    uint64_t ucx_tag;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_SEND_CONTINOUS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_SEND_CONTINOUS);

    ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(comm, rank);
    ucx_tag = MPIDI_CH4_NMI_UCX_init_tag(comm->context_id + context_offset, comm->rank, tag);

    ucp_request = (MPIDI_CH4_NMI_UCX_Ucp_request_t*) ucp_tag_send_nb(ep, buf, count, datatype->dev.netmod.ucx.ucp_datatype,
                                     ucx_tag, &MPIDI_CH4_NMI_UCX_Handle_send_callback);

    MPIDI_CH4_UCX_REQUEST(ucp_request, tag_send_nb);

    if (ucp_request == NULL) {
        req = MPIDI_CH4_NMI_UCX_Alloc_send_request_done();
        goto fn_exit;
   }

   if(ucp_request->req){
       req = ucp_request->req;
       ucp_request->req = NULL;
       ucp_request_release(ucp_request);
      }
    else{
       req = MPIDI_CH4_NMI_UCX_Request_create();
       (req)->kind = MPID_REQUEST_SEND;
       ucp_request->req = req;
       ucp_request_release(ucp_request);
   }


fn_exit:
    *request = req;

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_SEND_CONTINOUS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME ucx_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int ucx_send(const void *buf,
                           int count,
                           MPI_Datatype datatype,
                           int rank,
                           int tag,
                           MPID_Comm * comm, int context_offset, MPID_Request ** request,
                           int have_request)
{

    int dt_contig, mpi_errno;
    MPIDI_msg_sz_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_SEND);

    if (unlikely(rank == MPI_PROC_NULL)) {
        mpi_errno = MPI_SUCCESS;

        if (have_request) {
            *request = MPIDI_CH4_NMI_UCX_Request_create();
            (*request)->kind = MPID_REQUEST_SEND;
            MPIDI_CH4R_request_complete((*request));
        }

        goto fn_exit;
    }

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    if (dt_contig)
        mpi_errno =
            ucx_send_continous(buf + dt_true_lb, data_sz, rank, tag, comm, context_offset, request,
                               have_request);
    else
         mpi_errno =
            ucx_send_non_continous(buf, count, rank, tag, comm, context_offset, request,
                               have_request, dt_ptr);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send(const void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPID_Request ** request)
{

    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_SEND);
    mpi_errno = ucx_send(buf, count, datatype, rank, tag, comm, context_offset, request, 0);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_SEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_rsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_rsend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    //return MPIDI_CH4R_rsend(buf, count, datatype, rank, tag, comm, context_offset, request);

    return ucx_send(buf, count, datatype, rank, tag, comm, context_offset, request, 0);
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_irsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_irsend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    //return MPIDI_CH4R_irsend(buf, count, datatype, rank, tag, comm, context_offset, request);

    return ucx_send(buf, count, datatype, rank, tag, comm, context_offset, request, 1);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_ssend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_ssend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4R_ssend(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_CH4_NM_startall(int count, MPID_Request * requests[])
{
    return MPIDI_CH4R_startall(count, requests);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_send_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_send_init(const void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPID_Comm * comm,
                                         int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4R_send_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_ssend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_ssend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm * comm,
                                          int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4R_ssend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_bsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_bsend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm * comm,
                                          int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4R_bsend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_rsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_rsend_init(const void *buf,
                                          int count,
                                          MPI_Datatype datatype,
                                          int rank,
                                          int tag,
                                          MPID_Comm * comm,
                                          int context_offset, MPID_Request ** request)
{
    return MPIDI_CH4R_rsend_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_isend(const void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPID_Request ** request)
{

    return ucx_send(buf, count, datatype, rank, tag, comm, context_offset, request, 1);

}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_issend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_issend(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm, int context_offset, MPID_Request ** request)
{

    return ucx_send(buf, count, datatype, rank, tag, comm, context_offset, request, 1);

}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmode_cancel_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_cancel_send(MPID_Request * sreq)
{
    return MPIDI_CH4R_cancel_send(sreq);
}

#endif /* NETMOD_AM_OFI_SEND_H_INCLUDED */
