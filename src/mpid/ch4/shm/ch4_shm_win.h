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
#ifndef SHM_WIN_H_INCLUDED
#define SHM_WIN_H_INCLUDED

#include "ch4_shm_impl.h"

static inline int MPIDI_shm_win_set_info(MPID_Win * win, MPID_Info * info)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_shm_win_start(MPID_Group * group, int assert, MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_shm_win_complete(MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_post(MPID_Group * group, int assert, MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_shm_win_wait(MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_shm_win_test(MPID_Win * win, int *flag)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_lock(int lock_type, int rank, int assert, MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_shm_win_unlock(int rank, MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_get_info(MPID_Win * win, MPID_Info ** info_p_p)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_shm_win_free(MPID_Win ** win_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_fence(int assert, MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_create(void *base,
                                          MPI_Aint length,
                                          int disp_unit,
                                          MPID_Info * info,
                                          MPID_Comm * comm_ptr, MPID_Win ** win_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_attach(MPID_Win * win, void *base, MPI_Aint size)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_allocate_shared(MPI_Aint size,
                                                   int disp_unit,
                                                   MPID_Info * info_ptr,
                                                   MPID_Comm * comm_ptr,
                                                   void **base_ptr, MPID_Win ** win_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_detach(MPID_Win * win, const void *base)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_shared_query(MPID_Win * win,
                                                int rank,
                                                MPI_Aint * size, int *disp_unit, void *baseptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_allocate(MPI_Aint size,
                                            int disp_unit,
                                            MPID_Info * info,
                                            MPID_Comm * comm, void *baseptr, MPID_Win ** win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_flush(int rank, MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_flush_local_all(MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_unlock_all(MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_create_dynamic(MPID_Info * info,
                                                  MPID_Comm * comm, MPID_Win ** win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_flush_local(int rank, MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_sync(MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_flush_all(MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_win_lock_all(int assert, MPID_Win * win)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


#endif /* SHM_WIN_H_INCLUDED */
