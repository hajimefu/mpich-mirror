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
#include "ch4u_symheap.h"
#include "ch4u_util.h"
#include <opa_primitives.h>
#include "mpiinfo.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_win_allgather(MPID_Win  *win)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPID_Comm *comm_ptr = win->comm_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_ALLGATHER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_ALLGATHER);

    mpi_errno = MPIR_Allgather_impl(MPI_IN_PLACE,
                                    0,
                                    MPI_DATATYPE_NULL,
                                    MPIU_CH4U_WIN(win, info_table),
                                    sizeof(MPIDI_CH4I_win_info_t),
                                    MPI_BYTE,
                                    comm_ptr,
                                    &errflag);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_ALLGATHER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_set_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_set_info(MPID_Win *win, MPID_Info *info)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_CH4I_WIN_SET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_CH4I_WIN_SET_INFO);

    MPID_Info *curr_ptr;
    char      *value, *token, *savePtr;
    uint       save_ordering;

    curr_ptr = info->next;

    while(curr_ptr) {
        if(!strcmp(curr_ptr->key,"no_locks")) {
            if(!strcmp(curr_ptr->value,"true"))
                MPIU_CH4U_WIN(win, info_args).no_locks = 1;
            else
                MPIU_CH4U_WIN(win, info_args).no_locks = 0;
        } else if(!strcmp(curr_ptr->key,"accumulate_ordering")) {
            save_ordering = (uint) MPIU_CH4U_WIN(win, info_args).accumulate_ordering;
            MPIU_CH4U_WIN(win, info_args).accumulate_ordering = 0;
            value = curr_ptr->value;
            token = (char *) strtok_r(value,"," , &savePtr);

            while(token) {
                if(!memcmp(token,"rar",3))
                    MPIU_CH4U_WIN(win, info_args).accumulate_ordering =
                        (MPIU_CH4U_WIN(win, info_args).accumulate_ordering | MPIDI_CH4I_ACCU_ORDER_RAR);
                else if(!memcmp(token,"raw",3))
                    MPIU_CH4U_WIN(win, info_args).accumulate_ordering =
                        (MPIU_CH4U_WIN(win, info_args).accumulate_ordering | MPIDI_CH4I_ACCU_ORDER_RAW);
                else if(!memcmp(token,"war",3))
                    MPIU_CH4U_WIN(win, info_args).accumulate_ordering =
                        (MPIU_CH4U_WIN(win, info_args).accumulate_ordering | MPIDI_CH4I_ACCU_ORDER_WAR);
                else if(!memcmp(token,"waw",3))
                    MPIU_CH4U_WIN(win, info_args).accumulate_ordering =
                        (MPIU_CH4U_WIN(win, info_args).accumulate_ordering | MPIDI_CH4I_ACCU_ORDER_WAW);
                else
                    MPIU_Assert(0);

                token = (char *) strtok_r(NULL,"," , &savePtr);
            }

            if(MPIU_CH4U_WIN(win, info_args).accumulate_ordering == 0)
                MPIU_CH4U_WIN(win, info_args).accumulate_ordering = save_ordering;
        } else if(!strcmp(curr_ptr->key,"accumulate_ops")) {
            /* the default setting is MPIDI_ACCU_SAME_OP_NO_OP */
            if(!strcmp(curr_ptr->value,"same_op"))
                MPIU_CH4U_WIN(win, info_args).accumulate_ops = MPIDI_CH4I_ACCU_SAME_OP;
        }

        curr_ptr = curr_ptr->next;
    }

    mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_SET_INFO);
    return mpi_errno;
}

static inline int MPIDI_CH4I_win_init(MPI_Aint     length,
                                 int          disp_unit,
                                 MPID_Win   **win_ptr,
                                 MPID_Info   *info,
                                 MPID_Comm   *comm_ptr,
                                 int          create_flavor,
                                 int          model)
{
    int             mpi_errno = MPI_SUCCESS;
    int             rank, size;

    MPID_Win *win = (MPID_Win *)MPIU_Handle_obj_alloc(&MPID_Win_mem);
    MPIR_ERR_CHKANDSTMT(win == NULL,
                        mpi_errno,
                        MPI_ERR_NO_MEM,
                        goto fn_fail,
                        "**nomem");
    *win_ptr = win;

    memset(&win->dev.ch4u, 0, sizeof(MPIDI_CH4I_win_t));
    win->comm_ptr = comm_ptr;
    size          = comm_ptr->local_size;
    rank          = comm_ptr->rank;
    MPIR_Comm_add_ref(comm_ptr);

    MPIU_CH4U_WIN(win, info_table) = (MPIDI_CH4I_win_info_t *)
        MPIU_Calloc(size, sizeof(MPIDI_CH4I_win_info_t));
    MPIR_ERR_CHKANDSTMT(MPIU_CH4U_WIN(win, info_table) == NULL,mpi_errno,MPI_ERR_NO_MEM,
                        goto fn_fail,"**nomem");
    win->errhandler          = NULL;
    win->base                = NULL;
    win->size                = length;
    win->disp_unit           = disp_unit;
    win->create_flavor       = (MPIR_Win_flavor_t)create_flavor;
    win->model               = (MPIR_Win_model_t)model;
    win->copyCreateFlavor    = (MPIR_Win_flavor_t)0;
    win->copyModel           = (MPIR_Win_model_t)0;
    win->attributes          = NULL;
    win->comm_ptr            = comm_ptr;
    win->copyDispUnit        = 0;
    win->copySize            = 0;

    if((info != NULL) && ((int *)info != (int *) MPI_INFO_NULL)) {
        mpi_errno = MPIDI_CH4U_win_set_info(win, info);
        MPIU_Assert(mpi_errno == 0);
    }

    /* Initialize the info (hint) flags per window */
    MPIU_CH4U_WIN(win, info_args).no_locks = 0;
    MPIU_CH4U_WIN(win, info_args).accumulate_ordering = (MPIDI_CH4I_ACCU_ORDER_RAR |
                                                         MPIDI_CH4I_ACCU_ORDER_RAW |
                                                         MPIDI_CH4I_ACCU_ORDER_WAR |
                                                         MPIDI_CH4I_ACCU_ORDER_WAW);
    MPIU_CH4U_WIN(win, info_args).accumulate_ops         = MPIDI_CH4I_ACCU_SAME_OP_NO_OP;
    MPIU_CH4U_WIN(win, info_args).same_size              = 0;
    MPIU_CH4U_WIN(win, info_args).alloc_shared_noncontig = 0;
    MPIU_CH4U_WIN(win, mmap_sz)                          = 0;
    MPIU_CH4U_WIN(win, mmap_addr)                        = NULL;

    MPIDI_CH4I_win_info_t *winfo;
    winfo            = (MPIDI_CH4I_win_info_t *)MPIU_CH4U_WINFO(win, rank);
    winfo->disp_unit = disp_unit;

    /* context id lower bits, window instance upper bits */
    MPIU_CH4U_WIN(win, win_id) = 1 + (((uint64_t)comm_ptr->context_id) |
                                      ((uint64_t)((MPIU_CH4U_COMM(comm_ptr, window_instance))++)<<32));
    MPIDI_CH4I_map_set(MPIDI_CH4_Global.win_map, MPIU_CH4U_WIN(win, win_id), win);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_fill_ranks_in_win_grp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_fill_ranks_in_win_grp(MPID_Win * win_ptr, MPID_Group * group_ptr,
                                        int *ranks_in_win_grp)
{
    int mpi_errno = MPI_SUCCESS;
    int i, *ranks_in_grp;
    MPID_Group *win_grp_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_FILL_RANKS_IN_WIN_GRP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_FILL_RANKS_IN_WIN_GRP);

    ranks_in_grp = (int *) MPIU_Malloc(group_ptr->size * sizeof(int));
    MPIU_Assert(ranks_in_grp);
    for (i = 0; i < group_ptr->size; i++)
        ranks_in_grp[i] = i;

    mpi_errno = MPIR_Comm_group_impl(win_ptr->comm_ptr, &win_grp_ptr);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIR_Group_translate_ranks_impl(group_ptr, group_ptr->size,
                                                ranks_in_grp, win_grp_ptr, ranks_in_win_grp);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIR_Group_free_impl(win_grp_ptr);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

    MPIU_Free(ranks_in_grp);

  fn_exit:
    MPIDI_RMA_FUNC_EXIT(MPID_STATE_CH4I_FILL_RANKS_IN_WIN_GRP);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_progress_win_fence
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_progress_win_fence(MPID_Win *win)
{
    int      mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_CH4_PROGRESS_WIN_FENCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_PROGRESS_WIN_FENCE);

    do {
        MPIDI_CH4I_PROGRESS();
    } while(MPIU_CH4U_WIN(win, outstanding_ops) != 0);
        
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_PROGRESS_WIN_FENCE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_start
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_start(MPID_Group *group, int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_START);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_START);

    MPIDI_CH4I_EPOCH_CHECK2();

    MPIR_Group_add_ref(group);

    MPIDI_CH4I_PROGRESS_WHILE(group->size != (int)MPIU_CH4U_WIN(win, sync).pw.count);
    MPIU_CH4U_WIN(win, sync).pw.count = 0;

    MPIR_ERR_CHKANDJUMP((MPIU_CH4U_WIN(win, sync).sc.group != NULL),
                        mpi_errno,
                        MPI_ERR_GROUP,
                        "**group");
    MPIU_CH4U_WIN(win, sync).sc.group          = group;
    MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_START;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_START);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_complete(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_win_cntrl_msg_t msg;
    int index, peer;
    MPID_Group *group;
    int *ranks_in_win_grp;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_COMPLETE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_COMPLETE);

    mpi_errno = MPIDI_CH4I_progress_win_fence(win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    group = MPIU_CH4U_WIN(win, sync).sc.group;
    MPIU_Assert(group != NULL);

    msg.win_id = MPIU_CH4U_WIN(win, win_id);
    msg.origin_rank = win->comm_ptr->rank;
    msg.type = MPIDI_CH4U_WIN_COMPLETE;

    ranks_in_win_grp = (int *) MPIU_Malloc(sizeof(int) * group->size);
    MPIU_Assert(ranks_in_win_grp);

    mpi_errno = MPIDI_CH4I_fill_ranks_in_win_grp(win, group, ranks_in_win_grp);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

    for(index = 0; index < group->size; ++index) {
        peer = ranks_in_win_grp[index];
        mpi_errno = MPIDI_netmod_inject_am_hdr(peer, win->comm_ptr,
                                               MPIDI_CH4U_AM_WIN_CTRL,
                                               &msg, sizeof (msg), NULL);
        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
    }

    MPIU_Free(ranks_in_win_grp);
    MPIDI_CH4I_EPOCH_TARGET_EVENT();
    MPIR_Group_release(MPIU_CH4U_WIN(win, sync).sc.group);
    MPIU_CH4U_WIN(win, sync).sc.group = NULL;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_COMPLETE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_post
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_post(MPID_Group *group, int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_win_cntrl_msg_t msg;
    int index, peer;
    int *ranks_in_win_grp;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_WIN_POST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_WIN_POST);

    MPIDI_CH4I_EPOCH_POST_CHECK();

    MPIR_Group_add_ref(group);
    MPIR_ERR_CHKANDJUMP((MPIU_CH4U_WIN(win, sync).pw.group != NULL),
                        mpi_errno, MPI_ERR_GROUP,
                        "**group");
    
    MPIU_CH4U_WIN(win, sync).pw.group = group;
    MPIU_Assert(group != NULL);

    msg.win_id = MPIU_CH4U_WIN(win, win_id);
    msg.origin_rank = win->comm_ptr->rank;
    msg.type = MPIDI_CH4U_WIN_POST;

    ranks_in_win_grp = (int *) MPIU_Malloc(sizeof(int) * group->size);
    MPIU_Assert(ranks_in_win_grp);

    mpi_errno = MPIDI_CH4I_fill_ranks_in_win_grp(win, group, ranks_in_win_grp);
    if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);

    for(index=0; index < group->size; ++index) {
        peer = ranks_in_win_grp[index];
        mpi_errno = MPIDI_netmod_inject_am_hdr(peer, win->comm_ptr,
                                               MPIDI_CH4U_AM_WIN_CTRL,
                                               &msg, sizeof (msg), NULL);
        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
    }

    MPIU_Free(ranks_in_win_grp);
    MPIU_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4I_EPOTYPE_POST;
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_WIN_POST);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_wait
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_wait(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Group *group;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_WAIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_WAIT);

    MPIDI_CH4I_EPOCH_TARGET_CHECK(MPIDI_CH4I_EPOTYPE_POST);
    group = MPIU_CH4U_WIN(win, sync).pw.group;
    MPIDI_CH4I_PROGRESS_WHILE(group->size != (int)MPIU_CH4U_WIN(win, sync).sc.count);

    MPIU_CH4U_WIN(win, sync).sc.count = 0;
    MPIU_CH4U_WIN(win, sync).pw.group = NULL;
    MPIR_Group_release(group);
    MPIDI_CH4I_EPOCH_ORIGIN_EVENT();

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_WAIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_test
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_test(MPID_Win *win, int *flag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_TEST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_TEST);

    MPIDI_CH4I_EPOCH_TARGET_CHECK(MPIDI_CH4I_EPOTYPE_POST);

    MPID_Group *group;
    group = MPIU_CH4U_WIN(win, sync).pw.group;

    if(group->size == (int)MPIU_CH4U_WIN(win, sync).sc.count) {
        MPIU_CH4U_WIN(win, sync).sc.count = 0;
        MPIU_CH4U_WIN(win, sync).pw.group = NULL;
        *flag          = 1;
        MPIR_Group_release(group);
        MPIDI_CH4I_EPOCH_ORIGIN_EVENT();
    } else {
	MPIDI_CH4I_PROGRESS();
        *flag=0;
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_TEST);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_lock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_lock(int lock_type, int rank, int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_LOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_LOCK);

    MPIDI_CH4I_win_sync_lock *slock = &MPIU_CH4U_WIN(win, sync).lock;
    MPIDI_CH4I_EPOCH_CHECK2();

    if(rank == MPI_PROC_NULL) goto fn_exit0;

    MPIDI_CH4U_win_cntrl_msg_t msg;
    msg.win_id = MPIU_CH4U_WIN(win, win_id);
    msg.origin_rank = win->comm_ptr->rank;
    msg.type = MPIDI_CH4U_WIN_LOCK;
    msg.lock_type = lock_type;

    mpi_errno = MPIDI_netmod_inject_am_hdr(rank, win->comm_ptr,
                                           MPIDI_CH4U_AM_WIN_CTRL,
                                           &msg, sizeof (msg), NULL);
    if(mpi_errno != MPI_SUCCESS)
        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                            goto fn_fail, "**rmasync");
    MPIDI_CH4I_PROGRESS_WHILE(!slock->remote.locked);

fn_exit0:
    MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_LOCK;
    
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_LOCK);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_unlock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_unlock(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4U_win_cntrl_msg_t msg;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_UNLOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_UNLOCK);

    MPIDI_CH4I_EPOCH_ORIGIN_CHECK(MPIDI_CH4I_EPOTYPE_LOCK);
    if(rank == MPI_PROC_NULL) goto fn_exit0;

    mpi_errno = MPIDI_CH4I_progress_win_fence(win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    msg.win_id = MPIU_CH4U_WIN(win, win_id);
    msg.origin_rank = win->comm_ptr->rank;
    msg.type = MPIDI_CH4U_WIN_UNLOCK;
    mpi_errno = MPIDI_netmod_inject_am_hdr(rank, win->comm_ptr,
                                           MPIDI_CH4U_AM_WIN_CTRL,
                                           &msg, sizeof (msg), NULL);
    if(mpi_errno != MPI_SUCCESS)
        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                            goto fn_fail, "**rmasync");

    MPIDI_CH4I_PROGRESS_WHILE(MPIU_CH4U_WIN(win, sync).lock.remote.locked);
fn_exit0:
    MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_NONE;
    MPIU_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4I_EPOTYPE_NONE;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_UNLOCK);
    return mpi_errno;
fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_get_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_get_info(MPID_Win *win, MPID_Info **info_p_p)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_GET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_GET_INFO);

    mpi_errno = MPIU_Info_alloc(info_p_p);
    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    if(MPIU_CH4U_WIN(win, info_args).no_locks)
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "no_locks", "true");
    else
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "no_locks", "false");

    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    {
#define BUFSIZE 32
        char buf[BUFSIZE];
        int c = 0;

        if(MPIU_CH4U_WIN(win, info_args).accumulate_ordering & MPIDI_CH4I_ACCU_ORDER_RAR)
            c += snprintf(buf+c, BUFSIZE-c, "%srar", (c > 0) ? "," : "");

        if(MPIU_CH4U_WIN(win, info_args).accumulate_ordering & MPIDI_CH4I_ACCU_ORDER_RAW)
            c += snprintf(buf+c, BUFSIZE-c, "%sraw", (c > 0) ? "," : "");

        if(MPIU_CH4U_WIN(win, info_args).accumulate_ordering & MPIDI_CH4I_ACCU_ORDER_WAR)
            c += snprintf(buf+c, BUFSIZE-c, "%swar", (c > 0) ? "," : "");

        if(MPIU_CH4U_WIN(win, info_args).accumulate_ordering & MPIDI_CH4I_ACCU_ORDER_WAW)
            c += snprintf(buf+c, BUFSIZE-c, "%swaw", (c > 0) ? "," : "");

        if(c == 0) {
            memcpy(&buf[0],"not set   ",10);
        }

        MPIR_Info_set_impl(*info_p_p, "accumulate_ordering", buf);
        MPIU_Assert(mpi_errno == MPI_SUCCESS);
#undef BUFSIZE
    }

    if(MPIU_CH4U_WIN(win, info_args).accumulate_ops == MPIDI_CH4I_ACCU_SAME_OP)
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "accumulate_ops", "same_op");
    else
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "accumulate_ops", "same_op_no_op");

    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    if(win->create_flavor == MPI_WIN_FLAVOR_SHARED) {
        if(MPIU_CH4U_WIN(win, info_args).alloc_shared_noncontig)
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "alloc_shared_noncontig", "true");
        else
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "alloc_shared_noncontig", "false");

        MPIU_Assert(mpi_errno == MPI_SUCCESS);
    } else if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE) {
        if(MPIU_CH4U_WIN(win, info_args).same_size)
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "same_size", "true");
        else
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "same_size", "false");

        MPIU_Assert(mpi_errno == MPI_SUCCESS);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_GET_INFO);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_free
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_free(MPID_Win **win_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    MPID_Win      *win       = *win_ptr;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_FREE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_FREE);

    MPIDI_CH4I_EPOCH_FREE_CHECK();
    mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);
    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE  && win->base) {
        if(MPIU_CH4U_WIN(win, mmap_sz) > 0)
            munmap(MPIU_CH4U_WIN(win, mmap_addr), MPIU_CH4U_WIN(win, mmap_sz));
        else if(MPIU_CH4U_WIN(win, mmap_sz) == -1)
            MPIU_Free(win->base);
    }

    if(win->create_flavor == MPI_WIN_FLAVOR_SHARED) {
        if(MPIU_CH4U_WIN(win, mmap_addr))
            munmap(MPIU_CH4U_WIN(win, mmap_addr), MPIU_CH4U_WIN(win, mmap_sz));
        MPIU_Free(MPIU_CH4U_WIN(win, sizes));
    }

    if(MPIU_CH4U_WIN(win, msgQ))
        MPIU_Free(MPIU_CH4U_WIN(win, msgQ));

    MPIU_Free(MPIU_CH4U_WIN(win, info_table));
    MPIR_Comm_release(win->comm_ptr);
    MPIU_Handle_obj_free(&MPID_Win_mem, win);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_FREE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_fence
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_fence(int massert, MPID_Win *win)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_FENCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_FENCE);

    MPIDI_CH4I_EPOCH_FENCE_CHECK();
    mpi_errno = MPIDI_CH4I_progress_win_fence(win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    MPIDI_CH4I_EPOCH_FENCE_EVENT();

    if(!(massert & MPI_MODE_NOPRECEDE))
        mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_FENCE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_create(void *base,
                                        MPI_Aint length,
                                        int disp_unit,
                                        MPID_Info *info,
                                        MPID_Comm *comm_ptr, MPID_Win **win_ptr)
{
    int             mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t  errflag   = MPIR_ERR_NONE;
    MPID_Win       *win;
    int             rank;
    MPIDI_CH4I_win_info_t *winfo;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_CREATE);

    mpi_errno        = MPIDI_CH4I_win_init(length,
                                           disp_unit,
                                           win_ptr,
                                           info,
                                           comm_ptr,
                                           MPI_WIN_FLAVOR_CREATE,
                                           MPI_WIN_UNIFIED);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    win              = *win_ptr;
    win->base   = base;
    rank             = comm_ptr->rank;
    winfo            = (MPIDI_CH4I_win_info_t *)MPIU_CH4U_WINFO(win,rank);
    winfo->base_addr = (uint64_t) base;
    winfo->disp_unit = disp_unit;

    mpi_errno = MPIDI_CH4I_win_allgather(win);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    mpi_errno = MPIR_Barrier_impl(comm_ptr,&errflag);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_CREATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_attach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_attach(MPID_Win *win, void *base, MPI_Aint size)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_ATTACH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_ATTACH);

    MPIR_ERR_CHKANDSTMT((win->create_flavor != MPI_WIN_FLAVOR_DYNAMIC), mpi_errno,
                        MPI_ERR_RMA_FLAVOR, goto fn_fail, "**rmaflavor");
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_ATTACH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_allocate_shared
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_allocate_shared(MPI_Aint size,
                                                 int disp_unit,
                                                 MPID_Info *info_ptr,
                                                 MPID_Comm *comm_ptr,
                                                 void **base_ptr, MPID_Win **win_ptr)
{
    int            i=0, fd,rc,first=0,mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    void           *baseP      = NULL;
    MPIDI_CH4I_win_info_t *winfo      = NULL;
    MPID_Win       *win        = NULL;
    ssize_t         total_size = 0LL;
    MPI_Aint        *sizes, size_out   = 0;
    char shm_key[64];
    void *map_ptr;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_ALLOCATE_SHARED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_ALLOCATE_SHARED);

    mpi_errno = MPIDI_CH4I_win_init(size,disp_unit,win_ptr,info_ptr,comm_ptr,
                                    MPI_WIN_FLAVOR_SHARED, MPI_WIN_UNIFIED);

    win                   = *win_ptr;
    MPIU_CH4U_WIN(win, sizes)   = (MPI_Aint *)MPIU_Malloc(sizeof(MPI_Aint)*comm_ptr->local_size);
    sizes                 = MPIU_CH4U_WIN(win, sizes);
    sizes[comm_ptr->rank] = size;
    mpi_errno             = MPIR_Allgather_impl(MPI_IN_PLACE,
                                                0,
                                                MPI_DATATYPE_NULL,
                                                sizes,
                                                sizeof(MPI_Aint),
                                                MPI_BYTE,
                                                comm_ptr,
                                                &errflag);
    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    /* No allreduce here because this is a shared memory domain
     * and should be a relatively small number of processes
     * and a non performance sensitive API.
     */
    for(i=0; i<comm_ptr->local_size; i++)
        total_size += sizes[i];

    if(total_size == 0) goto fn_zero;

    sprintf(shm_key, "/mpi-%X-%" PRIx64,
            MPIDI_CH4_Global.jobid,
            MPIU_CH4U_WIN(win, win_id));

    rc    = shm_open(shm_key,
                     O_CREAT|O_EXCL|O_RDWR,
                     0600);
    first = (rc != -1);

    if(!first) {
        rc = shm_open(shm_key, O_RDWR, 0);

        if(rc == -1) {
            shm_unlink(shm_key);
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                                goto fn_fail, "**nomem");
        }
    }

    /* Make the addresses symmetric by using MAP_FIXED */
    size_t page_sz, mapsize;

    mapsize = MPIDI_CH4I_get_mapsize(total_size, &page_sz);
    fd      = rc;
    rc      = ftruncate(fd, mapsize);

    if(rc == -1) {
        close(fd);

        if(first)shm_unlink(shm_key);

        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                            goto fn_fail, "**nomem");
    }

    if(comm_ptr->rank == 0) {
        map_ptr = MPIDI_CH4I_generate_random_addr(mapsize);
        map_ptr = mmap(map_ptr, mapsize,
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED|MAP_FIXED,
                       fd, 0);

        if(map_ptr == NULL || map_ptr == MAP_FAILED) {
            close(fd);

            if(first)shm_unlink(shm_key);

            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                                goto fn_fail, "**nomem");
        }

        mpi_errno = MPIR_Bcast_impl(&map_ptr,1,MPI_UNSIGNED_LONG,
                                    0,comm_ptr,&errflag);

        if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

        MPIU_CH4U_WIN(win, mmap_addr) = map_ptr;
        MPIU_CH4U_WIN(win, mmap_sz)   = mapsize;
    } else {
        mpi_errno = MPIR_Bcast_impl(&map_ptr,1,MPI_UNSIGNED_LONG,
                                    0,comm_ptr,&errflag);

        if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

        rc = MPIDI_CH4I_check_maprange_ok(map_ptr,mapsize);
        /* If we hit this assert, we need to iterate
         * trying more addresses
         */
        MPIU_Assert(rc == 1);
        map_ptr = mmap(map_ptr, mapsize,
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED|MAP_FIXED,
                       fd, 0);
        MPIU_CH4U_WIN(win, mmap_addr) = map_ptr;
        MPIU_CH4U_WIN(win, mmap_sz)   = mapsize;

        if(map_ptr == NULL || map_ptr == MAP_FAILED) {
            close(fd);

            if(first)shm_unlink(shm_key);

            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                                goto fn_fail, "**nomem");
        }
    }

    /* Scan for my offset into the buffer             */
    /* Could use exscan if this is expensive at scale */
    for(i=0; i<comm_ptr->rank; i++)
        size_out+=sizes[i];

fn_zero:

    baseP            = (size==0)?NULL:(void *)((char *)map_ptr + size_out);
    win->base   =  baseP;
    win->size        =  size;

    winfo            = (MPIDI_CH4I_win_info_t *)MPIU_CH4U_WINFO(win, comm_ptr->rank);
    winfo->base_addr = (uint64_t) baseP;
    winfo->disp_unit = disp_unit;
    mpi_errno        = MPIDI_CH4I_win_allgather(win);

    if(mpi_errno != MPI_SUCCESS)
        return mpi_errno;

    *(void **) base_ptr = (void *) win->base;
    mpi_errno = MPIR_Barrier_impl(comm_ptr, &errflag);

    close(fd);

    if(first) shm_unlink(shm_key);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_ALLOCATE_SHARED);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_detach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_detach(MPID_Win *win, const void *base)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_DETACH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_DETACH);
    MPIR_ERR_CHKANDSTMT((win->create_flavor != MPI_WIN_FLAVOR_DYNAMIC), mpi_errno,
                        MPI_ERR_RMA_FLAVOR, goto fn_fail, "**rmaflavor");
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_DETACH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_shared_query
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_shared_query(MPID_Win *win,
                                              int rank,
                                              MPI_Aint *size, int *disp_unit, void *baseptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4I_win_info_t *win_info;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_SHARED_QUERY);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_SHARED_QUERY);
    int offset = rank;

    if(rank < 0)
        offset = 0;

    win_info = MPIU_CH4U_WINFO(win, offset);

    *(void **)baseptr = (void *)win_info->base_addr;
    *disp_unit        = win_info->disp_unit;
    *size             = MPIU_CH4U_WIN(win, sizes)[offset];

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_SHARED_QUERY);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_allocate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_allocate(MPI_Aint size,
                                          int disp_unit,
                                          MPID_Info *info,
                                          MPID_Comm *comm, void *baseptr, MPID_Win **win_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_ALLOCATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_ALLOCATE);

    void           *baseP;
    MPIDI_CH4I_win_info_t *winfo;
    MPID_Win       *win;

    mpi_errno = MPIDI_CH4I_win_init(size,disp_unit,win_ptr, info, comm,
                                    MPI_WIN_FLAVOR_ALLOCATE, MPI_WIN_UNIFIED);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    mpi_errno = MPIDI_CH4I_get_symmetric_heap(size,comm,&baseP,*win_ptr);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    win              = *win_ptr;
    win->base   =  baseP;
    winfo            = (MPIDI_CH4I_win_info_t *)MPIU_CH4U_WINFO(win, comm->rank);
    winfo->base_addr =  (uint64_t) baseP;
    winfo->disp_unit =  disp_unit;

    mpi_errno= MPIDI_CH4I_win_allgather(win);

    if(mpi_errno != MPI_SUCCESS)
        goto fn_fail;

    *(void **) baseptr = (void *) win->base;
    mpi_errno = MPIR_Barrier_impl(comm, &errflag);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_ALLOCATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_flush
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_flush(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_FLUSH);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_FLUSH);

    MPIDI_CH4I_EPOCH_LOCK_CHECK();
    mpi_errno = MPIDI_CH4I_progress_win_fence(win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_FLUSH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_flush_local_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_flush_local_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_FLUSH_LOCAL_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_FLUSH_LOCAL_ALL);

    MPIDI_CH4I_EPOCH_LOCK_CHECK();

    mpi_errno = MPIDI_CH4I_progress_win_fence(win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_FLUSH_LOCAL_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_unlock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_unlock_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_UNLOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_UNLOCK_ALL);
    int i;
    MPIDI_CH4I_winLock_info    *lockQ;

    MPIDI_CH4I_EPOCH_ORIGIN_CHECK(MPIDI_CH4I_EPOTYPE_LOCK_ALL);

    mpi_errno = MPIDI_CH4I_progress_win_fence(win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIU_Assert(MPIU_CH4U_WIN(win, msgQ) != NULL);
    lockQ = (MPIDI_CH4I_winLock_info *) MPIU_CH4U_WIN(win, msgQ);

    for(i = 0; i < win->comm_ptr->local_size; i++) {

        MPIDI_CH4U_win_cntrl_msg_t msg;
        msg.win_id = MPIU_CH4U_WIN(win, win_id);
        msg.origin_rank = win->comm_ptr->rank;
        msg.type = MPIDI_CH4U_WIN_UNLOCKALL;

        lockQ[i].done = 0;
        lockQ[i].peer = i;
        lockQ[i].win  = win;

        mpi_errno = MPIDI_netmod_inject_am_hdr(i, win->comm_ptr,
                                               MPIDI_CH4U_AM_WIN_CTRL,
                                               &msg, sizeof(msg), NULL);
        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
        
        if(MPIU_CH4U_WIN(win, sync).lock.remote.allLocked == 1)
            lockQ[i].done = 1;
    }

    MPIDI_CH4I_PROGRESS_WHILE(MPIU_CH4U_WIN(win, sync).lock.remote.allLocked);

    MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_NONE;
    MPIU_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4I_EPOTYPE_NONE;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_UNLOCK_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_create_dynamic
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_create_dynamic(MPID_Info *info,
                                                MPID_Comm *comm,
                                                MPID_Win **win_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    int            rc        = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_CREATE_DYNAMIC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_CREATE_DYNAMIC);

    MPID_Win       *win;

    rc = MPIDI_CH4I_win_init(0,1,win_ptr, info, comm,
                             MPI_WIN_FLAVOR_DYNAMIC,
                             MPI_WIN_UNIFIED);

    if(rc != MPI_SUCCESS)
        goto fn_fail;

    win       = *win_ptr;
    win->base =  MPI_BOTTOM;

    rc = MPIDI_CH4I_win_allgather(win);

    if(rc != MPI_SUCCESS)
        goto fn_fail;

    mpi_errno = MPIR_Barrier_impl(comm, &errflag);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_CREATE_DYNAMIC);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_flush_local
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_flush_local(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_FLUSH_LOCAL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_FLUSH_LOCAL);

    MPIDI_CH4I_EPOCH_LOCK_CHECK();

    mpi_errno = MPIDI_CH4I_progress_win_fence(win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_FLUSH_LOCAL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_win_sync
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_sync(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_SYNC);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_SYNC);

    MPIDI_CH4I_EPOCH_LOCK_CHECK();
    OPA_read_write_barrier();

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_SYNC);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_flush_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_flush_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_FLUSH_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_FLUSH_ALL);

    MPIDI_CH4I_EPOCH_LOCK_CHECK();

    mpi_errno = MPIDI_CH4I_progress_win_fence(win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_FLUSH_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_win_lock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4U_win_lock_all(int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_WIN_LOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_WIN_LOCK_ALL);

    MPIDI_CH4I_EPOCH_CHECK2();

    int size;
    size = win->comm_ptr->local_size;

    if(!MPIU_CH4U_WIN(win, msgQ)) {
        MPIU_CH4U_WIN(win, msgQ) = (void *) MPIU_Calloc(size, sizeof(MPIDI_CH4I_winLock_info));
        MPIU_Assert(MPIU_CH4U_WIN(win, msgQ) != NULL);
        MPIU_CH4U_WIN(win, count) = 0;
    }

    MPIDI_CH4I_winLock_info *lockQ;
    lockQ = (MPIDI_CH4I_winLock_info *) MPIU_CH4U_WIN(win, msgQ);
    int i;

    for(i = 0; i < size; i++) {
        MPIDI_CH4U_win_cntrl_msg_t msg;
        msg.win_id = MPIU_CH4U_WIN(win, win_id);
        msg.origin_rank = win->comm_ptr->rank;
        msg.type = MPIDI_CH4U_WIN_LOCKALL;
        msg.lock_type = MPI_LOCK_SHARED;

        lockQ[i].done      = 0;
        lockQ[i].peer      = i;
        lockQ[i].win       = win;
        lockQ[i].lock_type = MPI_LOCK_SHARED;

        mpi_errno = MPIDI_netmod_inject_am_hdr(i, win->comm_ptr,
                                               MPIDI_CH4U_AM_WIN_CTRL,
                                               &msg, sizeof(msg), NULL);
        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
        
        if(MPIU_CH4U_WIN(win, sync).lock.remote.allLocked == 1)
            lockQ[i].done = 1;
    }

    MPIDI_CH4I_PROGRESS_WHILE(size != (int)MPIU_CH4U_WIN(win, sync).lock.remote.allLocked);
    MPIU_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4I_EPOTYPE_LOCK_ALL;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_WIN_LOCK_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif
