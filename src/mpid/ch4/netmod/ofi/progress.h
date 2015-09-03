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
#ifndef NETMOD_OFI_PROGRESS_H_INCLUDED
#define NETMOD_OFI_PROGRESS_H_INCLUDED

#include "impl.h"
#include "events.h"
#define NUM_CQ_ENTRIES 8
static inline int handle_cq_error(ssize_t ret);
static inline int handle_cq_entries(cq_tagged_entry_t * wc,ssize_t num);

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_progress
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_progress(void *netmod_context, int blocking)
{
    int                mpi_errno;
    cq_tagged_entry_t  wc[NUM_CQ_ENTRIES];
    ssize_t            ret;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_PROGRESS);

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);
    ret = fi_cq_read(MPIDI_Global.p2p_cq, (void *) wc, NUM_CQ_ENTRIES);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);
    if(likely(ret > 0))
        mpi_errno = handle_cq_entries(wc,ret);
    else if (ret == -FI_EAGAIN)
        mpi_errno = MPI_SUCCESS;
    else
        mpi_errno = handle_cq_error(ret);

    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_PROGRESS);
    return mpi_errno;
}

static inline int handle_cq_entries(cq_tagged_entry_t * wc,ssize_t num)
{
    int i, mpi_errno;
    MPID_Request *req;
    for (i = 0; i < num; i++) {
        req = devreq_to_req(wc[i].op_context);
        MPI_RC_POP(dispatch_function(&wc[i],req));
    }
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME handle_cq_error
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int handle_cq_error(ssize_t ret)
{
    int mpi_errno = MPI_SUCCESS;
    cq_err_entry_t e;
    MPID_Request *req;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_CQ_ERROR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_CQ_ERROR);
    if (ret == -FI_EAVAIL) {
        fi_cq_readerr(MPIDI_Global.p2p_cq, &e, 0);
        if (e.err == FI_ETRUNC) {
            /* This error message should only be delivered on send
             * events.  We want to ignore truncation errors
             * on the sender side, but complete the request anyways
             * Other kinds of requests, this is fatal.
             */
            req = devreq_to_req(e.op_context);
            if (req->kind == MPID_REQUEST_SEND)
                mpi_errno = dispatch_function(NULL,req);
            else if (req->kind == MPID_REQUEST_RECV) {
                mpi_errno = dispatch_function((cq_tagged_entry_t *) &e, req);
                req->status.MPI_ERROR = MPI_ERR_TRUNCATE;
            }
            else
                MPIR_ERR_SETFATALANDJUMP4(mpi_errno, MPI_ERR_OTHER, "**ofid_poll",
                                          "**ofid_poll %s %d %s %s", __SHORT_FILE__,
                                          __LINE__, FCNAME, fi_strerror(e.err));
        }
        else if (e.err == FI_ECANCELED) {
            req = devreq_to_req(e.op_context);
            MPIR_STATUS_SET_CANCEL_BIT(req->status, TRUE);
        }
        else
            MPIR_ERR_SETFATALANDJUMP4(mpi_errno, MPI_ERR_OTHER, "**ofid_poll",
                                      "**ofid_poll %s %d %s %s", __SHORT_FILE__, __LINE__,
                                      FCNAME, fi_strerror(e.err));
    }
    else
        MPIR_ERR_SETFATALANDJUMP4(mpi_errno, MPI_ERR_OTHER, "**ofid_poll",
                                  "**ofid_poll %s %d %s %s", __SHORT_FILE__, __LINE__,
                                  FCNAME, fi_strerror(errno));
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* NETMOD_OFI_PROGRESS_H_INCLUDED */
