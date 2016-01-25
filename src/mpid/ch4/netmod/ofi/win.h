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


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Win_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Win_allgather(MPID_Win        *win,
                                      void            *base,
                                      int              disp_unit,
                                      int              do_optimize)
{
    int             i,same_disp,mpi_errno = MPI_SUCCESS;
    uint32_t        first;
    MPIR_Errflag_t  errflag      = MPIR_ERR_NONE;
    MPID_Comm      *comm_ptr     = win->comm_ptr;
    MPIDI_CH4R_win_info_t *my_winfo = NULL;
    int raw_prefix,idx,bitpos;
    unsigned gen_id;

    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_WIN_ALLGATHER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_WIN_ALLGATHER);

    /* Calculate a canonical context id */
    raw_prefix = MPID_CONTEXT_READ_FIELD(PREFIX,comm_ptr->context_id);
    idx        = raw_prefix / MPIR_CONTEXT_INT_BITS;
    bitpos     = raw_prefix % MPIR_CONTEXT_INT_BITS;
    gen_id     = (idx*MPIR_CONTEXT_INT_BITS) + (31-bitpos);

    int       total_bits_avail      = MPIDI_Global.max_mr_key_size * 8;
    uint64_t  window_instance       = ((uint64_t)MPIDI_CH4_NMI_OFI_WIN(win)->win_id)>>32;
    int       bits_for_instance_id  = MPIDI_Global.max_windows_bits;
    int       bits_for_context_id;
    uint64_t  max_contexts_allowed;
    uint64_t  max_instances_allowed;

    bits_for_context_id   = total_bits_avail -
        MPIDI_Global.max_windows_bits -
        MPIDI_Global.max_huge_rma_bits;
    max_contexts_allowed  = 1<<(bits_for_context_id);
    max_instances_allowed = 1<<(bits_for_instance_id);
    MPIR_ERR_CHKANDSTMT(gen_id >= max_contexts_allowed,mpi_errno,MPI_ERR_OTHER,
                        goto fn_fail,"**ofid_mr_reg");
    MPIR_ERR_CHKANDSTMT(window_instance >= max_instances_allowed,mpi_errno,MPI_ERR_OTHER,
                        goto fn_fail,"**ofid_mr_reg");

    /* Context id in lower bits, instance in upper bits */
    MPIDI_CH4_NMI_OFI_WIN(win)->mr_key = (gen_id<<MPIDI_Global.context_shift) | window_instance;
    MPIDI_CH4_NMI_OFI_CALL(fi_mr_reg(MPIDI_Global.domain,                /* In:  Domain Object       */
                    base,                               /* In:  Lower memory address*/
                    win->size,                          /* In:  Length              */
                    FI_REMOTE_READ | FI_REMOTE_WRITE,   /* In:  Expose MR for read  */
                    0ULL,                               /* In:  offset(not used)    */
                    MPIDI_CH4_NMI_OFI_WIN(win)->mr_key,               /* In:  requested key       */
                    0ULL,                               /* In:  flags               */
                    &MPIDI_CH4_NMI_OFI_WIN(win)->mr,                  /* Out: memregion object    */
                    NULL), mr_reg);                     /* In:  context             */

    my_winfo = (MPIDI_CH4R_win_info_t *)WINFO(win,0);
    my_winfo->base_addr = (uint64_t) base;
    my_winfo->disp_unit = disp_unit;

    my_winfo = (MPIDI_CH4R_win_info_t *)WINFO(win,comm_ptr->rank);
    my_winfo->base_addr = (uint64_t) base;
    my_winfo->disp_unit = disp_unit;
    mpi_errno        = MPIR_Allgather_impl(MPI_IN_PLACE,
                                           0,
                                           MPI_DATATYPE_NULL,
                                           MPIDI_CH4R_WIN(win, info_table),
                                           sizeof(MPIDI_CH4R_win_info_t),
                                           MPI_BYTE,
                                           comm_ptr,
                                           &errflag);
    if(do_optimize) {
        first     = MPIDI_CH4_NMI_OFI_WINFO_DISP_UNIT(win,0);
        same_disp = 1;
        for(i=1;i<comm_ptr->local_size;i++) {
            if(MPIDI_CH4_NMI_OFI_WINFO_DISP_UNIT(win,i) != first)
                same_disp=0;
                break;
        }
        if(same_disp) {
            MPIU_Free(MPIDI_CH4R_WIN(win, info_table));
            MPIDI_CH4R_WIN(win, info_table) = NULL;
        }
    }
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_PROGRESS_WIN_ALLGATHER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Win_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Win_init(MPI_Aint     length,
                                 int          disp_unit,
                                 MPID_Win   **win_ptr,
                                 MPID_Info   *info,
                                 MPID_Comm   *comm_ptr,
                                 int          create_flavor,
                                 int          model)
{
    int       mpi_errno = MPI_SUCCESS;
    uint64_t  window_instance;
    MPID_Win *win;
    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_WIN_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_WIN_INIT);

    CH4_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devwin_t)>=sizeof(MPIDI_CH4_NMI_OFI_Win_t));
    CH4_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devdt_t)>=sizeof(MPIDI_CH4_NMI_OFI_Datatype_t));

    mpi_errno = MPIDI_CH4R_win_init(length, disp_unit, &win, info, comm_ptr, create_flavor, model);
    MPIR_ERR_CHKANDSTMT(mpi_errno != MPI_SUCCESS,
                        mpi_errno,
                        MPI_ERR_NO_MEM,
                        goto fn_fail,
                        "**nomem");
    *win_ptr = win;

    memset(MPIDI_CH4_NMI_OFI_WIN(win), 0, sizeof(*MPIDI_CH4_NMI_OFI_WIN(win)));

    if((info != NULL) && ((int *)info != (int *) MPI_INFO_NULL)) {
        mpi_errno= MPIDI_Win_set_info(win, info);
        MPIU_Assert(mpi_errno == 0);
    }

    /* Initialize the info (hint) flags per window */
    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.no_locks               = 0;
    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering    = (MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t)
                                                     (MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_RAR |
                                                      MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_RAW |
                                                      MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_WAR |
                                                      MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_WAW);
    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ops         = MPIDI_CH4_NMI_OFI_ACCUMULATE_SAME_OP_NO_OP;
    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.same_size              = 0;
    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.alloc_shared_noncontig = 0;
    MPIDI_CH4_NMI_OFI_WIN(win)->mmap_sz                          = 0;
    MPIDI_CH4_NMI_OFI_WIN(win)->mmap_addr                        = NULL;
    /* context id lower bits, window instance upper bits */
    window_instance = MPIDI_CH4_NMI_OFI_Index_allocator_alloc(MPIDI_CH4_NMI_OFI_COMM(win->comm_ptr).win_id_allocator);
    MPIDI_CH4_NMI_OFI_WIN(win)->win_id = ((uint64_t)comm_ptr->context_id) | (window_instance<<32);
    MPIDI_CH4_NMI_OFI_Map_set(MPIDI_Global.win_map,MPIDI_CH4_NMI_OFI_WIN(win)->win_id,win);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_PROGRESS_WIN_INIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_Win_progress_fence
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_Win_progress_fence(MPID_Win *win)
{
    int      mpi_errno = MPI_SUCCESS;
    int      itercount = 0;
    int      ret;
    uint64_t tcount, donecount;
    MPIDI_CH4_NMI_OFI_Win_request_t *r;

    MPIDI_STATE_DECL(MPID_STATE_CH4_OFI_PROGRESS_WIN_COUNTER_FENCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OFI_PROGRESS_WIN_COUNTER_FENCE);

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);
    tcount    = MPIDI_Global.cntr;
    donecount = fi_cntr_read(MPIDI_Global.rma_ctr);

    MPIU_Assert(donecount <= tcount);

    while(tcount > donecount) {
        MPIU_Assert(donecount <= tcount);
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);
        MPIDI_CH4_NMI_OFI_PROGRESS();
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);
        donecount = fi_cntr_read(MPIDI_Global.rma_ctr);
        itercount++;
        if(itercount == 1000) {
            ret=fi_cntr_wait(MPIDI_Global.rma_ctr,tcount,0);
            MPIDI_CH4_NMI_OFI_ERR(ret < 0 && ret != -FI_ETIMEDOUT,
                                  mpi_errno,
                                  MPI_ERR_RMA_RANGE,
                                  "**ofid_cntr_wait",
                                  "**ofid_cntr_wait %s %d %s %s",
                                  __SHORT_FILE__,
                                  __LINE__,
                                  FCNAME,
                                  fi_strerror(-ret));
            itercount = 0;
        }
    }

    while (OPA_load_int(&MPIDI_CH4R_WIN(win, outstanding_ops)) != 0)
        MPIDI_CH4_NMI_OFI_PROGRESS();

    r = MPIDI_CH4_NMI_OFI_WIN(win)->syncQ;

    while(r)  {
        MPIDI_CH4_NMI_OFI_Win_request_t *next = r->next;
        MPIDI_CH4_NMI_OFI_Rma_done_event(NULL,(MPID_Request *)r);
        r = next;
    }

    MPIDI_CH4_NMI_OFI_WIN(win)->syncQ = NULL;
fn_exit:
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OFI_PROGRESS_WIN_COUNTER_FENCE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_set_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_set_info(MPID_Win *win, MPID_Info *info)
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
                MPIDI_CH4_NMI_OFI_WIN(win)->info_args.no_locks=1;
            else
                MPIDI_CH4_NMI_OFI_WIN(win)->info_args.no_locks=0;
        } else if(!strcmp(curr_ptr->key,"accumulate_ordering")) {
            save_ordering=(uint) MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering;
            MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering=(MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t)0;
            value = curr_ptr->value;
            token = (char *) strtok_r(value,"," , &savePtr);

            while(token) {
                if(!memcmp(token,"rar",3))
                    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering =
                        (MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t)
                        (MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering | MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_RAR);
                else if(!memcmp(token,"raw",3))
                    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering =
                        (MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t)
                        (MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering | MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_RAW);
                else if(!memcmp(token,"war",3))
                    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering =
                        (MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t)
                        (MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering | MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_WAR);
                else if(!memcmp(token,"waw",3))
                    MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering =
                        (MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t)
                        (MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering | MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_WAW);
                else
                    MPIU_Assert(0);

                token = (char *) strtok_r(NULL,"," , &savePtr);
            }

            if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering == 0)
                MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering=
                    (MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t) save_ordering;
        } else if(!strcmp(curr_ptr->key,"accumulate_ops")) {
            /* the default setting is MPIDI_CH4_NMI_OFI_ACCUMULATE_SAME_OP_NO_OP */
            if(!strcmp(curr_ptr->value,"same_op"))
                MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ops = MPIDI_CH4_NMI_OFI_ACCUMULATE_SAME_OP;
        }

        curr_ptr = curr_ptr->next;
    }

    mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_SET_INFO);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_start
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_start(MPID_Group *group, int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_START);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_START);

    MPIDI_CH4R_EPOCH_CHECK_TYPE(win,mpi_errno,goto fn_fail);

    MPIR_Group_add_ref(group);

    MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(group->size != (int)MPIDI_CH4R_WIN(win, sync).pw.count);

    MPIDI_CH4R_WIN(win, sync).pw.count = 0;

    MPIR_ERR_CHKANDJUMP((MPIDI_CH4R_WIN(win, sync).sc.group != NULL),
                        mpi_errno,
                        MPI_ERR_GROUP,
                        "**group");
    MPIDI_CH4R_WIN(win, sync).sc.group          = group;
    MPIDI_CH4R_WIN(win, sync).origin_epoch_type = MPIDI_CH4R_EPOTYPE_START;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_START);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_complete(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_COMPLETE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_COMPLETE);

    MPIDI_CH4R_EPOCH_START_CHECK2(win,mpi_errno,goto fn_fail);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_Win_progress_fence(win));

    MPID_Group *group;
    group = MPIDI_CH4R_WIN(win, sync).sc.group;
    MPIU_Assert(group != NULL);
    MPIDI_CH4_NMI_OFI_Win_control_t msg;
    msg.type = MPIDI_CH4_NMI_OFI_CTRL_COMPLETE;

    int index, peer;

    for(index=0; index < group->size; ++index) {
        peer      = group->lrank_to_lpid[index].lpid;
        mpi_errno = MPIDI_CH4_NMI_OFI_Do_control_win(&msg, peer, win, 0, 1);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
    }

    MPIDI_CH4R_EPOCH_TARGET_EVENT(win);

    MPIR_Group_release(MPIDI_CH4R_WIN(win, sync).sc.group);
    MPIDI_CH4R_WIN(win, sync).sc.group = NULL;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_COMPLETE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_post
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_post(MPID_Group *group, int assert, MPID_Win *win)
{
    int peer, index, mpi_errno = MPI_SUCCESS;
    MPIDI_CH4_NMI_OFI_Win_control_t msg;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_POST);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_POST);

    MPIDI_CH4R_EPOCH_POST_CHECK(win,mpi_errno,goto fn_fail);

    MPIR_Group_add_ref(group);
    MPIR_ERR_CHKANDJUMP((MPIDI_CH4R_WIN(win, sync).pw.group != NULL),
                        mpi_errno, MPI_ERR_GROUP,
                        "**group");

    MPIDI_CH4R_WIN(win, sync).pw.group = group;
    MPIU_Assert(group != NULL);

    msg.type = MPIDI_CH4_NMI_OFI_CTRL_POST;
    for(index=0; index < group->size; ++index) {
        peer      = group->lrank_to_lpid[index].lpid;
        mpi_errno = MPIDI_CH4_NMI_OFI_Do_control_win(&msg, peer, win, 0, 1);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
    }
    MPIDI_CH4R_WIN(win, sync).target_epoch_type = MPIDI_CH4R_EPOTYPE_POST;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_POST);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_wait
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_wait(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_WAIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_WAIT);

    MPIDI_CH4R_EPOCH_TARGET_CHECK(win,MPIDI_CH4R_EPOTYPE_POST,mpi_errno,return mpi_errno);

    MPID_Group *group;
    group = MPIDI_CH4R_WIN(win, sync).pw.group;

    MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(group->size != (int)MPIDI_CH4R_WIN(win, sync).sc.count);

    MPIDI_CH4R_WIN(win, sync).sc.count = 0;
    MPIDI_CH4R_WIN(win, sync).pw.group = NULL;

    MPIR_Group_release(group);

    MPIDI_CH4R_EPOCH_ORIGIN_EVENT(win);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_WAIT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_test
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_test(MPID_Win *win, int *flag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_TEST);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_TEST);

    MPIDI_CH4R_EPOCH_TARGET_CHECK(win,MPIDI_CH4R_EPOTYPE_POST,mpi_errno,return mpi_errno);

    MPID_Group *group;
    group = MPIDI_CH4R_WIN(win, sync).pw.group;

    if(group->size == (int)MPIDI_CH4R_WIN(win, sync).sc.count) {
        MPIDI_CH4R_WIN(win, sync).sc.count = 0;
        MPIDI_CH4R_WIN(win, sync).pw.group = NULL;
        *flag          = 1;
        MPIR_Group_release(group);
        MPIDI_CH4R_EPOCH_ORIGIN_EVENT(win);
    } else {
        MPIDI_CH4_NMI_OFI_PROGRESS();
        *flag=0;
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_TEST);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_lock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_lock(int lock_type, int rank, int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_LOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_LOCK);

    MPIDI_CH4R_win_sync_lock *slock = &MPIDI_CH4R_WIN(win, sync).lock;
    MPIDI_CH4R_EPOCH_CHECK_TYPE(win,mpi_errno,goto fn_fail);

    if(rank == MPI_PROC_NULL) goto fn_exit0;

    MPIDI_CH4_NMI_OFI_Win_control_t  msg;

    msg.type      = MPIDI_CH4_NMI_OFI_CTRL_LOCKREQ;
    msg.lock_type = lock_type;

    mpi_errno     = MPIDI_CH4_NMI_OFI_Do_control_win(&msg, rank, win,1, 1);

    if(mpi_errno != MPI_SUCCESS)
        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                            goto fn_fail, "**rmasync");

    MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(!slock->remote.locked);

fn_exit0:
    MPIDI_CH4R_WIN(win, sync).origin_epoch_type = MPIDI_CH4R_EPOTYPE_LOCK;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_LOCK);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_unlock
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_unlock(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_UNLOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_UNLOCK);

    MPIDI_CH4R_EPOCH_ORIGIN_CHECK(win, MPIDI_CH4R_EPOTYPE_LOCK, mpi_errno, return mpi_errno);

    if(rank == MPI_PROC_NULL) goto fn_exit0;

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_Win_progress_fence(win));

    MPIDI_CH4_NMI_OFI_Win_control_t msg;
    msg.type  = MPIDI_CH4_NMI_OFI_CTRL_UNLOCK;
    mpi_errno = MPIDI_CH4_NMI_OFI_Do_control_win(&msg, rank, win, 1, 1);

    if(mpi_errno != MPI_SUCCESS)
        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                            goto fn_fail, "**rmasync");

    MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(MPIDI_CH4R_WIN(win, sync).lock.remote.locked);
fn_exit0:
    MPIDI_CH4R_WIN(win, sync).origin_epoch_type = MPIDI_CH4R_EPOTYPE_NONE;
    MPIDI_CH4R_WIN(win, sync).target_epoch_type = MPIDI_CH4R_EPOTYPE_NONE;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_UNLOCK);
    return mpi_errno;
fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_get_info
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_get_info(MPID_Win *win, MPID_Info **info_p_p)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_GET_INFO);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_GET_INFO);

    mpi_errno = MPIU_Info_alloc(info_p_p);
    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.no_locks)
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "no_locks", "true");
    else
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "no_locks", "false");

    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    {
#define BUFSIZE 32
        char buf[BUFSIZE];
        int c = 0;

        if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering & MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_RAR)
            c += snprintf(buf+c, BUFSIZE-c, "%srar", (c > 0) ? "," : "");

        if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering & MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_RAW)
            c += snprintf(buf+c, BUFSIZE-c, "%sraw", (c > 0) ? "," : "");

        if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering & MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_WAR)
            c += snprintf(buf+c, BUFSIZE-c, "%swar", (c > 0) ? "," : "");

        if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ordering & MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_WAW)
            c += snprintf(buf+c, BUFSIZE-c, "%swaw", (c > 0) ? "," : "");

        if(c == 0) {
            memcpy(&buf[0],"not set   ",10);
        }

        MPIR_Info_set_impl(*info_p_p, "accumulate_ordering", buf);
        MPIU_Assert(mpi_errno == MPI_SUCCESS);
#undef BUFSIZE
    }

    if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.accumulate_ops == MPIDI_CH4_NMI_OFI_ACCUMULATE_SAME_OP)
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "accumulate_ops", "same_op");
    else
        mpi_errno = MPIR_Info_set_impl(*info_p_p, "accumulate_ops", "same_op_no_op");

    MPIU_Assert(mpi_errno == MPI_SUCCESS);

    if(win->create_flavor == MPI_WIN_FLAVOR_SHARED) {
        if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.alloc_shared_noncontig)
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "alloc_shared_noncontig", "true");
        else
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "alloc_shared_noncontig", "false");

        MPIU_Assert(mpi_errno == MPI_SUCCESS);
    } else if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE) {
        if(MPIDI_CH4_NMI_OFI_WIN(win)->info_args.same_size)
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "same_size", "true");
        else
            mpi_errno = MPIR_Info_set_impl(*info_p_p, "same_size", "false");

        MPIU_Assert(mpi_errno == MPI_SUCCESS);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_GET_INFO);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_free
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_free(MPID_Win **win_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    MPID_Win      *win       = *win_ptr;
    uint32_t       window_instance;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FREE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FREE);

    MPIDI_CH4R_EPOCH_FREE_CHECK(win,mpi_errno,return mpi_errno);

    mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE  && win->base) {
        if(MPIDI_CH4_NMI_OFI_WIN(win)->mmap_sz > 0)
            munmap(MPIDI_CH4_NMI_OFI_WIN(win)->mmap_addr, MPIDI_CH4_NMI_OFI_WIN(win)->mmap_sz);
        else if(MPIDI_CH4_NMI_OFI_WIN(win)->mmap_sz == -1)
            MPIU_Free(win->base);
    }

    if(win->create_flavor == MPI_WIN_FLAVOR_SHARED) {
        if(MPIDI_CH4_NMI_OFI_WIN(win)->mmap_addr)
            munmap(MPIDI_CH4_NMI_OFI_WIN(win)->mmap_addr, MPIDI_CH4_NMI_OFI_WIN(win)->mmap_sz);

        MPIU_Free(MPIDI_CH4R_WIN(win, sizes));
    }

    if(MPIDI_CH4_NMI_OFI_WIN(win)->msgQ)
        MPIU_Free(MPIDI_CH4_NMI_OFI_WIN(win)->msgQ);

    window_instance       = (uint32_t)(MPIDI_CH4_NMI_OFI_WIN(win)->win_id>>32);
    MPIDI_CH4_NMI_OFI_Index_allocator_free(MPIDI_CH4_NMI_OFI_COMM(win->comm_ptr).win_id_allocator,
                                   window_instance);

    MPIDI_CH4_NMI_OFI_CALL(fi_close(&MPIDI_CH4_NMI_OFI_WIN(win)->mr->fid), mr_unreg);

    MPIDI_CH4R_win_finalize(win_ptr);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FREE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_fence
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_fence(int massert, MPID_Win *win)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FENCE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FENCE);

    MPIDI_CH4R_EPOCH_FENCE_CHECK(win, mpi_errno, goto fn_fail);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_Win_progress_fence(win));

    MPIDI_CH4R_EPOCH_FENCE_EVENT(win,massert);

    if(!(massert & MPI_MODE_NOPRECEDE))
        mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FENCE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_create(void       *base,
                                          MPI_Aint    length,
                                          int         disp_unit,
                                          MPID_Info  *info,
                                          MPID_Comm  *comm_ptr,
                                          MPID_Win  **win_ptr)
{
    int             mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t  errflag   = MPIR_ERR_NONE;
    MPID_Win       *win;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_CREATE);

    mpi_errno        = MPIDI_CH4_NMI_OFI_Win_init(length,
                                      disp_unit,
                                      win_ptr,
                                      info,
                                      comm_ptr,
                                      MPI_WIN_FLAVOR_CREATE,
                                      MPI_WIN_UNIFIED);

    if(mpi_errno != MPI_SUCCESS) goto fn_fail;

    win              = *win_ptr;
    win->base        = base;

    mpi_errno = MPIDI_CH4_NMI_OFI_Win_allgather(win,base,disp_unit,1);

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
#define FUNCNAME MPIDI_CH4_NM_win_attach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_attach(MPID_Win *win, void *base, MPI_Aint size)
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
#define FUNCNAME MPIDI_CH4_NM_win_allocate_shared
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_allocate_shared(MPI_Aint    size,
                                                   int         disp_unit,
                                                   MPID_Info  *info_ptr,
                                                   MPID_Comm  *comm_ptr,
                                                   void      **base_ptr,
                                                   MPID_Win  **win_ptr)
{
    int            i=0, fd,rc,first=0,mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    void           *baseP      = NULL;
    MPID_Win       *win        = NULL;
    ssize_t         total_size = 0LL;
    MPI_Aint       *sizes, size_out = 0;
    char            shm_key[64];
    void           *map_ptr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE_SHARED);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE_SHARED);

    mpi_errno = MPIDI_CH4_NMI_OFI_Win_init(size,disp_unit,win_ptr,info_ptr,comm_ptr,
                               MPI_WIN_FLAVOR_SHARED, MPI_WIN_UNIFIED);

    win                   = *win_ptr;
    MPIDI_CH4R_WIN(win, sizes) = (MPI_Aint *)MPIU_Malloc(sizeof(MPI_Aint)*comm_ptr->local_size);
    sizes                 = MPIDI_CH4R_WIN(win, sizes);
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
            MPIDI_CH4_NMI_OFI_WIN(win)->win_id);

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

    mapsize = MPIDI_CH4_NMI_OFI_Get_mapsize(total_size, &page_sz);
    fd      = rc;
    rc      = ftruncate(fd, mapsize);

    if(rc == -1) {
        close(fd);

        if(first)shm_unlink(shm_key);

        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                            goto fn_fail, "**nomem");
    }

    if(comm_ptr->rank == 0) {
        map_ptr = MPIDI_CH4_NMI_OFI_Generate_random_addr(mapsize);
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

        MPIDI_CH4_NMI_OFI_WIN(win)->mmap_addr = map_ptr;
        MPIDI_CH4_NMI_OFI_WIN(win)->mmap_sz   = mapsize;
    } else {
        mpi_errno = MPIR_Bcast_impl(&map_ptr,1,MPI_UNSIGNED_LONG,
                                    0,comm_ptr,&errflag);

        if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

        rc = MPIDI_CH4_NMI_OFI_Check_maprange(map_ptr,mapsize);
        /* If we hit this assert, we need to iterate
         * trying more addresses
         */
        MPIU_Assert(rc == 1);
        map_ptr = mmap(map_ptr, mapsize,
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED|MAP_FIXED,
                       fd, 0);
        MPIDI_CH4_NMI_OFI_WIN(win)->mmap_addr = map_ptr;
        MPIDI_CH4_NMI_OFI_WIN(win)->mmap_sz   = mapsize;

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

    mpi_errno = MPIDI_CH4_NMI_OFI_Win_allgather(win,baseP,disp_unit,0);

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
#define FUNCNAME MPIDI_CH4_NM_win_detach
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_detach(MPID_Win *win, const void *base)
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
#define FUNCNAME MPIDI_CH4_NM_win_shared_query
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_shared_query(MPID_Win *win,
                                                int       rank,
                                                MPI_Aint *size,
                                                int      *disp_unit,
                                                void     *baseptr)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_SHARED_QUERY);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_SHARED_QUERY);
    int offset = rank;

    if(rank < 0)
        offset = 0;

    *(void **)baseptr = MPIDI_CH4_NMI_OFI_WINFO_BASE_FORCE(win, offset);
    *size             = MPIDI_CH4R_WIN(win, sizes)[offset];
    *disp_unit        = MPIDI_CH4_NMI_OFI_WINFO_DISP_UNIT(win,offset);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_SHARED_QUERY);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_allocate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_allocate(MPI_Aint     size,
                                            int          disp_unit,
                                            MPID_Info   *info,
                                            MPID_Comm   *comm,
                                            void        *baseptr,
                                            MPID_Win   **win_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
    void           *baseP;
    MPID_Win       *win;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_ALLOCATE);

    mpi_errno = MPIDI_CH4_NMI_OFI_Win_init(size,disp_unit,win_ptr, info, comm,
                               MPI_WIN_FLAVOR_ALLOCATE, MPI_WIN_UNIFIED);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    mpi_errno = MPIDI_CH4_NMI_OFI_Get_symmetric_heap(size,comm,&baseP,*win_ptr);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    win       = *win_ptr;
    win->base =  baseP;
    mpi_errno = MPIDI_CH4_NMI_OFI_Win_allgather(win,baseP,disp_unit,1);

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
#define FUNCNAME MPIDI_CH4_NM_win_flush
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_flush(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FLUSH);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FLUSH);

    MPIDI_CH4R_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_Win_progress_fence(win));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FLUSH);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_flush_local_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_flush_local_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL_ALL);

    MPIDI_CH4R_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_Win_progress_fence(win));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_unlock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_unlock_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_UNLOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_UNLOCK_ALL);
    int i;
    MPIDI_CH4R_winLock_info *lockQ;

    MPIDI_CH4R_EPOCH_ORIGIN_CHECK(win, MPIDI_CH4R_EPOTYPE_LOCK_ALL, mpi_errno, return mpi_errno);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_Win_progress_fence(win));

    MPIU_Assert(MPIDI_CH4_NMI_OFI_WIN(win)->msgQ != NULL);

    lockQ = (MPIDI_CH4R_winLock_info *) MPIDI_CH4_NMI_OFI_WIN(win)->msgQ;

    for(i = 0; i < win->comm_ptr->local_size; i++) {
        MPIDI_CH4_NMI_OFI_Win_control_t msg;
        lockQ[i].done = 0;
        lockQ[i].peer = i;
        lockQ[i].win  = win;
        msg.type      = MPIDI_CH4_NMI_OFI_CTRL_UNLOCKALL;
        mpi_errno     = MPIDI_CH4_NMI_OFI_Do_control_win(&msg, lockQ[i].peer, win, 1, 1);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");

        if(MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked == 1)
            lockQ[i].done = 1;
    }

    MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked);

    MPIDI_CH4R_WIN(win, sync).origin_epoch_type = MPIDI_CH4R_EPOTYPE_NONE;
    MPIDI_CH4R_WIN(win, sync).target_epoch_type = MPIDI_CH4R_EPOTYPE_NONE;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_UNLOCK_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_create_dynamic
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_create_dynamic(MPID_Info *info,
                                                  MPID_Comm *comm,
                                                  MPID_Win **win_ptr)
{
    int             mpi_errno = MPI_SUCCESS;
    int             rc        = MPI_SUCCESS;
    MPIR_Errflag_t  errflag   = MPIR_ERR_NONE;
    MPID_Win       *win;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_CREATE_DYNAMIC);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_CREATE_DYNAMIC);

    rc = MPIDI_CH4_NMI_OFI_Win_init(UINTPTR_MAX-(uintptr_t)MPI_BOTTOM,
                        1,win_ptr, info, comm,
                        MPI_WIN_FLAVOR_DYNAMIC,
                        MPI_WIN_UNIFIED);

    if(rc != MPI_SUCCESS)
        goto fn_fail;

    win       = *win_ptr;
    win->base =  MPI_BOTTOM;

    rc = MPIDI_CH4_NMI_OFI_Win_allgather(win,win->base,1,1);

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
#define FUNCNAME MPIDI_CH4_NM_win_flush_local
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_flush_local(int rank, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL);

    MPIDI_CH4R_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_Win_progress_fence(win));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FLUSH_LOCAL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_sync
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_sync(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_SYNC);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_SYNC);

    MPIDI_CH4R_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);

    OPA_read_write_barrier();

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_SYNC);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_flush_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_flush_all(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_FLUSH_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_FLUSH_ALL);

    MPIDI_CH4R_EPOCH_LOCK_CHECK(win,mpi_errno,goto fn_fail);
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_Win_progress_fence(win));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_FLUSH_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_win_lock_all
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_win_lock_all(int assert, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_LOCK_ALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_LOCK_ALL);

    MPIDI_CH4R_EPOCH_CHECK_TYPE(win,mpi_errno,goto fn_fail);

    int size;
    size = win->comm_ptr->local_size;

    if(!MPIDI_CH4_NMI_OFI_WIN(win)->msgQ) {
        MPIDI_CH4_NMI_OFI_WIN(win)->msgQ = (void *) MPIU_Calloc(size, sizeof(MPIDI_CH4R_winLock_info));
        MPIU_Assert(MPIDI_CH4_NMI_OFI_WIN(win)->msgQ != NULL);
        MPIDI_CH4_NMI_OFI_WIN(win)->count=0;
    }

    MPIDI_CH4R_winLock_info *lockQ;
    lockQ = (MPIDI_CH4R_winLock_info *) MPIDI_CH4_NMI_OFI_WIN(win)->msgQ;
    int i;

    for(i = 0; i < size; i++) {
        MPIDI_CH4_NMI_OFI_Win_control_t msg;

        lockQ[i].done      = 0;
        lockQ[i].peer      = i;
        lockQ[i].win       = win;
        lockQ[i].lock_type = MPI_LOCK_SHARED;

        msg.type           = MPIDI_CH4_NMI_OFI_CTRL_LOCKALLREQ;
        msg.lock_type      = MPI_LOCK_SHARED;
        mpi_errno          = MPIDI_CH4_NMI_OFI_Do_control_win(&msg, lockQ[i].peer, lockQ[i].win, 1, 1);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");

        if(MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked == 1)
            lockQ[i].done = 1;
    }

    MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(size != (int)MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked);

    MPIDI_CH4R_WIN(win, sync).origin_epoch_type = MPIDI_CH4R_EPOTYPE_LOCK_ALL;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_LOCK_ALL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#endif /* NETMOD_OFI_WIN_H_INCLUDED */
