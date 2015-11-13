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
#ifndef SHM_PROBE_H_INCLUDED
#define SHM_PROBE_H_INCLUDED

#include "ch4_shm_impl.h"

static inline int MPIDI_shm_probe(int source,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS;
    MPIU_Assert(0);
    return mpi_errno;
}


static inline int MPIDI_shm_mprobe(int source,
                                   int tag,
                                   MPID_Comm * comm,
                                   int context_offset, MPID_Request ** message, MPI_Status * status)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_improbe(int source,
                                    int tag,
                                    MPID_Comm * comm,
                                    int context_offset,
                                    int *flag, MPID_Request ** message, MPI_Status * status)
{
/*
    MPIU_Assert(0);
    return MPI_SUCCESS;
*/
    int mpi_errno=MPI_SUCCESS, comm_idx;
    MPID_Comm *root_comm;
    MPID_Request *req;
    uint64_t match_bits, mask_bits;
    int count = 0;

    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_IMPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_IMPROBE);

    *message = NULL;

    if (unlikely(source == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(status);
        *flag = true;
        goto fn_exit;
    }

    comm_idx = MPIDI_CH4I_get_context_index(comm->context_id);
    //root_comm = MPIDI_CH4_Global.comm_req_lists[comm_idx].comm;
    root_comm = comm;

    for (req = MPIDI_shm_recvq_unexpected.head; req; req=REQ_SHM(req)->next) {
        if (ENVELOPE_MATCH(REQ_SHM(req), source, tag, root_comm->recvcontext_id + context_offset))
        {
            *message = req;
            break;
        }
    }
    for (req = MPIDI_shm_recvq_unexpected.head; req; req=REQ_SHM(req)->next) {
        if (ENVELOPE_MATCH(REQ_SHM(req), source, tag, root_comm->recvcontext_id + context_offset))
        {
            while (req && REQ_SHM(req)->type == TYPE_LMT) {
                count += MPIR_STATUS_GET_COUNT(req->status);
                req = REQ_SHM(req)->next;
            }
            if (req && REQ_SHM(req)->type == TYPE_EAGER) {
                *message = req;
                count += MPIR_STATUS_GET_COUNT(req->status);
            }
            break;
        }
    }

    if (*message) {
        (*message)->kind = MPID_REQUEST_MPROBE;
        (*message)->comm = comm;
        MPIR_Comm_add_ref(comm);
        *flag = 1;
        (*message)->status.MPI_ERROR = MPI_SUCCESS;
        MPIR_STATUS_SET_COUNT((*message)->status, count);

        status->MPI_TAG = (*message)->status.MPI_TAG;
        status->MPI_SOURCE = (*message)->status.MPI_SOURCE;
        MPIR_STATUS_SET_COUNT(*status, count);
    }
    else {
        *flag = 0;
        MPIDI_Progress_test();
    }
    /* MPIDI_CS_EXIT(); */

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_IMPROBE);
    return mpi_errno;
}

static inline int MPIDI_shm_iprobe(int source,
                                   int tag,
                                   MPID_Comm * comm,
                                   int context_offset, int *flag, MPI_Status * status)
{
    int mpi_errno=MPI_SUCCESS, comm_idx;
    MPID_Comm *root_comm;
    MPID_Request *req, *matched_req=NULL;
    uint64_t match_bits, mask_bits;
    int count = 0;

    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_IPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_IPROBE);

    if (unlikely(source == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(status);
        *flag = true;
        goto fn_exit;
    }

    comm_idx = MPIDI_CH4I_get_context_index(comm->context_id);
    root_comm = comm;

    req = MPIDI_shm_recvq_unexpected.head;
    while (req) {
        if (ENVELOPE_MATCH(REQ_SHM(req), source, tag, root_comm->recvcontext_id + context_offset)) {
            count += MPIR_STATUS_GET_COUNT(req->status);
            if (REQ_SHM(req)->type == TYPE_EAGER) {
                matched_req = req;
                break;
            }
        }
        req = REQ_SHM(req)->next;
    }

    if (matched_req) {
        *flag = 1;
        status->MPI_TAG = matched_req->status.MPI_TAG;
        status->MPI_SOURCE = matched_req->status.MPI_SOURCE;
        MPIR_STATUS_SET_COUNT(*status, count);
    }
    else {
        *flag = 0;
        MPIDI_Progress_test();
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_IPROBE);
    return mpi_errno;
}

#endif /* SHM_PROBE_H_INCLUDED */
