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

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <map>
#include <queue>
#include <new>
#include <algorithm>
#include <functional>
#include <vector>
#include <mpidimpl.h>
#include "impl.h"
#include "events.h"
#include "control.h"
EXTERN_C_BEGIN

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Handle_cq_error_util
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int   MPIDI_CH4_NMI_OFI_Handle_cq_error_util(ssize_t ret) {
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_HANDLE_CQ_ERROR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_HANDLE_CQ_ERROR);

    mpi_errno = MPIDI_CH4_NMI_OFI_Handle_cq_error(ret);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_HANDLE_CQ_ERROR);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_VCRT_Create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_CH4_NMI_OFI_VCRT_Create(int size, struct MPIDI_CH4_NMI_OFI_VCRT **vcrt_ptr)
{
    struct MPIDI_CH4_NMI_OFI_VCRT *vcrt;
    int i,mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_VCRT_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_VCRT_CREATE);

    vcrt = (struct MPIDI_CH4_NMI_OFI_VCRT *)MPIU_Malloc(sizeof(struct MPIDI_CH4_NMI_OFI_VCRT) +
                                            size*sizeof(MPIDI_CH4_NMI_OFI_VCR));

    if(vcrt != NULL) {
        MPIU_Object_set_ref(vcrt, 1);
        vcrt->size = size;
        *vcrt_ptr = vcrt;

        for(i=0; i<size; i++)
            vcrt->vcr_table[i].addr_idx = i;
        for(i=0; i<size; i++)
            vcrt->vcr_table[i].is_local = 0;

        mpi_errno = MPI_SUCCESS;
    } else
        mpi_errno = MPIR_ERR_MEMALLOCFAILED;

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_VCRT_EXIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_VCRT_Release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_CH4_NMI_OFI_VCRT_Release(struct MPIDI_CH4_NMI_OFI_VCRT *vcrt)
{
    int count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_VCRT_RELEASE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_VCRT_RELEASE);
    MPIU_Object_release_ref(vcrt, &count);

    if(count == 0)
        MPIU_Free(vcrt);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_VCRT_RELEASE);
    return MPI_SUCCESS;
}

typedef std::map<uint64_t,void *>  uint64_map;
void MPIDI_CH4_NMI_OFI_Map_create(void **_map)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    *_map = (void *)MPIU_Malloc(sizeof(uint64_map));
    new(*_map) uint64_map();
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
}

void MPIDI_CH4_NMI_OFI_Map_destroy(void *_map)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    m->~uint64_map();
    MPIU_Free(_map);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
}

void MPIDI_CH4_NMI_OFI_Map_set(void     *_map,
                       uint64_t  id,
                       void     *val)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    (*m)[id] = val;
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
}

void MPIDI_CH4_NMI_OFI_Map_erase(void     *_map,
                         uint64_t  id)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    m->erase(id);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
}

void *MPIDI_CH4_NMI_OFI_Map_lookup(void     *_map,
                           uint64_t  id)
{
    uint64_map *m = (uint64_map *)_map;
    void       *rc;

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    if(m->find(id) == m->end())
        rc = MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND;
    else
        rc = (*m)[id];
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    return rc;
}


typedef struct {
    int              high;
    std::vector<int> free_pool;
} MPIDI_CH4_NMI_OFI_Index_allocator_t;

void MPIDI_CH4_NMI_OFI_Index_allocator_create(void **_indexmap, int start)
{
    MPIDI_CH4_NMI_OFI_Index_allocator_t *alloc;
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    *_indexmap  = (void *)MPIU_Malloc(sizeof(MPIDI_CH4_NMI_OFI_Index_allocator_t));
    alloc       = (MPIDI_CH4_NMI_OFI_Index_allocator_t *)(*_indexmap);
    alloc->high = start;
    new(&alloc->free_pool)std::vector<int>();
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
}

int MPIDI_CH4_NMI_OFI_Index_allocator_alloc(void *_indexmap)
{
    MPIDI_CH4_NMI_OFI_Index_allocator_t *alloc = (MPIDI_CH4_NMI_OFI_Index_allocator_t*)_indexmap;
    int back;
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    if (alloc->free_pool.empty()) {
        int alloc_high=alloc->high++;
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
        return alloc_high;
    }
    std::pop_heap(alloc->free_pool.begin(), alloc->free_pool.end(), std::greater<int>());
    back = alloc->free_pool.back();
    alloc->free_pool.pop_back();
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    return back;

}
void MPIDI_CH4_NMI_OFI_Index_allocator_free(void *_indexmap, int index)
{
    MPIDI_CH4_NMI_OFI_Index_allocator_t *alloc = (MPIDI_CH4_NMI_OFI_Index_allocator_t*)_indexmap;
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    alloc->free_pool.push_back(index);
    std::push_heap(alloc->free_pool.begin(), alloc->free_pool.end(), std::greater<int>());
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
}

void MPIDI_CH4_NMI_OFI_Index_allocator_destroy(void *_indexmap)
{
    MPIDI_CH4_NMI_OFI_Index_allocator_t *alloc = (MPIDI_CH4_NMI_OFI_Index_allocator_t*)_indexmap;
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
    alloc->free_pool.~vector();
    MPIU_Free(_indexmap);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX);
}

static inline int
MPIDI_CH4_NMI_OFI_Win_lock_advance(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    struct MPIDI_CH4R_win_sync_lock *slock = &MPIDI_CH4R_WIN(win, sync).lock;
    struct MPIDI_CH4R_win_queue     *q     = &slock->local.requested;

    if(
        (q->head != NULL) &&
        ((slock->local.count == 0) ||
         (
             (slock->local.type == MPI_LOCK_SHARED) &&
             (q->head->type     == MPI_LOCK_SHARED)
         )
        )
    ) {
        struct MPIDI_CH4R_win_lock *lock = q->head;
        q->head = lock->next;

        if(q->head == NULL)
            q->tail = NULL;

        ++slock->local.count;
        slock->local.type = lock->type;

        if(lock->mtype == MPIDI_CH4_NMI_OFI_REQUEST_LOCK) {
            MPIDI_CH4_NMI_OFI_Win_control_t info;
            info.type   = MPIDI_CH4_NMI_OFI_CTRL_LOCKACK;
            mpi_errno   = MPIDI_CH4_NMI_OFI_Do_control_win(&info,lock->rank,win,1,0);
            if(mpi_errno != MPI_SUCCESS)
                MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                    goto fn_fail, "**rmasync");

        } else if(lock->mtype == MPIDI_CH4_NMI_OFI_REQUEST_LOCKALL) {
            MPIDI_CH4_NMI_OFI_Win_control_t info;
            info.type = MPIDI_CH4_NMI_OFI_CTRL_LOCKALLACK;
            mpi_errno = MPIDI_CH4_NMI_OFI_Do_control_win(&info,lock->rank,win,1,0);
        } else
            MPIU_Assert(0);

        MPIU_Free(lock);
        mpi_errno = MPIDI_CH4_NMI_OFI_Win_lock_advance(win);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline void MPIDI_CH4_NMI_OFI_Win_lock_request_proc(const MPIDI_CH4_NMI_OFI_Win_control_t  *info,
                                   MPID_Win                   *win,
                                   unsigned                    peer)
{
    struct MPIDI_CH4R_win_lock *lock =
        (struct MPIDI_CH4R_win_lock *)MPIU_Calloc(1, sizeof(struct MPIDI_CH4R_win_lock));

    if(info->type == MPIDI_CH4_NMI_OFI_CTRL_LOCKREQ)
        lock->mtype = MPIDI_CH4_NMI_OFI_REQUEST_LOCK;
    else if(info->type == MPIDI_CH4_NMI_OFI_CTRL_LOCKALLREQ)
        lock->mtype = MPIDI_CH4_NMI_OFI_REQUEST_LOCKALL;

    lock->rank                = info->origin_rank;
    lock->type                = info->lock_type;
    struct MPIDI_CH4R_win_queue *q = &MPIDI_CH4R_WIN(win, sync).lock.local.requested;
    MPIU_Assert((q->head != NULL) ^ (q->tail == NULL));

    if(q->tail == NULL)
        q->head = lock;
    else
        q->tail->next = lock;

    q->tail = lock;

    MPIDI_CH4_NMI_OFI_Win_lock_advance(win);

}

static inline void MPIDI_CH4_NMI_OFI_Win_lock_ack_proc(const MPIDI_CH4_NMI_OFI_Win_control_t *info,
                                   MPID_Win                   *win,
                                   unsigned                    peer)
{
    if(info->type == MPIDI_CH4_NMI_OFI_CTRL_LOCKACK)
        MPIDI_CH4R_WIN(win, sync).lock.remote.locked = 1;
    else  if(info->type == MPIDI_CH4_NMI_OFI_CTRL_LOCKALLACK)
        MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked += 1;
}


static inline void MPIDI_CH4_NMI_OFI_Win_unlock_proc(const MPIDI_CH4_NMI_OFI_Win_control_t *info,
                                  MPID_Win                   *win,
                                  unsigned                    peer)
{
    --MPIDI_CH4R_WIN(win, sync).lock.local.count;
    MPIU_Assert((int)MPIDI_CH4R_WIN(win, sync).lock.local.count >= 0);
    MPIDI_CH4_NMI_OFI_Win_lock_advance(win);

    MPIDI_CH4_NMI_OFI_Win_control_t new_info;
    new_info.type = MPIDI_CH4_NMI_OFI_CTRL_UNLOCKACK;
    MPIDI_CH4_NMI_OFI_Do_control_win(&new_info,peer,win,1,0);
}

static inline void MPIDI_CH4_NMI_OFI_Win_complete_proc(const MPIDI_CH4_NMI_OFI_Win_control_t *info,
                                    MPID_Win                   *win,
                                    unsigned                    peer)
{
    ++MPIDI_CH4R_WIN(win, sync).sc.count;
}

static inline void MPIDI_CH4_NMI_OFI_Win_post_proc(const MPIDI_CH4_NMI_OFI_Win_control_t *info,
                                MPID_Win                   *win,
                                unsigned                    peer)
{
    ++MPIDI_CH4R_WIN(win, sync).pw.count;
}


static inline void MPIDI_CH4_NMI_OFI_Win_unlock_done_proc(const MPIDI_CH4_NMI_OFI_Win_control_t *info,
                                   MPID_Win                   *win,
                                   unsigned                    peer)
{
    if(MPIDI_CH4R_WIN(win, sync).origin_epoch_type == MPIDI_CH4R_EPOTYPE_LOCK)
        MPIDI_CH4R_WIN(win, sync).lock.remote.locked = 0;
    else if(MPIDI_CH4R_WIN(win, sync).origin_epoch_type == MPIDI_CH4R_EPOTYPE_LOCK_ALL) {
        MPIU_Assert((int)MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked > 0);
        MPIDI_CH4R_WIN(win, sync).lock.remote.allLocked -= 1;
    } else
        MPIU_Assert(0);

}

static inline void MPIDI_CH4_NMI_OFI_Get_huge_cleanup(MPIDI_CH4_NMI_OFI_Send_control_t *info)
{
    MPIDI_CH4_NMI_OFI_Huge_recv_t  *recv;
    MPID_Comm          *comm_ptr;
    uint64_t            mapid;
    /* Look up the communicator */
    mapid    = ((uint64_t)info->endpoint_id<<32) | info->comm_id;
    comm_ptr = MPIDI_CH4R_context_id_to_comm(mapid);
    /* Look up the per destination receive queue object */
    recv = (MPIDI_CH4_NMI_OFI_Huge_recv_t *)MPIDI_CH4_NMI_OFI_Map_lookup(MPIDI_CH4_NMI_OFI_COMM(comm_ptr).huge_recv_counters,
                                                     info->origin_rank);
    MPIDI_CH4_NMI_OFI_Map_destroy(recv->chunk_q);
    MPIDI_CH4_NMI_OFI_Map_erase(MPIDI_CH4_NMI_OFI_COMM(comm_ptr).huge_recv_counters,info->origin_rank);
    MPIU_Free(recv);
}

static inline void MPIDI_CH4_NMI_OFI_Get_huge(MPIDI_CH4_NMI_OFI_Send_control_t *info)
{
    MPIDI_CH4_NMI_OFI_Huge_recv_t  *recv;
    MPIDI_CH4_NMI_OFI_Huge_chunk_t *hc;
    MPID_Comm          *comm_ptr;
    /* Look up the communicator */
    comm_ptr = MPIDI_CH4R_context_id_to_comm(info->comm_id);
    /* Look up the per destination receive queue object */
    recv = (MPIDI_CH4_NMI_OFI_Huge_recv_t *)MPIDI_CH4_NMI_OFI_Map_lookup(MPIDI_CH4_NMI_OFI_COMM(comm_ptr).huge_recv_counters,
                                                     info->origin_rank);
    if(recv == MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND) {
        recv        = (MPIDI_CH4_NMI_OFI_Huge_recv_t *)MPIU_Malloc(sizeof(*recv));
        recv->seqno = 0;
        MPIDI_CH4_NMI_OFI_Map_create(&recv->chunk_q);
        MPIDI_CH4_NMI_OFI_Map_set(MPIDI_CH4_NMI_OFI_COMM(comm_ptr).huge_recv_counters,
                          info->origin_rank, recv);
    }

    hc = (MPIDI_CH4_NMI_OFI_Huge_chunk_t *)MPIDI_CH4_NMI_OFI_Map_lookup(recv->chunk_q,info->seqno);

    if(hc == MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND) {
        hc = (MPIDI_CH4_NMI_OFI_Huge_chunk_t *)MPIU_Malloc(sizeof(*hc));
        memset(hc,0, sizeof(*hc));
        hc->event_id = MPIDI_CH4_NMI_OFI_EVENT_GET_HUGE;
        MPIDI_CH4_NMI_OFI_Map_set(recv->chunk_q,info->seqno,hc);
    }

    hc->cur_offset     = MPIDI_Global.max_send;
    hc->remote_info    = *info;
    hc->comm_ptr       = comm_ptr;
    MPIDI_CH4_NMI_OFI_Get_huge_event(NULL, (MPID_Request *)hc);
}

int MPIDI_CH4_NMI_OFI_Control_handler(void      *am_hdr,
                              size_t     am_hdr_sz,
                              uint64_t   reply_token,
                              void     **data,
                              size_t    *data_sz,
                              int       *is_contig,
                              MPIDI_CH4_NM_am_completion_handler_fn *cmpl_handler_fn,
                              MPID_Request ** req)
{
    int                  senderrank;
    int                  mpi_errno  = MPI_SUCCESS;
    void                *buf        = am_hdr;
    MPIDI_CH4_NMI_OFI_Win_control_t *control    = (MPIDI_CH4_NMI_OFI_Win_control_t *)buf;
    *req                            = NULL;
    *cmpl_handler_fn                = NULL;
    *data                           = NULL;
    *data_sz                        = 0;
    *is_contig                      = 1;

    switch(control->type) {
        case MPIDI_CH4_NMI_OFI_CTRL_HUGEACK: {
            MPIDI_CH4_NMI_OFI_Send_control_t *ctrlsend = (MPIDI_CH4_NMI_OFI_Send_control_t *)buf;
            MPIDI_CH4_NMI_OFI_Dispatch_function(NULL,ctrlsend->ackreq,0);
            goto fn_exit;
        }
        break;

        case MPIDI_CH4_NMI_OFI_CTRL_HUGE: {
            MPIDI_CH4_NMI_OFI_Send_control_t *ctrlsend = (MPIDI_CH4_NMI_OFI_Send_control_t *)buf;
            MPIDI_CH4_NMI_OFI_Get_huge(ctrlsend);
            goto fn_exit;
        }
        break;

        case MPIDI_CH4_NMI_OFI_CTRL_HUGE_CLEANUP: {
            MPIDI_CH4_NMI_OFI_Send_control_t *ctrlsend = (MPIDI_CH4_NMI_OFI_Send_control_t *)buf;
            MPIDI_CH4_NMI_OFI_Get_huge_cleanup(ctrlsend);
            goto fn_exit;
        }
        break;
    }

    MPID_Win *win;
    senderrank = control->origin_rank;
    win        = (MPID_Win *)MPIDI_CH4_NMI_OFI_Map_lookup(MPIDI_Global.win_map,control->win_id);
    MPIU_Assert(win != MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND);

    switch(control->type) {
        case MPIDI_CH4_NMI_OFI_CTRL_LOCKREQ:
        case MPIDI_CH4_NMI_OFI_CTRL_LOCKALLREQ:
            MPIDI_CH4_NMI_OFI_Win_lock_request_proc(control, win, senderrank);
            break;

        case MPIDI_CH4_NMI_OFI_CTRL_LOCKACK:
        case MPIDI_CH4_NMI_OFI_CTRL_LOCKALLACK:
            MPIDI_CH4_NMI_OFI_Win_lock_ack_proc(control, win, senderrank);
            break;

        case MPIDI_CH4_NMI_OFI_CTRL_UNLOCK:
        case MPIDI_CH4_NMI_OFI_CTRL_UNLOCKALL:
            MPIDI_CH4_NMI_OFI_Win_unlock_proc(control, win, senderrank);
            break;

        case MPIDI_CH4_NMI_OFI_CTRL_UNLOCKACK:
        case MPIDI_CH4_NMI_OFI_CTRL_UNLOCKALLACK:
            MPIDI_CH4_NMI_OFI_Win_unlock_done_proc(control, win, senderrank);
            break;

        case MPIDI_CH4_NMI_OFI_CTRL_COMPLETE:
            MPIDI_CH4_NMI_OFI_Win_complete_proc(control, win, senderrank);
            break;

        case MPIDI_CH4_NMI_OFI_CTRL_POST:
            MPIDI_CH4_NMI_OFI_Win_post_proc(control, win, senderrank);
            break;

        default:
            fprintf(stderr, "Bad control type: 0x%08x  %d\n",
                    control->type,
                    control->type);
            MPIU_Assert(0);
    }

fn_exit:
    return mpi_errno;
}


/* MPI Datatype Processing for RMA */
#define isS_INT(x) ( (x)==MPI_INTEGER ||                                \
    (x) == MPI_INT32_T || (x) == MPI_INTEGER4 ||       \
                     (x) == MPI_INT)
#define isUS_INT(x) ( (x) == MPI_UINT32_T || (x) == MPI_UNSIGNED)
#define isS_SHORT(x) ( (x) == MPI_SHORT || (x) == MPI_INT16_T ||        \
                       (x) == MPI_INTEGER2)
#define isUS_SHORT(x) ( (x) == MPI_UNSIGNED_SHORT || (x) == MPI_UINT16_T)
#define isS_CHAR(x) ( (x) == MPI_SIGNED_CHAR || (x) == MPI_INT8_T ||    \
                      (x) == MPI_INTEGER1 || (x) == MPI_CHAR)
#define isUS_CHAR(x) ( (x) == MPI_BYTE ||                               \
                       (x) == MPI_UNSIGNED_CHAR || (x) == MPI_UINT8_T )
#define isS_LONG(x) ( (x) == MPI_LONG || (x) == MPI_AINT )
#define isUS_LONG(x) ( (x) == MPI_UNSIGNED_LONG )
#define isS_LONG_LONG(x) ( (x) == MPI_INT64_T || (x) == MPI_OFFSET ||   \
    (x) == MPI_INTEGER8 || (x) == MPI_LONG_LONG || \
                           (x) == MPI_LONG_LONG_INT || (x) == MPI_COUNT )
#define isUS_LONG_LONG(x) ( (x) == MPI_UINT64_T || (x) == MPI_UNSIGNED_LONG_LONG )
#define isFLOAT(x) ( (x) == MPI_FLOAT || (x) == MPI_REAL)
#define isDOUBLE(x) ( (x) == MPI_DOUBLE || (x) == MPI_DOUBLE_PRECISION)
#define isLONG_DOUBLE(x) ( (x) == MPI_LONG_DOUBLE )
#define isLOC_TYPE(x) ( (x) == MPI_2REAL || (x) == MPI_2DOUBLE_PRECISION || \
    (x) == MPI_2INTEGER || (x) == MPI_FLOAT_INT ||  \
    (x) == MPI_DOUBLE_INT || (x) == MPI_LONG_INT || \
    (x) == MPI_2INT || (x) == MPI_SHORT_INT ||      \
                        (x) == MPI_LONG_DOUBLE_INT )
#define isBOOL(x) ( (x) == MPI_C_BOOL )
#define isLOGICAL(x) ( (x) == MPI_LOGICAL )
#define isSINGLE_COMPLEX(x) ( (x) == MPI_COMPLEX || (x) == MPI_C_FLOAT_COMPLEX)
#define isDOUBLE_COMPLEX(x) ( (x) == MPI_DOUBLE_COMPLEX || (x) == MPI_COMPLEX8 || \
                              (x) == MPI_C_DOUBLE_COMPLEX)


#undef FCNAME
#define FCNAME DECL_FUNC(mpi_to_ofi)
static inline int mpi_to_ofi(MPI_Datatype  dt,
                             fi_datatype  *fi_dt,
                             MPI_Op        op,
                             fi_op        *fi_op)
{
    *fi_dt = FI_DATATYPE_LAST;
    *fi_op = FI_ATOMIC_OP_LAST;

    if(isS_INT(dt))               *fi_dt = FI_INT32;
    else if(isUS_INT(dt))         *fi_dt = FI_UINT32;
    else if(isFLOAT(dt))          *fi_dt = FI_FLOAT;
    else if(isDOUBLE(dt))         *fi_dt = FI_DOUBLE;
    else if(isLONG_DOUBLE(dt))    *fi_dt = FI_LONG_DOUBLE;
    else if(isS_CHAR(dt))         *fi_dt = FI_INT8;
    else if(isUS_CHAR(dt))        *fi_dt = FI_UINT8;
    else if(isS_SHORT(dt))        *fi_dt = FI_INT16;
    else if(isUS_SHORT(dt))       *fi_dt = FI_UINT16;
    else if(isS_LONG(dt))         *fi_dt = FI_INT64;
    else if(isUS_LONG(dt))        *fi_dt = FI_UINT64;
    else if(isS_LONG_LONG(dt))    *fi_dt = FI_INT64;
    else if(isUS_LONG_LONG(dt))   *fi_dt = FI_UINT64;
    else if(isSINGLE_COMPLEX(dt)) *fi_dt = FI_FLOAT_COMPLEX;
    else if(isDOUBLE_COMPLEX(dt)) *fi_dt = FI_DOUBLE_COMPLEX;
    else if(isLOC_TYPE(dt))       *fi_dt = FI_DATATYPE_LAST;
    else if(isLOGICAL(dt))        *fi_dt = FI_UINT32;
    else if(isBOOL(dt))           *fi_dt = FI_UINT8;

    if(*fi_dt == FI_DATATYPE_LAST)  goto fn_fail;

    *fi_op = FI_ATOMIC_OP_LAST;

    switch(op) {
        case MPI_SUM:
            *fi_op = FI_SUM;
            goto fn_exit;
            break;

        case MPI_PROD:
            *fi_op = FI_PROD;
            goto fn_exit;
            break;

        case MPI_MAX:
            *fi_op = FI_MAX;
            goto fn_exit;
            break;

        case MPI_MIN:
            *fi_op = FI_MIN;
            goto fn_exit;
            break;

        case MPI_BAND:
            *fi_op = FI_BAND;
            goto fn_exit;
            break;

        case MPI_BOR:
            *fi_op = FI_BOR;
            goto fn_exit;
            break;

        case MPI_BXOR:
            *fi_op = FI_BXOR;
            goto fn_exit;
            break;

        case MPI_LAND:
            if(isLONG_DOUBLE(dt)) goto fn_fail;

            *fi_op = FI_LAND;
            goto fn_exit;
            break;

        case MPI_LOR:
            if(isLONG_DOUBLE(dt)) goto fn_fail;

            *fi_op = FI_LOR;
            goto fn_exit;
            break;

        case MPI_LXOR:
            if(isLONG_DOUBLE(dt)) goto fn_fail;

            *fi_op = FI_LXOR;
            goto fn_exit;
            break;

        case MPI_REPLACE: {
            *fi_op = FI_ATOMIC_WRITE;
            goto fn_exit;
            break;
        }

        case MPI_NO_OP: {
            *fi_op = FI_ATOMIC_READ;
            goto fn_exit;
            break;
        }

        case MPI_OP_NULL: {
            *fi_op = FI_CSWAP;
            goto fn_exit;
            break;
        }

        default:
            goto fn_fail;
            break;
    }

fn_exit:
    return MPI_SUCCESS;
fn_fail:
    return -1;
}

static MPI_Datatype mpi_dtypes[] = {
    MPI_CHAR,     MPI_UNSIGNED_CHAR, MPI_SIGNED_CHAR,    MPI_BYTE,
    MPI_WCHAR,    MPI_SHORT,         MPI_UNSIGNED_SHORT, MPI_INT,
    MPI_UNSIGNED, MPI_LONG,          MPI_UNSIGNED_LONG,  MPI_FLOAT,
    MPI_DOUBLE,   MPI_LONG_DOUBLE,   MPI_LONG_LONG,      MPI_UNSIGNED_LONG_LONG,
    MPI_PACKED,   MPI_LB,            MPI_UB,MPI_2INT,

    MPI_INT8_T,           MPI_INT16_T,          MPI_INT32_T,
    MPI_INT64_T,          MPI_UINT8_T,          MPI_UINT16_T,
    MPI_UINT32_T,         MPI_UINT64_T,         MPI_C_BOOL,
    MPI_C_FLOAT_COMPLEX,  MPI_C_DOUBLE_COMPLEX, MPI_C_LONG_DOUBLE_COMPLEX,
    /* address/offset/count types */
    MPI_AINT,MPI_OFFSET,MPI_COUNT,
    /* Fortran types */
#ifdef HAVE_FORTRAN_BINDING
    MPI_COMPLEX,          MPI_DOUBLE_COMPLEX, MPI_LOGICAL, MPI_REAL,
    MPI_DOUBLE_PRECISION, MPI_INTEGER,        MPI_2INTEGER,

#ifdef MPICH_DEFINE_2COMPLEX
    MPI_2COMPLEX, MPI_2DOUBLE_COMPLEX,
#endif
    MPI_2REAL, MPI_2DOUBLE_PRECISION, MPI_CHARACTER,
    MPI_REAL4,     MPI_REAL8,    MPI_REAL16,   MPI_COMPLEX8, MPI_COMPLEX16,
    MPI_COMPLEX32, MPI_INTEGER1, MPI_INTEGER2, MPI_INTEGER4, MPI_INTEGER8,
    MPI_INTEGER16,
#endif
    MPI_FLOAT_INT, MPI_DOUBLE_INT,
    MPI_LONG_INT,  MPI_SHORT_INT,
    MPI_LONG_DOUBLE_INT,
    (MPI_Datatype) -1,
};

static MPI_Op mpi_ops[] = {
    MPI_MAX,     MPI_MIN,   MPI_SUM,     MPI_PROD,
    MPI_LAND,    MPI_BAND,  MPI_LOR,     MPI_BOR,
    MPI_LXOR,    MPI_BXOR,  MPI_MINLOC,  MPI_MAXLOC,
    MPI_REPLACE, MPI_NO_OP, MPI_OP_NULL,
};

#define _TBL MPIDI_Global.win_op_table[i][j]
#define CHECK_ATOMIC(fcn,field1,field2)            \
  atomic_count = 0;                                \
  ret = fcn(MPIDI_CH4_NMI_OFI_EP_TX_RMA(0),                          \
    fi_dt,                                 \
    fi_op,                                 \
            &atomic_count);                        \
  if(ret == 0 && atomic_count != 0)                \
    {                                              \
  _TBL.field1 = 1;                             \
  _TBL.field2 = atomic_count;                  \
    }

static inline void create_dt_map()
{
    int i, j;
    size_t dtsize[FI_DATATYPE_LAST];
    dtsize[FI_INT8]                = sizeof(int8_t);
    dtsize[FI_UINT8]               = sizeof(uint8_t);
    dtsize[FI_INT16]               = sizeof(int16_t);
    dtsize[FI_UINT16]              = sizeof(uint16_t);
    dtsize[FI_INT32]               = sizeof(int32_t);
    dtsize[FI_UINT32]              = sizeof(uint32_t);
    dtsize[FI_INT64]               = sizeof(int64_t);
    dtsize[FI_UINT64]              = sizeof(uint64_t);
    dtsize[FI_FLOAT]               = sizeof(float);
    dtsize[FI_DOUBLE]              = sizeof(double);
    dtsize[FI_FLOAT_COMPLEX]       = sizeof(float complex);
    dtsize[FI_DOUBLE_COMPLEX]      = sizeof(double complex);
    dtsize[FI_LONG_DOUBLE]         = sizeof(long double);
    dtsize[FI_LONG_DOUBLE_COMPLEX] = sizeof(long double complex);

    for(i=0; i<MPIDI_CH4_NMI_OFI_DT_SIZES; i++)
        for(j=0; j<MPIDI_CH4_NMI_OFI_OP_SIZES; j++) {
            enum fi_datatype fi_dt=(enum fi_datatype)-1;
            enum fi_op       fi_op=(enum fi_op)-1;
            mpi_to_ofi(mpi_dtypes[i],
                                 &fi_dt,
                                 mpi_ops[j],
                                 &fi_op);
            MPIU_Assert(fi_dt != (enum fi_datatype)-1);
            MPIU_Assert(fi_op != (enum fi_op)-1);
            _TBL.dt                       = fi_dt;
            _TBL.op                       = fi_op;
            _TBL.atomic_valid             = 0;
            _TBL.max_atomic_count         = 0;
            _TBL.max_fetch_atomic_count   = 0;
            _TBL.max_compare_atomic_count = 0;
            ssize_t ret;
            size_t  atomic_count;

            if(fi_dt!=FI_DATATYPE_LAST && fi_op !=FI_ATOMIC_OP_LAST) {
                CHECK_ATOMIC(fi_atomicvalid,atomic_valid,max_atomic_count);
                CHECK_ATOMIC(fi_fetch_atomicvalid,fetch_atomic_valid,max_fetch_atomic_count);
                CHECK_ATOMIC(fi_compare_atomicvalid,compare_atomic_valid,max_compare_atomic_count);
                _TBL.dtsize = dtsize[fi_dt];
            }
        }
}

static inline void add_index(MPI_Datatype  datatype,
                             int          *index)
{
    MPID_Datatype *dt_ptr;
    MPID_Datatype_get_ptr(datatype, dt_ptr);
    MPIDI_CH4_NMI_OFI_DATATYPE(dt_ptr)->index=*index;
    (*index)++;
}

void MPIDI_CH4_NMI_OFI_Index_datatypes()
{
    int            index=0;

    add_index(MPI_CHAR,&index);
    add_index(MPI_UNSIGNED_CHAR,&index);
    add_index(MPI_SIGNED_CHAR,&index);
    add_index(MPI_BYTE,&index);
    add_index(MPI_WCHAR,&index);
    add_index(MPI_SHORT,&index);
    add_index(MPI_UNSIGNED_SHORT,&index);
    add_index(MPI_INT,&index);
    add_index(MPI_UNSIGNED,&index);
    add_index(MPI_LONG,&index);
    add_index(MPI_UNSIGNED_LONG,&index);        /* 10 */

    add_index(MPI_FLOAT,&index);
    add_index(MPI_DOUBLE,&index);
    add_index(MPI_LONG_DOUBLE,&index);
    add_index(MPI_LONG_LONG,&index);
    add_index(MPI_UNSIGNED_LONG_LONG,&index);
    add_index(MPI_PACKED,&index);
    add_index(MPI_LB,&index);
    add_index(MPI_UB,&index);
    add_index(MPI_2INT,&index);

    /* C99 types */
    add_index(MPI_INT8_T,&index); /* 20 */           add_index(MPI_INT16_T,&index);
    add_index(MPI_INT32_T,&index);
    add_index(MPI_INT64_T,&index);
    add_index(MPI_UINT8_T,&index);
    add_index(MPI_UINT16_T,&index);
    add_index(MPI_UINT32_T,&index);
    add_index(MPI_UINT64_T,&index);
    add_index(MPI_C_BOOL,&index);
    add_index(MPI_C_FLOAT_COMPLEX,&index);
    add_index(MPI_C_DOUBLE_COMPLEX,&index); /* 30 */ add_index(MPI_C_LONG_DOUBLE_COMPLEX,&index);

    /* address/offset/count types */
    add_index(MPI_AINT,&index);
    add_index(MPI_OFFSET,&index);
    add_index(MPI_COUNT,&index);

    /* Fortran types */
#ifdef HAVE_FORTRAN_BINDING
    add_index(MPI_COMPLEX,&index);
    add_index(MPI_DOUBLE_COMPLEX,&index);
    add_index(MPI_LOGICAL,&index);
    add_index(MPI_REAL,&index);
    add_index(MPI_DOUBLE_PRECISION,&index);
    add_index(MPI_INTEGER,&index); /* 40 */
    add_index(MPI_2INTEGER,&index);
#ifdef MPICH_DEFINE_2COMPLEX
    add_index(MPI_2COMPLEX,&index);
    add_index(MPI_2DOUBLE_COMPLEX,&index);
#endif
    add_index(MPI_2REAL,&index);
    add_index(MPI_2DOUBLE_PRECISION,&index);
    add_index(MPI_CHARACTER,&index);
    add_index(MPI_REAL4,&index);
    add_index(MPI_REAL8,&index);
    add_index(MPI_REAL16,&index);
    add_index(MPI_COMPLEX8,&index); /* 50 */ add_index(MPI_COMPLEX16,&index);
    add_index(MPI_COMPLEX32,&index);
    add_index(MPI_INTEGER1,&index);
    add_index(MPI_INTEGER2,&index);
    add_index(MPI_INTEGER4,&index);
    add_index(MPI_INTEGER8,&index);

    if(MPI_INTEGER16 == MPI_DATATYPE_NULL)
        index++;
    else
        add_index(MPI_INTEGER16,&index);

#endif
    add_index(MPI_FLOAT_INT,&index);
    add_index(MPI_DOUBLE_INT,&index);
    add_index(MPI_LONG_INT,&index);
    add_index(MPI_SHORT_INT,&index);/* 60 */
    add_index(MPI_LONG_DOUBLE_INT,&index);
    create_dt_map();
}

EXTERN_C_END
