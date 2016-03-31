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
struct _UCX_share {
    int disp;
    MPI_Aint addr;
};

char ucx_dummy_buffer[4096];

static inline int MPIDI_CH4_NMI_UCX_Win_allgather(MPID_Win *win, size_t length,
                                                   uint32_t disp_unit,  void **base_ptr) {

    MPIR_Errflag_t err = MPIR_ERR_NONE;
    int mpi_errno = MPI_SUCCESS;
    ucs_status_t status;
    ucp_mem_h mem_h;
    int cntr = 0;
    size_t rkey_size;
    int *rkey_sizes, *recv_disps, i;
    char *rkey_buffer, *rkey_recv_buff = NULL;
    struct _UCX_share *share_data;
    size_t size = MPL_MAX(length, 4096);
    void *base;

    MPID_Comm *comm_ptr     = win->comm_ptr;

    ucp_context_h ucp_context = MPIDI_CH4_NMI_UCX_global.context;

    MPIDI_CH4_NMI_UCX_WIN(win).info_table =
                MPL_malloc(sizeof(MPIDI_CH4_NMI_UCX_win_info_t) * comm_ptr->local_size);
    if(length == 0)
        base = &ucx_dummy_buffer;
    else
        base = *base_ptr;

    status = ucp_mem_map(MPIDI_CH4_NMI_UCX_global.context, &base, size, 0, &mem_h);
    MPIDI_CH4_NMI_UCX_CHK_STATUS(status, ucp_mem_map);
    if(length > 0)
        *base_ptr = base;

    MPIDI_CH4_NMI_UCX_WIN(win).mem_h = mem_h;

    /* pack the key */
    status = ucp_rkey_pack(ucp_context, mem_h,  (void**)&rkey_buffer, &rkey_size);

    MPIDI_CH4_NMI_UCX_CHK_STATUS(status, ucp_mem_map);

    rkey_sizes = (int *) MPL_malloc(sizeof(int) * comm_ptr->local_size);
    rkey_sizes[comm_ptr->rank] =(int) rkey_size;
    mpi_errno = MPIR_Allgather_impl(MPI_IN_PLACE, 1, MPI_INT,
                                    rkey_sizes, 1, MPI_INT, comm_ptr, &err);

    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    recv_disps = (int *) MPL_malloc(sizeof(int) * comm_ptr->local_size);


    for (i = 0; i < comm_ptr->local_size; i++) {
        recv_disps[i] = cntr;
        cntr += rkey_sizes[i];
    }

    rkey_recv_buff = MPL_malloc(cntr);

    /* allgather */
    mpi_errno = MPIR_Allgatherv_impl(rkey_buffer, rkey_size, MPI_BYTE,
                                     rkey_recv_buff, rkey_sizes, recv_disps, MPI_BYTE,
                                     comm_ptr, &err);

    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

/* If we use the shared memory support in UCX, we have to distinguish between local
    and remote windows (at least now). If win_create is used, the key cannot be unpackt -
    then we need our fallback-solution */

    for (i = 0; i < comm_ptr->local_size; i++){
        status = ucp_ep_rkey_unpack(MPIDI_CH4_NMI_UCX_COMM_TO_EP(comm_ptr, i),
                            &rkey_recv_buff[recv_disps[i]],
                            &(MPIDI_CH4_NMI_UCX_WIN_INFO(win, i).rkey));
#ifdef MPIDI_CH4_NMI_UCX_SHM
        if(status == UCS_ERR_UNREACHABLE) {
            MPIDI_CH4_NMI_UCX_WIN_INFO(win, i).rkey= NULL;
        }
        else
#endif
            MPIDI_CH4_NMI_UCX_CHK_STATUS(status, ucp_mem_map);
     }
    share_data = MPL_malloc(comm_ptr->local_size * sizeof(struct _UCX_share));

    share_data[comm_ptr->rank].disp = disp_unit;
    share_data[comm_ptr->rank].addr = (MPI_Aint) base;

    mpi_errno =
        MPIR_Allgather(MPI_IN_PLACE, sizeof(struct _UCX_share), MPI_BYTE, share_data,
                       sizeof(struct _UCX_share), MPI_BYTE, comm_ptr, &err);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    for (i = 0; i < comm_ptr->local_size; i++) {
        MPIDI_CH4_NMI_UCX_WIN_INFO(win, i).disp = share_data[i].disp;
        MPIDI_CH4_NMI_UCX_WIN_INFO(win, i).addr = share_data[i].addr;
    }

  fn_exit:
    /* buffer release */
    if (rkey_buffer)
        ucp_rkey_buffer_release(rkey_buffer);
    /* free temps */
    MPL_free(share_data);
    MPL_free(rkey_sizes);
    MPL_free(recv_disps);
    MPL_free(rkey_recv_buff);
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_UCX_Win_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_UCX_Win_init(MPI_Aint     length,
                                             int          disp_unit,
                                             MPID_Win   **win_ptr,
                                             MPID_Info   *info,
                                             MPID_Comm   *comm_ptr,
                                             int          create_flavor,
                                             int          model)
{
    int       mpi_errno = MPI_SUCCESS;
    MPID_Win *win;
    MPIDI_STATE_DECL(MPID_STATE_CH4_UCX_WIN_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_UCX_WIN_INIT);

    mpi_errno = MPIDI_CH4R_win_init(length, disp_unit, &win, info, comm_ptr, create_flavor, model);
    MPIR_ERR_CHKANDSTMT(mpi_errno != MPI_SUCCESS,
                        mpi_errno,
                        MPI_ERR_NO_MEM,
                        goto fn_fail,
                        "**nomem");
    *win_ptr = win;

    memset(&MPIDI_CH4_NMI_UCX_WIN(win), 0, sizeof(MPIDI_CH4_NMI_UCX_win_t));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_UCX_PROGRESS_WIN_INIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;

}

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

    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    MPID_Win      *win       = *win_ptr;
    MPIDI_CH4U_EPOCH_FREE_CHECK(win,mpi_errno,return mpi_errno);
    mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);
    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    ucp_mem_unmap(MPIDI_CH4_NMI_UCX_global.context, MPIDI_CH4_NMI_UCX_WIN(win).mem_h);
    MPL_free(MPIDI_CH4_NMI_UCX_WIN(win).info_table);
    if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE)
        win->base = NULL;
    MPIDI_CH4R_win_finalize(win_ptr);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FREE);
    return mpi_errno;
fn_fail:
    goto fn_exit;

}

static inline int MPIDI_CH4_NM_win_fence(int assert, MPID_Win * win)
{
    int mpi_errno;
    ucs_status_t ucp_status;
    /*keep this for now to fence all none-natice operations */
    mpi_errno =  MPIDI_CH4R_win_fence(assert, win);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

/* make sure all local and remote operations are completed */
    ucp_status = ucp_worker_flush(MPIDI_CH4_NMI_UCX_global.worker);

    MPIDI_CH4_NMI_UCX_CHK_STATUS(ucp_status, ucp_worker_fence);
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NM_win_create(void *base,
                                          MPI_Aint length,
                                          int disp_unit,
                                          MPID_Info * info,
                                          MPID_Comm * comm_ptr, MPID_Win ** win_ptr)
{

    int             mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t  errflag   = MPIR_ERR_NONE;
    MPID_Win       *win;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_WIN_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_WIN_CREATE);

    MPIDI_CH4_NMI_UCX_Win_init(length, disp_unit, win_ptr, info,
                               comm_ptr,MPI_WIN_FLAVOR_CREATE,
                               MPI_WIN_UNIFIED);

    win              = *win_ptr;

    mpi_errno = MPIDI_CH4_NMI_UCX_Win_allgather(win, length, disp_unit,  &base);
    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    win->base        = base;



    mpi_errno = MPIR_Barrier_impl(comm_ptr,&errflag);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_WIN_CREATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;


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

static inline int MPIDI_CH4_NM_win_allocate(MPI_Aint length,
                                            int disp_unit,
                                            MPID_Info * info,
                                            MPID_Comm * comm_ptr, void *baseptr, MPID_Win ** win_ptr)
{

    int             mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t  errflag   = MPIR_ERR_NONE;
    MPID_Win       *win;
    void *base = NULL;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_UCX_WIN_ALLOCATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_UCX_WIN_WIN_ALLOCATE);

    MPIDI_CH4_NMI_UCX_Win_init(length, disp_unit, win_ptr, info,
                               comm_ptr,MPI_WIN_FLAVOR_ALLOCATE,
                               MPI_WIN_UNIFIED);
    win              = *win_ptr;
    mpi_errno = MPIDI_CH4_NMI_UCX_Win_allgather(win, length, disp_unit,  &base);
    if(mpi_errno != MPI_SUCCESS) goto fn_fail;
    win->base        = base;


    *(void **) baseptr = (void *)base;


    mpi_errno = MPIR_Barrier_impl(comm_ptr,&errflag);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;


fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_UCX_WIN_ALLOCATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;


}

static inline int MPIDI_CH4_NM_win_flush(int rank, MPID_Win * win)
{

    int mpi_errno;
    ucs_status_t ucp_status;

    ucp_ep_h ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(win->comm_ptr, rank);

    mpi_errno =  MPIDI_CH4R_win_flush(rank, win);
    if (mpi_errno)
       MPIR_ERR_POP(mpi_errno);
/* only flush the endpoint */
    ucp_status = ucp_ep_flush(ep);

    MPIDI_CH4_NMI_UCX_CHK_STATUS(ucp_status, ucp_worker_fence);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;

}

static inline int MPIDI_CH4_NM_win_flush_local_all(MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    ucs_status_t ucp_status;
    mpi_errno =  MPIDI_CH4R_win_flush_local_all(win);

   if (mpi_errno)
       MPIR_ERR_POP(mpi_errno);
    /* currently, UCP does not support local flush, so we have to call
       a global flush. This is not good for performance - but OK for now*/
    ucp_status = ucp_worker_flush(MPIDI_CH4_NMI_UCX_global.worker);

    MPIDI_CH4_NMI_UCX_CHK_STATUS(ucp_status, ucp_worker_fence);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
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
    int mpi_errno = MPI_SUCCESS;
    ucs_status_t ucp_status;
    mpi_errno = MPIDI_CH4R_win_flush_local(rank, win);

    ucp_ep_h ep = MPIDI_CH4_NMI_UCX_COMM_TO_EP(win->comm_ptr, rank);
   if (mpi_errno)
       MPIR_ERR_POP(mpi_errno);
    /* currently, UCP does not support local flush, so we have to call
       a global flush. This is not good for performance - but OK for now*/ 
    ucp_status = ucp_ep_flush(ep);

  MPIDI_CH4_NMI_UCX_CHK_STATUS(ucp_status, ucp_worker_fence);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;

}

static inline int MPIDI_CH4_NM_win_sync(MPID_Win * win)
{
    return MPIDI_CH4R_win_sync(win);
}

static inline int MPIDI_CH4_NM_win_flush_all(MPID_Win * win)
{

/*maybe we just flush all eps here? More efficient for smaller communicators...*/
    int mpi_errno = MPI_SUCCESS;
    ucs_status_t ucp_status;
    mpi_errno =   MPIDI_CH4R_win_flush_all(win);
    if (mpi_errno)
       MPIR_ERR_POP(mpi_errno);

    ucp_status = ucp_worker_flush(MPIDI_CH4_NMI_UCX_global.worker);

    MPIDI_CH4_NMI_UCX_CHK_STATUS(ucp_status, ucp_worker_fence);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;

}

static inline int MPIDI_CH4_NM_win_lock_all(int assert, MPID_Win * win)
{
    return MPIDI_CH4R_win_lock_all(assert, win);
}


#endif /* WIN_H_INCLUDED */
