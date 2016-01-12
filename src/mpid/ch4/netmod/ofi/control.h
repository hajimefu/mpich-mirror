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
#ifndef NETMOD_AM_OFI_CONTROL_H_INCLUDED
#define NETMOD_AM_OFI_CONTROL_H_INCLUDED

#include "am_impl.h"

#undef FUNCNAME
#define FUNCNAME do_control_win
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_control_win(MPIDI_Win_control_t *control,
                                 int                  rank,
                                 MPID_Win            *win,
                                 int                  use_comm,
                                 int                  use_lock)
{
    int                        mpi_errno = MPI_SUCCESS;
    MPIDI_AM_OFI_reply_token_t reply_token;
    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_DO_CONTROL_WIN);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_DO_CONTROL_WIN);

    control->win_id      = WIN_OFI(win)->win_id;
    control->origin_rank = win->comm_ptr->rank;

    if(use_lock)
        mpi_errno = MPIDI_netmod_do_inject(rank,
                                           win->comm_ptr,
                                           reply_token,
                                           MPIDI_INTERNAL_HANDLER_CONTROL,
                                           (void*)control,
                                           sizeof(*control),NULL,
                                           FALSE,
                                           use_comm);
    else
        mpi_errno = MPIDI_netmod_do_inject(rank,
                                           win->comm_ptr,
                                           reply_token,
                                           MPIDI_INTERNAL_HANDLER_CONTROL,
                                           (void*)control,
                                           sizeof(*control),NULL,
                                           FALSE,
                                           use_comm);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_DO_CONTROL_WIN);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME do_control_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_control_send(MPIDI_Send_control_t *control,
                                  char                 *send_buf,
                                  size_t                msgsize,
                                  int                   rank,
                                  MPID_Comm            *comm_ptr,
                                  MPID_Request         *ackreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_AM_OFI_reply_token_t reply_token;
    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_DO_CONTROL_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_DO_CONTROL_SEND);

    control->origin_rank = comm_ptr->rank;
    control->send_buf    = send_buf;
    control->msgsize     = msgsize;
    control->comm_id     = comm_ptr->context_id;
    control->endpoint_id = COMM_TO_EP(comm_ptr, comm_ptr->rank);
    control->ackreq      = ackreq;
    MPIU_Assert(sizeof(*control) <= MPIDI_Global.max_buffered_send);

    mpi_errno = MPIDI_netmod_do_inject(rank,comm_ptr,
                                       reply_token,
                                       MPIDI_INTERNAL_HANDLER_CONTROL,
                                       (void*)control,
                                       sizeof(*control),NULL,
                                       FALSE,TRUE);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_DO_CONTROL_SEND);
    return mpi_errno;
}


#endif /* NETMOD_AM_OFI_CONTROL_H_INCLUDED */
