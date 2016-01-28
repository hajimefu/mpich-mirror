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
    int mpi_errno = MPI_SUCCESS, n_iov, c;
    MPID_Request *sreq = NULL;
    MPIDI_CH4R_put_msg_t am_hdr;
    uint64_t offset;
    size_t data_sz;
    MPI_Aint last, num_iov;
    MPID_Segment *segment_ptr;
    struct iovec *dt_iov, am_iov[2];

    MPIDI_STATE_DECL(MPID_STATE_CH4I_DO_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_DO_PUT);

    MPIDI_CH4R_EPOCH_CHECK1();

    offset = target_disp * MPIDI_CH4R_WINFO_DISP_UNIT(win, target_rank);

    sreq = MPIDI_CH4I_am_win_request_create();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;
    if (request) {
        *request = sreq;
        MPIDI_Request_add_ref(sreq);
    }

    MPIDI_CH4R_REQUEST(sreq, req->preq.win_ptr) = (uint64_t) win;
    MPIDI_Datatype_check_size(origin_datatype, origin_count, data_sz);
    if (data_sz == 0 || target_rank == MPI_PROC_NULL) {
        MPIDI_CH4I_am_request_complete(sreq);
        goto fn_exit;
    }

    if (target_rank == win->comm_ptr->rank) {
        MPIDI_CH4I_am_request_complete(sreq);
        return MPIR_Localcopy(origin_addr,
                              origin_count,
                              origin_datatype,
                              (char *)win->base + offset,
                              target_count,
                              target_datatype);
    }

    MPIDI_CH4R_EPOCH_START_CHECK(win, mpi_errno, goto fn_fail);
    MPIR_cc_incr(sreq->cc_ptr, &c);
    am_hdr.addr = MPIDI_CH4I_win_base_at_origin(win, target_rank) + offset;
    am_hdr.count = target_count;
    am_hdr.datatype = target_datatype;
    am_hdr.preq_ptr = (uint64_t) sreq;
    am_hdr.win_id = MPIDI_CH4R_WIN(win, win_id);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    if (HANDLE_GET_KIND(target_datatype) == HANDLE_KIND_BUILTIN) {
        am_hdr.n_iov = 0;
        MPIDI_CH4R_REQUEST(sreq, req->preq.dt_iov) = NULL;

        mpi_errno = MPIDI_CH4_NM_send_am(target_rank, win->comm_ptr, MPIDI_CH4R_PUT_REQ,
                                         &am_hdr, sizeof(am_hdr), origin_addr,
                                         origin_count, origin_datatype, sreq, NULL);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        goto fn_exit;
    }

    segment_ptr = MPID_Segment_alloc();
    MPIU_Assert(segment_ptr);

    MPID_Segment_init((void *)am_hdr.addr, target_count, target_datatype, segment_ptr, 0);
    last = data_sz;
    MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, &num_iov);
    n_iov = (int) num_iov;
    MPIU_Assert(n_iov > 0);
    am_hdr.n_iov = n_iov;
    dt_iov = (struct iovec *) MPL_malloc(n_iov * sizeof(struct iovec));
    MPIU_Assert(dt_iov);

    last = data_sz;
    MPID_Segment_pack_vector(segment_ptr, 0, &last, dt_iov, &n_iov);
    MPIU_Assert(last == (MPI_Aint)data_sz);
    MPL_free(segment_ptr);

    am_iov[0].iov_base = &am_hdr;
    am_iov[0].iov_len = sizeof(am_hdr);
    am_iov[1].iov_base = dt_iov;
    am_iov[1].iov_len = sizeof(struct iovec) * am_hdr.n_iov;

    MPIDI_CH4R_REQUEST(sreq, req->preq.dt_iov) = dt_iov;

    if ((am_iov[0].iov_len + am_iov[1].iov_len) <= MPIDI_CH4_NM_am_hdr_max_sz()) {
        mpi_errno = MPIDI_CH4_NM_send_amv(target_rank, win->comm_ptr, MPIDI_CH4R_PUT_REQ,
                                          &am_iov[0], 2, origin_addr, origin_count, origin_datatype,
                                          sreq, NULL);
    } else {
        MPIDI_CH4R_REQUEST(sreq, req->preq.origin_addr) = (void *)origin_addr;
        MPIDI_CH4R_REQUEST(sreq, req->preq.origin_count) = origin_count;
        MPIDI_CH4R_REQUEST(sreq, req->preq.origin_datatype) = origin_datatype;
        dtype_add_ref_if_not_builtin(origin_datatype);

        mpi_errno = MPIDI_CH4_NM_send_am(target_rank, win->comm_ptr, MPIDI_CH4R_PUT_IOV_REQ,
                                         &am_hdr, sizeof(am_hdr), am_iov[1].iov_base,
                                         am_iov[1].iov_len, MPI_BYTE, sreq, NULL);
    }
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

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
    int                mpi_errno = MPI_SUCCESS, n_iov, c;
    size_t             offset;
    MPID_Request      *sreq = NULL;
    MPIDI_CH4R_get_req_msg_t am_hdr;
    size_t data_sz;
    MPI_Aint last, num_iov;
    MPID_Segment *segment_ptr;
    struct iovec *dt_iov;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_MPIDI_CH4I_DO_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_MPIDI_CH4I_DO_GET);

    MPIDI_CH4R_EPOCH_CHECK1();

    offset = target_disp * MPIDI_CH4R_WINFO_DISP_UNIT(win, target_rank);

    sreq = MPIDI_CH4I_am_win_request_create();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;
    if (request) {
        *request = sreq;
        MPIDI_Request_add_ref(sreq);
    }

    MPIDI_Datatype_check_size(origin_datatype, origin_count, data_sz);
    if (data_sz == 0 || target_rank == MPI_PROC_NULL) {
        MPIDI_CH4I_am_request_complete(sreq);
        goto fn_exit;
    }

    MPIDI_CH4R_REQUEST(sreq, req->greq.win_ptr) = (uint64_t) win;
    MPIDI_CH4R_REQUEST(sreq, req->greq.addr) = (uint64_t)((char *) origin_addr);
    MPIDI_CH4R_REQUEST(sreq, req->greq.count) = origin_count;
    MPIDI_CH4R_REQUEST(sreq, req->greq.datatype) = origin_datatype;

    if (target_rank == win->comm_ptr->rank) {
        MPIDI_CH4I_am_request_complete(sreq);
        return MPIR_Localcopy((char *)win->base + offset,
                              target_count,
                              target_datatype,
                              origin_addr,
                              origin_count,
                              origin_datatype);
    }

    MPIDI_CH4R_EPOCH_START_CHECK(win, mpi_errno, goto fn_fail);
    MPIR_cc_incr(sreq->cc_ptr, &c);
    am_hdr.addr = MPIDI_CH4I_win_base_at_origin(win, target_rank) + offset;
    am_hdr.count = target_count;
    am_hdr.datatype = target_datatype;
    am_hdr.greq_ptr = (uint64_t) sreq;
    am_hdr.win_id = MPIDI_CH4R_WIN(win, win_id);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    if (HANDLE_GET_KIND(target_datatype) == HANDLE_KIND_BUILTIN) {
        am_hdr.n_iov = 0;
        MPIDI_CH4R_REQUEST(sreq, req->greq.dt_iov) = NULL;

        mpi_errno = MPIDI_CH4_NM_send_am_hdr(target_rank, win->comm_ptr,
                                             MPIDI_CH4R_GET_REQ,
                                             &am_hdr, sizeof(am_hdr), sreq, NULL);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        goto fn_exit;
    }

    segment_ptr = MPID_Segment_alloc();
    MPIU_Assert(segment_ptr);

    MPID_Segment_init((void *)am_hdr.addr, target_count, target_datatype, segment_ptr, 0);
    last = data_sz;
    MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, &num_iov);
    n_iov = (int) num_iov;
    MPIU_Assert(n_iov > 0);
    am_hdr.n_iov = n_iov;
    dt_iov = (struct iovec *) MPL_malloc(n_iov * sizeof(struct iovec));
    MPIU_Assert(dt_iov);

    last = data_sz;
    MPID_Segment_pack_vector(segment_ptr, 0, &last, dt_iov, &n_iov);
    MPIU_Assert(last == (MPI_Aint)data_sz);
    MPL_free(segment_ptr);

    MPIDI_CH4R_REQUEST(sreq, req->greq.dt_iov) = dt_iov;
    mpi_errno = MPIDI_CH4_NM_send_am(target_rank, win->comm_ptr, MPIDI_CH4R_GET_REQ,
                                     &am_hdr, sizeof(am_hdr), dt_iov,
                                     sizeof(struct iovec) * am_hdr.n_iov,
                                     MPI_BYTE, sreq, NULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_MPIDI_CH4I_DO_GET);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_put
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_put(const void *origin_addr,
                                  int origin_count,
                                  MPI_Datatype origin_datatype,
                                  int target_rank,
                                  MPI_Aint target_disp,
                                  int target_count, MPI_Datatype target_datatype, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_PUT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_PUT);

    mpi_errno = MPIDI_CH4I_do_put(origin_addr, origin_count, origin_datatype,
                                  target_rank, target_disp, target_count,
                                  target_datatype, win, NULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_PUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_rput
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_rput(const void *origin_addr,
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

    mpi_errno = MPIDI_CH4I_do_put(origin_addr, origin_count, origin_datatype,
                                  target_rank, target_disp, target_count,
                                  target_datatype, win, request);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RPUT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_get
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_get(void *origin_addr,
                                  int origin_count,
                                  MPI_Datatype origin_datatype,
                                  int target_rank,
                                  MPI_Aint target_disp,
                                  int target_count, MPI_Datatype target_datatype, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_GET);

    mpi_errno = MPIDI_CH4I_do_get(origin_addr, origin_count, origin_datatype,
                                  target_rank, target_disp, target_count,
                                  target_datatype, win, NULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_rget
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_rget(void *origin_addr,
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

    mpi_errno = MPIDI_CH4I_do_get(origin_addr, origin_count, origin_datatype,
                                  target_rank, target_disp, target_count,
                                  target_datatype, win, request);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RGET);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_do_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4I_do_accumulate(const void *origin_addr,
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
    int                mpi_errno = MPI_SUCCESS, c, n_iov;
    size_t             offset, basic_type_size;
    MPIDI_CH4R_acc_req_msg_t am_hdr;
    uint64_t data_sz, result_data_sz, target_data_sz;
    MPI_Aint last, num_iov;
    MPID_Segment *segment_ptr;
    struct iovec *dt_iov, am_iov[2];
    MPID_Datatype *dt_ptr;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_DO_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_DO_ACCUMULATE);

    MPIDI_CH4R_EPOCH_CHECK1();

    offset = target_disp * MPIDI_CH4R_WINFO_DISP_UNIT(win, target_rank);

    MPIDI_Datatype_get_size_dt_ptr(origin_count, origin_datatype, data_sz, dt_ptr);
    MPIDI_Datatype_check_size(target_datatype, target_count, target_data_sz);

    if ((data_sz == 0 && do_get == 0) ||
        target_rank == MPI_PROC_NULL || target_count == 0 || target_data_sz == 0 ||
        (do_get == 1 && origin_count == 0 && MPIDI_CH4R_REQUEST(sreq, req->areq.result_count) == 0)) {
        if (do_get)
            dtype_release_if_not_builtin(MPIDI_CH4R_REQUEST(sreq, req->areq.result_datatype));
        MPIDI_CH4I_am_request_complete(sreq);
        goto fn_exit;
    }

    MPIDI_CH4R_REQUEST(sreq, req->areq.win_ptr) = (uint64_t) win;
    MPIDI_CH4R_EPOCH_START_CHECK(win, mpi_errno, goto fn_fail);
    MPIR_cc_incr(sreq->cc_ptr, &c);

    am_hdr.req_ptr = (uint64_t) sreq;
    am_hdr.origin_count = origin_count;
    am_hdr.do_get = do_get;

    if (HANDLE_GET_KIND(origin_datatype) == HANDLE_KIND_BUILTIN) {
        am_hdr.origin_datatype = origin_datatype;
    } else {
        am_hdr.origin_datatype = (dt_ptr) ? dt_ptr->basic_type : MPI_DATATYPE_NULL;
        MPID_Datatype_get_size_macro(am_hdr.origin_datatype, basic_type_size);
        am_hdr.origin_count = (basic_type_size > 0) ? data_sz / basic_type_size : 0;
    }

    am_hdr.target_count = target_count;
    am_hdr.target_datatype = target_datatype;
    am_hdr.target_addr = MPIDI_CH4I_win_base_at_origin(win, target_rank) + offset;
    am_hdr.op = op;
    am_hdr.win_id = MPIDI_CH4R_WIN(win, win_id);

    if (do_get) {
        MPIDI_Datatype_check_size(MPIDI_CH4R_REQUEST(sreq, req->areq.result_datatype),
                                  MPIDI_CH4R_REQUEST(sreq, req->areq.result_count),
                                  result_data_sz);
        am_hdr.result_data_sz = result_data_sz;
    }

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    MPIDI_CH4R_REQUEST(sreq, req->areq.data_sz) = data_sz;
    if (HANDLE_GET_KIND(target_datatype) == HANDLE_KIND_BUILTIN) {
        am_hdr.n_iov = 0;
        MPIDI_CH4R_REQUEST(sreq, req->areq.dt_iov) = NULL;

        mpi_errno = MPIDI_CH4_NM_send_am(target_rank, win->comm_ptr, MPIDI_CH4R_ACC_REQ,
                                         &am_hdr, sizeof(am_hdr), origin_addr,
                                         (op == MPI_NO_OP) ? 0 : origin_count,
                                         origin_datatype, sreq, NULL);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
        goto fn_exit;
    }

    MPIDI_Datatype_get_size_dt_ptr(target_count, target_datatype, data_sz, dt_ptr);
    am_hdr.target_datatype = dt_ptr->basic_type;
    am_hdr.target_count = dt_ptr->n_builtin_elements;

    segment_ptr = MPID_Segment_alloc();
    MPIU_Assert(segment_ptr);

    MPID_Segment_init((void *)am_hdr.target_addr, target_count, target_datatype, segment_ptr, 0);
    last = data_sz;
    MPID_Segment_count_contig_blocks(segment_ptr, 0, &last, &num_iov);
    n_iov = (int) num_iov;
    MPIU_Assert(n_iov > 0);
    am_hdr.n_iov = n_iov;
    dt_iov = (struct iovec *) MPL_malloc(n_iov * sizeof(struct iovec));
    MPIU_Assert(dt_iov);

    last = data_sz;
    MPID_Segment_pack_vector(segment_ptr, 0, &last, dt_iov, &n_iov);
    MPIU_Assert(last == (MPI_Aint)data_sz);
    MPL_free(segment_ptr);

    am_iov[0].iov_base = &am_hdr;
    am_iov[0].iov_len = sizeof(am_hdr);
    am_iov[1].iov_base = dt_iov;
    am_iov[1].iov_len = sizeof(struct iovec) * am_hdr.n_iov;
    MPIDI_CH4R_REQUEST(sreq, req->areq.dt_iov) = dt_iov;

    if ((am_iov[0].iov_len + am_iov[1].iov_len) <= MPIDI_CH4_NM_am_hdr_max_sz()) {
        mpi_errno = MPIDI_CH4_NM_send_amv(target_rank, win->comm_ptr, MPIDI_CH4R_ACC_REQ,
                                          &am_iov[0], 2, origin_addr,
                                          (op == MPI_NO_OP) ? 0 : origin_count,
                                          origin_datatype, sreq, NULL);
    } else {
        MPIDI_CH4R_REQUEST(sreq, req->areq.origin_addr) = (void *) origin_addr;
        MPIDI_CH4R_REQUEST(sreq, req->areq.origin_count) = origin_count;
        MPIDI_CH4R_REQUEST(sreq, req->areq.origin_datatype) = origin_datatype;
        dtype_add_ref_if_not_builtin(origin_datatype);

        mpi_errno = MPIDI_CH4_NM_send_am(target_rank, win->comm_ptr, MPIDI_CH4R_ACC_IOV_REQ,
                                         &am_hdr, sizeof(am_hdr), am_iov[1].iov_base,
                                         am_iov[1].iov_len, MPI_BYTE, sreq, NULL);
    }
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_DO_ACCUMULATE);
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_raccumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_raccumulate(const void *origin_addr,
                                          int origin_count,
                                          MPI_Datatype origin_datatype,
                                          int target_rank,
                                          MPI_Aint target_disp,
                                          int target_count,
                                          MPI_Datatype target_datatype,
                                          MPI_Op op, MPID_Win * win,
                                          MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RACCUMULATE);

    sreq = MPIDI_CH4I_am_win_request_create();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;
    if (request) {
        *request = sreq;
        MPIDI_Request_add_ref(sreq);
    }

    mpi_errno = MPIDI_CH4I_do_accumulate(origin_addr,
                                         origin_count,
                                         origin_datatype,
                                         target_rank,
                                         target_disp,
                                         target_count,
                                         target_datatype,
                                         op, win, 0, sreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_accumulate(const void *origin_addr,
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

    mpi_errno = MPIDI_CH4R_raccumulate(origin_addr,
                                       origin_count,
                                       origin_datatype,
                                       target_rank,
                                       target_disp,
                                       target_count,
                                       target_datatype,
                                       op, win, NULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_rget_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_rget_accumulate(const void *origin_addr,
                                              int origin_count,
                                              MPI_Datatype origin_datatype,
                                              void *result_addr,
                                              int result_count,
                                              MPI_Datatype result_datatype,
                                              int target_rank,
                                              MPI_Aint target_disp,
                                              int target_count,
                                              MPI_Datatype target_datatype,
                                              MPI_Op op, MPID_Win * win,
                                              MPID_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RGET_ACCUMULATE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RGET_ACCUMULATE);

    sreq = MPIDI_CH4I_am_win_request_create();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;

    MPIDI_CH4R_REQUEST(sreq, req->areq.result_addr) = result_addr;
    MPIDI_CH4R_REQUEST(sreq, req->areq.result_count) = result_count;
    MPIDI_CH4R_REQUEST(sreq, req->areq.result_datatype) = result_datatype;
    dtype_add_ref_if_not_builtin(result_datatype);

    if (request) {
        *request = sreq;
        MPIDI_Request_add_ref(sreq);
    }

    mpi_errno = MPIDI_CH4I_do_accumulate(origin_addr,
                                         origin_count,
                                         origin_datatype,
                                         target_rank,
                                         target_disp,
                                         target_count,
                                         target_datatype,
                                         op, win, 1, sreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RGET_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_get_accumulate
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_get_accumulate(const void *origin_addr,
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

    mpi_errno = MPIDI_CH4R_rget_accumulate(origin_addr,
                                           origin_count,
                                           origin_datatype,
                                           result_addr,
                                           result_count,
                                           result_datatype,
                                           target_rank,
                                           target_disp,
                                           target_count,
                                           target_datatype,
                                           op, win, NULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET_ACCUMULATE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_compare_and_swap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_compare_and_swap(const void *origin_addr,
                                               const void *compare_addr,
                                               void *result_addr,
                                               MPI_Datatype datatype,
                                               int target_rank,
                                               MPI_Aint target_disp, MPID_Win * win)
{
    int                mpi_errno = MPI_SUCCESS, c;
    size_t             offset;
    MPID_Request      *sreq = NULL;
    MPIDI_CH4R_cswap_req_msg_t am_hdr;
    size_t data_sz;
    void *p_data;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_COMPARE_AND_SWAP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_COMPARE_AND_SWAP);

    MPIDI_CH4R_EPOCH_CHECK1();

    offset = target_disp * MPIDI_CH4R_WINFO_DISP_UNIT(win, target_rank);

    sreq = MPIDI_CH4I_am_win_request_create();
    MPIU_Assert(sreq);
    sreq->kind = MPID_WIN_REQUEST;

    MPIDI_Datatype_check_size(datatype, 1, data_sz);
    if (data_sz == 0 || target_rank == MPI_PROC_NULL) {
        MPIDI_CH4I_am_request_complete(sreq);
        goto fn_exit;
    }

    p_data = MPL_malloc(data_sz * 2);
    MPIU_Assert(p_data);
    MPIU_Memcpy(p_data, (char *)origin_addr, data_sz);
    MPIU_Memcpy((char *)p_data + data_sz, (char *)compare_addr, data_sz);

    MPIDI_CH4R_REQUEST(sreq, req->creq.win_ptr) = (uint64_t) win;
    MPIDI_CH4R_REQUEST(sreq, req->creq.addr) = (uint64_t)((char *) result_addr);
    MPIDI_CH4R_REQUEST(sreq, req->creq.datatype) = datatype;
    MPIDI_CH4R_REQUEST(sreq, req->creq.result_addr) = result_addr;
    MPIDI_CH4R_REQUEST(sreq, req->creq.data) = p_data;

    MPIDI_CH4R_EPOCH_START_CHECK(win, mpi_errno, goto fn_fail);
    MPIR_cc_incr(sreq->cc_ptr, &c);

    am_hdr.addr = MPIDI_CH4I_win_base_at_origin(win, target_rank) + offset;
    am_hdr.datatype = datatype;
    am_hdr.req_ptr = (uint64_t) sreq;
    am_hdr.win_id = MPIDI_CH4R_WIN(win, win_id);

    /* MPIDI_CS_ENTER(); */
    OPA_incr_int(&MPIDI_CH4R_WIN(win, outstanding_ops));
    /* MPIDI_CS_EXIT(); */

    mpi_errno = MPIDI_CH4_NM_send_am(target_rank, win->comm_ptr, MPIDI_CH4R_CSWAP_REQ,
                                     &am_hdr, sizeof(am_hdr),
                                     (char *)p_data, 2, datatype, sreq, NULL);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_COMPARE_AND_SWAP);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_fetch_and_op
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4R_fetch_and_op(const void *origin_addr,
                                           void *result_addr,
                                           MPI_Datatype datatype,
                                           int target_rank,
                                           MPI_Aint target_disp, MPI_Op op, MPID_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_FETCH_AND_OP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_FETCH_AND_OP);

    mpi_errno = MPIDI_CH4R_get_accumulate(origin_addr, 1, datatype,
                                          result_addr, 1, datatype,
                                          target_rank, target_disp, 1, datatype,
                                          op, win);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_FETCH_AND_OP);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* MPIDCH4U_RMA_H_INCLUDED */
