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
#ifndef NETMOD_OFI_PROC_H_INCLUDED
#define NETMOD_OFI_PROC_H_INCLUDED

#include "impl.h"

static inline int MPIDI_CH4_NM_rank_is_local(int rank, MPIR_Comm *comm)
{
    int ret;

    MPIDI_STATE_DECL(MPIDI_NETMOD_RANK_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_NETMOD_RANK_IS_LOCAL);

#ifndef MPIDI_BUILD_CH4_LOCALITY_INFO
    if (comm->comm_kind == MPIR_COMM_KIND__INTERCOMM) {
        ret = 0;
        goto fn_exit;
    }
    ret = MPIDI_OFI_AV(MPIDIU_comm_rank_to_av(comm, rank)).is_local;
    MPL_DBG_MSG_FMT(MPIDI_CH4_DBG_MAP, VERBOSE,
            (MPL_DBG_FDEST, " is_local=%d, rank=%d",
             ret, rank));
#else
    ret = MPIDI_CH4U_rank_is_local(rank, comm);
#endif

    MPIDI_FUNC_EXIT(MPIDI_NETMOD_RANK_IS_LOCAL);
    return ret;
}

#endif /*NETMOD_OFI_PROC_H_INCLUDED */
