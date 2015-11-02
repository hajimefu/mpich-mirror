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
#ifndef NETMOD_OFI_WIN_H_INCLUDED
#define NETMOD_OFI_WIN_H_INCLUDED

#include "impl.h"
#include "symheap.h"
#include <opa_primitives.h>

static inline int MPIDI_Win_allgather(MPID_Win  *win)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPID_Comm *comm_ptr = win->comm_ptr;
    mpi_errno = MPIR_Allgather_impl(MPI_IN_PLACE,
                                    0,
                                    MPI_DATATYPE_NULL,
                                    WIN_OFI(win)->winfo,
                                    sizeof(MPIDI_Win_info),
                                    MPI_BYTE,
                                    comm_ptr,
                                    &errflag);
    return mpi_errno;
}

static inline int MPIDI_Win_init(MPI_Aint     length,
                                 int          disp_unit,
                                 MPID_Win   **win_ptr,
                                 MPID_Info   *info,
                                 MPID_Comm   *comm_ptr,
                                 int          create_flavor,
                                 int          model)
{
    int             mpi_errno = MPI_SUCCESS;
    int             rank, size;

    CH4_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devwin_t)>=sizeof(MPIDI_OFIWin_t));
    CH4_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devdt_t)>=sizeof(MPIDI_OFIdt_t));

    MPID_Win *win = (MPID_Win *)MPIU_Handle_obj_alloc(&MPID_Win_mem);
    MPIR_ERR_CHKANDSTMT(win == NULL,
                        mpi_errno,
                        MPI_ERR_NO_MEM,
                        goto fn_fail,
                        "**nomem");
    *win_ptr = win;

    memset(WIN_OFI(win), 0, sizeof(*WIN_OFI(win)));
    win->comm_ptr = comm_ptr;
    size          = comm_ptr->local_size;
    rank          = comm_ptr->rank;
    MPIR_Comm_add_ref(comm_ptr);

    WIN_OFI(win)->winfo = (MPIDI_Win_info *)MPIU_Calloc(size,sizeof(MPIDI_Win_info));

    MPIR_ERR_CHKANDSTMT(WIN_OFI(win)->winfo == NULL,mpi_errno,MPI_ERR_NO_MEM,
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
        mpi_errno= MPIDI_Win_set_info(win, info);
        MPIU_Assert(mpi_errno == 0);
    }

    /* Initialize the info (hint) flags per window */
    WIN_OFI(win)->info_args.no_locks               = 0;
    WIN_OFI(win)->info_args.accumulate_ordering    = (MPIDI_Win_info_accumulate_ordering)
                                                     (MPIDI_ACCU_ORDER_RAR |
                                                      MPIDI_ACCU_ORDER_RAW |
                                                      MPIDI_ACCU_ORDER_WAR |
                                                      MPIDI_ACCU_ORDER_WAW);
    WIN_OFI(win)->info_args.accumulate_ops         = MPIDI_ACCU_SAME_OP_NO_OP;
    WIN_OFI(win)->info_args.same_size              = 0;
    WIN_OFI(win)->info_args.alloc_shared_noncontig = 0;
    WIN_OFI(win)->mmap_sz                          = 0;
    WIN_OFI(win)->mmap_addr                        = NULL;
    MPIDI_Win_info *winfo;
    winfo            = (MPIDI_Win_info *)WINFO(win,rank);
    winfo->disp_unit = disp_unit;
    /* Fill out MR later, if required */

    /* context id lower bits, window instance upper bits */
    WIN_OFI(win)->win_id = 1+(((uint64_t)comm_ptr->context_id) |
                              ((uint64_t)((COMM_OFI(comm_ptr).window_instance)++)<<32));
    MPIDI_OFI_Map_set(MPIDI_Global.win_map,WIN_OFI(win)->win_id,win);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Progress_win_counter_fence
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_Progress_win_counter_fence(MPID_Win *win)
{
    int      mpi_errno = MPI_SUCCESS;
    uint64_t tcount, donecount;
    MPIDI_Win_request *r;

    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_PROGRESS_WIN_COUNTER_FENCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_PROGRESS_WIN_COUNTER_FENCE);

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);
    tcount    = MPIDI_Global.cntr;
    donecount = fi_cntr_read(MPIDI_Global.rma_ctr);
    MPIU_Assert(donecount <= tcount);

    while(tcount > donecount) {
        MPIU_Assert(donecount <= tcount);
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);
        PROGRESS();
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);
        donecount = fi_cntr_read(MPIDI_Global.rma_ctr);
    }

    r = WIN_OFI(win)->syncQ;

    while(r)  {
        MPIDI_Win_request *next = r->next;
        rma_done_event(NULL,(MPID_Request *)r);
        r = next;
    }

    WIN_OFI(win)->syncQ = NULL;
fn_exit:
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_PROGRESS_WIN_COUNTER_FENCE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_set_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_set_info(MPID_Win *win, MPID_Info *info)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_SET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_SET_INFO);

    MPID_Info *curr_ptr;
    char      *value, *token, *savePtr;
    uint       save_ordering;

    curr_ptr = info->next;

    while(curr_ptr) {
        if(!strcmp(curr_ptr->key,"no_locks")) {
            if(!strcmp(curr_ptr->value,"true"))
                WIN_OFI(win)->info_args.no_locks=1;
            else
                WIN_OFI(win)->info_args.no_locks=0;
        } else if(!strcmp(curr_ptr->key,"accumulate_ordering")) {
            save_ordering=(uint) WIN_OFI(win)->info_args.accumulate_ordering;
            WIN_OFI(win)->info_args.accumulate_ordering=(MPIDI_Win_info_accumulate_ordering)0;
            value = curr_ptr->value;
            token = (char *) strtok_r(value,"," , &savePtr);

            while(token) {
                if(!memcmp(token,"rar",3))
                    WIN_OFI(win)->info_args.accumulate_ordering =
                        (MPIDI_Win_info_accumulate_ordering)
                        (WIN_OFI(win)->info_args.accumulate_ordering | MPIDI_ACCU_ORDER_RAR);
                else if(!memcmp(token,"raw",3))
                    WIN_OFI(win)->info_args.accumulate_ordering =
                        (MPIDI_Win_info_accumulate_ordering)
                        (WIN_OFI(win)->info_args.accumulate_ordering | MPIDI_ACCU_ORDER_RAW);
                else if(!memcmp(token,"war",3))
                    WIN_OFI(win)->info_args.accumulate_ordering =
                        (MPIDI_Win_info_accumulate_ordering)
                        (WIN_OFI(win)->info_args.accumulate_ordering | MPIDI_ACCU_ORDER_WAR);
                else if(!memcmp(token,"waw",3))
                    WIN_OFI(win)->info_args.accumulate_ordering =
                        (MPIDI_Win_info_accumulate_ordering)
                        (WIN_OFI(win)->info_args.accumulate_ordering | MPIDI_ACCU_ORDER_WAW);
                else
                    MPIU_Assert(0);

                token = (char *) strtok_r(NULL,"," , &savePtr);
            }

            if(WIN_OFI(win)->info_args.accumulate_ordering == 0)
                WIN_OFI(win)->info_args.accumulate_ordering=
                    (MPIDI_Win_info_accumulate_ordering) save_ordering;
        } else if(!strcmp(curr_ptr->key,"accumulate_ops")) {
            /* the default setting is MPIDI_ACCU_SAME_OP_NO_OP */
            if(!strcmp(curr_ptr->value,"same_op"))
                WIN_OFI(win)->info_args.accumulate_ops = MPIDI_ACCU_SAME_OP;
        }

        curr_ptr = curr_ptr->next;
    }

    mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_SET_INFO);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_start
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_start(MPID_Group *group, int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_START);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_START);

    MPIDI_EPOCH_CHECK_TYPE(win,mpi_errno,goto fn_fail);

    MPIR_Group_add_ref(group);

    PROGRESS_WHILE(group->size != (int)WIN_OFI(win)->sync.pw.count);

    WIN_OFI(win)->sync.pw.count = 0;

    MPIR_ERR_CHKANDJUMP((WIN_OFI(win)->sync.sc.group != NULL),
                        mpi_errno,
                        MPI_ERR_GROUP,
                        "**group");
    WIN_OFI(win)->sync.sc.group          = group;
    WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_START;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_START);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_complete(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_COMPLETE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_COMPLETE);

    MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

    MPID_Group *group;
    group = WIN_OFI(win)->sync.sc.group;
    MPIU_Assert(group != NULL);
    MPIDI_Win_control_t msg;
    msg.type = MPIDI_CTRL_COMPLETE;

    int index, peer;

    for(index=0; index < group->size; ++index) {
        peer      = group->lrank_to_lpid[index].lpid;
        mpi_errno = do_control_win(&msg, peer, win, 0);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
    }

    MPIDI_EPOCH_TARGET_EVENT(win);

    MPIR_Group_release(WIN_OFI(win)->sync.sc.group);
    WIN_OFI(win)->sync.sc.group = NULL;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_COMPLETE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_post
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_post(MPID_Group *group, int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_POST);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_POST);

    MPIDI_EPOCH_POST_CHECK(win,mpi_errno,goto fn_fail);

    MPIR_Group_add_ref(group);
    MPIR_ERR_CHKANDJUMP((WIN_OFI(win)->sync.pw.group != NULL),
                        mpi_errno, MPI_ERR_GROUP,
                        "**group");

    WIN_OFI(win)->sync.pw.group = group;
    MPIU_Assert(group != NULL);
    MPIDI_Win_control_t msg;
    msg.type = MPIDI_CTRL_POST;

    int index;

    for(index=0; index < group->size; ++index) {
        int peer  = group->lrank_to_lpid[index].lpid;
        mpi_errno = do_control_win(&msg, peer, win, 0);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
    }

    WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_POST;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_POST);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_wait
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_wait(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_WAIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_WAIT);

    MPIDI_EPOCH_TARGET_CHECK(win,MPID_EPOTYPE_POST,mpi_errno,return mpi_errno);

    MPID_Group *group;
    group = WIN_OFI(win)->sync.pw.group;

    PROGRESS_WHILE(group->size != (int)WIN_OFI(win)->sync.sc.count);

    WIN_OFI(win)->sync.sc.count = 0;
    WIN_OFI(win)->sync.pw.group = NULL;

    MPIR_Group_release(group);

    MPIDI_EPOCH_ORIGIN_EVENT(win);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_WAIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_test
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_test(MPID_Win *win, int *flag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_TEST);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_TEST);

    MPIDI_EPOCH_TARGET_CHECK(win,MPID_EPOTYPE_POST,mpi_errno,return mpi_errno);

    MPID_Group *group;
    group = WIN_OFI(win)->sync.pw.group;

    if(group->size == (int)WIN_OFI(win)->sync.sc.count) {
        WIN_OFI(win)->sync.sc.count = 0;
        WIN_OFI(win)->sync.pw.group = NULL;
        *flag          = 1;
        MPIR_Group_release(group);
        MPIDI_EPOCH_ORIGIN_EVENT(win);
    } else {
        PROGRESS();
        *flag=0;
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_TEST);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_lock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_lock(int lock_type, int rank, int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_LOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_LOCK);

    MPIDI_Win_sync_lock *slock = &WIN_OFI(win)->sync.lock;
    MPIDI_EPOCH_CHECK_TYPE(win,mpi_errno,goto fn_fail);

    if(rank == MPI_PROC_NULL) goto fn_exit0;

    MPIDI_Win_control_t  msg;

    msg.type      = MPIDI_CTRL_LOCKREQ;
    msg.lock_type = lock_type;

    mpi_errno     = do_control_win(&msg, rank, win,1);

    if(mpi_errno != MPI_SUCCESS)
        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                            goto fn_fail, "**rmasync");

    PROGRESS_WHILE(!slock->remote.locked);

fn_exit0:
    WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_LOCK;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_LOCK);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_unlock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_unlock(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_UNLOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_UNLOCK);

    MPIDI_EPOCH_ORIGIN_CHECK(win, MPID_EPOTYPE_LOCK, mpi_errno, return mpi_errno);

    if(rank == MPI_PROC_NULL) goto fn_exit0;

    MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

    MPIDI_Win_control_t msg;
    msg.type  = MPIDI_CTRL_UNLOCK;
    mpi_errno = do_control_win(&msg, rank, win, 1);

    if(mpi_errno != MPI_SUCCESS)
        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                            goto fn_fail, "**rmasync");

    PROGRESS_WHILE(WIN_OFI(win)->sync.lock.remote.locked);
fn_exit0:
    WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_NONE;
    WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_NONE;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_UNLOCK);
    return mpi_errno;
fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_get_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_get_info(MPID_Win *win, MPID_Info **info_p_p)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_GET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_GET_INFO);

    mpi_errno = MPIU_Info_alloc(info_p_p);
    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    if(WIN_OFI(win)->info_args.no_locks)
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "no_locks", "true");
    else
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "no_locks", "false");

    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    {
#define BUFSIZE 32
        char buf[BUFSIZE];
        int c = 0;

        if(WIN_OFI(win)->info_args.accumulate_ordering & MPIDI_ACCU_ORDER_RAR)
            c += snprintf(buf+c, BUFSIZE-c, "%srar", (c > 0) ? "," : "");

        if(WIN_OFI(win)->info_args.accumulate_ordering & MPIDI_ACCU_ORDER_RAW)
            c += snprintf(buf+c, BUFSIZE-c, "%sraw", (c > 0) ? "," : "");

        if(WIN_OFI(win)->info_args.accumulate_ordering & MPIDI_ACCU_ORDER_WAR)
            c += snprintf(buf+c, BUFSIZE-c, "%swar", (c > 0) ? "," : "");

        if(WIN_OFI(win)->info_args.accumulate_ordering & MPIDI_ACCU_ORDER_WAW)
            c += snprintf(buf+c, BUFSIZE-c, "%swaw", (c > 0) ? "," : "");

        if(c == 0) {
            memcpy(&buf[0],"not set   ",10);
        }

        MPIR_Info_set_impl(*info_p_p, "accumulate_ordering", buf);
        MPIU_Assert(mpi_errno == MPI_SUCCESS);
#undef BUFSIZE
    }

    if(WIN_OFI(win)->info_args.accumulate_ops == MPIDI_ACCU_SAME_OP)
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "accumulate_ops", "same_op");
    else
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "accumulate_ops", "same_op_no_op");

    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    if(win->create_flavor == MPI_WIN_FLAVOR_SHARED) {
        if(WIN_OFI(win)->info_args.alloc_shared_noncontig)
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "alloc_shared_noncontig", "true");
        else
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "alloc_shared_noncontig", "false");

        MPIU_Assert(mpi_errno == MPI_SUCCESS);
    } else if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE) {
        if(WIN_OFI(win)->info_args.same_size)
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "same_size", "true");
        else
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "same_size", "false");

        MPIU_Assert(mpi_errno == MPI_SUCCESS);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_GET_INFO);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_free
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_free(MPID_Win **win_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    MPID_Win      *win       = *win_ptr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FREE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FREE);

    MPIDI_EPOCH_FREE_CHECK(win,mpi_errno,return mpi_errno);

    mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE  && win->base) {
        if(WIN_OFI(win)->mmap_sz > 0)
            munmap(WIN_OFI(win)->mmap_addr, WIN_OFI(win)->mmap_sz);
        else if(WIN_OFI(win)->mmap_sz == -1)
            MPIU_Free(win->base);
    }

    if(win->create_flavor == MPI_WIN_FLAVOR_SHARED) {
        if(WIN_OFI(win)->mmap_addr)
            munmap(WIN_OFI(win)->mmap_addr, WIN_OFI(win)->mmap_sz);

        MPIU_Free(WIN_OFI(win)->sizes);
    }

    if(WIN_OFI(win)->msgQ)
        MPIU_Free(WIN_OFI(win)->msgQ);

    MPIU_Free(WIN_OFI(win)->winfo);
    MPIR_Comm_release(win->comm_ptr);
    MPIU_Handle_obj_free(&MPID_Win_mem, win);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FREE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_fence
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_fence(int massert, MPID_Win *win)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FENCE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FENCE);

    MPIDI_EPOCH_FENCE_CHECK(win, mpi_errno, goto fn_fail);

    MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

    MPIDI_EPOCH_FENCE_EVENT(win,massert);

    if(!(massert & MPI_MODE_NOPRECEDE))
        mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FENCE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_create(void *base,
                                          MPI_Aint length,
                                          int disp_unit,
                                          MPID_Info *info,
                                          MPID_Comm *comm_ptr, MPID_Win **win_ptr)
{
    int             mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t  errflag   = MPIR_ERR_NONE;
    MPID_Win       *win;
    int             rank;
    MPIDI_Win_info *winfo;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_CREATE);

    mpi_errno        = MPIDI_Win_init(length,
                                      disp_unit,
                                      win_ptr,
                                      info,
                                      comm_ptr,
                                      MPI_WIN_FLAVOR_CREATE,
                                      MPI_WIN_UNIFIED);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    win              = *win_ptr;
    win->base        = base;
    rank             = comm_ptr->rank;
    winfo            = (MPIDI_Win_info *)WINFO(win,rank);
    winfo->base_addr = base;
    winfo->disp_unit = disp_unit;

    mpi_errno = MPIDI_Win_allgather(win);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    mpi_errno = MPIR_Barrier_impl(comm_ptr,&errflag);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_CREATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_attach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_attach(MPID_Win *win, void *base, MPI_Aint size)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_ATTACH);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_ATTACH);

    MPIR_ERR_CHKANDSTMT((win->create_flavor != MPI_WIN_FLAVOR_DYNAMIC), mpi_errno,
                        MPI_ERR_RMA_FLAVOR, goto fn_fail, "**rmaflavor");
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_ATTACH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_allocate_shared
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_allocate_shared(MPI_Aint size,
                                                   int disp_unit,
                                                   MPID_Info *info_ptr,
                                                   MPID_Comm *comm_ptr,
                                                   void **base_ptr, MPID_Win **win_ptr)
{
    int            i=0, fd,rc,first=0,mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    void           *baseP      = NULL;
    MPIDI_Win_info *winfo      = NULL;
    MPID_Win       *win        = NULL;
    ssize_t         total_size = 0LL;
    MPI_Aint        *sizes, size_out   = 0;
    char shm_key[64];
    void *map_ptr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE_SHARED);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE_SHARED);

    mpi_errno = MPIDI_Win_init(size,disp_unit,win_ptr,info_ptr,comm_ptr,
                               MPI_WIN_FLAVOR_SHARED, MPI_WIN_UNIFIED);

    win                   = *win_ptr;
    WIN_OFI(win)->sizes   = (MPI_Aint *)MPIU_Malloc(sizeof(MPI_Aint)*comm_ptr->local_size);
    sizes                 = WIN_OFI(win)->sizes;
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
        total_size+=sizes[i];

    if(total_size == 0) goto fn_zero;

    sprintf(shm_key, "/mpi-%X-%" PRIx64,
            MPIDI_Global.jobid,
            WIN_OFI(win)->win_id);

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

    mapsize = get_mapsize(total_size, &page_sz);
    fd      = rc;
    rc      = ftruncate(fd, mapsize);

    if(rc == -1) {
        close(fd);

        if(first)shm_unlink(shm_key);

        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                            goto fn_fail, "**nomem");
    }

    if(comm_ptr->rank == 0) {
        map_ptr = generate_random_addr(mapsize);
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

        WIN_OFI(win)->mmap_addr = map_ptr;
        WIN_OFI(win)->mmap_sz   = mapsize;
    } else {
        mpi_errno = MPIR_Bcast_impl(&map_ptr,1,MPI_UNSIGNED_LONG,
                                    0,comm_ptr,&errflag);

        if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

        rc = check_maprange_ok(map_ptr,mapsize);
        /* If we hit this assert, we need to iterate
         * trying more addresses
         */
        MPIU_Assert(rc == 1);
        map_ptr = mmap(map_ptr, mapsize,
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED|MAP_FIXED,
                       fd, 0);
        WIN_OFI(win)->mmap_addr = map_ptr;
        WIN_OFI(win)->mmap_sz   = mapsize;

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
    win->base        =  baseP;
    win->size        =  size;

    winfo            = (MPIDI_Win_info *)WINFO(win,comm_ptr->rank);
    winfo->base_addr = baseP;
    winfo->disp_unit = disp_unit;
    mpi_errno        = MPIDI_Win_allgather(win);

    if(mpi_errno != MPI_SUCCESS)
        return mpi_errno;

    *(void **) base_ptr = (void *) win->base;
    mpi_errno = MPIR_Barrier_impl(comm_ptr, &errflag);

    close(fd);

    if(first) shm_unlink(shm_key);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE_SHARED);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_detach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_detach(MPID_Win *win, const void *base)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_DETACH);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_DETACH);
    MPIR_ERR_CHKANDSTMT((win->create_flavor != MPI_WIN_FLAVOR_DYNAMIC), mpi_errno,
                        MPI_ERR_RMA_FLAVOR, goto fn_fail, "**rmaflavor");
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_DETACH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_shared_query
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_shared_query(MPID_Win *win,
                                                int rank,
                                                MPI_Aint *size, int *disp_unit, void *baseptr)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_SHARED_QUERY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_SHARED_QUERY);
    int offset = rank;

    if(rank < 0)
        offset = 0;

    *(void **)baseptr = WINFO_BASE(win, offset);
    *size             = WIN_OFI(win)->sizes[offset];
    *disp_unit        = WINFO_DISP_UNIT(win,offset);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_SHARED_QUERY);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_allocate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_allocate(MPI_Aint size,
                                            int disp_unit,
                                            MPID_Info *info,
                                            MPID_Comm *comm, void *baseptr, MPID_Win **win_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE);

    void           *baseP;
    MPIDI_Win_info *winfo;
    MPID_Win       *win;

    mpi_errno = MPIDI_Win_init(size,disp_unit,win_ptr, info, comm,
                               MPI_WIN_FLAVOR_ALLOCATE, MPI_WIN_UNIFIED);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    mpi_errno = get_symmetric_heap(size,comm,&baseP,*win_ptr);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    win              = *win_ptr;
    win->base        =  baseP;
    winfo            = (MPIDI_Win_info *)WINFO(win,comm->rank);
    winfo->base_addr =  baseP;
    winfo->disp_unit =  disp_unit;

    mpi_errno= MPIDI_Win_allgather(win);

    if(mpi_errno != MPI_SUCCESS)
        goto fn_fail;

    *(void **) baseptr = (void *) win->base;
    mpi_errno = MPIR_Barrier_impl(comm, &errflag);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_flush
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_flush(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FLUSH);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FLUSH);

    MPIDI_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);

    MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FLUSH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_flush_local_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_flush_local_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL_ALL);

    MPIDI_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);
    MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_unlock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_unlock_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_UNLOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_UNLOCK_ALL);
    int i;
    MPIDI_WinLock_info    *lockQ;

    MPIDI_EPOCH_ORIGIN_CHECK(win, MPID_EPOTYPE_LOCK_ALL, mpi_errno, return mpi_errno);

    MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

    MPIU_Assert(WIN_OFI(win)->msgQ != NULL);

    lockQ = (MPIDI_WinLock_info *) WIN_OFI(win)->msgQ;

    for(i = 0; i < win->comm_ptr->local_size; i++) {
        MPIDI_Win_control_t msg;
        lockQ[i].done = 0;
        lockQ[i].peer = i;
        lockQ[i].win  = win;
        msg.type      = MPIDI_CTRL_UNLOCKALL;
        mpi_errno     = do_control_win(&msg, lockQ[i].peer, win, 1);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");

        if(WIN_OFI(win)->sync.lock.remote.allLocked == 1)
            lockQ[i].done = 1;
    }

    PROGRESS_WHILE(WIN_OFI(win)->sync.lock.remote.allLocked);

    WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_NONE;
    WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_NONE;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_UNLOCK_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_create_dynamic
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_create_dynamic(MPID_Info *info,
                                                  MPID_Comm *comm,
                                                  MPID_Win **win_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    int            rc        = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_CREATE_DYNAMIC);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_CREATE_DYNAMIC);

    MPID_Win       *win;

    rc = MPIDI_Win_init(0,1,win_ptr, info, comm,
                        MPI_WIN_FLAVOR_DYNAMIC,
                        MPI_WIN_UNIFIED);

    if(rc != MPI_SUCCESS)
        goto fn_fail;

    win       = *win_ptr;
    win->base =  MPI_BOTTOM;

    rc = MPIDI_Win_allgather(win);

    if(rc != MPI_SUCCESS)
        goto fn_fail;

    mpi_errno = MPIR_Barrier_impl(comm, &errflag);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_CREATE_DYNAMIC);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_flush_local
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_flush_local(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL);

    MPIDI_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);

    MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_sync
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_sync(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_SYNC);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_SYNC);

    MPIDI_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);

    OPA_read_write_barrier();

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_SYNC);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_flush_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_flush_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FLUSH_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FLUSH_ALL);

    MPIDI_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);
    MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FLUSH_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_win_lock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_win_lock_all(int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_LOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_LOCK_ALL);

    MPIDI_EPOCH_CHECK_TYPE(win,mpi_errno,goto fn_fail);

    int size;
    size = win->comm_ptr->local_size;

    if(!WIN_OFI(win)->msgQ) {
        WIN_OFI(win)->msgQ = (void *) MPIU_Calloc(size, sizeof(MPIDI_WinLock_info));
        MPIU_Assert(WIN_OFI(win)->msgQ != NULL);
        WIN_OFI(win)->count=0;
    }

    MPIDI_WinLock_info *lockQ;
    lockQ = (MPIDI_WinLock_info *) WIN_OFI(win)->msgQ;
    int i;

    for(i = 0; i < size; i++) {
        MPIDI_Win_control_t msg;

        lockQ[i].done      = 0;
        lockQ[i].peer      = i;
        lockQ[i].win       = win;
        lockQ[i].lock_type = MPI_LOCK_SHARED;

        msg.type           = MPIDI_CTRL_LOCKALLREQ;
        msg.lock_type      = MPI_LOCK_SHARED;
        mpi_errno          = do_control_win(&msg, lockQ[i].peer, lockQ[i].win, 1);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");

        if(WIN_OFI(win)->sync.lock.remote.allLocked == 1)
            lockQ[i].done = 1;
    }

    PROGRESS_WHILE(size != (int)WIN_OFI(win)->sync.lock.remote.allLocked);

    WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_LOCK_ALL;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_LOCK_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#endif /* NETMOD_OFI_WIN_H_INCLUDED */
