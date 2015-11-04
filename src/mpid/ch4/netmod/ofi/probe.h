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
#ifndef NETMOD_OFI_PROBE_H_INCLUDED
#define NETMOD_OFI_PROBE_H_INCLUDED

#include "impl.h"

#undef FUNCNAME
#define FUNCNAME do_iprobe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_iprobe(int source,
                            int tag,
                            MPID_Comm * comm,
                            int context_offset,
                            int *flag,
                            MPI_Status * status,
                            MPID_Request ** message,
                            uint64_t peek_flags)
{
    int mpi_errno = MPI_SUCCESS;
    fi_addr_t remote_proc;
    uint64_t match_bits, mask_bits;
    MPID_Request r, *rreq;      /* don't need to init request, output only */
    msg_tagged_t msg;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_DO_PROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_DO_PROBE);

    if (unlikely(source == MPI_PROC_NULL)) {
        MPIR_Status_set_procnull(status);
        *flag = true;
        if (message)
            *message = NULL;
        goto fn_exit;
    }
    else if (unlikely(MPI_ANY_SOURCE == source))
        remote_proc = FI_ADDR_UNSPEC;
    else
        remote_proc = _comm_to_phys(comm, source, MPIDI_API_TAG);

    if (message)
        REQ_CREATE(rreq);
    else
        rreq = &r;

    match_bits = init_recvtag(&mask_bits, comm->context_id + context_offset, source, tag);

    REQ_OFI(rreq, event_id) = MPIDI_EVENT_PEEK;
    REQ_OFI(rreq, util_id)  = MPIDI_PEEK_START;

    msg.msg_iov = NULL;
    msg.desc = NULL;
    msg.iov_count = 0;
    msg.addr = remote_proc;
    msg.tag = match_bits;
    msg.ignore = mask_bits;
    msg.context = (void *) &(REQ_OFI(rreq, context));
    msg.data = 0;

    FI_RC(fi_trecvmsg(G_RXC_TAG(0), &msg, peek_flags | FI_PEEK | FI_COMPLETION), trecv);
    MPIDI_NM_PROGRESS_WHILE(REQ_OFI(rreq, util_id) == MPIDI_PEEK_START);

    switch (REQ_OFI(rreq, util_id)) {
    case  MPIDI_PEEK_NOT_FOUND:
        *flag = 0;
        if (message) MPIU_Handle_obj_free(&MPIDI_Request_mem, rreq);
        goto fn_exit;
        break;
    case  MPIDI_PEEK_FOUND:
        MPIR_Request_extract_status(rreq, status);
        *flag = 1;
        if (message) *message = rreq;
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
#define FUNCNAME MPIDI_netmod_probe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_probe(int source,
                                     int tag,
                                     MPID_Comm * comm, int context_offset, MPI_Status * status)
{
    int mpi_errno = MPI_SUCCESS, flag = 0;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_PROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_PROBE);
    while (!flag) {
        mpi_errno = MPIDI_Iprobe(source, tag, comm, context_offset, &flag, status);
        if (mpi_errno)
            MPIR_ERR_POP(mpi_errno);
        MPIDI_NM_PROGRESS();
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_PROBE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_improbe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_improbe(int source,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset,
                                       int *flag, MPID_Request ** message, MPI_Status * status)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_IMPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_IMPROBE);
    /* Set flags for mprobe peek, when ready */
    int mpi_errno = do_iprobe(source, tag, comm, context_offset,
                              flag, status, message, FI_CLAIM | FI_COMPLETION);

    if (*flag && *message) {
        (*message)->kind = MPID_REQUEST_MPROBE;
        (*message)->comm = comm;
        MPIU_Object_add_ref(comm);
    }
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_IMPROBE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_iprobe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_iprobe(int source,
                                      int tag,
                                      MPID_Comm * comm,
                                      int context_offset, int *flag, MPI_Status * status)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NETMOD_IPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NETMOD_IPROBE);
    mpi_errno = do_iprobe(source, tag, comm, context_offset, flag, status, NULL, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NETMOD_IPROBE);
    return mpi_errno;
}

#endif /* NETMOD_OFI_PROBE_H_INCLUDED */
