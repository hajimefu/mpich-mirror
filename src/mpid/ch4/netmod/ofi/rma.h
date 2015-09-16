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
        memset(&dt, 0, sizeof(*dt));

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_WIN_DATATYPE_BASIC);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_valid_group_rank
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_valid_group_rank(int         lpid,
                                         MPID_Group *grp)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_VALID_GROUP_RANK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_VALID_GROUP_RANK);

    int size = grp->size;
    int z;

    for(z = 0; z < size &&lpid != grp->lrank_to_lpid[z].lpid; ++z) {}

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_VALID_GROUP_RANK);
    return (z < size);
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

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_do_put
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
                         MPID_Request **signal)
{
    int                rc,mpi_errno = MPI_SUCCESS;
    MPIDI_Win_request *req;
    size_t             offset;
    uint64_t           flags=0ULL;
    MPID_Request      *sigreq;
    fid_ep_t           ep;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_PUT);
    EPOCH_CHECK1();

    WINREQ_CREATE(req);
    SETUP_SIGNAL_REQUEST();

    offset   = target_disp * WINFO_DISP_UNIT(win,target_rank);

    MPIDI_Win_datatype_basic(origin_count,
                             origin_datatype,
                             &req->noncontig->origin_dt);
    MPIDI_Win_datatype_basic(target_count,
                             target_datatype,
                             &req->noncontig->target_dt);
    MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size),
                        mpi_errno,MPI_ERR_SIZE,"**rmasize");

    if((req->noncontig->origin_dt.size == 0) ||
       (target_rank == MPI_PROC_NULL)) {
        MPIDI_Win_request_complete(req);

        if(signal) MPIDI_Request_release(sigreq);

        goto fn_exit;
    }

    if(target_rank == win->comm_ptr->rank) {
        MPIDI_Win_request_complete(req);

        if(signal) MPIDI_Request_release(sigreq);

        return MPIR_Localcopy(origin_addr,
                              origin_count,
                              origin_datatype,
                              (char *)win->base + offset,
                              target_count,
                              target_datatype);
    }

    EPOCH_START_CHECK();
    req->target_rank = target_rank;

    MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
    MPIDI_Win_datatype_map(&req->noncontig->target_dt);

    msg_rma_t msg;
    void *desc;
    desc = fi_mr_desc(MPIDI_Global.mr);
    req->event_id = MPIDI_EVENT_ABORT;
    msg.desc      = &desc;
    msg.addr      = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
    msg.context   = NULL;
    msg.data      = 0;
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
        iovec_t   originv[1];
        rma_iov_t targetv[1];
        size_t    omax;
        size_t    tmax;
        size_t    tout, oout;
        unsigned  i;
        omax=tmax=1;
        rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,originv,
                                  omax,targetv,tmax,&oout,&tout);

        if(rc==MPIDI_IOV_DONE) break;

        for(i=0; i<tout; i++) targetv[i].key = WINFO_MR_KEY(win,target_rank);

        MPIU_Assert(rc != MPIDI_IOV_ERROR);
        msg.msg_iov       = originv;
        msg.iov_count     = oout;
        msg.rma_iov       = targetv;
        msg.rma_iov_count = tout;
        FI_RC_RETRY2(SETUP_CHUNK_CONTEXT(),
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
#define FUNCNAME MPIDI_netmod_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_put(const void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count,
                                   MPI_Datatype target_datatype,
                                   MPID_Win *win)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_PUT);
    int            mpi_errno = MPI_SUCCESS,target_contig,origin_contig;
    MPIDI_msg_sz_t target_bytes,origin_bytes;
    MPI_Aint       origin_true_lb,target_true_lb;

    MPIDI_Datatype_check_contig_size_lb(target_datatype,target_count,
                                        target_contig,target_bytes,
                                        target_true_lb);
    MPIDI_Datatype_check_contig_size_lb(origin_datatype,origin_count,
                                        origin_contig,origin_bytes,
                                        origin_true_lb);

    if(origin_contig && target_contig && origin_bytes &&
       target_rank != MPI_PROC_NULL       &&
       target_rank != win->comm_ptr->rank &&
       (origin_bytes<=MPIDI_Global.max_buffered_write)) {
        EPOCH_CHECK1();
        MPIR_ERR_CHKANDJUMP((origin_bytes != target_bytes),
                            mpi_errno,MPI_ERR_SIZE,"**rmasize");
        EPOCH_START_CHECK();
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
#define FUNCNAME do_get_lw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int do_get_lw(void               *origin_addr,
                            int                 origin_count,
                            MPI_Datatype        origin_datatype,
                            int                 target_rank,
                            MPI_Aint            target_disp,
                            int                 target_count,
                            MPI_Datatype        target_datatype,
                            MPID_Win           *win)
{
    int          mpi_errno = MPI_SUCCESS;
    MPIDI_Win_dt origin_dt, target_dt;
    size_t       offset;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_GET_LW);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_GET_LW);
    EPOCH_CHECK1();

    offset = target_disp * WINFO_DISP_UNIT(win,target_rank);
    MPIDI_Win_datatype_basic(origin_count,origin_datatype,&origin_dt);
    MPIDI_Win_datatype_basic(target_count,target_datatype,&target_dt);
    MPIR_ERR_CHKANDJUMP((origin_dt.size != target_dt.size),
                        mpi_errno, MPI_ERR_SIZE, "**rmasize");

    if((origin_dt.size == 0)||(target_rank == MPI_PROC_NULL))
        goto fn_exit;

    if(target_rank == win->comm_ptr->rank)
        return MPIR_Localcopy((char *)win->base + offset,
                              target_count,
                              target_datatype,
                              origin_addr,
                              origin_count,
                              origin_datatype);

    void *buffer,*tbuffer;
    buffer  = (char *)origin_addr + origin_dt.true_lb;
    tbuffer = (char *)WINFO_BASE(win,target_rank) + offset + target_dt.true_lb;

    EPOCH_START_CHECK();
    rma_iov_t riov;
    iovec_t   iov;
    msg_rma_t msg;
    void     *desc;
    desc = fi_mr_desc(MPIDI_Global.mr);
    msg.desc          = &desc;
    msg.msg_iov       = &iov;
    msg.iov_count     = 1;
    msg.addr          = _comm_to_phys(win->comm_ptr,target_rank,MPIDI_API_CTR);
    msg.rma_iov       = &riov;
    msg.rma_iov_count = 1;
    msg.context       = NULL;
    msg.data          = 0;
    iov.iov_base      = buffer;
    iov.iov_len       = target_dt.size;
    riov.addr         = (uint64_t)tbuffer;
    riov.len          = target_dt.size;
    riov.key          = WINFO_MR_KEY(win,target_rank);
    FI_RC_RETRY2(GLOBAL_CNTR_INCR(),
                 fi_readmsg(G_TXC_CTR(0), &msg, 0),
                 rdma_write);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_GET_LW);
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
                         MPID_Request **signal)
{
    int                rc, mpi_errno = MPI_SUCCESS;
    MPIDI_Win_request *req;
    size_t             offset;
    uint64_t           flags=0ULL;
    MPID_Request      *sigreq;
    fid_ep_t           ep;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_GET);

    EPOCH_CHECK1();

    WINREQ_CREATE(req);
    SETUP_SIGNAL_REQUEST();

    offset   = target_disp * WINFO_DISP_UNIT(win,target_rank);

    MPIDI_Win_datatype_basic(origin_count,
                             origin_datatype,
                             &req->noncontig->origin_dt);
    MPIDI_Win_datatype_basic(target_count,
                             target_datatype,
                             &req->noncontig->target_dt);
    MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size),
                        mpi_errno, MPI_ERR_SIZE, "**rmasize");

    if((req->noncontig->origin_dt.size == 0) ||
       (target_rank == MPI_PROC_NULL)) {
        MPIDI_Win_request_complete(req);

        if(signal) MPIDI_Request_release(sigreq);

        goto fn_exit;
    }

    if(target_rank == win->comm_ptr->rank) {
        MPIDI_Win_request_complete(req);

        if(signal) MPIDI_Request_release(sigreq);

        return MPIR_Localcopy((char *)win->base + offset,
                              target_count,
                              target_datatype,
                              origin_addr,
                              origin_count,
                              origin_datatype);
    }

    EPOCH_START_CHECK();
    req->target_rank = target_rank;

    MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
    MPIDI_Win_datatype_map(&req->noncontig->target_dt);

    req->event_id = MPIDI_EVENT_ABORT;
    msg_rma_t  msg;
    void      *desc;
    desc = fi_mr_desc(MPIDI_Global.mr);
    msg.desc      = &desc;
    msg.addr      = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
    msg.context   = NULL;
    msg.data      = 0;
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
        iovec_t   originv[1];
        rma_iov_t targetv[1];
        size_t    omax,tmax,tout,oout;
        unsigned  i;
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
        FI_RC_RETRY2(SETUP_CHUNK_CONTEXT(),
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
#define FUNCNAME MPIDI_netmod_get
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_get(void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count, MPI_Datatype target_datatype, MPID_Win *win)
{
    int origin_contig,target_contig, mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_GET);

    MPIDI_Datatype_check_contig(origin_datatype,origin_contig);
    MPIDI_Datatype_check_contig(target_datatype,target_contig);

    if(origin_contig && target_contig)
        mpi_errno = do_get_lw(origin_addr,
                              origin_count,
                              origin_datatype,
                              target_rank,
                              target_disp,
                              target_count,
                              target_datatype,
                              win);
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

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_GET);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_rput
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_rput(const void *origin_addr,
                                    int origin_count,
                                    MPI_Datatype origin_datatype,
                                    int target_rank,
                                    MPI_Aint target_disp,
                                    int target_count,
                                    MPI_Datatype target_datatype,
                                    MPID_Win *win, MPID_Request **request)
{
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RPUT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RPUT);
    MPID_Request  *rreq;
    int mpi_errno = do_put((void *)origin_addr,
                           origin_count,
                           origin_datatype,
                           target_rank,
                           target_disp,
                           target_count,
                           target_datatype,
                           win,
                           &rreq);
    *request = rreq;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RPUT);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_compare_and_swap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_compare_and_swap(const void *origin_addr,
                                                const void *compare_addr,
                                                void *result_addr,
                                                MPI_Datatype datatype,
                                                int target_rank,
                                                MPI_Aint target_disp, MPID_Win *win)
{
    int mpi_errno = MPI_SUCCESS;
    fi_op_t fi_op;
    fi_datatype_t fi_dt;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_COMPARE_AND_SWAP);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_COMPARE_AND_SWAP);

    EPOCH_CHECK1();

    MPIDI_Win_dt origin_dt, target_dt, result_dt;
    size_t       offset;

    offset = target_disp * WINFO_DISP_UNIT(win,target_rank);

    MPIDI_Win_datatype_basic(1,datatype,&origin_dt);
    MPIDI_Win_datatype_basic(1,datatype,&result_dt);
    MPIDI_Win_datatype_basic(1,datatype,&target_dt);

    if((origin_dt.size==0) ||(target_rank==MPI_PROC_NULL))
        goto fn_exit;

    void *buffer, *tbuffer, *rbuffer;
    buffer  = (char *)origin_addr + origin_dt.true_lb;
    rbuffer = (char *)result_addr + result_dt.true_lb;
    tbuffer = (char *)WINFO_BASE(win,target_rank) + offset;

    EPOCH_START_CHECK();
    size_t max_size,dt_size;
    max_size=MPIDI_QUERY_COMPARE_ATOMIC_COUNT;
    MPIDI_Query_dt(datatype,&fi_dt,MPI_OP_NULL,&fi_op,&max_size,&dt_size);

    ioc_t     originv, resultv, comparev;
    rma_ioc_t targetv;
    originv.addr   = (void *)buffer;
    originv.count  = 1;
    resultv.addr   = (void *)rbuffer;
    resultv.count  = 1;
    comparev.addr  = (void *)compare_addr;
    comparev.count = 1;
    targetv.addr   = (uint64_t)tbuffer;
    targetv.count  = 1;
    targetv.key    = WINFO_MR_KEY(win,target_rank);;

    msg_atomic_t msg;
    void *desc;
    desc = fi_mr_desc(MPIDI_Global.mr);
    msg.msg_iov       = &originv;
    msg.desc      = &desc;
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
#define FUNCNAME MPIDI_netmod_do_accumulate
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
                                MPID_Request **signal)
{
    int                rc,acccheck=0,mpi_errno = MPI_SUCCESS;
    uint64_t           flags=0;
    MPI_Datatype       tt,ot;
    MPIDI_Win_request *req;
    size_t             offset;
    MPID_Request      *sigreq;
    fid_ep_t           ep;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_ACCUMULATE);


    WINREQ_CREATE(req);
    SETUP_SIGNAL_REQUEST();

    EPOCH_CHECK1();

    offset = target_disp * WINFO_DISP_UNIT(win, target_rank);
    ot=origin_datatype;
    tt=target_datatype;

    MPIDI_Win_datatype_basic(origin_count,ot,&req->noncontig->origin_dt);
    MPIDI_Win_datatype_basic(target_count,tt,&req->noncontig->target_dt);
    MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size),
                        mpi_errno,MPI_ERR_SIZE,"**rmasize");

    if((req->noncontig->origin_dt.size == 0) ||
       (target_rank == MPI_PROC_NULL)) {
        MPIDI_Win_request_complete(req);

        if(signal) MPIDI_Request_release(sigreq);

        return MPI_SUCCESS;
    }

    EPOCH_START_CHECK();
    req->target_rank = target_rank;

    MPI_Datatype basic_type;
    GET_BASIC_TYPE(tt, basic_type);

    switch(basic_type) {
        /* 8 byte types */
        case MPI_FLOAT_INT:
        case MPI_2INT:
        case MPI_LONG_INT:
#ifdef HAVE_FORTRAN_BINDING
        case MPI_2REAL:
        case MPI_2INTEGER:
#endif
        {basic_type=tt=ot=MPI_LONG_LONG; acccheck=1; break;}

        /* 16-byte types */
#ifdef HAVE_FORTRAN_BINDING
        case MPI_2DOUBLE_PRECISION:
#endif
#ifdef MPICH_DEFINE_2COMPLEX
        case MPI_2COMPLEX:
#endif
        {basic_type=tt=ot=MPI_DOUBLE_COMPLEX; acccheck=1; break;}

        /* Types with pads or too large to handle*/
        case MPI_DATATYPE_NULL:
        case MPI_SHORT_INT:
        case MPI_DOUBLE_INT:
        case MPI_LONG_DOUBLE_INT:
#ifdef MPICH_DEFINE_2COMPLEX
        case MPI_2DOUBLE_COMPLEX:
#endif
            MPIR_ERR_SETANDSTMT(mpi_errno,MPI_ERR_TYPE,goto fn_fail,
                                "**rmatypenotatomic");
            break;
    }

    MPIR_ERR_CHKANDSTMT((acccheck && op != MPI_REPLACE),
                        mpi_errno,MPI_ERR_TYPE,
                        goto fn_fail, "**rmatypenotatomic");

    fi_op_t       fi_op;
    fi_datatype_t fi_dt;
    size_t        max_size, dt_size;
    max_size=MPIDI_QUERY_ATOMIC_COUNT;
    MPIDI_Win_datatype_map(&req->noncontig->target_dt);
    MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
    MPIDI_Query_dt(basic_type,&fi_dt,op,&fi_op,&max_size,&dt_size);
    req->event_id          = MPIDI_EVENT_ABORT;
    req->next              = WIN_OFI(win)->syncQ;
    WIN_OFI(win)->syncQ    = req;
    max_size = max_size*dt_size;
    MPIR_ERR_CHKANDSTMT((max_size == 0), mpi_errno,MPI_ERR_TYPE,
                        goto fn_fail, "**rmatypenotatomic");

    MPIDI_Init_iovec_state(&req->noncontig->iovs,
                           (uintptr_t)origin_addr,
                           (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                           req->noncontig->origin_dt.num_contig,
                           req->noncontig->target_dt.num_contig,
                           max_size,
                           req->noncontig->origin_dt.map,
                           req->noncontig->target_dt.map);
    msg_atomic_t msg;
    void *desc;
    desc = fi_mr_desc(MPIDI_Global.mr);
    rc = MPIDI_IOV_EAGAIN;
    msg.desc          = &desc;
    msg.addr          = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
    msg.context       = NULL;
    msg.data          = 0;
    msg.datatype      = fi_dt;
    msg.op            = fi_op;

    while(rc==MPIDI_IOV_EAGAIN) {
        ioc_t     originv[1];
        rma_ioc_t targetv[1];
        size_t    omax;
        size_t    tmax;
        size_t    tout, oout;
        unsigned  i;
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
        FI_RC_RETRY2(SETUP_CHUNK_CONTEXT(),
                     fi_atomicmsg(ep, &msg, flags),
                     rdma_atomicto);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_get_accumulate
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
                                    MPID_Request **signal)
{
    int                rc, acccheck=0,mpi_errno = MPI_SUCCESS;
    uint64_t           flags=0ULL;
    MPI_Datatype       tt,ot,rt;
    MPIDI_Win_request *req;
    size_t             offset;
    MPID_Request      *sigreq;
    fid_ep_t           ep;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);
    WINREQ_CREATE(req);
    SETUP_SIGNAL_REQUEST();

    EPOCH_CHECK1();

    offset = target_disp * WINFO_DISP_UNIT(win,target_rank);
    ot=origin_datatype;
    tt=target_datatype;
    rt=result_datatype;

    MPIDI_Win_datatype_basic(origin_count,ot,&req->noncontig->origin_dt);
    MPIDI_Win_datatype_basic(target_count,tt,&req->noncontig->target_dt);
    MPIDI_Win_datatype_basic(result_count,rt,&req->noncontig->result_dt);

    MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size && op != MPI_NO_OP),
                        mpi_errno, MPI_ERR_SIZE, "**rmasize");
    MPIR_ERR_CHKANDJUMP((req->noncontig->result_dt.size != req->noncontig->target_dt.size),
                        mpi_errno, MPI_ERR_SIZE, "**rmasize");

    if((req->noncontig->result_dt.size == 0) ||
       (target_rank == MPI_PROC_NULL)) {
        MPIDI_Win_request_complete(req);

        if(signal) MPIDI_Request_release(sigreq);

        goto fn_exit;
    }

    EPOCH_START_CHECK();
    req->target_rank     = target_rank;

    MPI_Datatype basic_type;
    GET_BASIC_TYPE(tt, basic_type);

    switch(basic_type) {
        /* 8 byte types */
        case MPI_FLOAT_INT:
        case MPI_2INT:
        case MPI_LONG_INT:
#ifdef HAVE_FORTRAN_BINDING
        case MPI_2REAL:
        case MPI_2INTEGER:
#endif
        {basic_type=tt=ot=rt=MPI_LONG_LONG; acccheck=1; break;}

        /* 16-byte types */
#ifdef HAVE_FORTRAN_BINDING
        case MPI_2DOUBLE_PRECISION:
#endif
#ifdef MPICH_DEFINE_2COMPLEX
        case MPI_2COMPLEX:
#endif
        {basic_type=tt=ot=rt=MPI_DOUBLE_COMPLEX; acccheck=1; break;}

        /* Types with pads or too large to handle*/
        case MPI_DATATYPE_NULL:
        case MPI_SHORT_INT:
        case MPI_DOUBLE_INT:
        case MPI_LONG_DOUBLE_INT:
#ifdef MPICH_DEFINE_2COMPLEX
        case MPI_2DOUBLE_COMPLEX:
#endif
            MPIR_ERR_SETANDSTMT(mpi_errno,MPI_ERR_TYPE,goto fn_fail,
                                "**rmatypenotatomic");
            break;
    }

    MPIR_ERR_CHKANDSTMT((acccheck && op != MPI_REPLACE && op != MPI_NO_OP),
                        mpi_errno,MPI_ERR_TYPE,
                        goto fn_fail, "**rmatypenotatomic");

    MPI_Datatype basic_type_res;
    GET_BASIC_TYPE(rt, basic_type_res);

    MPIU_Assert(basic_type_res != MPI_DATATYPE_NULL);

    fi_op_t       fi_op;
    fi_datatype_t fi_dt;
    size_t        max_size,dt_size;
    max_size = MPIDI_QUERY_FETCH_ATOMIC_COUNT;
    MPIDI_Win_datatype_map(&req->noncontig->target_dt);

    if(op != MPI_NO_OP) MPIDI_Win_datatype_map(&req->noncontig->origin_dt);

    MPIDI_Win_datatype_map(&req->noncontig->result_dt);
    MPIDI_Query_dt(basic_type_res,&fi_dt,op,&fi_op,&max_size,&dt_size);
    req->event_id       = MPIDI_EVENT_RMA_DONE;
    req->next           = WIN_OFI(win)->syncQ;
    WIN_OFI(win)->syncQ = req;


    max_size = max_size*dt_size;
    MPIR_ERR_CHKANDSTMT((max_size == 0), mpi_errno,MPI_ERR_TYPE,
                        goto fn_fail, "**rmatypenotatomic");

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

    msg_atomic_t  msg;
    void *desc;
    desc = fi_mr_desc(MPIDI_Global.mr);
    rc = MPIDI_IOV_EAGAIN;
    msg.desc          = &desc;
    msg.addr          = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
    msg.context       = NULL;
    msg.data          = 0;
    msg.datatype      = fi_dt;
    msg.op            = fi_op;

    while(rc==MPIDI_IOV_EAGAIN) {
        ioc_t     originv[1]= {{0}};
        ioc_t     resultv[1]= {{0}};
        rma_ioc_t targetv[1]= {{0}};
        size_t    omax,rmax,tmax;
        size_t    tout,rout,oout;
        unsigned  i;

        omax=rmax=tmax=1;

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
        FI_RC_RETRY2(SETUP_CHUNK_CONTEXT(),
                     fi_fetch_atomicmsg(ep, &msg,resultv,
                                        NULL,rout,flags),
                     rdma_readfrom);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_GET_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_raccumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_raccumulate(const void *origin_addr,
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
#define FUNCNAME MPIDI_netmod_rget_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_rget_accumulate(const void *origin_addr,
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
#define FUNCNAME MPIDI_netmod_fetch_and_op
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_fetch_and_op(const void *origin_addr,
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
#define FUNCNAME MPIDI_netmod_rget
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_rget(void *origin_addr,
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
    MPID_Request  *rreq;
    int mpi_errno = do_get(origin_addr,
                           origin_count,
                           origin_datatype,
                           target_rank,
                           target_disp,
                           target_count,
                           target_datatype,
                           win,
                           &rreq);
    *request               = rreq;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RGET);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_get_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_get_accumulate(const void *origin_addr,
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
#define FUNCNAME MPIDI_netmod_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_accumulate(const void *origin_addr,
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
