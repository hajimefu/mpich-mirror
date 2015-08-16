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
#ifndef NETMOD_AM_OFI_PROC_H_INCLUDED
#define NETMOD_AM_OFI_PROC_H_INCLUDED

#include "impl.h"

static inline int MPIDI_netmod_rank_is_local(int rank, MPID_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_NETMOD_RANK_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_NETMOD_RANK_IS_LOCAL);

    ret = MPIDI_CH4U_rank_is_local(rank, comm);

    MPIDI_FUNC_EXIT(MPIDI_NETMOD_RANK_IS_LOCAL);
    return ret;
}
#endif /*NETMOD_AM_OFI_PROC_H_INCLUDED */
