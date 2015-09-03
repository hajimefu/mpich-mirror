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
#ifndef MPIDCH4U_PROBE_H_INCLUDED
#define MPIDCH4U_PROBE_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_iprobe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_iprobe(int source,
                                     int tag,
                                     MPID_Comm * comm,
                                     int context_offset, int *flag, MPI_Status * status)
{
    int mpi_errno=MPI_SUCCESS, comm_idx;
    MPID_Comm *root_comm;
    MPID_Request *unexp_req;
    uint64_t match_bits, mask_bits;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_IPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_IPROBE);

    if (unlikely(source == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(status);
        *flag = true;
        goto fn_exit;
    }

    comm_idx = MPIDI_CH4I_get_context_index(comm->context_id);
    root_comm = MPIDI_CH4_Global.comm_req_lists[comm_idx].comm;

    match_bits = MPIDI_CH4I_init_recvtag(&mask_bits, root_comm->recvcontext_id +
                                        context_offset, source, tag);

    /* MPIDI_CS_ENTER(); */
    unexp_req = MPIDI_CH4I_find_unexp(match_bits, mask_bits,
                                      &MPIU_CH4U_COMM(root_comm, unexp_list));

    if (unexp_req) {
        *flag = 1;
        unexp_req->status.MPI_ERROR = MPI_SUCCESS;
        unexp_req->status.MPI_SOURCE =
            MPIDI_CH4I_get_source(MPIU_CH4U_REQUEST(unexp_req, tag));
        unexp_req->status.MPI_TAG =
            MPIDI_CH4I_get_tag(MPIU_CH4U_REQUEST(unexp_req, tag));
        MPIR_STATUS_SET_COUNT(unexp_req->status, MPIU_CH4U_REQUEST(unexp_req, count));

        status->MPI_TAG = unexp_req->status.MPI_TAG;
        status->MPI_SOURCE = unexp_req->status.MPI_SOURCE;
        MPIR_STATUS_SET_COUNT(*status, MPIU_CH4U_REQUEST(unexp_req, count));
    }
    else {
        *flag = 0;
        MPIDI_Progress_test();
    }
    /* MPIDI_CS_EXIT(); */

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_IPROBE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Probe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_probe(int source,
                                    int tag,
                                    MPID_Comm * comm, int context_offset, MPI_Status * status)
{
    int mpi_errno, flag = 0;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_PROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_PROBE);

    while (!flag) {
        MPIDU_RC_POP(MPIDI_CH4U_iprobe(source, tag, comm, context_offset, &flag, status));
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_PROBE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_improbe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_improbe(int source,
                                      int tag,
                                      MPID_Comm * comm,
                                      int context_offset,
                                      int *flag, MPID_Request ** message, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS, comm_idx;
    MPID_Comm *root_comm;
    MPID_Request *unexp_req;
    uint64_t match_bits, mask_bits;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_IMPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_IMPROBE);

    if (unlikely(source == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(status);
        *flag = true;
        goto fn_exit;
    }

    comm_idx = MPIDI_CH4I_get_context_index(comm->context_id);
    root_comm = MPIDI_CH4_Global.comm_req_lists[comm_idx].comm;

    match_bits = MPIDI_CH4I_init_recvtag(&mask_bits, root_comm->recvcontext_id +
                                        context_offset, source, tag);

    /* MPIDI_CS_ENTER(); */
    unexp_req = MPIDI_CH4I_dequeue_unexp(match_bits, mask_bits,
                                         &MPIU_CH4U_COMM(root_comm, unexp_list));

    if (unexp_req) {
        *flag = 1;
        *message = unexp_req;

        (*message)->kind = MPID_REQUEST_MPROBE;
        (*message)->comm = comm;

        unexp_req->status.MPI_ERROR = MPI_SUCCESS;
        unexp_req->status.MPI_SOURCE =
            MPIDI_CH4I_get_source(MPIU_CH4U_REQUEST(unexp_req, tag));
        unexp_req->status.MPI_TAG =
            MPIDI_CH4I_get_tag(MPIU_CH4U_REQUEST(unexp_req, tag));
        MPIR_STATUS_SET_COUNT(unexp_req->status, MPIU_CH4U_REQUEST(unexp_req, count));
        MPIU_CH4U_REQUEST(unexp_req, status) |= MPIDI_CH4U_REQ_UNEXP_DQUED;

        status->MPI_TAG = unexp_req->status.MPI_TAG;
        status->MPI_SOURCE = unexp_req->status.MPI_SOURCE;
        MPIR_STATUS_SET_COUNT(*status, MPIU_CH4U_REQUEST(unexp_req, count));
    }
    else {
        *flag = 0;
        MPIDI_Progress_test();
    }
    /* MPIDI_CS_EXIT(); */

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_IMPROBE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_mprobe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_mprobe(int source,
                                     int tag,
                                     MPID_Comm * comm,
                                     int context_offset,
                                     MPID_Request ** message, MPI_Status * status)
{
    int mpi_errno, flag = 0;
    MPIDI_STATE_DECL(MPID_STATE_CH4_MPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_MPROBE);
    while (!flag) {
        MPIDI_CH4U_improbe(source, tag, comm, context_offset, &flag, message, status);
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_MPROBE);
    return mpi_errno;
}

#endif /* MPIDCH4U_WIN_H_INCLUDED */