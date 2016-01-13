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
#ifndef NETMOD_OFI_RMA_H_INCLUDED
#define NETMOD_OFI_RMA_H_INCLUDED

#include "impl.h"

#define MPIDI_QUERY_ATOMIC_COUNT 0
#define MPIDI_QUERY_FETCH_ATOMIC_COUNT 1
#define MPIDI_QUERY_COMPARE_ATOMIC_COUNT 2

#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
#define CONDITIONAL_GLOBAL_CNTR_INCR()
#define GLOBAL_CNTR_INCR() MPIDI_Global.cntr++
#else
#define CONDITIONAL_GLOBAL_CNTR_INCR() MPIDI_Global.cntr++
#define GLOBAL_CNTR_INCR() MPIDI_Global.cntr++
#endif

#define SETUP_CHUNK_CONTEXT(sigreq)                             \
  ({                                                            \
      if(sigreq)                                                \
      {                                                         \
        int tmp;                                                \
        MPIDI_Chunk_request *creq;                              \
        MPID_cc_incr((*sigreq)->cc_ptr, &tmp);                  \
        creq=(MPIDI_Chunk_request*)MPIU_Malloc(sizeof(*creq));  \
        creq->event_id = MPIDI_EVENT_CHUNK_DONE;                \
        creq->parent   = *sigreq;                               \
        msg.context    = &creq->context;                        \
        CONDITIONAL_GLOBAL_CNTR_INCR();                         \
      }                                                         \
    else GLOBAL_CNTR_INCR();                                    \
  })

#define SETUP_SIGNAL_REQUEST(sigreq,flags,ep)                   \
  ({                                                            \
      if(sigreq)                                                \
      {                                                         \
        REQ_CREATE((*(sigreq)));                                \
        (*sigreq)->kind             = MPID_WIN_REQUEST;         \
        MPID_cc_set((*(sigreq))->cc_ptr, 0);                    \
        *(flags)                    = FI_COMPLETION;            \
        *(ep)                       = G_TXC_RMA(0);             \
      }                                                         \
    else {                                                      \
        *(ep) = G_TXC_CTR(0);                                   \
        *(flags)                    = 0ULL;                     \
    }                                                           \
  })

#define GET_BASIC_TYPE(a,b)                             \
  ({                                                    \
    if (MPIR_DATATYPE_IS_PREDEFINED(a))                 \
      b = a;                                            \
    else {                                              \
      MPID_Datatype *dt_ptr;                            \
      MPID_Datatype_get_ptr(a,dt_ptr);                  \
      b = dt_ptr->basic_type;                           \
    }                                                   \
  })

#undef FUNCNAME
#define FUNCNAME MPIDI_Query_dt
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_Query_dt(MPI_Datatype   dt,
                                 fi_datatype_t *fi_dt,
                                 MPI_Op         op,
                                 fi_op_t       *fi_op,
                                 size_t        *count,
                                 size_t        *dtsize)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_QUERY_DT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_QUERY_DT);
    MPID_Datatype *dt_ptr;
    int            op_index,dt_index, rc;

    MPID_Datatype_get_ptr(dt,dt_ptr);

    /* OP_NULL is the oddball                          */
    /* todo...change configure to table this correctly */
    dt_index = DT_OFI(dt_ptr)->index;

    if(op == MPI_OP_NULL) op_index = 14;
    else op_index = (0x000000FFU & op) - 1;

    *fi_dt  = (fi_datatype_t)MPIDI_Global.win_op_table[dt_index][op_index].dt;
    *fi_op  = (fi_op_t)MPIDI_Global.win_op_table[dt_index][op_index].op;
    *dtsize = MPIDI_Global.win_op_table[dt_index][op_index].dtsize;

    if(*count == MPIDI_QUERY_ATOMIC_COUNT)
        *count = MPIDI_Global.win_op_table[dt_index][op_index].max_atomic_count;

    if(*count == MPIDI_QUERY_FETCH_ATOMIC_COUNT)
        *count = MPIDI_Global.win_op_table[dt_index][op_index].max_fetch_atomic_count;

    if(*count == MPIDI_QUERY_COMPARE_ATOMIC_COUNT)
        *count = MPIDI_Global.win_op_table[dt_index][op_index].max_compare_atomic_count;

    if( ((int) *fi_dt) == -1 || ((int) *fi_op) == -1)
        rc = -1;
    else
        rc = MPI_SUCCESS;

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_QUERY_DT);
    return rc;
}


static inline void MPIDI_Win_datatype_basic(int           count,
                                            MPI_Datatype  datatype,
                                            MPIDI_Win_dt *dt)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_DATATYPE_BASIC);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_DATATYPE_BASIC);

    if(datatype != MPI_DATATYPE_NULL)
        MPIDI_Datatype_get_info(dt->count = count,
                                dt->type  = datatype,
                                dt->contig,
                                dt->size,
                                dt->pointer,
                                dt->true_lb);
    else
        memset(dt, 0, sizeof(*dt));

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_DATATYPE_BASIC);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Win_datatype_map
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_Win_datatype_map(MPIDI_Win_dt *dt)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_WIN_DATATYPE_MAP);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_WIN_DATATYPE_MAP);

    if(dt->contig) {
        dt->num_contig              = 1;
        dt->map                     = &dt->__map;
        dt->map[0].DLOOP_VECTOR_BUF = (void *)(size_t)dt->true_lb;
        dt->map[0].DLOOP_VECTOR_LEN = dt->size;
    } else {
        unsigned map_size = dt->pointer->max_contig_blocks*dt->count + 1;
        dt->num_contig    = map_size;
        dt->map           = (DLOOP_VECTOR *)MPIU_Malloc(map_size * sizeof(DLOOP_VECTOR));
        MPIU_Assert(dt->map != NULL);

        MPID_Segment seg;
        DLOOP_Offset last = dt->pointer->size*dt->count;
        MPID_Segment_init(NULL, dt->count, dt->type, &seg, 0);
        MPID_Segment_pack_vector(&seg, 0, &last, dt->map, &dt->num_contig);
        MPIU_Assert((unsigned)dt->num_contig <= map_size);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_DATATYPE_MAP);
}



__ALWAYS_INLINE__ int MPIDI_Allocate_win_request_put_get(int                 origin_count,
                                                         int                 target_count,
                                                         int                 target_rank,
                                                         MPI_Datatype        origin_datatype,
                                                         MPI_Datatype        target_datatype,
                                                         MPIDI_Win_request **winreq,
                                                         uint64_t           *flags,
                                                         fid_ep_t           *ep,
                                                         MPID_Request      **sigreq)
{
    int   mpi_errno = MPI_SUCCESS;
    size_t o_size, t_size;
    MPIDI_Win_request *req;

    o_size=sizeof(iovec_t);
    t_size=sizeof(rma_iov_t);
    MPIDI_AM_Win_request_alloc_and_init(req,1,o_size+t_size);
    *winreq = req;

    req->noncontig->buf.iov.put_get.originv = (iovec_t*)&req->noncontig->buf.iov_store[0];
    req->noncontig->buf.iov.put_get.targetv = (rma_iov_t*)&req->noncontig->buf.iov_store[o_size];
    SETUP_SIGNAL_REQUEST(sigreq, flags, ep);
    MPIDI_Win_datatype_basic(origin_count,
                             origin_datatype,
                             &req->noncontig->origin_dt);
    MPIDI_Win_datatype_basic(target_count,
                             target_datatype,
                             &req->noncontig->target_dt);
    MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size),
                        mpi_errno,MPI_ERR_SIZE,"**rmasize");

    MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
    MPIDI_Win_datatype_map(&req->noncontig->target_dt);
    req->target_rank = target_rank;
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

__ALWAYS_INLINE__ int MPIDI_Allocate_win_request_accumulate(int                 origin_count,
                                                            int                 target_count,
                                                            int                 target_rank,
                                                            MPI_Datatype        origin_datatype,
                                                            MPI_Datatype        target_datatype,
                                                            MPIDI_Win_request **winreq,
                                                            uint64_t           *flags,
                                                            fid_ep_t           *ep,
                                                            MPID_Request      **sigreq)
{
    int   mpi_errno = MPI_SUCCESS;
    size_t o_size, t_size;
    MPIDI_Win_request *req;

    o_size=sizeof(ioc_t);
    t_size=sizeof(rma_ioc_t);
    MPIDI_AM_Win_request_alloc_and_init(req,1,o_size+t_size);
    *winreq = req;

    req->noncontig->buf.iov.accumulate.originv = (ioc_t*)&req->noncontig->buf.iov_store[0];
    req->noncontig->buf.iov.accumulate.targetv = (rma_ioc_t*)&req->noncontig->buf.iov_store[o_size];
    SETUP_SIGNAL_REQUEST(sigreq, flags, ep);
    MPIDI_Win_datatype_basic(origin_count,
                             origin_datatype,
                             &req->noncontig->origin_dt);
    MPIDI_Win_datatype_basic(target_count,
                             target_datatype,
                             &req->noncontig->target_dt);
    MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size),
                        mpi_errno,MPI_ERR_SIZE,"**rmasize");

    MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
    MPIDI_Win_datatype_map(&req->noncontig->target_dt);
    req->target_rank = target_rank;
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

__ALWAYS_INLINE__ int MPIDI_Allocate_win_request_get_accumulate(int                 origin_count,
                                                                int                 target_count,
                                                                int                 result_count,
                                                                int                 target_rank,
                                                                MPI_Op              op,
                                                                MPI_Datatype        origin_datatype,
                                                                MPI_Datatype        target_datatype,
                                                                MPI_Datatype        result_datatype,
                                                                MPIDI_Win_request **winreq,
                                                                uint64_t           *flags,
                                                                fid_ep_t           *ep,
                                                                MPID_Request      **sigreq)
{
    int   mpi_errno = MPI_SUCCESS;
    size_t o_size, t_size, r_size;
    MPIDI_Win_request *req;

    o_size=sizeof(ioc_t);
    t_size=sizeof(rma_ioc_t);
    r_size=sizeof(ioc_t);
    MPIDI_AM_Win_request_alloc_and_init(req,1,o_size+t_size+r_size);
    *winreq = req;

    req->noncontig->buf.iov.get_accumulate.originv = (ioc_t*)&req->noncontig->buf.iov_store[0];
    req->noncontig->buf.iov.get_accumulate.targetv = (rma_ioc_t*)&req->noncontig->buf.iov_store[o_size];
    req->noncontig->buf.iov.get_accumulate.resultv = (ioc_t*)&req->noncontig->buf.iov_store[o_size+t_size];
    SETUP_SIGNAL_REQUEST(sigreq, flags, ep);
    MPIDI_Win_datatype_basic(origin_count,origin_datatype,&req->noncontig->origin_dt);
    MPIDI_Win_datatype_basic(target_count,target_datatype,&req->noncontig->target_dt);
    MPIDI_Win_datatype_basic(result_count,result_datatype,&req->noncontig->result_dt);

    MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size && op != MPI_NO_OP),
                        mpi_errno, MPI_ERR_SIZE, "**rmasize");
    MPIR_ERR_CHKANDJUMP((req->noncontig->result_dt.size != req->noncontig->target_dt.size),
                        mpi_errno, MPI_ERR_SIZE, "**rmasize");

    if(op != MPI_NO_OP) MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
    MPIDI_Win_datatype_map(&req->noncontig->target_dt);
    MPIDI_Win_datatype_map(&req->noncontig->result_dt);

    req->target_rank = target_rank;
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME do_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_put(const void    *origin_addr,
                         int            origin_count,
                         MPI_Datatype   origin_datatype,
                         int            target_rank,
                         MPI_Aint       target_disp,
                         int            target_count,
                         MPI_Datatype   target_datatype,
                         MPID_Win      *win,
                         MPID_Request **sigreq)
{
    int                rc,mpi_errno = MPI_SUCCESS;
    MPIDI_Win_request *req;
    size_t             offset,omax,tmax,tout,oout;
    uint64_t           flags;
    fid_ep_t           ep;
    msg_rma_t          msg;
    unsigned           i;
    iovec_t           *originv;
    rma_iov_t         *targetv;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_PUT);

    MPIDI_CH4_NMI_MPI_RC_POP(MPIDI_Allocate_win_request_put_get(origin_count,
                                                           target_count,
                                                           target_rank,
                                                           origin_datatype,
                                                           target_datatype,
                                                           &req,&flags,&ep,sigreq));

    offset   = target_disp * WINFO_DISP_UNIT(win,target_rank);

    req->event_id          = MPIDI_EVENT_ABORT;
    msg.desc               = NULL;
    msg.addr               = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
    msg.context            = NULL;
    msg.data               = 0;
    req->next              = WIN_OFI(win)->syncQ;
    WIN_OFI(win)->syncQ    = req;
    MPIDI_Init_iovec_state(&req->noncontig->iovs,
                           (uintptr_t)origin_addr,
                           (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                           req->noncontig->origin_dt.num_contig,
                           req->noncontig->target_dt.num_contig,
                           INT64_MAX,
                           req->noncontig->origin_dt.map,
                           req->noncontig->target_dt.map);
    rc = MPIDI_IOV_EAGAIN;
    while(rc==MPIDI_IOV_EAGAIN) {
        originv   = req->noncontig->buf.iov.put_get.originv;
        targetv   = req->noncontig->buf.iov.put_get.targetv;
        omax=tmax = 1;

        rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,originv,
                                  omax,targetv,tmax,&oout,&tout);

        if(rc==MPIDI_IOV_DONE) break;

        for(i=0; i<tout; i++) targetv[i].key = WINFO_MR_KEY(win,target_rank);

        MPIU_Assert(rc != MPIDI_IOV_ERROR);
        msg.msg_iov       = originv;
        msg.iov_count     = oout;
        msg.rma_iov       = targetv;
        msg.rma_iov_count = tout;
        FI_RC_RETRY2(SETUP_CHUNK_CONTEXT(sigreq),
                     fi_writemsg(ep, &msg, flags),
                     rdma_write);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_PUT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_put(const void   *origin_addr,
                                   int           origin_count,
                                   MPI_Datatype  origin_datatype,
                                   int           target_rank,
                                   MPI_Aint      target_disp,
                                   int           target_count,
                                   MPI_Datatype  target_datatype,
                                   MPID_Win     *win)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_PUT);
    int            target_contig,origin_contig,mpi_errno = MPI_SUCCESS;
    MPIDI_msg_sz_t target_bytes,origin_bytes;
    MPI_Aint       origin_true_lb,target_true_lb;
    size_t         offset;

    MPIDI_CH4R_EPOCH_CHECK_SYNC(win,mpi_errno,goto fn_fail);
    MPIDI_CH4R_EPOCH_START_CHECK(win,mpi_errno,goto fn_fail);

    MPIDI_Datatype_check_contig_size_lb(target_datatype,target_count,
                                        target_contig,target_bytes,
                                        target_true_lb);
    MPIDI_Datatype_check_contig_size_lb(origin_datatype,origin_count,
                                        origin_contig,origin_bytes,
                                        origin_true_lb);

    MPIR_ERR_CHKANDJUMP((origin_bytes != target_bytes),mpi_errno,MPI_ERR_SIZE,"**rmasize");

    if(unlikely((origin_bytes == 0) ||(target_rank == MPI_PROC_NULL)))
        goto fn_exit;

    if(target_rank == win->comm_ptr->rank) {
        offset   = target_disp * WINFO_DISP_UNIT(win,target_rank);
        mpi_errno = MPIR_Localcopy(origin_addr,
                                   origin_count,
                                   origin_datatype,
                                   (char *)win->base + offset,
                                   target_count,
                                   target_datatype);
        goto fn_exit;
    }

    if(origin_contig && target_contig && origin_bytes <= MPIDI_Global.max_buffered_write) {
        FI_RC_RETRY2(GLOBAL_CNTR_INCR(),
                     fi_inject_write(G_TXC_CTR(0),(char *)origin_addr+origin_true_lb,
                                     target_bytes,_comm_to_phys(win->comm_ptr,target_rank,MPIDI_API_CTR),
                                     (uint64_t)(char *)WINFO_BASE(win,target_rank)+
                                     target_disp*WINFO_DISP_UNIT(win,target_rank)+target_true_lb,
                                     WINFO_MR_KEY(win,target_rank)),
                     rdma_inject_write);
    } else {
        mpi_errno = do_put(origin_addr,
                           origin_count,
                           origin_datatype,
                           target_rank,
                           target_disp,
                           target_count,
                           target_datatype,
                           win,
                           NULL);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_PUT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME do_get
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_get(void          *origin_addr,
                         int            origin_count,
                         MPI_Datatype   origin_datatype,
                         int            target_rank,
                         MPI_Aint       target_disp,
                         int            target_count,
                         MPI_Datatype   target_datatype,
                         MPID_Win      *win,
                         MPID_Request **sigreq)
{
    int                rc, mpi_errno = MPI_SUCCESS;
    MPIDI_Win_request *req;
    size_t             offset,omax,tmax,tout,oout;
    uint64_t           flags;
    fid_ep_t           ep;
    msg_rma_t          msg;
    iovec_t           *originv;
    rma_iov_t         *targetv;
    unsigned           i;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_GET);

    MPIDI_CH4_NMI_MPI_RC_POP(MPIDI_Allocate_win_request_put_get(origin_count,target_count,
                                                           target_rank,
                                                           origin_datatype,target_datatype,
                                                           &req,&flags,&ep,sigreq));

    offset                 = target_disp * WINFO_DISP_UNIT(win,target_rank);
    req->event_id          = MPIDI_EVENT_ABORT;
    msg.desc               = NULL;
    msg.addr               = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
    msg.context            = NULL;
    msg.data               = 0;
    req->next              = WIN_OFI(win)->syncQ;
    WIN_OFI(win)->syncQ    = req;
    MPIDI_Init_iovec_state(&req->noncontig->iovs,
                           (uintptr_t)origin_addr,
                           (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                           req->noncontig->origin_dt.num_contig,
                           req->noncontig->target_dt.num_contig,
                           INT64_MAX,
                           req->noncontig->origin_dt.map,
                           req->noncontig->target_dt.map);
    rc = MPIDI_IOV_EAGAIN;
    while(rc==MPIDI_IOV_EAGAIN) {
        originv=req->noncontig->buf.iov.put_get.originv;
        targetv=req->noncontig->buf.iov.put_get.targetv;
        omax=tmax=1;
        rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,originv,
                                  omax,targetv,tmax,&oout,&tout);

        if(rc==MPIDI_IOV_DONE) break;

        MPIU_Assert(rc != MPIDI_IOV_ERROR);

        for(i=0; i<tout; i++) targetv[i].key = WINFO_MR_KEY(win,target_rank);

        msg.msg_iov       = originv;
        msg.iov_count     = oout;
        msg.rma_iov       = targetv;
        msg.rma_iov_count = tout;
        FI_RC_RETRY2(SETUP_CHUNK_CONTEXT(sigreq),
                     fi_readmsg(ep, &msg, flags),
                     rdma_write);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_GET);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_get
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_get(void         *origin_addr,
                                   int           origin_count,
                                   MPI_Datatype  origin_datatype,
                                   int           target_rank,
                                   MPI_Aint      target_disp,
                                   int           target_count,
                                   MPI_Datatype  target_datatype,
                                   MPID_Win     *win)
{
    int            origin_contig,target_contig, mpi_errno = MPI_SUCCESS;
    MPIDI_Win_dt   origin_dt, target_dt;
    MPIDI_msg_sz_t origin_bytes;
    size_t         offset;
    rma_iov_t      riov;
    iovec_t        iov;
    msg_rma_t      msg;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_GET);

    MPIDI_CH4R_EPOCH_CHECK_SYNC(win,mpi_errno,goto fn_fail);
    MPIDI_CH4R_EPOCH_START_CHECK(win,mpi_errno,goto fn_fail);

    MPIDI_Datatype_check_contig_size(origin_datatype,origin_count,
                                     origin_contig,origin_bytes);
    if(unlikely((origin_bytes == 0) || (target_rank == MPI_PROC_NULL)))
        goto fn_exit;

    if(target_rank == win->comm_ptr->rank) {
        offset    = target_disp * WINFO_DISP_UNIT(win,target_rank);
        mpi_errno = MPIR_Localcopy((char *)win->base + offset,
                                   target_count,
                                   target_datatype,
                                   origin_addr,
                                   origin_count,
                                   origin_datatype);
    }

    MPIDI_Datatype_check_contig(origin_datatype,origin_contig);
    MPIDI_Datatype_check_contig(target_datatype,target_contig);
    if(origin_contig && target_contig) {
        offset = target_disp * WINFO_DISP_UNIT(win,target_rank);
        MPIDI_Win_datatype_basic(origin_count,origin_datatype,&origin_dt);
        MPIDI_Win_datatype_basic(target_count,target_datatype,&target_dt);
        MPIR_ERR_CHKANDJUMP((origin_dt.size != target_dt.size),
                            mpi_errno, MPI_ERR_SIZE, "**rmasize");

        msg.desc          = NULL;
        msg.msg_iov       = &iov;
        msg.iov_count     = 1;
        msg.addr          = _comm_to_phys(win->comm_ptr,target_rank,MPIDI_API_CTR);
        msg.rma_iov       = &riov;
        msg.rma_iov_count = 1;
        msg.context       = NULL;
        msg.data          = 0;
        iov.iov_base      = (char *)origin_addr + origin_dt.true_lb;
        iov.iov_len       = target_dt.size;
        riov.addr         = (uint64_t)((char *)WINFO_BASE(win,target_rank) + offset + target_dt.true_lb);
        riov.len          = target_dt.size;
        riov.key          = WINFO_MR_KEY(win,target_rank);
        FI_RC_RETRY2(GLOBAL_CNTR_INCR(),
                     fi_readmsg(G_TXC_CTR(0), &msg, 0),
                     rdma_write);
    }
    else {
        mpi_errno = do_get(origin_addr,
                           origin_count,
                           origin_datatype,
                           target_rank,
                           target_disp,
                           target_count,
                           target_datatype,
                           win,
                           NULL);
    }
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_GET);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_rput
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_rput(const void   *origin_addr,
                                    int           origin_count,
                                    MPI_Datatype  origin_datatype,
                                    int           target_rank,
                                    MPI_Aint      target_disp,
                                    int           target_count,
                                    MPI_Datatype  target_datatype,
                                    MPID_Win      *win,
                                    MPID_Request **request)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RPUT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RPUT);
    int            mpi_errno;
    MPIDI_msg_sz_t origin_bytes;
    size_t         offset;
    MPID_Request  *rreq;

    MPIDI_Datatype_check_size(origin_datatype,origin_count,origin_bytes);

    if(unlikely((origin_bytes == 0) ||(target_rank == MPI_PROC_NULL))) {
        rreq       = MPIDI_Request_alloc_and_init(2);
        rreq->kind = MPID_WIN_REQUEST;
        MPIDI_Request_complete(rreq);
        goto fn_exit;
    }

    if(target_rank == win->comm_ptr->rank) {
        rreq       = MPIDI_Request_alloc_and_init(2);
        rreq->kind = MPID_WIN_REQUEST;
        offset     = target_disp * WINFO_DISP_UNIT(win,target_rank);
        mpi_errno = MPIR_Localcopy(origin_addr,
                                   origin_count,
                                   origin_datatype,
                                   (char *)win->base + offset,
                                   target_count,
                                   target_datatype);
        MPIDI_Request_complete(rreq);
        goto fn_exit;
    }

    mpi_errno = do_put((void *)origin_addr,
                       origin_count,
                       origin_datatype,
                       target_rank,
                       target_disp,
                       target_count,
                       target_datatype,
                       win,
                       &rreq);

fn_exit:
    *request = rreq;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RPUT);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_compare_and_swap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_compare_and_swap(const void *origin_addr,
                                                const void *compare_addr,
                                                void *result_addr,
                                                MPI_Datatype datatype,
                                                int target_rank,
                                                MPI_Aint target_disp,
                                                MPID_Win *win)
{
    int            mpi_errno = MPI_SUCCESS;
    fi_op_t        fi_op;
    fi_datatype_t  fi_dt;
    MPIDI_Win_dt   origin_dt, target_dt, result_dt;
    size_t         offset,max_size,dt_size;
    void          *buffer, *tbuffer, *rbuffer;
    ioc_t          originv, resultv, comparev;
    rma_ioc_t      targetv;
    msg_atomic_t   msg;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_COMPARE_AND_SWAP);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_COMPARE_AND_SWAP);

    MPIDI_CH4R_EPOCH_CHECK_SYNC(win,mpi_errno,goto fn_fail);

    offset = target_disp * WINFO_DISP_UNIT(win,target_rank);

    MPIDI_Win_datatype_basic(1,datatype,&origin_dt);
    MPIDI_Win_datatype_basic(1,datatype,&result_dt);
    MPIDI_Win_datatype_basic(1,datatype,&target_dt);

    if((origin_dt.size==0) ||(target_rank==MPI_PROC_NULL))
        goto fn_exit;

    buffer  = (char *)origin_addr + origin_dt.true_lb;
    rbuffer = (char *)result_addr + result_dt.true_lb;
    tbuffer = (char *)WINFO_BASE(win,target_rank) + offset;

    MPIDI_CH4R_EPOCH_START_CHECK(win,mpi_errno,goto fn_fail);

    max_size=MPIDI_QUERY_COMPARE_ATOMIC_COUNT;
    MPIDI_Query_dt(datatype,&fi_dt,MPI_OP_NULL,&fi_op,&max_size,&dt_size);

    originv.addr   = (void *)buffer;
    originv.count  = 1;
    resultv.addr   = (void *)rbuffer;
    resultv.count  = 1;
    comparev.addr  = (void *)compare_addr;
    comparev.count = 1;
    targetv.addr   = (uint64_t)tbuffer;
    targetv.count  = 1;
    targetv.key    = WINFO_MR_KEY(win,target_rank);;

    msg.msg_iov       = &originv;
    msg.desc          = NULL;
    msg.iov_count     = 1;
    msg.addr          = _comm_to_phys(win->comm_ptr,target_rank,MPIDI_API_CTR);
    msg.rma_iov       = &targetv;
    msg.rma_iov_count = 1;
    msg.datatype      = fi_dt;
    msg.op            = fi_op;
    msg.context       = NULL;
    msg.data          = 0;
    FI_RC_RETRY2(GLOBAL_CNTR_INCR(),
                 fi_compare_atomicmsg(G_TXC_CTR(0),&msg,
                                      &comparev,NULL,1,
                                      &resultv,NULL,1,0),
                 atomicto);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_COMPARE_AND_SWAP);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME do_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_accumulate(const void    *origin_addr,
                                int            origin_count,
                                MPI_Datatype   origin_datatype,
                                int            target_rank,
                                MPI_Aint       target_disp,
                                int            target_count,
                                MPI_Datatype   target_datatype,
                                MPI_Op         op,
                                MPID_Win      *win,
                                MPID_Request **sigreq)
{
    int                rc,acccheck=0,mpi_errno = MPI_SUCCESS;
    uint64_t           flags;
    MPIDI_Win_request *req;
    size_t             offset,max_size, dt_size,omax,tmax,tout,oout;
    fid_ep_t           ep;
    MPI_Datatype       basic_type;
    fi_op_t            fi_op;
    fi_datatype_t      fi_dt;
    msg_atomic_t       msg;
    ioc_t             *originv;
    rma_ioc_t         *targetv;
    unsigned           i;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_ACCUMULATE);

    MPIDI_CH4R_EPOCH_CHECK_SYNC(win,mpi_errno,goto fn_fail);

    MPIDI_CH4_NMI_MPI_RC_POP(MPIDI_Allocate_win_request_accumulate(origin_count,
                                                              target_count,
                                                              target_rank,
                                                              origin_datatype,
                                                              target_datatype,
                                                              &req,&flags,&ep,sigreq));

    if((req->noncontig->origin_dt.size == 0) ||
       (target_rank == MPI_PROC_NULL)) {
        MPIDI_Win_request_complete(req);

        if(sigreq) MPIDI_Request_release(*sigreq);

        return MPI_SUCCESS;
    }

    offset = target_disp * WINFO_DISP_UNIT(win, target_rank);

    MPIDI_CH4R_EPOCH_START_CHECK(win,mpi_errno,goto fn_fail);

    GET_BASIC_TYPE(target_datatype, basic_type);
    switch(basic_type) {
        /* 8 byte types */
        case MPI_FLOAT_INT:
        case MPI_2INT:
        case MPI_LONG_INT:
#ifdef HAVE_FORTRAN_BINDING
        case MPI_2REAL:
        case MPI_2INTEGER:
#endif
        {basic_type=MPI_LONG_LONG; acccheck=1; break;}

        /* 16-byte types */
#ifdef HAVE_FORTRAN_BINDING
        case MPI_2DOUBLE_PRECISION:
#endif
#ifdef MPICH_DEFINE_2COMPLEX
        case MPI_2COMPLEX:
#endif
        {basic_type=MPI_DOUBLE_COMPLEX; acccheck=1; break;}

        /* Types with pads or too large to handle*/
        case MPI_DATATYPE_NULL:
        case MPI_SHORT_INT:
        case MPI_DOUBLE_INT:
        case MPI_LONG_DOUBLE_INT:
#ifdef MPICH_DEFINE_2COMPLEX
        case MPI_2DOUBLE_COMPLEX:
#endif
            goto am_fallback;
    }

    if (acccheck && op != MPI_REPLACE)
        goto am_fallback;

    max_size               = MPIDI_QUERY_ATOMIC_COUNT;

    MPIDI_Query_dt(basic_type,&fi_dt,op,&fi_op,&max_size,&dt_size);

    req->event_id          = MPIDI_EVENT_ABORT;
    req->next              = WIN_OFI(win)->syncQ;
    WIN_OFI(win)->syncQ    = req;
    max_size               = max_size*dt_size;
    if (max_size == 0)
        goto am_fallback;

    MPIDI_Init_iovec_state(&req->noncontig->iovs,
                           (uintptr_t)origin_addr,
                           (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                           req->noncontig->origin_dt.num_contig,
                           req->noncontig->target_dt.num_contig,
                           max_size,
                           req->noncontig->origin_dt.map,
                           req->noncontig->target_dt.map);

    msg.desc          = NULL;
    msg.addr          = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
    msg.context       = NULL;
    msg.data          = 0;
    msg.datatype      = fi_dt;
    msg.op            = fi_op;
    rc                = MPIDI_IOV_EAGAIN;
    while(rc==MPIDI_IOV_EAGAIN) {
        originv=req->noncontig->buf.iov.accumulate.originv;
        targetv=req->noncontig->buf.iov.accumulate.targetv;
        omax=tmax=1;
        rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,(iovec_t*)originv,omax,
                                  (rma_iov_t*)targetv,tmax,&oout,&tout);

        if(rc==MPIDI_IOV_DONE) break;

        MPIU_Assert(rc != MPIDI_IOV_ERROR);

        for(i=0; i<tout; i++) targetv[i].key = WINFO_MR_KEY(win,target_rank);

        for(i=0; i<oout; i++)originv[i].count/=dt_size;

        for(i=0; i<tout; i++)targetv[i].count/=dt_size;

        msg.msg_iov       = originv;
        msg.iov_count     = oout;
        msg.rma_iov       = targetv;
        msg.rma_iov_count = tout;
        FI_RC_RETRY2(SETUP_CHUNK_CONTEXT(sigreq),
                     fi_atomicmsg(ep, &msg, flags),
                     rdma_atomicto);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
am_fallback:
    /* Fall back to active message */
    MPIDI_Win_request_complete(req);
    return MPIDI_CH4R_accumulate(origin_addr, origin_count, origin_datatype,
                                 target_rank, target_disp,
                                 target_count, target_datatype,
                                 op, win);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_get_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_get_accumulate(const void    *origin_addr,
                                    int            origin_count,
                                    MPI_Datatype   origin_datatype,
                                    void          *result_addr,
                                    int            result_count,
                                    MPI_Datatype   result_datatype,
                                    int            target_rank,
                                    MPI_Aint       target_disp,
                                    int            target_count,
                                    MPI_Datatype   target_datatype,
                                    MPI_Op         op,
                                    MPID_Win      *win,
                                    MPID_Request **sigreq)
{
    int                rc, acccheck=0,mpi_errno = MPI_SUCCESS;
    uint64_t           flags;
    MPIDI_Win_request *req;
    size_t             offset,max_size,dt_size,omax,rmax,tmax,tout,rout,oout;
    fid_ep_t           ep;
    MPI_Datatype       rt,basic_type,basic_type_res;
    fi_op_t            fi_op;
    fi_datatype_t      fi_dt;
    msg_atomic_t       msg;
    ioc_t             *originv,*resultv;
    rma_ioc_t         *targetv;
    unsigned           i;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);

    MPIDI_CH4R_EPOCH_CHECK_SYNC(win,mpi_errno,goto fn_fail);

    MPIDI_CH4_NMI_MPI_RC_POP(MPIDI_Allocate_win_request_get_accumulate(origin_count,
                                                                  target_count,
                                                                  result_count,
                                                                  target_rank,
                                                                  op,
                                                                  origin_datatype,
                                                                  target_datatype,
                                                                  result_datatype,
                                                                  &req,&flags,&ep,sigreq));

    if((req->noncontig->result_dt.size == 0) ||
       (target_rank == MPI_PROC_NULL)) {
        MPIDI_Win_request_complete(req);

        if(sigreq) MPIDI_Request_release(*sigreq);

        goto fn_exit;
    }

    offset = target_disp * WINFO_DISP_UNIT(win,target_rank);

    MPIDI_CH4R_EPOCH_START_CHECK(win,mpi_errno,goto fn_fail);

    GET_BASIC_TYPE(target_datatype, basic_type);
    rt=result_datatype;
    switch(basic_type) {
        /* 8 byte types */
        case MPI_FLOAT_INT:
        case MPI_2INT:
        case MPI_LONG_INT:
#ifdef HAVE_FORTRAN_BINDING
        case MPI_2REAL:
        case MPI_2INTEGER:
#endif
        {basic_type=rt=MPI_LONG_LONG; acccheck=1; break;}

        /* 16-byte types */
#ifdef HAVE_FORTRAN_BINDING
        case MPI_2DOUBLE_PRECISION:
#endif
#ifdef MPICH_DEFINE_2COMPLEX
        case MPI_2COMPLEX:
#endif
        {basic_type=rt=MPI_DOUBLE_COMPLEX; acccheck=1; break;}

        /* Types with pads or too large to handle*/
        case MPI_DATATYPE_NULL:
        case MPI_SHORT_INT:
        case MPI_DOUBLE_INT:
        case MPI_LONG_DOUBLE_INT:
#ifdef MPICH_DEFINE_2COMPLEX
        case MPI_2DOUBLE_COMPLEX:
#endif
            goto am_fallback;
            break;
    }

    if (acccheck && op != MPI_REPLACE && op != MPI_NO_OP)
        goto am_fallback;

    GET_BASIC_TYPE(rt, basic_type_res);
    MPIU_Assert(basic_type_res != MPI_DATATYPE_NULL);

    max_size            = MPIDI_QUERY_FETCH_ATOMIC_COUNT;
    MPIDI_Query_dt(basic_type_res,&fi_dt,op,&fi_op,&max_size,&dt_size);
    req->event_id       = MPIDI_EVENT_RMA_DONE;
    req->next           = WIN_OFI(win)->syncQ;
    WIN_OFI(win)->syncQ = req;
    max_size            = max_size*dt_size;
    if (max_size == 0)
        goto am_fallback;

    if(op != MPI_NO_OP)
        MPIDI_Init_iovec_state2(&req->noncontig->iovs,
                                (uintptr_t)origin_addr,
                                (uintptr_t)result_addr,
                                (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                                req->noncontig->origin_dt.num_contig,
                                req->noncontig->result_dt.num_contig,
                                req->noncontig->target_dt.num_contig,
                                max_size,
                                req->noncontig->origin_dt.map,
                                req->noncontig->result_dt.map,
                                req->noncontig->target_dt.map);
    else
        MPIDI_Init_iovec_state(&req->noncontig->iovs,
                               (uintptr_t)result_addr,
                               (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                               req->noncontig->result_dt.num_contig,
                               req->noncontig->target_dt.num_contig,
                               max_size,
                               req->noncontig->result_dt.map,
                               req->noncontig->target_dt.map);

    msg.desc          = NULL;
    msg.addr          = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
    msg.context       = NULL;
    msg.data          = 0;
    msg.datatype      = fi_dt;
    msg.op            = fi_op;
    rc                = MPIDI_IOV_EAGAIN;
    while(rc==MPIDI_IOV_EAGAIN) {
        originv        = req->noncontig->buf.iov.get_accumulate.originv;
        targetv        = req->noncontig->buf.iov.get_accumulate.targetv;
        resultv        = req->noncontig->buf.iov.get_accumulate.resultv;
        omax=rmax=tmax = 1;

        if(op != MPI_NO_OP)
            rc = MPIDI_Merge_iov_list2(&req->noncontig->iovs,(iovec_t*)originv,
                                       omax,(iovec_t*)resultv,rmax,(rma_iov_t*)targetv,tmax,
                                       &oout,&rout,&tout);
        else {
            oout = 0;
            rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,(iovec_t*)resultv,
                                      rmax,(rma_iov_t*)targetv,tmax,&rout,&tout);
        }

        if(rc==MPIDI_IOV_DONE) break;

        MPIU_Assert(rc != MPIDI_IOV_ERROR);
        for(i=0; i<oout; i++)originv[i].count/=dt_size;

        for(i=0; i<rout; i++)resultv[i].count/=dt_size;

        for(i=0; i<tout; i++) {
            targetv[i].count/=dt_size;
            targetv[i].key = WINFO_MR_KEY(win,target_rank);
        }

        msg.msg_iov       = originv;
        msg.iov_count     = oout;
        msg.rma_iov       = targetv;
        msg.rma_iov_count = tout;
        FI_RC_RETRY2(SETUP_CHUNK_CONTEXT(sigreq),
                     fi_fetch_atomicmsg(ep, &msg,resultv,
                                        NULL,rout,flags),
                     rdma_readfrom);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
am_fallback:
    MPIDI_Win_request_complete(req);
    return MPIDI_CH4R_get_accumulate(origin_addr, origin_count, origin_datatype,
                                     result_addr, result_count, result_datatype,
                                     target_rank, target_disp, target_count,
                                     target_datatype, op, win);
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_raccumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_raccumulate(const void *origin_addr,
                                           int origin_count,
                                           MPI_Datatype origin_datatype,
                                           int target_rank,
                                           MPI_Aint target_disp,
                                           int target_count,
                                           MPI_Datatype target_datatype,
                                           MPI_Op op,
                                           MPID_Win *win,
                                           MPID_Request **request)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RACCUMULATE);
    MPID_Request *rreq;
    int mpi_errno = do_accumulate((void *)origin_addr,
                                  origin_count,
                                  origin_datatype,
                                  target_rank,
                                  target_disp,
                                  target_count,
                                  target_datatype,
                                  op,
                                  win,
                                  &rreq);
    *request               = rreq;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RACCUMULATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_rget_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_rget_accumulate(const void *origin_addr,
                                               int origin_count,
                                               MPI_Datatype origin_datatype,
                                               void *result_addr,
                                               int result_count,
                                               MPI_Datatype result_datatype,
                                               int target_rank,
                                               MPI_Aint target_disp,
                                               int target_count,
                                               MPI_Datatype target_datatype,
                                               MPI_Op op, MPID_Win *win,
                                               MPID_Request **request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RGET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RGET_ACCUMULATE);

    mpi_errno = do_get_accumulate(origin_addr, origin_count, origin_datatype,
                                  result_addr, result_count, result_datatype,
                                  target_rank, target_disp, target_count,
                                  target_datatype, op, win, &rreq);
    *request = rreq;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RGET_ACCUMULATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_fetch_and_op
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_fetch_and_op(const void *origin_addr,
                                            void *result_addr,
                                            MPI_Datatype datatype,
                                            int target_rank,
                                            MPI_Aint target_disp, MPI_Op op, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_FETCH_AND_OP);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_FETCH_AND_OP);

    /*  This can be optimized by directly calling the fi directly
     *  and avoiding all the datatype processing of the full
     *  MPIDI_Get_accumulate
     */
    mpi_errno = do_get_accumulate(origin_addr,1,datatype,
                                  result_addr,1,datatype,
                                  target_rank,target_disp,1,
                                  datatype,op,win,NULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_FETCH_AND_OP);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_rget
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_rget(void *origin_addr,
                                    int origin_count,
                                    MPI_Datatype origin_datatype,
                                    int target_rank,
                                    MPI_Aint target_disp,
                                    int target_count,
                                    MPI_Datatype target_datatype,
                                    MPID_Win *win, MPID_Request **request)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RGET);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RGET);
    int            mpi_errno;
    MPIDI_msg_sz_t origin_bytes;
    size_t         offset;
    MPID_Request  *rreq;

    MPIDI_Datatype_check_size(origin_datatype,origin_count,origin_bytes);

    if(unlikely((origin_bytes == 0) || (target_rank == MPI_PROC_NULL))) {
        rreq       = MPIDI_Request_alloc_and_init(2);
        rreq->kind = MPID_WIN_REQUEST;
        MPIDI_Request_complete(rreq);
        goto fn_exit;
    }

    if(target_rank == win->comm_ptr->rank) {
        rreq       = MPIDI_Request_alloc_and_init(2);
        rreq->kind = MPID_WIN_REQUEST;
        offset     = target_disp * WINFO_DISP_UNIT(win,target_rank);
        mpi_errno  = MPIR_Localcopy((char *)win->base + offset,
                              target_count,
                              target_datatype,
                              origin_addr,
                              origin_count,
                              origin_datatype);
        MPIDI_Request_complete(rreq);
        goto fn_exit;
    }
    mpi_errno = do_get(origin_addr,
                           origin_count,
                           origin_datatype,
                           target_rank,
                           target_disp,
                           target_count,
                           target_datatype,
                           win,
                           &rreq);
fn_exit:
    *request = rreq;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RGET);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_get_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_get_accumulate(const void *origin_addr,
                                              int origin_count,
                                              MPI_Datatype origin_datatype,
                                              void *result_addr,
                                              int result_count,
                                              MPI_Datatype result_datatype,
                                              int target_rank,
                                              MPI_Aint target_disp,
                                              int target_count,
                                              MPI_Datatype target_datatype,
                                              MPI_Op op, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);
    mpi_errno = do_get_accumulate(origin_addr, origin_count, origin_datatype,
                                  result_addr, result_count, result_datatype,
                                  target_rank, target_disp, target_count,
                                  target_datatype, op, win, NULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_accumulate(const void *origin_addr,
                                          int origin_count,
                                          MPI_Datatype origin_datatype,
                                          int target_rank,
                                          MPI_Aint target_disp,
                                          int target_count,
                                          MPI_Datatype target_datatype, MPI_Op op, MPID_Win *win)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_ACCUMULATE);
    int mpi_errno = do_accumulate(origin_addr,
                                  origin_count,
                                  origin_datatype,
                                  target_rank,
                                  target_disp,
                                  target_count,
                                  target_datatype,
                                  op,
                                  win,
                                  NULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_ACCUMULATE);
    return mpi_errno;
}

#endif /* NETMOD_OFI_RMA_H_INCLUDED */
