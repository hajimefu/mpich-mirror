/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef MPIDCH4U_REQUEST_H_INCLUDED
#define MPIDCH4U_REQUEST_H_INCLUDED

#include "ch4_types.h"

/* This function should be called any time an anysource request is matched so
 * the upper layer will have a chance to arbitrate who wins the race between
 * the netmod and the shmod. This will cancel the request of the other side and
 * take care of copying any relevant data. */
static inline int MPIDI_CH4U_anysource_matched(MPID_Request *rreq, int caller, int *continue_matching)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPIDI_CH4U_ANYSOURCE_MATCHED);
    MPIDI_FUNC_ENTER(MPIDI_CH4U_ANYSOURCE_MATCHED);

    MPIU_Assert(MPIDI_CH4U_NETMOD == caller || MPIDI_CH4U_SHM == caller);

    if (MPIDI_CH4U_NETMOD == caller) {
#if MPIDI_BUILD_CH4_SHM
        mpi_errno = MPIDI_shm_cancel_recv(rreq);

        /* If the netmod is cancelling the request, then shared memory will
         * just copy the status from the shared memory side because the netmod
         * will always win the race condition here. */
        if (MPIR_STATUS_GET_CANCEL_BIT(rreq->status)) {
            /* If the request is cancelled, copy the status object from the
             * partner request */
            rreq->status = MPIU_CH4_REQUEST_ANYSOURCE_PARTNER(rreq)->status;
        }
#endif
        *continue_matching = 1;
    } else if (MPIDI_CH4U_SHM == caller) {
        mpi_errno = MPIDI_netmod_cancel_recv(rreq);

        /* If the netmod has already matched this request, shared memory will
         * lose and should stop matching this request */
        *continue_matching = MPIR_STATUS_GET_CANCEL_BIT(rreq->status);
    }

    MPIDI_FUNC_EXIT(MPIDI_CH4U_ANYSOURCE_MATCHED);
    return mpi_errno;
}

#endif

