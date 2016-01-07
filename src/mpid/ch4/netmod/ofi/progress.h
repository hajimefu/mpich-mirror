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
#include "am_events.h"

#define NUM_CQ_ENTRIES 8
static inline int handle_cq_error(ssize_t ret);
static inline int handle_cq_entries(cq_tagged_entry_t * wc,ssize_t num);
static inline int am_progress(void *netmod_context,
                              int   blocking);

__attribute__((__always_inline__)) static inline
int MPIDI_netmod_progress_generic(void *netmod_context,
                                  int   blocking,
                                  int   do_am,
                                  int   do_tagged)
{
    int                mpi_errno;
    if(do_tagged) {
        cq_tagged_entry_t  wc[NUM_CQ_ENTRIES];
        ssize_t            ret;

        MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);
        ret = fi_cq_read(MPIDI_Global.p2p_cq, (void *) wc, NUM_CQ_ENTRIES);

        if(likely(ret > 0))
            mpi_errno = handle_cq_entries(wc,ret);
        else if (ret == -FI_EAGAIN)
            mpi_errno = MPI_SUCCESS;
        else
            mpi_errno = handle_cq_error(ret);

        MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);
        MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
        MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    }
    if(do_am)
        mpi_errno = am_progress(netmod_context,blocking);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_progress
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_progress(void *netmod_context, int blocking)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_PROGRESS);
    mpi_errno = MPIDI_netmod_progress_generic(netmod_context,
                                              blocking,
                                              MPIDI_ENABLE_AM,
                                              MPIDI_ENABLE_TAGGED);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_PROGRESS);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME am_progress
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int am_progress(void *netmod_context, int blocking)
{
    int mpi_errno = MPI_SUCCESS, found = 0, ret, buffered;
    struct fi_cq_data_entry cq_entry;
    struct fi_cq_err_entry  cq_err_entry;
    fi_addr_t source;

    MPIDI_STATE_DECL(MPID_STATE_AM_NETMOD_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_AM_NETMOD_PROGRESS);

    do {
        if ((MPIDI_Global.cq_buff_head != MPIDI_Global.cq_buff_tail) ||
            !slist_empty(&MPIDI_Global.cq_buff_list)) {

            if (MPIDI_Global.cq_buff_head != MPIDI_Global.cq_buff_tail) {
                source = MPIDI_Global.cq_buffered[MPIDI_Global.cq_buff_tail].source;
                cq_entry = MPIDI_Global.cq_buffered[MPIDI_Global.cq_buff_tail].cq_entry;
                MPIDI_Global.cq_buff_tail = (MPIDI_Global.cq_buff_tail + 1) % MPIDI_NUM_CQ_BUFFERED;
            } else {
                struct cq_list *cq_list_entry;
                struct slist_entry *entry = slist_remove_head(&MPIDI_Global.cq_buff_list);
                cq_list_entry = container_of(entry, struct cq_list, entry);
                source = cq_list_entry->source;
                cq_entry = cq_list_entry->cq_entry;
                MPIU_Free((void *)cq_list_entry);
            }
            buffered = 1;
        } else {
            ret = fi_cq_readfrom(MPIDI_Global.p2p_cq, &cq_entry, 1, &source);
            if (ret == -FI_EAGAIN)
                continue;
            if (ret < 0) {
                fi_cq_readerr(MPIDI_Global.p2p_cq, &cq_err_entry, 0);
                fprintf(stderr, "fi_cq_read failed with error: %s\n", fi_strerror(cq_err_entry.err));
                goto fn_fail;
            }
            buffered = 0;
        }
        found = 1;
        if (cq_entry.flags & FI_SEND) {
/*            mpi_errno = MPIDI_netmod_handle_send_completion(&cq_entry);*/
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);

        }
        else if (cq_entry.flags & FI_RECV) {
/*            mpi_errno = MPIDI_netmod_handle_recv_completion(&cq_entry, source, netmod_context);*/
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);

            if ((cq_entry.flags & FI_MULTI_RECV) && !buffered) {
/*                mpi_errno = MPIDI_netmod_repost_buffer(cq_entry.op_context, netmod_context);*/
                if (mpi_errno) MPIR_ERR_POP(mpi_errno);
            }

        }
        else if (cq_entry.flags & FI_READ) {
            /*mpi_errno = MPIDI_netmod_handle_read_completion(&cq_entry, source, netmod_context); */
            if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        }
        else {
            MPIU_Assert(0);
        }
    } while (blocking && !found);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_AM_NETMOD_PROGRESS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int handle_cq_entries(cq_tagged_entry_t * wc,ssize_t num)
{
    int i, mpi_errno;
    MPID_Request *req;
    for (i = 0; i < num; i++) {
        req = devreq_to_req(wc[i].op_context);
        MPIDI_CH4_NMI_MPI_RC_POP(dispatch_function(&wc[i],req));
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
    switch (ret) {
    case -FI_EAVAIL:
        fi_cq_readerr(MPIDI_Global.p2p_cq, &e, 0);
        switch (e.err) {
        case FI_ETRUNC:
            req = devreq_to_req(e.op_context);
            switch(req->kind) {
            case MPID_REQUEST_SEND:
                mpi_errno = dispatch_function(NULL,req);
                break;
            case MPID_REQUEST_RECV:
                mpi_errno = dispatch_function((cq_tagged_entry_t *) &e, req);
                req->status.MPI_ERROR = MPI_ERR_TRUNCATE;
                break;
            default:
                MPIR_ERR_SETFATALANDJUMP4(mpi_errno, MPI_ERR_OTHER, "**ofid_poll",
                                          "**ofid_poll %s %d %s %s", __SHORT_FILE__,
                                          __LINE__, FCNAME, fi_strerror(e.err));
            }
            break;
        case FI_ECANCELED:
            req = devreq_to_req(e.op_context);
            MPIR_STATUS_SET_CANCEL_BIT(req->status, TRUE);
            break;
        case FI_ENOMSG:
            req = devreq_to_req(e.op_context);
            peek_empty_event(NULL, req);
            break;
        }
        break;
    default:
        MPIR_ERR_SETFATALANDJUMP4(mpi_errno, MPI_ERR_OTHER, "**ofid_poll",
                                  "**ofid_poll %s %d %s %s", __SHORT_FILE__, __LINE__,
                                  FCNAME, fi_strerror(errno));
        break;
    }
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_CQ_ERROR);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* NETMOD_OFI_PROGRESS_H_INCLUDED */
