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
#ifndef MPIDCH4U_RMA_H_INCLUDED
#define MPIDCH4U_RMA_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_do_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_do_put(const void *origin_addr,
                                    int origin_count,
                                    MPI_Datatype origin_datatype,
                                    int target_rank,
                                    MPI_Aint target_disp,
                                    int target_count,
                                    MPI_Datatype target_datatype,
                                    MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS, c, n_iov;
    MPID_Request *sreq = NULL;
    MPIDI_CH4U_put_msg_t am_hdr;
    uint64_t offset;
    MPIDI_CH4I_win_info_t *winfo;
    uint64_t dt_contig, data_sz, dt_true_lb;
    MPI_Aint last;
    MPID_Datatype *dt_ptr;
    MPID_Segment *segment_ptr;
    struct iovec *dt_iov, am_iov[2];

    MPIDI_STATE_DECL(MPID_STATE_CH4I_DO_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_DO_PUT);

    MPIDI_CH4I_EPOCH_CHECK1();

    winfo = MPIU_CH4U_WINFO(win, target_rank);
    offset = target_disp * winfo->disp_unit;

    sreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;
    if (request)
        *request = sreq;

    MPIU_CH4U_REQUEST(sreq, preq.win_ptr) = (uint64_t) win;
    MPIDI_Datatype_get_info(origin_count, origin_datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    if (data_sz == 0) {
        MPID_cc_decr(sreq->cc_ptr, &c);
        MPIU_Assert(c >= 0);
        MPIDI_Request_release(sreq);
        goto fn_exit;
    }

    if (target_rank == win->comm_ptr->rank) {
        if (request) MPIDI_Request_release(sreq);
        return MPIR_Localcopy(origin_addr,
                              origin_count,
                              origin_datatype,
                              (char *)win->base + offset,
                              target_count,
                              target_datatype);
    }

    MPID_cc_incr(sreq->cc_ptr, &c);
    MPIDI_CH4I_EPOCH_START_CHECK();
    am_hdr.addr = winfo->base_addr + offset;
    am_hdr.count = target_count;
    am_hdr.datatype = target_datatype;
    am_hdr.preq_ptr = (uint64_t) sreq;

    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)++;
    /* MPIDI_CS_EXIT(); */
    
    if (HANDLE_GET_KIND(target_datatype) == HANDLE_KIND_BUILTIN) {
        am_hdr.n_iov = 0;
        MPIU_CH4U_REQUEST(sreq, preq.dt_iov) = NULL;
        MPIDU_RC_POP(MPIDI_netmod_send_am(target_rank, win->comm_ptr, MPIDI_CH4U_AM_PUT_REQ,
                                          &am_hdr, sizeof(am_hdr), origin_addr,
                                          origin_count, origin_datatype, sreq, NULL));
        goto fn_exit;
    }

    segment_ptr = MPID_Segment_alloc();
    MPIU_Assert(segment_ptr);

    MPID_Segment_init((void *)am_hdr.addr, target_count, target_datatype, segment_ptr, 0);
    last = data_sz;
    MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, (MPI_Aint *) &n_iov);
    MPIU_Assert(n_iov > 0);
    am_hdr.n_iov = n_iov;
    dt_iov = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
    MPIU_Assert(dt_iov);

    last = data_sz;
    MPID_Segment_pack_vector(segment_ptr, 0, &last, dt_iov, &n_iov);
    MPIU_Assert(last == (MPI_Aint)data_sz);
    MPIU_Free(segment_ptr);

    am_iov[0].iov_base = &am_hdr;
    am_iov[0].iov_len = sizeof(am_hdr);
    am_iov[1].iov_base = dt_iov;
    am_iov[1].iov_len = sizeof(struct iovec) * am_hdr.n_iov;

    MPIU_CH4U_REQUEST(sreq, preq.dt_iov) = dt_iov;
    MPIDU_RC_POP(MPIDI_netmod_send_amv(target_rank, win->comm_ptr, MPIDI_CH4U_AM_PUT_REQ,
                                       &am_iov[0], 2, origin_addr, origin_count, origin_datatype,
                                       sreq, NULL));
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_DO_PUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_do_get
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_do_get(void          *origin_addr,
                         int            origin_count,
                         MPI_Datatype   origin_datatype,
                         int            target_rank,
                         MPI_Aint       target_disp,
                         int            target_count,
                         MPI_Datatype   target_datatype,
                         MPID_Win      *win,
                         MPID_Request **request)
{
    int                rc, mpi_errno = MPI_SUCCESS, c, n_iov;
    size_t             offset;
    MPID_Request      *sreq = NULL;
    MPIDI_CH4U_get_req_msg_t am_hdr;
    MPIDI_CH4I_win_info_t *winfo;
    uint64_t dt_contig, data_sz, dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPI_Aint last;
    MPID_Segment *segment_ptr;
    struct iovec *dt_iov, am_iov[2];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_MPIDI_CH4I_DO_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_MPIDI_CH4I_DO_GET);

    MPIDI_CH4I_EPOCH_CHECK1();

    winfo = MPIU_CH4U_WINFO(win, target_rank);
    offset   = target_disp * winfo->disp_unit;

    sreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;
    if (request)
        *request = sreq;

    MPIDI_Datatype_get_info(origin_count, origin_datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    if (data_sz == 0) {
        MPID_cc_decr(sreq->cc_ptr, &c);
        MPIU_Assert(c >= 0);
        MPIDI_Request_release(sreq);
        goto fn_exit;
    }
    
    MPIU_CH4U_REQUEST(sreq, greq.win_ptr) = (uint64_t) win;
    MPIU_CH4U_REQUEST(sreq, greq.addr) = (uint64_t)((char *) origin_addr + dt_true_lb);
    MPIU_CH4U_REQUEST(sreq, greq.count) = origin_count;
    MPIU_CH4U_REQUEST(sreq, greq.datatype) = origin_datatype;

    if (target_rank == win->comm_ptr->rank) {
        if (request) MPIDI_Request_release(sreq);
        return MPIR_Localcopy((char *)win->base + offset,
                              target_count,
                              target_datatype,
                              origin_addr,
                              origin_count,
                              origin_datatype);
    }

    MPID_cc_incr(sreq->cc_ptr, &c);
    MPIDI_CH4I_EPOCH_START_CHECK();
    am_hdr.addr = winfo->base_addr + offset;
    am_hdr.count = target_count;
    am_hdr.datatype = target_datatype;
    am_hdr.greq_ptr = (uint64_t) sreq;

    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)++;
    /* MPIDI_CS_EXIT(); */

    if (HANDLE_GET_KIND(target_datatype) == HANDLE_KIND_BUILTIN) {
        am_hdr.n_iov = 0;
        MPIU_CH4U_REQUEST(sreq, greq.dt_iov) = NULL;
        MPIDU_RC_POP(MPIDI_netmod_send_am_hdr(target_rank, win->comm_ptr,
                                              MPIDI_CH4U_AM_GET_REQ,
                                              &am_hdr, sizeof(am_hdr), sreq, NULL));
        goto fn_exit;
    }

    segment_ptr = MPID_Segment_alloc();
    MPIU_Assert(segment_ptr);

    MPID_Segment_init((void *)am_hdr.addr, target_count, target_datatype, segment_ptr, 0);
    last = data_sz;
    MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, (MPI_Aint *) &n_iov);
    MPIU_Assert(n_iov > 0);
    am_hdr.n_iov = n_iov;
    dt_iov = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
    MPIU_Assert(dt_iov);

    last = data_sz;
    MPID_Segment_pack_vector(segment_ptr, 0, &last, dt_iov, &n_iov);
    MPIU_Assert(last == (MPI_Aint)data_sz);
    MPIU_Free(segment_ptr);

    am_iov[0].iov_base = &am_hdr;
    am_iov[0].iov_len = sizeof(am_hdr);
    am_iov[1].iov_base = dt_iov;
    am_iov[1].iov_len = sizeof(struct iovec) * am_hdr.n_iov;

    MPIU_CH4U_REQUEST(sreq, greq.dt_iov) = dt_iov;
    MPIDU_RC_POP(MPIDI_netmod_send_amv_hdr(target_rank, win->comm_ptr,
                                           MPIDI_CH4U_AM_GET_REQ, &am_iov[0], 2,
                                           sreq, NULL));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_MPIDI_CH4I_DO_GET);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_put(const void *origin_addr,
                                  int origin_count,
                                  MPI_Datatype origin_datatype,
                                  int target_rank,
                                  MPI_Aint target_disp,
                                  int target_count, MPI_Datatype target_datatype, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_PUT);
    MPIDU_RC_POP(MPIDI_CH4I_do_put(origin_addr, origin_count, origin_datatype,
                                   target_rank, target_disp, target_count,
                                   target_datatype, win, NULL));
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_PUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_rput
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_rput(const void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count,
                                   MPI_Datatype target_datatype,
                                   MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RPUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RPUT);
    MPIDU_RC_POP(MPIDI_CH4I_do_put(origin_addr, origin_count, origin_datatype,
                                   target_rank, target_disp, target_count,
                                   target_datatype, win, request));
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RPUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_get
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_get(void *origin_addr,
                                  int origin_count,
                                  MPI_Datatype origin_datatype,
                                  int target_rank,
                                  MPI_Aint target_disp,
                                  int target_count, MPI_Datatype target_datatype, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_GET);
    MPIDU_RC_POP(MPIDI_CH4I_do_get(origin_addr, origin_count, origin_datatype,
                                   target_rank, target_disp, target_count,
                                   target_datatype, win, NULL));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_rget
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_rget(void *origin_addr,
                                   int origin_count,
                                   MPI_Datatype origin_datatype,
                                   int target_rank,
                                   MPI_Aint target_disp,
                                   int target_count,
                                   MPI_Datatype target_datatype,
                                   MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RGET);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RGET);
    MPIDU_RC_POP(MPIDI_CH4I_do_get(origin_addr, origin_count, origin_datatype,
                                   target_rank, target_disp, target_count,
                                   target_datatype, win, request));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RGET);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_do_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_do_accumulate(const void *origin_addr,
                                            int origin_count,
                                            MPI_Datatype origin_datatype,
                                            int target_rank,
                                            MPI_Aint target_disp,
                                            int target_count,
                                            MPI_Datatype target_datatype, 
                                            MPI_Op op, MPID_Win * win,
                                            int do_get,
                                            MPID_Request *sreq)
{
    int                rc, mpi_errno = MPI_SUCCESS, c, n_iov;
    size_t             offset;
    MPIDI_CH4U_acc_req_msg_t am_hdr;
    MPIDI_CH4I_win_info_t *winfo;
    uint64_t dt_contig, data_sz, dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPI_Aint last;
    MPID_Segment *segment_ptr;
    struct iovec *dt_iov, am_iov[2];
    int op_type;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_DO_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_DO_ACCUMULATE);

    MPIDI_CH4I_EPOCH_CHECK1();

    winfo = MPIU_CH4U_WINFO(win, target_rank);
    offset = target_disp * winfo->disp_unit;

    op_type = (do_get == 1) ? MPIDI_CH4U_AM_GET_ACC_REQ : MPIDI_CH4U_AM_ACC_REQ;
    MPIDI_Datatype_get_info(origin_count, origin_datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    if (data_sz == 0) {
        MPID_cc_decr(sreq->cc_ptr, &c);
        MPIU_Assert(c >= 0);
        MPIDI_Request_release(sreq);
        goto fn_exit;
    }

    MPIU_CH4U_REQUEST(sreq, areq.win_ptr) = (uint64_t) win;
    MPID_cc_incr(sreq->cc_ptr, &c);
    MPIDI_CH4I_EPOCH_START_CHECK();

    am_hdr.req_ptr = (uint64_t) sreq;
    am_hdr.origin_count = origin_count;
    am_hdr.origin_datatype =
        (HANDLE_GET_KIND(origin_datatype) == HANDLE_KIND_BUILTIN) ?
        origin_datatype : dt_ptr->basic_type;
    am_hdr.target_count = target_count;
    am_hdr.target_datatype = target_datatype;
    am_hdr.target_addr = winfo->base_addr + offset;
    am_hdr.op = op;

    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)++;
    /* MPIDI_CS_EXIT(); */

    MPIU_CH4U_REQUEST(sreq, areq.data_sz) = data_sz;
    if (HANDLE_GET_KIND(target_datatype) == HANDLE_KIND_BUILTIN) {
        am_hdr.n_iov = 0;
        MPIU_CH4U_REQUEST(sreq, areq.dt_iov) = NULL;
        MPIDU_RC_POP(MPIDI_netmod_send_am(target_rank, win->comm_ptr, op_type,
                                          &am_hdr, sizeof(am_hdr), origin_addr,
                                          origin_count, origin_datatype, sreq, NULL));
        goto fn_exit;
    }

    MPIDI_Datatype_get_info(target_count, target_datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    am_hdr.target_datatype = dt_ptr->basic_type;

    segment_ptr = MPID_Segment_alloc();
    MPIU_Assert(segment_ptr);

    MPID_Segment_init((void *)am_hdr.target_addr, target_count, target_datatype, segment_ptr, 0);
    last = data_sz;
    MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, (MPI_Aint *) &n_iov);
    MPIU_Assert(n_iov > 0);
    am_hdr.n_iov = n_iov;
    dt_iov = (struct iovec *) MPIU_Malloc(n_iov * sizeof(struct iovec));
    MPIU_Assert(dt_iov);

    last = data_sz;
    MPID_Segment_pack_vector(segment_ptr, 0, &last, dt_iov, &n_iov);
    MPIU_Assert(last == (MPI_Aint)data_sz);
    MPIU_Free(segment_ptr);

    am_iov[0].iov_base = &am_hdr;
    am_iov[0].iov_len = sizeof(am_hdr);
    am_iov[1].iov_base = dt_iov;
    am_iov[1].iov_len = sizeof(struct iovec) * am_hdr.n_iov;

    MPIU_CH4U_REQUEST(sreq, areq.dt_iov) = dt_iov;
    MPIDU_RC_POP(MPIDI_netmod_send_amv(target_rank, win->comm_ptr, op_type,
                                       &am_iov[0], 2, origin_addr, origin_count, origin_datatype,
                                       sreq, NULL));

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_DO_ACCUMULATE);
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_raccumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_raccumulate(const void *origin_addr,
                                          int origin_count,
                                          MPI_Datatype origin_datatype,
                                          int target_rank,
                                          MPI_Aint target_disp,
                                          int target_count,
                                          MPI_Datatype target_datatype,
                                          MPI_Op op, MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RACCUMULATE);

    sreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;
    if (request)
        *request = sreq;

    MPIDU_RC_POP(MPIDI_CH4U_do_accumulate(origin_addr,
                                          origin_count,
                                          origin_datatype,
                                          target_rank,
                                          target_disp,
                                          target_count,
                                          target_datatype, 
                                          op, win, 0, sreq));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_accumulate(const void *origin_addr,
                                         int origin_count,
                                         MPI_Datatype origin_datatype,
                                         int target_rank,
                                         MPI_Aint target_disp,
                                         int target_count,
                                         MPI_Datatype target_datatype, 
                                         MPI_Op op, MPID_Win * win)
{
    int mpi_errno=MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_ACCUMULATE);
    MPIDU_RC_POP(MPIDI_CH4U_raccumulate(origin_addr,
                                        origin_count,
                                        origin_datatype,
                                        target_rank,
                                        target_disp,
                                        target_count,
                                        target_datatype,
                                        op, win, NULL));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_rget_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_rget_accumulate(const void *origin_addr,
                                              int origin_count,
                                              MPI_Datatype origin_datatype,
                                              void *result_addr,
                                              int result_count,
                                              MPI_Datatype result_datatype,
                                              int target_rank,
                                              MPI_Aint target_disp,
                                              int target_count,
                                              MPI_Datatype target_datatype,
                                              MPI_Op op, MPID_Win * win, MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RGET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RGET_ACCUMULATE);

    sreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;

    MPIU_CH4U_REQUEST(sreq, areq.result_addr) = result_addr;
    MPIU_CH4U_REQUEST(sreq, areq.result_count) = result_count;
    MPIU_CH4U_REQUEST(sreq, areq.result_datatype) = result_datatype;
    dtype_add_ref_if_not_builtin(result_datatype);

    if (request)
        *request = sreq;

    MPIDU_RC_POP(MPIDI_CH4U_do_accumulate(origin_addr,
                                          origin_count,
                                          origin_datatype,
                                          target_rank,
                                          target_disp,
                                          target_count,
                                          target_datatype, 
                                          op, win, 1, sreq));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RGET_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_get_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_get_accumulate(const void *origin_addr,
                                             int origin_count,
                                             MPI_Datatype origin_datatype,
                                             void *result_addr,
                                             int result_count,
                                             MPI_Datatype result_datatype,
                                             int target_rank,
                                             MPI_Aint target_disp,
                                             int target_count,
                                             MPI_Datatype target_datatype,
                                             MPI_Op op, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_GET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_GET_ACCUMULATE);
    MPIDU_RC_POP(MPIDI_CH4U_rget_accumulate(origin_addr,
                                            origin_count,
                                            origin_datatype,
                                            result_addr,
                                            result_count,
                                            result_datatype,
                                            target_rank,
                                            target_disp,
                                            target_count,
                                            target_datatype,
                                            op, win, NULL));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_compare_and_swap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_compare_and_swap(const void *origin_addr,
                                               const void *compare_addr,
                                               void *result_addr,
                                               MPI_Datatype datatype,
                                               int target_rank,
                                               MPI_Aint target_disp, MPID_Win * win)
{
    int                rc, mpi_errno = MPI_SUCCESS, c;
    size_t             offset;
    MPID_Request      *sreq = NULL;
    MPIDI_CH4U_cswap_req_msg_t am_hdr;
    MPIDI_CH4I_win_info_t *winfo;
    uint64_t dt_contig, data_sz, dt_true_lb;
    MPID_Datatype *dt_ptr;
    void *p_data;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_COMPARE_AND_SWAP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_COMPARE_AND_SWAP);

    MPIDI_CH4I_EPOCH_CHECK1();

    winfo = MPIU_CH4U_WINFO(win, target_rank);
    offset   = target_disp * winfo->disp_unit;

    sreq = MPIDI_CH4I_create_win_req();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;

    MPIDI_Datatype_get_info(1, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    if (data_sz == 0) {
        MPID_cc_decr(sreq->cc_ptr, &c);
        MPIU_Assert(c >= 0);
        MPIDI_Request_release(sreq);
        goto fn_exit;
    }

    p_data = MPIU_Malloc(data_sz * 2);
    MPIU_Assert(p_data);
    MPIU_Memcpy(p_data, (char *)origin_addr + dt_true_lb, data_sz);
    MPIU_Memcpy((char *)p_data + data_sz, (char *)compare_addr + dt_true_lb, data_sz);

    MPIU_CH4U_REQUEST(sreq, creq.win_ptr) = (uint64_t) win;
    MPIU_CH4U_REQUEST(sreq, creq.addr) = (uint64_t)((char *) result_addr + dt_true_lb);
    MPIU_CH4U_REQUEST(sreq, creq.datatype) = datatype;
    MPIU_CH4U_REQUEST(sreq, creq.result_addr) = result_addr;
    MPIU_CH4U_REQUEST(sreq, creq.data) = p_data;

    MPID_cc_incr(sreq->cc_ptr, &c);
    MPIDI_CH4I_EPOCH_START_CHECK();

    am_hdr.addr = winfo->base_addr + offset;
    am_hdr.datatype = datatype;
    am_hdr.req_ptr = (uint64_t) sreq;

    /* MPIDI_CS_ENTER(); */
    MPIU_CH4U_WIN(win, outstanding_ops)++;
    /* MPIDI_CS_EXIT(); */

    MPIDU_RC_POP(MPIDI_netmod_send_am(target_rank, win->comm_ptr, MPIDI_CH4U_AM_CSWAP_REQ,
                                      &am_hdr, sizeof(am_hdr),
                                      (char *)p_data - dt_true_lb, 2, datatype, sreq, NULL));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_COMPARE_AND_SWAP);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_fetch_and_op
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4U_fetch_and_op(const void *origin_addr,
                                           void *result_addr,
                                           MPI_Datatype datatype,
                                           int target_rank,
                                           MPI_Aint target_disp, MPI_Op op, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FETCH_AND_OP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FETCH_AND_OP);
    MPIDU_RC_POP(MPIDI_CH4U_get_accumulate(origin_addr, 1, datatype,
                                           result_addr, 1, datatype,
                                           target_rank, target_disp, 1, datatype,
                                           op, win));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FETCH_AND_OP);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* MPIDCH4U_RMA_H_INCLUDED */
