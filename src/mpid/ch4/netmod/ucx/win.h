/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef WIN_H_INCLUDED
#define WIN_H_INCLUDED

#include "impl.h"

static inline int MPIDI_CH4_NM_win_set_info(MPID_Win * win, MPID_Info * info)
{
    return MPIDI_CH4R_win_set_info(win, info);
}


static inline int MPIDI_CH4_NM_win_start(MPID_Group * group, int assert, MPID_Win * win)
{
    return MPIDI_CH4R_win_start(group, assert, win);
}


static inline int MPIDI_CH4_NM_win_complete(MPID_Win * win)
{
    return MPIDI_CH4R_win_complete(win);
}

static inline int MPIDI_CH4_NM_win_post(MPID_Group * group, int assert, MPID_Win * win)
{
    return MPIDI_CH4R_win_post(group, assert, win);
}


static inline int MPIDI_CH4_NM_win_wait(MPID_Win * win)
{
    return MPIDI_CH4R_win_wait(win);
}


static inline int MPIDI_CH4_NM_win_test(MPID_Win * win, int *flag)
{
    return MPIDI_CH4R_win_test(win, flag);
}

static inline int MPIDI_CH4_NM_win_lock(int lock_type, int rank, int assert, MPID_Win * win)
{
    return MPIDI_CH4R_win_lock(lock_type, rank, assert, win);
}


static inline int MPIDI_CH4_NM_win_unlock(int rank, MPID_Win * win)
{
    return MPIDI_CH4R_win_unlock(rank, win);
}

static inline int MPIDI_CH4_NM_win_get_info(MPID_Win * win, MPID_Info ** info_p_p)
{
    return MPIDI_CH4R_win_get_info(win, info_p_p);
}


static inline int MPIDI_CH4_NM_win_free(MPID_Win ** win_ptr)
{
    return MPIDI_CH4R_win_free(win_ptr);
}

static inline int MPIDI_CH4_NM_win_fence(int assert, MPID_Win * win)
{
    return MPIDI_CH4R_win_fence(assert, win);
}

static inline int MPIDI_CH4_NM_win_create(void *base,
                                          MPI_Aint length,
                                          int disp_unit,
                                          MPID_Info * info,
                                          MPID_Comm * comm_ptr, MPID_Win ** win_ptr)
{
    return MPIDI_CH4R_win_create(base, length, disp_unit, info, comm_ptr, win_ptr);
}

static inline int MPIDI_CH4_NM_win_attach(MPID_Win * win, void *base, MPI_Aint size)
{
    return MPIDI_CH4R_win_attach(win, base, size);
}

static inline int MPIDI_CH4_NM_win_allocate_shared(MPI_Aint size,
                                                   int disp_unit,
                                                   MPID_Info * info_ptr,
                                                   MPID_Comm * comm_ptr,
                                                   void **base_ptr, MPID_Win ** win_ptr)
{
    return MPIDI_CH4R_win_allocate_shared(size, disp_unit, info_ptr, comm_ptr, base_ptr, win_ptr);
}

static inline int MPIDI_CH4_NM_win_detach(MPID_Win * win, const void *base)
{
    return MPIDI_CH4R_win_detach(win, base);
}

static inline int MPIDI_CH4_NM_win_shared_query(MPID_Win * win,
                                                int rank,
                                                MPI_Aint * size, int *disp_unit, void *baseptr)
{
    return MPIDI_CH4R_win_shared_query(win, rank, size, disp_unit, baseptr);
}

static inline int MPIDI_CH4_NM_win_allocate(MPI_Aint size,
                                            int disp_unit,
                                            MPID_Info * info,
                                            MPID_Comm * comm, void *baseptr, MPID_Win ** win)
{
    return MPIDI_CH4R_win_allocate(size, disp_unit, info, comm, baseptr, win);
}

static inline int MPIDI_CH4_NM_win_flush(int rank, MPID_Win * win)
{
    return MPIDI_CH4R_win_flush(rank, win);
}

static inline int MPIDI_CH4_NM_win_flush_local_all(MPID_Win * win)
{
    return MPIDI_CH4R_win_flush_local_all(win);
}

static inline int MPIDI_CH4_NM_win_unlock_all(MPID_Win * win)
{
    return MPIDI_CH4R_win_unlock_all(win);
}

static inline int MPIDI_CH4_NM_win_create_dynamic(MPID_Info * info,
                                                  MPID_Comm * comm, MPID_Win ** win)
{
    return MPIDI_CH4R_win_create_dynamic(info, comm, win);
}

static inline int MPIDI_CH4_NM_win_flush_local(int rank, MPID_Win * win)
{
    return MPIDI_CH4R_win_flush_local(rank, win);
}

static inline int MPIDI_CH4_NM_win_sync(MPID_Win * win)
{
    return MPIDI_CH4R_win_sync(win);
}

static inline int MPIDI_CH4_NM_win_flush_all(MPID_Win * win)
{
    return MPIDI_CH4R_win_flush_all(win);
}

static inline int MPIDI_CH4_NM_win_lock_all(int assert, MPID_Win * win)
{
    return MPIDI_CH4R_win_lock_all(assert, win);
}


#endif /* WIN_H_INCLUDED */
