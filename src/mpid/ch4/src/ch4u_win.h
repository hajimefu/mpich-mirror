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
#ifndef MPIDCH4U_WIN_H_INCLUDED
#define MPIDCH4U_WIN_H_INCLUDED

#include "ch4_impl.h"

#define WIN_TRUE_STR	 	   "true"
#define WIN_FALSE_STR	 	   "False"
#define WIN_NO_LOCKS_STR 	   "no_locks"
#define WIN_ALLOC_SHM 	   	   "alloc_shm"
#define WIN_ALLOC_SHRD_NCONTIG_STR "alloc_shared_noncontig"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_gather_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_gather_info(void *base, MPI_Aint size, int disp_unit,
                                              MPID_Info * info, MPID_Comm * comm_ptr,
                                              MPID_Win ** win_ptr)
{
    int mpi_errno = MPI_SUCCESS, comm_size, rank;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;

    MPIDI_STATE_DECL(MPID_STATE_MPIDI_CH4U_WIN_GATHER_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_CH4U_WIN_GATHER_INFO);

    comm_size = (*win_ptr)->comm_ptr->local_size;
    rank = (*win_ptr)->comm_ptr->rank;

    /* allocate memory for the basic win info of all processes */
    MPIU_CH4U_WIN(*win_ptr, basic_info_table) = (MPIDI_CH4U_win_basic_info_t *)
        MPIU_Malloc(comm_size * sizeof(MPIDI_CH4U_win_basic_info_t));
    MPIU_Assert(MPIU_CH4U_WIN(*win_ptr, basic_info_table) != NULL);

    MPIU_CH4U_WIN(*win_ptr, basic_info_table)[rank].base = (uint64_t) base;
    MPIU_CH4U_WIN(*win_ptr, basic_info_table)[rank].size = size;
    MPIU_CH4U_WIN(*win_ptr, basic_info_table)[rank].disp_unit = disp_unit;
    MPIU_CH4U_WIN(*win_ptr, basic_info_table)[rank].win_ptr = (uint64_t) (*win_ptr);

    mpi_errno = MPIR_Allgather_impl(MPI_IN_PLACE, sizeof(MPIDI_CH4U_win_basic_info_t),
                                    MPI_BYTE, MPIU_CH4U_WIN((*win_ptr), basic_info_table),
                                    sizeof(MPIDI_CH4U_win_basic_info_t), MPI_BYTE,
                                    (*win_ptr)->comm_ptr, &errflag);
    if (mpi_errno) {
        MPIDU_RC_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_RMA_FUNC_EXIT(MPID_STATE_MPIDI_CH4U_WIN_GATHER_INFO);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_set_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_set_info(MPID_Win * win, MPID_Info * info)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_SET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_SET_INFO);

    /* check for info no_locks */
    if (info != NULL) {
        int info_flag = 0;
        char info_value[MPI_MAX_INFO_VAL + 1];
        MPIR_Info_get_impl(info, WIN_NO_LOCKS_STR, MPI_MAX_INFO_VAL, info_value, &info_flag);
        if (info_flag) {
            if (!strncmp(info_value, WIN_TRUE_STR, strlen(WIN_TRUE_STR)))
                MPIU_CH4U_WIN(win, info_args).no_locks = 1;
            if (!strncmp(info_value, WIN_FALSE_STR, strlen(WIN_FALSE_STR)))
                MPIU_CH4U_WIN(win, info_args).no_locks = 0;
        }
    }

    /* check for info alloc_shm */
    if (win->create_flavor == MPI_WIN_FLAVOR_CREATE)
        MPIU_CH4U_WIN(win, info_args).alloc_shm = FALSE;
    if (win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE ||
        win->create_flavor == MPI_WIN_FLAVOR_SHARED)
        MPIU_CH4U_WIN(win, info_args).alloc_shm = TRUE;

    if (info != NULL) {
        int info_flag = 0;
        char info_value[MPI_MAX_INFO_VAL + 1];
        MPIR_Info_get_impl(info, WIN_ALLOC_SHM, MPI_MAX_INFO_VAL, info_value, &info_flag);
        if (info_flag) {
            if (!strncmp(info_value, WIN_TRUE_STR, sizeof(WIN_TRUE_STR)))
                MPIU_CH4U_WIN(win, info_args).alloc_shm = TRUE;
            if (!strncmp(info_value, WIN_FALSE_STR, sizeof(WIN_FALSE_STR)))
                MPIU_CH4U_WIN(win, info_args).alloc_shm = FALSE;
        }
    }

    if (win->create_flavor == MPI_WIN_FLAVOR_DYNAMIC)
        MPIU_CH4U_WIN(win, info_args).alloc_shm = FALSE;

    /* check for info alloc_shared_noncontig */
    if (win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE)
        MPIU_CH4U_WIN(win, info_args).alloc_shared_noncontig = 1;
    if (info != NULL) {
        int info_flag = 0;
        char info_value[MPI_MAX_INFO_VAL + 1];
        MPIR_Info_get_impl(info, WIN_ALLOC_SHRD_NCONTIG_STR, MPI_MAX_INFO_VAL,
                           info_value, &info_flag);
        if (info_flag) {
            if (!strncmp(info_value, WIN_TRUE_STR, strlen(WIN_TRUE_STR)))
                MPIU_CH4U_WIN(win, info_args).alloc_shared_noncontig = 1;
            if (!strncmp(info_value, WIN_FALSE_STR, strlen(WIN_FALSE_STR)))
                MPIU_CH4U_WIN(win, info_args).alloc_shared_noncontig = 0;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_SET_INFO);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_start
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_start(MPID_Group * group, int assert, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_START);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_START);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_START);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_complete(MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_COMPLETE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_COMPLETE);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_COMPLETE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_post
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_post(MPID_Group * group, int assert, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_POST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_POST);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_POST);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_wait
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_wait(MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_WAIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_WAIT);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_WAIT);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_test
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_test(MPID_Win * win, int *flag)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_TEST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_TEST);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_TEST);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_lock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_lock(int lock_type, int rank, int assert, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_LOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_LOCK);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_LOCK);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_unlock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_unlock(int rank, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_UNLOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_UNLOCK);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_UNLOCK);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_get_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_get_info(MPID_Win * win, MPID_Info ** info_p_p)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_GET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_GET_INFO);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_GET_INFO);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_free
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_free(MPID_Win ** win_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_FREE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_FREE);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_FREE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_fence
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_fence(int assert, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_FENCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_FENCE);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_FENCE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_create(void *base,
                                         MPI_Aint length,
                                         int disp_unit,
                                         MPID_Info * info,
                                         MPID_Comm * comm_ptr, MPID_Win ** win_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_CREATE);

    mpi_errno = MPIDI_CH4U_win_gather_info(base, length, disp_unit, info, comm_ptr, win_ptr);
    if (mpi_errno != MPI_SUCCESS)
        MPIDU_RC_POP(mpi_errno);

    if (MPIU_CH4U_WIN(*win_ptr, info_args).alloc_shm == TRUE) {
        /* TODO */
        MPIU_Assert(0);
        goto fn_exit;
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_CREATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_attach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_attach(MPID_Win * win, void *base, MPI_Aint size)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_ATTACH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_ATTACH);

    /* no op, all of memory is exposed */

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_ATTACH);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_allocate_shared
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_allocate_shared(MPI_Aint size,
                                                  int disp_unit,
                                                  MPID_Info * info_ptr,
                                                  MPID_Comm * comm_ptr,
                                                  void **base_ptr, MPID_Win ** win_ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_ALLOCATE_SHARED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_ALLOCATE_SHARED);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_ALLOCATE_SHARED);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_flush_local
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_flush_local(int rank, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_FLUSH_LOCAL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_FLUSH_LOCAL);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_FLUSH_LOCAL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_detach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_detach(MPID_Win * win, const void *base)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_DETACH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_DETACH);

    /* no op, all of memory is exposed */

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_DETACH);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_shared_query
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_shared_query(MPID_Win * win,
                                               int rank,
                                               MPI_Aint * size, int *disp_unit, void *baseptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_SHARED_QUERY);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_SHARED_QUERY);

    *(void **) baseptr = (void *) MPIU_CH4U_WIN(win, basic_info_table)[rank].base;
    *size = MPIU_CH4U_WIN(win, basic_info_table)[rank].size;
    *disp_unit = MPIU_CH4U_WIN(win, basic_info_table)[rank].disp_unit;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_SHARED_QUERY);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_allocate_no_shm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_allocate_no_shm(MPI_Aint size, int disp_unit,
                                                  MPID_Info * info, MPID_Comm * comm_ptr,
                                                  void *baseptr, MPID_Win ** win_ptr)
{
    void **base_pp = (void **) baseptr;
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_MPIDI_CH4U_WIN_ALLOCATE_NO_SHM);
    MPIDI_FUNC_ENTER(MPID_STATE_MPIDI_CH4U_WIN_ALLOCATE_NO_SHM);

    *base_pp = (size > 0) ? MPIU_Malloc(size) : NULL;
    (*win_ptr)->base = *base_pp;

    mpi_errno = MPIDI_CH4U_win_gather_info(*base_pp, size, disp_unit, info, comm_ptr, win_ptr);
    if (mpi_errno != MPI_SUCCESS) {
        MPIDU_RC_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPIDI_CH4U_WIN_ALLOCATE_NO_SHM);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_allocate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_allocate(MPI_Aint size,
                                           int disp_unit,
                                           MPID_Info * info,
                                           MPID_Comm * comm, void *baseptr, MPID_Win ** win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_ALLOCATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_ALLOCATE);

    if (MPIU_CH4U_WIN((*win), info_args).alloc_shm == TRUE) {
        /* TODO */
        MPIU_Assert(0);
        goto fn_exit;
    }
    mpi_errno = MPIDI_CH4U_win_allocate_no_shm(size, disp_unit, info, comm, baseptr, win);
    if (mpi_errno != MPI_SUCCESS)
        MPIDU_RC_POP(mpi_errno);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_ALLOCATE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_flush
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_flush(int rank, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_FLUSH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_FLUSH);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_FLUSH);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_flush_local_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_flush_local_all(MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_FLUSH_LOCAL_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_FLUSH_LOCAL_ALL);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_FLUSH_LOCAL_ALL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_unlock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_unlock_all(MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_UNLOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_UNLOCK_ALL);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_UNLOCK_ALL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_create_dynamic
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_create_dynamic(MPID_Info * info,
                                                 MPID_Comm * comm, MPID_Win ** win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_CREATE_DYNAMIC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_CREATE_DYNAMIC);

    mpi_errno = MPIDI_CH4U_win_gather_info(MPI_BOTTOM, 0, 1, info, comm, win);
    if (mpi_errno != MPI_SUCCESS)
        MPIDU_RC_POP(mpi_errno);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_CREATE_DYNAMIC);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_sync
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_sync(MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_SYNC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_SYNC);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_SYNC);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_flush_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_flush_all(MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_FLUSH_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_FLUSH_ALL);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_FLUSH_ALL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_lock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_win_lock_all(int assert, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_LOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_LOCK_ALL);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_LOCK_ALL);
    return mpi_errno;
}

#endif /* MPIDCH4U_WIN_H_INCLUDED */
