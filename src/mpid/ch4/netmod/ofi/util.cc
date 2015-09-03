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
#include <mpidimpl.h>
#include "impl.h"
#include "events.h"
EXTERN_C_BEGIN

#undef FUNCNAME
#define FUNCNAME MPIDI_OFI_VCRT_Create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_OFI_VCRT_Create(int size, struct MPIDI_VCRT **vcrt_ptr)
{
    struct MPIDI_VCRT *vcrt;
    int i,mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_VCRT_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_VCRT_CREATE);

    vcrt = (struct MPIDI_VCRT *)MPIU_Malloc(sizeof(struct MPIDI_VCRT) +
                                            size*sizeof(MPIDI_VCR));

    if(vcrt != NULL) {
        MPIU_Object_set_ref(vcrt, 1);
        vcrt->size = size;
        *vcrt_ptr = vcrt;

        for(i=0; i<size; i++)
            vcrt->vcr_table[i].addr_idx = i;

        mpi_errno = MPI_SUCCESS;
    } else
        mpi_errno = MPIR_ERR_MEMALLOCFAILED;

    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_VCRT_EXIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_OFI_VCRT_Release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_OFI_VCRT_Release(struct MPIDI_VCRT *vcrt)
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
void MPIDI_OFI_Map_create(void **_map)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    *_map = (void *)MPIU_Malloc(sizeof(uint64_map));
    new(*_map) uint64_map();
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
}

void MPIDI_OFI_Map_destroy(void *_map)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    m->~uint64_map();
    MPIU_Free(_map);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
}

void MPIDI_OFI_Map_set(void     *_map,
                       uint64_t  id,
                       void     *val)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    (*m)[id] = val;
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
}

void MPIDI_OFI_Map_erase(void     *_map,
                         uint64_t  id)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    m->erase(id);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
}

void *MPIDI_OFI_Map_lookup(void     *_map,
                           uint64_t  id)
{
    uint64_map *m = (uint64_map *)_map;
    void       *rc;

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    if(m->find(id) == m->end())
        rc = MPIDI_MAP_NOT_FOUND;
    else
        rc = (*m)[id];
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    return rc;
}

void MPIDI_OFI_build_nodemap(uint32_t       *in_nodeids,
                             MPID_Node_id_t *out_nodemap,
                             int             sz,
                             MPID_Node_id_t *sz_out)
{
    typedef std::multimap<uint32_t,uint32_t> nodemap;
    typedef std::pair<uint32_t,uint32_t>     nodepair;
    typedef nodemap::iterator                mapiter;
    typedef std::pair<mapiter, mapiter>      mappair;

    nodemap m;
    int i;

    for(i=0; i<sz; i++)
        m.insert(nodepair(in_nodeids[i],i));

    for(i=0; i<sz; i++)
        out_nodemap[i] = 0xFFFF;

    MPID_Node_id_t node_id = 0;

    for(mapiter it = m.begin(), end=m.end();
        it != end;
        it = m.upper_bound(it->first)) {
        mappair p;
        p = m.equal_range(it->first);

        for(mapiter it2 = p.first;
            it2 != p.second;
            it2++)
            out_nodemap[it2->second] = node_id;

        node_id++;
        MPIU_Assert(node_id != 0xFFFF);
    }

    *sz_out = node_id;
}

static inline int
WinLockAdvance(MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    struct MPIDI_Win_sync_lock *slock = &WIN_OFI(win)->sync.lock;
    struct MPIDI_Win_queue     *q     = &slock->local.requested;

    if(
        (q->head != NULL) &&
        ((slock->local.count == 0) ||
         (
             (slock->local.type == MPI_LOCK_SHARED) &&
             (q->head->type     == MPI_LOCK_SHARED)
         )
        )
    ) {
        struct MPIDI_Win_lock *lock = q->head;
        q->head = lock->next;

        if(q->head == NULL)
            q->tail = NULL;

        ++slock->local.count;
        slock->local.type = lock->type;

        if(lock->mtype == MPIDI_REQUEST_LOCK) {
            MPIDI_Win_control_t info;
            info.type       = MPIDI_CTRL_LOCKACK,
                 mpi_errno = do_control_win(&info,lock->rank,win,1);

            if(mpi_errno != MPI_SUCCESS)
                MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                    goto fn_fail, "**rmasync");

        } else if(lock->mtype == MPIDI_REQUEST_LOCKALL) {
            MPIDI_Win_control_t info;
            info.type = MPIDI_CTRL_LOCKALLACK;
            mpi_errno = do_control_win(&info,lock->rank,win,1);
        } else
            MPIU_Assert(0);

        MPIU_Free(lock);
        mpi_errno = WinLockAdvance(win);

        if(mpi_errno != MPI_SUCCESS)
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                                goto fn_fail, "**rmasync");
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline void WinLockReq_proc(const MPIDI_Win_control_t  *info,
                                   MPID_Win                   *win,
                                   unsigned                    peer)
{
    struct MPIDI_Win_lock *lock = (struct MPIDI_Win_lock *)MPIU_Calloc(1, sizeof(struct MPIDI_Win_lock));

    if(info->type == MPIDI_CTRL_LOCKREQ)
        lock->mtype = MPIDI_REQUEST_LOCK;
    else if(info->type == MPIDI_CTRL_LOCKALLREQ)
        lock->mtype = MPIDI_REQUEST_LOCKALL;

    lock->rank                = info->origin_rank;
    lock->type                = info->lock_type;
    struct MPIDI_Win_queue *q = &WIN_OFI(win)->sync.lock.local.requested;
    MPIU_Assert((q->head != NULL) ^ (q->tail == NULL));

    if(q->tail == NULL)
        q->head = lock;
    else
        q->tail->next = lock;

    q->tail = lock;

    WinLockAdvance(win);

}

static inline void WinLockAck_proc(const MPIDI_Win_control_t *info,
                                   MPID_Win                   *win,
                                   unsigned                    peer)
{
    if(info->type == MPIDI_CTRL_LOCKACK)
        WIN_OFI(win)->sync.lock.remote.locked = 1;
    else  if(info->type == MPIDI_CTRL_LOCKALLACK)
        WIN_OFI(win)->sync.lock.remote.allLocked += 1;
}


static inline void WinUnlock_proc(const MPIDI_Win_control_t *info,
                                  MPID_Win                   *win,
                                  unsigned                    peer)
{
    --WIN_OFI(win)->sync.lock.local.count;
    MPIU_Assert((int)WIN_OFI(win)->sync.lock.local.count >= 0);
    WinLockAdvance(win);

    MPIDI_Win_control_t new_info;
    new_info.type = MPIDI_CTRL_UNLOCKACK;

    do_control_win(&new_info,peer,win,1);
}

static inline void WinComplete_proc(const MPIDI_Win_control_t *info,
                                    MPID_Win                   *win,
                                    unsigned                    peer)
{
    ++WIN_OFI(win)->sync.sc.count;
}

static inline void WinPost_proc(const MPIDI_Win_control_t *info,
                                MPID_Win                   *win,
                                unsigned                    peer)
{
    ++WIN_OFI(win)->sync.pw.count;
}


static inline void WinUnlockDoneCB(const MPIDI_Win_control_t *info,
                                   MPID_Win                   *win,
                                   unsigned                    peer)
{
    if(WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_LOCK)
        WIN_OFI(win)->sync.lock.remote.locked = 0;
    else if(WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_LOCK_ALL) {
        MPIU_Assert((int)WIN_OFI(win)->sync.lock.remote.allLocked > 0);
        WIN_OFI(win)->sync.lock.remote.allLocked -= 1;
    } else
        MPIU_Assert(0);

}

static inline void MPIDI_Gethuge_cleanup(MPIDI_Send_control_t *info)
{
    MPIDI_Huge_recv_t  *recv;
    MPID_Comm          *comm_ptr;
    uint64_t            mapid;
    /* Look up the communicator */
    mapid = ((uint64_t)info->endpoint_id<<32) | info->comm_id;
    comm_ptr              = (MPID_Comm *)MPIDI_OFI_Map_lookup(MPIDI_Global.comm_map,mapid);

    /* Look up the per destination receive queue object */
    recv = (MPIDI_Huge_recv_t *)MPIDI_OFI_Map_lookup(COMM_OFI(comm_ptr).huge_recv_counters,
                                                     info->origin_rank);
    MPIDI_OFI_Map_destroy(recv->chunk_q);
    MPIDI_OFI_Map_erase(COMM_OFI(comm_ptr).huge_recv_counters,info->origin_rank);
    MPIU_Free(recv);
}

static inline void MPIDI_Gethuge(MPIDI_Send_control_t *info)
{
    MPIDI_Huge_recv_t  *recv;
    MPIDI_Huge_chunk_t *hc;
    MPID_Comm          *comm_ptr;
    /* Look up the communicator */
    comm_ptr              = (MPID_Comm *)MPIDI_OFI_Map_lookup(MPIDI_Global.comm_map,
                                                              info->comm_id);
    /* Look up the per destination receive queue object */
    recv = (MPIDI_Huge_recv_t *)MPIDI_OFI_Map_lookup(COMM_OFI(comm_ptr).huge_recv_counters,
                                                     info->origin_rank);

    if(recv == MPIDI_MAP_NOT_FOUND) {
        recv        = (MPIDI_Huge_recv_t *)MPIU_Malloc(sizeof(*recv));
        recv->seqno = 0;
        MPIDI_OFI_Map_create(&recv->chunk_q);
        MPIDI_OFI_Map_set(COMM_OFI(comm_ptr).huge_recv_counters,
                          info->origin_rank, recv);
    }

    hc = (MPIDI_Huge_chunk_t *)MPIDI_OFI_Map_lookup(recv->chunk_q,info->seqno);

    if(hc == MPIDI_MAP_NOT_FOUND) {
        hc = (MPIDI_Huge_chunk_t *)MPIU_Malloc(sizeof(*hc));
        memset(hc,0, sizeof(*hc));
        hc->event_id = MPIDI_EVENT_GET_HUGE;
        MPIDI_OFI_Map_set(recv->chunk_q,info->seqno,hc);
    }

    hc->cur_offset     = MPIDI_Global.max_send;
    hc->remote_info    = *info;
    hc->comm_ptr       = comm_ptr;
    get_huge_event(NULL, (MPID_Request *)hc);
}

int MPIDI_OFI_control_dispatch(void *buf)
{
    int mpi_errno = MPI_SUCCESS;
    int senderrank;

    MPIDI_Win_control_t *control = (MPIDI_Win_control_t *)buf;

    switch(control->type) {
        case MPIDI_CTRL_HUGEACK: {
            MPIDI_Send_control_t *ctrlsend = (MPIDI_Send_control_t *)buf;
            dispatch_function(NULL,ctrlsend->ackreq);
            goto fn_exit;
        }
        break;

        case MPIDI_CTRL_HUGE: {
            MPIDI_Send_control_t *ctrlsend = (MPIDI_Send_control_t *)buf;
            MPIDI_Gethuge(ctrlsend);
            goto fn_exit;
        }
        break;

        case MPIDI_CTRL_HUGE_CLEANUP: {
            MPIDI_Send_control_t *ctrlsend = (MPIDI_Send_control_t *)buf;
            MPIDI_Gethuge_cleanup(ctrlsend);
            goto fn_exit;
        }
        break;
    }

    MPID_Win *win;
    senderrank = control->origin_rank;
    win        = (MPID_Win *)MPIDI_OFI_Map_lookup(MPIDI_Global.win_map,control->win_id);
    MPIU_Assert(win != MPIDI_MAP_NOT_FOUND);

    switch(control->type) {
        case MPIDI_CTRL_LOCKREQ:
        case MPIDI_CTRL_LOCKALLREQ:
            WinLockReq_proc(control, win, senderrank);
            break;

        case MPIDI_CTRL_LOCKACK:
        case MPIDI_CTRL_LOCKALLACK:
            WinLockAck_proc(control, win, senderrank);
            break;

        case MPIDI_CTRL_UNLOCK:
        case MPIDI_CTRL_UNLOCKALL:
            WinUnlock_proc(control, win, senderrank);
            break;

        case MPIDI_CTRL_UNLOCKACK:
        case MPIDI_CTRL_UNLOCKALLACK:
            WinUnlockDoneCB(control, win, senderrank);
            break;

        case MPIDI_CTRL_COMPLETE:
            WinComplete_proc(control, win, senderrank);
            break;

        case MPIDI_CTRL_POST:
            WinPost_proc(control, win, senderrank);
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
#define FCNAME DECL_FUNC(MPIDI_Datatype_to_fi)
static inline int MPIDI_Datatype_to_fi(MPI_Datatype  dt,
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
  ret = fcn(G_TXC_RMA(0),                          \
    fi_dt,                                 \
    fi_op,                                 \
            &atomic_count);                        \
  if(ret == 0 && atomic_count != 0)                \
    {                                              \
  _TBL.field1 = 1;                             \
  _TBL.field2 = atomic_count;                  \
    }

static inline void MPIDI_Create_dt_map()
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

    for(i=0; i<DT_SIZES; i++)
        for(j=0; j<OP_SIZES; j++) {
            enum fi_datatype fi_dt=(enum fi_datatype)-1;
            enum fi_op       fi_op=(enum fi_op)-1;
            MPIDI_Datatype_to_fi(mpi_dtypes[i],
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

#define ADD_INDEX(datatype)                     \
  ({                                            \
  MPID_Datatype_get_ptr(datatype, dt_ptr);    \
  DT_OFI(dt_ptr)->index=index++;              \
  })

void MPIDI_OFI_Index_datatypes()
{
    MPID_Datatype *dt_ptr;
    int            index=0;

    ADD_INDEX(MPI_CHAR);
    ADD_INDEX(MPI_UNSIGNED_CHAR);
    ADD_INDEX(MPI_SIGNED_CHAR);
    ADD_INDEX(MPI_BYTE);
    ADD_INDEX(MPI_WCHAR);
    ADD_INDEX(MPI_SHORT);
    ADD_INDEX(MPI_UNSIGNED_SHORT);
    ADD_INDEX(MPI_INT);
    ADD_INDEX(MPI_UNSIGNED);
    ADD_INDEX(MPI_LONG);
    ADD_INDEX(MPI_UNSIGNED_LONG);        /* 10 */

    ADD_INDEX(MPI_FLOAT);
    ADD_INDEX(MPI_DOUBLE);
    ADD_INDEX(MPI_LONG_DOUBLE);
    ADD_INDEX(MPI_LONG_LONG);
    ADD_INDEX(MPI_UNSIGNED_LONG_LONG);
    ADD_INDEX(MPI_PACKED);
    ADD_INDEX(MPI_LB);
    ADD_INDEX(MPI_UB);
    ADD_INDEX(MPI_2INT);

    /* C99 types */
    ADD_INDEX(MPI_INT8_T); /* 20 */           ADD_INDEX(MPI_INT16_T);
    ADD_INDEX(MPI_INT32_T);
    ADD_INDEX(MPI_INT64_T);
    ADD_INDEX(MPI_UINT8_T);
    ADD_INDEX(MPI_UINT16_T);
    ADD_INDEX(MPI_UINT32_T);
    ADD_INDEX(MPI_UINT64_T);
    ADD_INDEX(MPI_C_BOOL);
    ADD_INDEX(MPI_C_FLOAT_COMPLEX);
    ADD_INDEX(MPI_C_DOUBLE_COMPLEX); /* 30 */ ADD_INDEX(MPI_C_LONG_DOUBLE_COMPLEX);

    /* address/offset/count types */
    ADD_INDEX(MPI_AINT);
    ADD_INDEX(MPI_OFFSET);
    ADD_INDEX(MPI_COUNT);

    /* Fortran types */
#ifdef HAVE_FORTRAN_BINDING
    ADD_INDEX(MPI_COMPLEX);
    ADD_INDEX(MPI_DOUBLE_COMPLEX);
    ADD_INDEX(MPI_LOGICAL);
    ADD_INDEX(MPI_REAL);
    ADD_INDEX(MPI_DOUBLE_PRECISION);
    ADD_INDEX(MPI_INTEGER); /* 40 */
    ADD_INDEX(MPI_2INTEGER);
#ifdef MPICH_DEFINE_2COMPLEX
    ADD_INDEX(MPI_2COMPLEX);
    ADD_INDEX(MPI_2DOUBLE_COMPLEX);
#endif
    ADD_INDEX(MPI_2REAL);
    ADD_INDEX(MPI_2DOUBLE_PRECISION);
    ADD_INDEX(MPI_CHARACTER);
    ADD_INDEX(MPI_REAL4);
    ADD_INDEX(MPI_REAL8);
    ADD_INDEX(MPI_REAL16);
    ADD_INDEX(MPI_COMPLEX8); /* 50 */ ADD_INDEX(MPI_COMPLEX16);
    ADD_INDEX(MPI_COMPLEX32);
    ADD_INDEX(MPI_INTEGER1);
    ADD_INDEX(MPI_INTEGER2);
    ADD_INDEX(MPI_INTEGER4);
    ADD_INDEX(MPI_INTEGER8);

    if(MPI_INTEGER16 == MPI_DATATYPE_NULL)
        index++;
    else
        ADD_INDEX(MPI_INTEGER16);

#endif
    ADD_INDEX(MPI_FLOAT_INT);
    ADD_INDEX(MPI_DOUBLE_INT);
    ADD_INDEX(MPI_LONG_INT);
    ADD_INDEX(MPI_SHORT_INT);/* 60 */
    ADD_INDEX(MPI_LONG_DOUBLE_INT);
    MPIDI_Create_dt_map();
}

EXTERN_C_END
