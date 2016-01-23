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
#ifndef MPIDCH4R_REQUEST_H_INCLUDED
#define MPIDCH4R_REQUEST_H_INCLUDED

#include "ch4_types.h"
#include "ch4r_buf.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_request_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline MPID_Request *MPIDI_CH4I_am_request_create()
{
    MPID_Request *req;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_REQUEST_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_REQUEST_CREATE);

    req = MPIDI_CH4I_alloc_and_init_req(2);
    MPIDI_CH4_NM_am_request_init(req);

    CH4_COMPILE_TIME_ASSERT(sizeof(MPIDI_CH4R_req_t) <= MPIDI_CH4I_BUF_POOL_SZ);
    MPIDI_CH4R_REQUEST(req, req)    =
        (MPIDI_CH4R_req_t *) MPIDI_CH4R_get_buf(MPIDI_CH4_Global.buf_pool);
    MPIU_Assert(MPIDI_CH4R_REQUEST(req, req));
    MPIDI_CH4R_REQUEST(req, req->status) = 0;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_REQUEST_CREATE);

    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_win_request_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline MPID_Request *MPIDI_CH4I_am_win_request_create()
{
    MPID_Request *req;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_REQUEST_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_REQUEST_CREATE);

    req = MPIDI_CH4I_alloc_and_init_req(1);
    MPIDI_CH4_NM_am_request_init(req);

    CH4_COMPILE_TIME_ASSERT(sizeof(MPIDI_CH4R_req_t) <= MPIDI_CH4I_BUF_POOL_SZ);
    MPIDI_CH4R_REQUEST(req, req)         =
        (MPIDI_CH4R_req_t *) MPIDI_CH4R_get_buf(MPIDI_CH4_Global.buf_pool);
    MPIU_Assert(MPIDI_CH4R_REQUEST(req, req));
    MPIDI_CH4R_REQUEST(req, req->status) = 0;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_REQUEST_CREATE);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_am_request_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4I_am_request_complete(MPID_Request *req)
{
    int count;
    MPID_cc_decr(req->cc_ptr, &count);
    MPIU_Assert(count >= 0);
    if (count == 0) {
        if (MPIDI_CH4R_REQUEST(req, req) && MPID_cc_is_complete(&req->cc)) {
            MPIDI_CH4R_release_buf(MPIDI_CH4R_REQUEST(req, req));
            MPIDI_CH4R_REQUEST(req, req) = NULL;
        }
        MPIDI_CH4_NM_am_request_finalize(req);
        MPIDI_CH4R_Request_release(req);
    }
}

/* This function should be called any time an anysource request is matched so
 * the upper layer will have a chance to arbitrate who wins the race between
 * the netmod and the shmod. This will cancel the request of the other side and
 * take care of copying any relevant data. */
static inline int MPIDI_CH4R_anysource_matched(MPID_Request *rreq, int caller, int *continue_matching)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPIDI_CH4R_ANYSOURCE_MATCHED);
    MPIDI_FUNC_ENTER(MPIDI_CH4R_ANYSOURCE_MATCHED);

    MPIU_Assert(MPIDI_CH4R_NETMOD == caller || MPIDI_CH4R_SHM == caller);

    if (MPIDI_CH4R_NETMOD == caller) {
#if MPIDI_BUILD_CH4_SHM
        mpi_errno = MPIDI_shm_cancel_recv(rreq);

        /* If the netmod is cancelling the request, then shared memory will
         * just copy the status from the shared memory side because the netmod
         * will always win the race condition here. */
        if (MPIR_STATUS_GET_CANCEL_BIT(rreq->status)) {
            /* If the request is cancelled, copy the status object from the
             * partner request */
            rreq->status = MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(rreq)->status;
        }
#endif
        *continue_matching = 0;
    } else if (MPIDI_CH4R_SHM == caller) {
        mpi_errno = MPIDI_CH4_NM_cancel_recv(rreq);

        /* If the netmod has already matched this request, shared memory will
         * lose and should stop matching this request */
        *continue_matching = !MPIR_STATUS_GET_CANCEL_BIT(rreq->status);
    }

    MPIDI_FUNC_EXIT(MPIDI_CH4R_ANYSOURCE_MATCHED);
    return mpi_errno;
}

#endif

