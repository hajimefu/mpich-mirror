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
#ifndef MPIDCH4U_PROC_H_INCLUDED
#define MPIDCH4U_PROC_H_INCLUDED

#include "ch4_types.h"

static inline int MPIDI_CH4U_rank_is_local(int rank, MPID_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_CH4U_STATE_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_CH4U_STATE_IS_LOCAL);

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    ret = MPIDI_netmod_rank_is_local(rank, comm);
#else
    ret = MPIU_CH4U_COMM(comm,locality)[rank].is_local;
#endif

    MPIDI_FUNC_EXIT(MPIDI_CH4U_STATE_IS_LOCAL);
    return ret;
}


static inline int MPIDI_CH4U_rank_to_lpid(int rank, MPID_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_CH4U_STATE_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_CH4U_STATE_IS_LOCAL);

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    int lpid;
    ret = MPIDI_netmod_comm_get_lpid(comm, rank, &lpid, FALSE);
#else
    ret = MPIU_CH4U_COMM(comm,locality)[rank].index;
#endif

    MPIDI_FUNC_EXIT(MPIDI_CH4U_STATE_IS_LOCAL);
    return ret;
}


#endif /*MPIDCH4U_PROC_H_INCLUDED */
