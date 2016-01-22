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
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *req, *matched_req = NULL;
    int count = 0;

    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_IMPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_IMPROBE);

    MPID_THREAD_CS_ENTER(POBJ,MPID_NEM_SHM_MUTEX);
    *message = NULL;

    if (unlikely(source == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(status);
        *flag = true;
        goto fn_exit;
    }

    for (req = MPIDI_shm_recvq_unexpected.head; req; req=REQ_SHM(req)->next) {
        if (ENVELOPE_MATCH(REQ_SHM(req), source, tag, comm->recvcontext_id + context_offset))
        {
            if (!matched_req)
                matched_req = req;
            if (req && REQ_SHM(req)->type == TYPE_EAGER) {
                *message = matched_req;
                break;
            }
        }
    }

    if (*message)
    {
        MPIDI_shm_queue_t mqueue = {NULL,NULL};
        MPID_Request *prev_req = NULL, *next_req = NULL;
        req = MPIDI_shm_recvq_unexpected.head;
        while (req) {
            next_req = REQ_SHM(req)->next;
            if (ENVELOPE_MATCH(REQ_SHM(req), source, tag, comm->recvcontext_id + context_offset))
            {
                if (mqueue.head == NULL)
                    MPIU_Assert(req == matched_req);
                count += MPIR_STATUS_GET_COUNT(req->status);
                REQ_SHM_DEQUEUE(&req, prev_req, MPIDI_shm_recvq_unexpected);
                REQ_SHM_ENQUEUE(req, mqueue);
                if (req && REQ_SHM(req)->type == TYPE_EAGER)
                    break;
            }
            else
                prev_req = req;
            req = next_req;
        }
        *flag = 1;
        matched_req->kind = MPID_REQUEST_MPROBE;
        matched_req->comm = comm;
        MPIR_Comm_add_ref(comm);
        status->MPI_TAG = matched_req->status.MPI_TAG;
        status->MPI_SOURCE = matched_req->status.MPI_SOURCE;
        MPIR_STATUS_SET_COUNT(*status, count);
    }
    else
    {
        *flag = 0;
        MPIDI_Progress_test();
    }

fn_exit:
    MPID_THREAD_CS_EXIT(POBJ,MPID_NEM_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_IMPROBE);
    return mpi_errno;
}

static inline int MPIDI_shm_iprobe(int source,
                                   int tag,
                                   MPID_Comm * comm,
                                   int context_offset, int *flag, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *req, *matched_req = NULL;
    int count = 0;
    MPIDI_STATE_DECL(MPID_STATE_MPIDI_SHM_IPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_SHM_IPROBE);
    MPID_THREAD_CS_ENTER(POBJ,MPID_NEM_SHM_MUTEX);
    if (unlikely(source == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(status);
        *flag = true;
        goto fn_exit;
    }

    for (req = MPIDI_shm_recvq_unexpected.head; req; req = REQ_SHM(req)->next) {
        if (ENVELOPE_MATCH(REQ_SHM(req), source, tag, comm->recvcontext_id + context_offset)) {
            count += MPIR_STATUS_GET_COUNT(req->status);
            if (REQ_SHM(req)->type == TYPE_EAGER) {
                matched_req = req;
                break;
            }
        }
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
    MPID_THREAD_CS_EXIT(POBJ,MPID_NEM_SHM_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_SHM_IPROBE);
    return mpi_errno;
}

#endif /* SHM_PROBE_H_INCLUDED */
