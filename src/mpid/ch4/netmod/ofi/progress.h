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

#define NUM_CQ_ENTRIES 8

static inline MPID_Request *devreq_to_req(void *context)
{
    char *base = (char *) context;
    return (MPID_Request *) container_of(base, MPID_Request, dev.netmod);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_progress
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_progress(void *netmod_context, int blocking)
{
    int mpi_errno = MPI_SUCCESS;
    int ret, count;
    cq_tagged_entry_t wc[NUM_CQ_ENTRIES];
    cq_err_entry_t e;
    MPID_Request *req;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_PROGRESS);

    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);

    do {
        MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
        ret = fi_cq_read(MPIDI_Global.p2p_cq, (void *) wc, NUM_CQ_ENTRIES);
        MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);

        if (ret > 0) {
            for (count = 0; count < ret; count++) {
                req = devreq_to_req(wc[count].op_context);
                mpi_errno = REQ_OFI(req, callback) (&wc[count], req);
            }
        }
        else if (ret == -FI_EAGAIN) {
        }
        else if (ret < 0) {
            if (ret == -FI_EAVAIL) {
                MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
                fi_cq_readerr(MPIDI_Global.p2p_cq, &e, 0);
                MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);

                if (e.err == FI_ETRUNC) {
                    /* This error message should only be delivered on send
                     * events.  We want to ignore truncation errors
                     * on the sender side, but complete the request anyways
                     * Other kinds of requests, this is fatal.
                     */
                    req = devreq_to_req(e.op_context);

                    if (req->kind == MPID_REQUEST_SEND)
                        mpi_errno = REQ_OFI(req, callback) (NULL, req);
                    else if (req->kind == MPID_REQUEST_RECV) {
                        mpi_errno = REQ_OFI(req, callback) ((cq_tagged_entry_t *) & e, req);
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
        }
    } while (ret > 0);

  fn_exit:
    /* fixme:  lock cycling an artifact of per-object switch */
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_PROGRESS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* NETMOD_OFI_PROGRESS_H_INCLUDED */
