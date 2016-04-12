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
#ifndef CH4_SHM_STUB_PROGRESS_H_INCLUDED
#define CH4_SHM_STUB_PROGRESS_H_INCLUDED

#include "ch4_shm_impl.h"

static inline int MPIDI_CH4_SHM_do_progress_recv(int blocking, int *completion_count)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_do_progress_send(int blocking, int *completion_count)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress(int blocking)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_test(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_poke(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline void MPIDI_CH4_SHM_progress_start(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return;
}

static inline void MPIDI_CH4_SHM_progress_end(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return;
}

static inline int MPIDI_CH4_SHM_progress_wait(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_register(int (*progress_fn) (int *))
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_deregister(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_activate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_progress_deactivate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_PROGRESS_H_INCLUDED */
