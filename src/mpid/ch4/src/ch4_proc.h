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
#ifndef MPIDCH4_PROC_H_INCLUDED
#define MPIDCH4_PROC_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_rank_is_local
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4_rank_is_local(int rank, MPID_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_STATE_CH4_RANK_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_STATE_CH4_RANK_IS_LOCAL);

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    /* Ask the netmod for locality information. If it decided not to build it,
     * it will call back up to the CH4U function to get the infomration. */
    ret = MPIDI_netmod_rank_is_local(rank, comm);
#else
    ret = MPIDI_CH4U_rank_is_local(rank, comm);
#endif

    MPIDI_FUNC_EXIT(MPIDI_STATE_CH4_RANK_IS_LOCAL);
    return ret;
}
#endif