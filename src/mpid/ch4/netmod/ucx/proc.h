/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef NETMOD_UCX_PROC_H_INCLUDED
#define NETMOD_UCX_PROC_H_INCLUDED

#include "impl.h"

static inline int MPIDI_CH4_NM_rank_is_local(int rank, MPIR_Comm * comm)
{
    int ret;
    MPIDI_STATE_DECL(MPIDI_NETMOD_RANK_IS_LOCAL);
    MPIDI_FUNC_ENTER(MPIDI_NETMOD_RANK_IS_LOCAL);

    ret = MPIDI_CH4U_rank_is_local(rank, comm);

    MPIDI_FUNC_EXIT(MPIDI_NETMOD_RANK_IS_LOCAL);
    return ret;
}

#endif /* NETMOD_UCX_PROC_H_INCLUDED */
