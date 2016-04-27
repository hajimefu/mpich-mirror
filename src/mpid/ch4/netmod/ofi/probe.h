/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef NETMOD_OFI_PROBE_H_INCLUDED
#define NETMOD_OFI_PROBE_H_INCLUDED

#include "impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Do_iprobe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Do_iprobe(int source,
                                              int tag,
                                              MPIR_Comm *comm,
                                              int context_offset,
                                              int *flag,
                                              MPI_Status *status,
                                              MPIR_Request **message,
                                              uint64_t peek_flags)
{
    int mpi_errno = MPI_SUCCESS;
    fi_addr_t remote_proc;
    uint64_t match_bits, mask_bits;
    MPIR_Request r, *rreq;      /* don't need to init request, output only */
    struct fi_msg_tagged msg;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_DO_PROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_DO_PROBE);

    if(unlikely(source == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(status);
        *flag = true;

        if(message)
            *message = NULL;

        goto fn_exit;
    } else if(unlikely(MPI_ANY_SOURCE == source))
        remote_proc = FI_ADDR_UNSPEC;
    else
        remote_proc = MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, source, MPIDI_CH4_NMI_OFI_API_TAG);

    if(message)
        MPIDI_CH4_NMI_OFI_REQUEST_CREATE(rreq);
    else
        rreq = &r;

    match_bits = MPIDI_CH4_NMI_OFI_Init_recvtag(&mask_bits, comm->context_id + context_offset, source, tag);

    MPIDI_CH4_NMI_OFI_REQUEST(rreq, event_id) = MPIDI_CH4_NMI_OFI_EVENT_PEEK;
    MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_id)  = MPIDI_CH4_NMI_OFI_PEEK_START;

    msg.msg_iov = NULL;
    msg.desc = NULL;
    msg.iov_count = 0;
    msg.addr = remote_proc;
    msg.tag = match_bits;
    msg.ignore = mask_bits;
    msg.context = (void *) &(MPIDI_CH4_NMI_OFI_REQUEST(rreq, context));
    msg.data = 0;

    MPIDI_CH4_NMI_OFI_CALL(fi_trecvmsg(MPIDI_CH4_NMI_OFI_EP_RX_TAG(0), &msg, peek_flags | FI_PEEK | FI_COMPLETION), trecv);
    MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_id) == MPIDI_CH4_NMI_OFI_PEEK_START);

    switch(MPIDI_CH4_NMI_OFI_REQUEST(rreq, util_id)) {
        case  MPIDI_CH4_NMI_OFI_PEEK_NOT_FOUND:
            *flag = 0;

            if(message) MPIU_Handle_obj_free(&MPIR_Request_mem, rreq);

            goto fn_exit;
            break;

        case  MPIDI_CH4_NMI_OFI_PEEK_FOUND:
            MPIR_Request_extract_status(rreq, status);
            *flag = 1;

            if(message) *message = rreq;

            break;

        default:
            MPIU_Assert(0);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_DO_PROBE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_probe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_probe(int source,
                                     int tag,
                                     MPIR_Comm *comm, int context_offset, MPI_Status *status)
{
    int mpi_errno = MPI_SUCCESS, flag = 0;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_PROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_PROBE);

    while(!flag) {
        mpi_errno = MPIDI_Iprobe(source, tag, comm, context_offset, &flag, status);

        if(mpi_errno)
            MPIR_ERR_POP(mpi_errno);

        MPIDI_CH4_NMI_OFI_PROGRESS();
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_PROBE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_improbe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_improbe(int source,
                                       int tag,
                                       MPIR_Comm *comm,
                                       int context_offset,
                                       int *flag, MPIR_Request **message, MPI_Status *status)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_IMPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_IMPROBE);
    /* Set flags for mprobe peek, when ready */
    int mpi_errno = MPIDI_CH4_NMI_OFI_Do_iprobe(source, tag, comm, context_offset,
                                                flag, status, message, FI_CLAIM | FI_COMPLETION);

    if(*flag && *message) {
        (*message)->kind = MPIR_REQUEST_KIND__MPROBE;
        (*message)->comm = comm;
        MPIU_Object_add_ref(comm);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_IMPROBE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_iprobe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_iprobe(int source,
                                      int tag,
                                      MPIR_Comm *comm,
                                      int context_offset, int *flag, MPI_Status *status)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_IPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_IPROBE);
    mpi_errno = MPIDI_CH4_NMI_OFI_Do_iprobe(source, tag, comm, context_offset, flag, status, NULL, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_IPROBE);
    return mpi_errno;
}

#endif /* NETMOD_OFI_PROBE_H_INCLUDED */
