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
#ifndef CH4_WIN_H_INCLUDED
#define CH4_WIN_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_set_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_set_info(MPID_Win * win, MPID_Info * info)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_SET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_SET_INFO);
    mpi_errno = MPIDI_CH4_NM_win_set_info(win, info);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_SET_INFO);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_start
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_start(MPID_Group * group, int assert, MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_START);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_START);
    mpi_errno = MPIDI_CH4_NM_win_start(group, assert, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_START);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_complete(MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_COMPLETE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_COMPLETE);
    mpi_errno = MPIDI_CH4_NM_win_complete(win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_COMPLETE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_post
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_post(MPID_Group * group, int assert, MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_POST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_POST);
    mpi_errno = MPIDI_CH4_NM_win_post(group, assert, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_POST);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_wait
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_wait(MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_WAIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_WAIT);
    mpi_errno = MPIDI_CH4_NM_win_wait(win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_WAIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Win_test
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_test(MPID_Win * win, int *flag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_TEST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_TEST);
    mpi_errno = MPIDI_CH4_NM_win_test(win, flag);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_TEST);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_lock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_lock(int lock_type, int rank, int assert, MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_LOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_LOCK);
    mpi_errno = MPIDI_CH4_NM_win_lock(lock_type, rank, assert, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_LOCK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_unlock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_unlock(int rank, MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_UNLOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_UNLOCK);
    mpi_errno = MPIDI_CH4_NM_win_unlock(rank, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_UNLOCK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_get_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_get_info(MPID_Win * win, MPID_Info ** info_p_p)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_GET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_GET_INFO);
    mpi_errno = MPIDI_CH4_NM_win_get_info(win, info_p_p);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_GET_INFO);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_free
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_free(MPID_Win ** win_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_FREE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_FREE);
    mpi_errno = MPIDI_CH4_NM_win_free(win_ptr);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_FREE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_fence
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_fence(int assert, MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_FENCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_FENCE);
    mpi_errno = MPIDI_CH4_NM_win_fence(assert, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_FENCE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_create(void *base,
                                    MPI_Aint length,
                                    int disp_unit,
                                    MPID_Info * info, MPID_Comm * comm_ptr, MPID_Win ** win_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_CREATE);
    mpi_errno = MPIDI_CH4_NM_win_create(base, length, disp_unit, info, comm_ptr, win_ptr);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_CREATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_attach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_attach(MPID_Win * win, void *base, MPI_Aint size)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_ATTACH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_ATTACH);
    mpi_errno = MPIDI_CH4_NM_win_attach(win, base, size);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_ATTACH);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_allocate_shared
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_allocate_shared(MPI_Aint size,
                                             int disp_unit,
                                             MPID_Info * info_ptr,
                                             MPID_Comm * comm_ptr,
                                             void **base_ptr, MPID_Win ** win_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_ALLOCATE_SHARED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_ALLOCATE_SHARED);
    mpi_errno = MPIDI_CH4_NM_win_allocate_shared(size, disp_unit,
                                                 info_ptr, comm_ptr, base_ptr, win_ptr);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_ALLOCATE_SHARED);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_flush_local
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_flush_local(int rank, MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_FLUSH_LOCAL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_FLUSH_LOCAL);
    mpi_errno = MPIDI_CH4_NM_win_flush_local(rank, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_FLUSH_LOCAL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_detach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_detach(MPID_Win * win, const void *base)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_DETACH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_DETACH);
    mpi_errno = MPIDI_CH4_NM_win_detach(win, base);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_DETACH);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Win_shared_query
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_shared_query(MPID_Win * win,
                                          int rank, MPI_Aint * size, int *disp_unit, void *baseptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_SHARED_QUERY);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_SHARED_QUERY);
    mpi_errno = MPIDI_CH4_NM_win_shared_query(win, rank, size, disp_unit, baseptr);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_SHARED_QUERY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_allocate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_allocate(MPI_Aint size,
                                      int disp_unit,
                                      MPID_Info * info,
                                      MPID_Comm * comm, void *baseptr, MPID_Win ** win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_ALLOCATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_ALLOCATE);
    mpi_errno = MPIDI_CH4_NM_win_allocate(size, disp_unit, info, comm, baseptr, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_ALLOCATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_flush
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_flush(int rank, MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_FLUSH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_FLUSH);
    mpi_errno = MPIDI_CH4_NM_win_flush(rank, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_FLUSH);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_flush_local_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_flush_local_all(MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_FLUSH_LOCAL_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_FLUSH_LOCAL_ALL);
    mpi_errno = MPIDI_CH4_NM_win_flush_local_all(win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_FLUSH_LOCAL_ALL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_unlock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_unlock_all(MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_UNLOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_UNLOCK_ALL);
    mpi_errno = MPIDI_CH4_NM_win_unlock_all(win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_UNLOCK_ALL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_create_dynamic
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_create_dynamic(MPID_Info * info, MPID_Comm * comm, MPID_Win ** win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_CREATE_DYNAMIC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_CREATE_DYNAMIC);
    mpi_errno = MPIDI_CH4_NM_win_create_dynamic(info, comm, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_CREATE_DYNAMIC);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_sync
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_sync(MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_SYNC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_SYNC);
    mpi_errno = MPIDI_CH4_NM_win_sync(win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_SYNC);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_flush_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_flush_all(MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_FLUSH_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_FLUSH_ALL);
    mpi_errno = MPIDI_CH4_NM_win_flush_all(win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_FLUSH_ALL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_lock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Win_lock_all(int assert, MPID_Win * win)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_WIN_LOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_WIN_LOCK_ALL);
    mpi_errno = MPIDI_CH4_NM_win_lock_all(assert, win);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_WIN_LOCK_ALL);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* CH4_WIN_H_INCLUDED */
