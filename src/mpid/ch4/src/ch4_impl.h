/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef CH4_IMPL_H_INCLUDED
#define CH4_IMPL_H_INCLUDED

#include "ch4_types.h"
#include <mpidch4.h>

/* Static inlines */
static inline int MPIDI_CH4U_get_tag(uint64_t match_bits)
{
   int tag = (match_bits & MPIDI_CH4U_TAG_MASK);
   /* Left shift and right shift by MPIDI_CH4U_TAG_SHIFT_UNPACK is to make sure the sign of tag is retained */
    return ((tag << MPIDI_CH4U_TAG_SHIFT_UNPACK) >> MPIDI_CH4U_TAG_SHIFT_UNPACK);
}

static inline int MPIDI_CH4U_get_context(uint64_t match_bits)
{
    return ((int) ((match_bits & MPIDI_CH4U_CONTEXT_MASK) >>
                   (MPIDI_CH4U_TAG_SHIFT + MPIDI_CH4U_SOURCE_SHIFT)));
}

static inline int MPIDI_CH4U_get_context_index(uint64_t context_id)
{
    int raw_prefix, idx, bitpos, gen_id;
    raw_prefix = MPID_CONTEXT_READ_FIELD(PREFIX, context_id);
    idx = raw_prefix / MPIR_CONTEXT_INT_BITS;
    bitpos = raw_prefix % MPIR_CONTEXT_INT_BITS;
    gen_id = (idx * MPIR_CONTEXT_INT_BITS) + (31 - bitpos);
    return gen_id;
}

static inline MPID_Comm *MPIDI_CH4U_context_id_to_comm(uint64_t context_id)
{
    int comm_idx     = MPIDI_CH4U_get_context_index(context_id);
    int subcomm_type = MPID_CONTEXT_READ_FIELD(SUBCOMM, context_id);
    int is_localcomm = MPID_CONTEXT_READ_FIELD(IS_LOCALCOMM, context_id);
    MPIU_Assert(subcomm_type <= 3);
    MPIU_Assert(is_localcomm <= 2);
    return MPIDI_CH4_Global.comm_req_lists[comm_idx].comm[is_localcomm][subcomm_type];
}

static inline MPIDI_CH4U_rreq_t **MPIDI_CH4U_context_id_to_uelist(uint64_t context_id)
{
    int comm_idx     = MPIDI_CH4U_get_context_index(context_id);
    int subcomm_type = MPID_CONTEXT_READ_FIELD(SUBCOMM, context_id);
    int is_localcomm = MPID_CONTEXT_READ_FIELD(IS_LOCALCOMM, context_id);
    MPIU_Assert(subcomm_type <= 3);
    MPIU_Assert(is_localcomm <= 2);
    return &MPIDI_CH4_Global.comm_req_lists[comm_idx].uelist[is_localcomm][subcomm_type];
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_alloc_and_init_req
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline MPID_Request *MPIDI_CH4I_alloc_and_init_req(int refcount)
{
    MPID_Request *req;

    MPIDI_STATE_DECL(MPID_STATE_CH4I_ALLOC_AND_INIT_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_ALLOC_AND_INIT_REQ);

    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIDI_CH4U_REQUEST(req, req) = NULL;
    MPIR_cc_set(&req->cc, 1);
    MPIU_Object_set_ref(req, refcount);
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->cc_ptr            = &req->cc;
    req->greq_fns          = NULL;
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG    = MPI_UNDEFINED;
    req->status.MPI_ERROR  = MPI_SUCCESS;
    req->comm              = NULL;
    req->errflag           = MPIR_ERR_NONE;
    MPIR_REQUEST_CLEAR_DBG(req);
#ifdef MPIDI_BUILD_CH4_SHM
    MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req) = NULL;
#endif
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_ALLOC_AND_INIT_REQ);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_request_release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4I_request_release(MPID_Request * req)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4I_REQUEST_RELEASE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_REQEUST_RELEASE);
    int count;
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIU_Object_release_ref(req, &count);
    MPIU_Assert(count >= 0);

    if (count == 0) {
        MPIU_Assert(MPIR_cc_is_complete(&req->cc));

        if (req->comm)
            MPIR_Comm_release(req->comm);

        if (req->greq_fns)
            MPL_free(req->greq_fns);

        MPIU_Handle_obj_free(&MPIDI_Request_mem, req);
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_REQUEST_RELEASE);
    return;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_request_release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4U_request_release(MPID_Request * req)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4I_REQUEST_RELEASE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_REQEUST_RELEASE);

    if (req->kind == MPID_PREQUEST_RECV && NULL != MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req)) {
        MPIDI_CH4I_request_release(MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req));
    }
    MPIDI_CH4I_request_release(req);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_REQUEST_RELEASE);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4U_request_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4U_request_complete(MPID_Request *req)
{
    int incomplete;
    MPIR_cc_decr(req->cc_ptr, &incomplete);
    if (!incomplete)
        MPIDI_CH4U_request_release(req);
}

#ifndef dtype_add_ref_if_not_builtin
#define dtype_add_ref_if_not_builtin(datatype_)                         \
    do {								\
	if ((datatype_) != MPI_DATATYPE_NULL &&				\
	    HANDLE_GET_KIND((datatype_)) != HANDLE_KIND_BUILTIN)	\
	{								\
	    MPID_Datatype *dtp_ = NULL;					\
	    MPID_Datatype_get_ptr((datatype_), dtp_);			\
	    MPID_Datatype_add_ref(dtp_);				\
	}								\
    } while (0)
#endif

#ifndef dtype_release_if_not_builtin
#define dtype_release_if_not_builtin(datatype_)				\
    do {								\
	if ((datatype_) != MPI_DATATYPE_NULL &&				\
	    HANDLE_GET_KIND((datatype_)) != HANDLE_KIND_BUILTIN)	\
	{								\
	    MPID_Datatype *dtp_ = NULL;					\
	    MPID_Datatype_get_ptr((datatype_), dtp_);			\
	    MPID_Datatype_release(dtp_);				\
	}								\
    } while (0)
#endif

#define MPIDI_Datatype_get_info(_count, _datatype,                      \
                                _dt_contig_out, _data_sz_out,           \
                                _dt_ptr, _dt_true_lb)                   \
    do {								\
	if (IS_BUILTIN(_datatype))					\
	{								\
	    (_dt_ptr)        = NULL;					\
	    (_dt_contig_out) = TRUE;					\
	    (_dt_true_lb)    = 0;					\
	    (_data_sz_out)   = (size_t)(_count) *		\
		MPID_Datatype_get_basic_size(_datatype);		\
	}								\
	else								\
	{								\
	    MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
            if (_dt_ptr)                                                \
            {                                                           \
                (_dt_contig_out) = (_dt_ptr)->is_contig;                \
                (_dt_true_lb)    = (_dt_ptr)->true_lb;                  \
                (_data_sz_out)   = (size_t)(_count) *           \
                    (_dt_ptr)->size;                                    \
            }                                                           \
            else                                                        \
            {                                                           \
                (_dt_contig_out) = 1;                                   \
                (_dt_true_lb)    = 0;                                   \
                (_data_sz_out)   = 0;                                   \
            }								\
        }                                                               \
    } while (0)

#define MPIDI_Datatype_get_size_dt_ptr(_count, _datatype,               \
                                       _data_sz_out, _dt_ptr)           \
    do {								\
	if (IS_BUILTIN(_datatype))					\
	{								\
	    (_dt_ptr)        = NULL;					\
	    (_data_sz_out)   = (size_t)(_count) *		\
		MPID_Datatype_get_basic_size(_datatype);		\
	}								\
	else								\
	{								\
	    MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
	    (_data_sz_out)   = (_dt_ptr) ? (size_t)(_count) *   \
                (_dt_ptr)->size : 0;                                    \
	}								\
    } while (0)

#define MPIDI_Datatype_check_contig(_datatype,_dt_contig_out)	\
    do {							\
      if (IS_BUILTIN(_datatype))				\
      {								\
       (_dt_contig_out) = TRUE;					\
       }							\
      else							\
      {								\
       MPID_Datatype *_dt_ptr;					\
       MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
       (_dt_contig_out) = (_dt_ptr) ? (_dt_ptr)->is_contig : 1; \
      }                                                         \
    } while (0)

#define MPIDI_Datatype_check_contig_size(_datatype,_count,              \
                                         _dt_contig_out,                \
                                         _data_sz_out)                  \
    do {								\
      if (IS_BUILTIN(_datatype))					\
      {                                                                 \
	  (_dt_contig_out) = TRUE;					\
	  (_data_sz_out)   = (size_t)(_count) *			\
	      MPID_Datatype_get_basic_size(_datatype);			\
      }                                                                 \
      else								\
      {                                                                 \
	  MPID_Datatype *_dt_ptr;					\
	  MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
          if (_dt_ptr)                                                  \
          {                                                             \
              (_dt_contig_out) = (_dt_ptr)->is_contig;                  \
              (_data_sz_out)   = (size_t)(_count) *             \
                  (_dt_ptr)->size;                                      \
          }                                                             \
          else                                                          \
          {                                                             \
              (_dt_contig_out) = 1;                                     \
              (_data_sz_out)   = 0;                                     \
          }                                                             \
      }                                                                 \
    } while (0)

#define MPIDI_Datatype_check_size(_datatype,_count,_data_sz_out)        \
    do {								\
        if (IS_BUILTIN(_datatype))                                      \
        {                                                               \
            (_data_sz_out)   = (size_t)(_count) *               \
                MPID_Datatype_get_basic_size(_datatype);                \
        }                                                               \
        else                                                            \
        {                                                               \
            MPID_Datatype *_dt_ptr;                                     \
            MPID_Datatype_get_ptr((_datatype), (_dt_ptr));              \
            (_data_sz_out)   = (_dt_ptr) ? (size_t)(_count) *   \
                (_dt_ptr)->size : 0;                                    \
        }                                                               \
    } while (0)

#define MPIDI_Datatype_check_contig_size_lb(_datatype,_count,           \
                                            _dt_contig_out,             \
                                            _data_sz_out,               \
                                            _dt_true_lb)                \
    do {								\
	if (IS_BUILTIN(_datatype))					\
	{								\
	    (_dt_contig_out) = TRUE;					\
	    (_data_sz_out)   = (size_t)(_count) *		\
		MPID_Datatype_get_basic_size(_datatype);		\
	    (_dt_true_lb)    = 0;					\
	}								\
	else								\
	{								\
	    MPID_Datatype *_dt_ptr;					\
	    MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
            if (_dt_ptr)                                                \
            {                                                           \
                (_dt_contig_out) = (_dt_ptr)->is_contig;                \
                (_data_sz_out)   = (size_t)(_count) *           \
                    (_dt_ptr)->size;                                    \
                (_dt_true_lb)    = (_dt_ptr)->true_lb;                  \
            }                                                           \
            else                                                        \
            {                                                           \
                (_dt_contig_out) = 1;                                   \
                (_data_sz_out)   = 0;                                   \
                (_dt_true_lb)    = 0;                                   \
            }                                                           \
	}								\
    } while (0)

#define MPIDI_Request_create_null_rreq(rreq_, mpi_errno_, FAIL_)        \
  do {                                                                  \
    (rreq_) = MPIDI_Request_create();                                   \
    if ((rreq_) != NULL) {                                              \
      MPIU_Object_set_ref((rreq_), 1);                                  \
      MPIR_cc_set(&(rreq_)->cc, 0);                                     \
      (rreq_)->kind = MPID_REQUEST_RECV;                                \
      MPIR_Status_set_procnull(&(rreq_)->status);                       \
    }                                                                   \
    else {                                                              \
      MPIR_ERR_SETANDJUMP(mpi_errno_,MPI_ERR_OTHER,"**nomemreq");       \
    }                                                                   \
  } while (0)

#define IS_BUILTIN(_datatype)				\
    (HANDLE_GET_KIND(_datatype) == HANDLE_KIND_BUILTIN)

#ifndef container_of
#define container_of(ptr, type, field)			\
    ((type *) ((char *)ptr - offsetof(type, field)))
#endif

static inline uint64_t MPIDI_CH4U_init_send_tag(MPIU_Context_id_t contextid, int source, int tag)
{
    uint64_t match_bits;
    match_bits = contextid;
    match_bits = (match_bits << MPIDI_CH4U_SOURCE_SHIFT);
    match_bits |= (source & (MPIDI_CH4U_SOURCE_MASK >> MPIDI_CH4U_TAG_SHIFT));
    match_bits = (match_bits << MPIDI_CH4U_TAG_SHIFT);
    match_bits |= (MPIDI_CH4U_TAG_MASK & tag);
    return match_bits;
}

static inline uint64_t MPIDI_CH4U_init_recvtag(uint64_t * mask_bits,
                                              MPIU_Context_id_t contextid, int source, int tag)
{
    uint64_t match_bits = 0;
    *mask_bits = MPIDI_CH4U_PROTOCOL_MASK;
    match_bits = contextid;
    match_bits = (match_bits << MPIDI_CH4U_SOURCE_SHIFT);

    if (MPI_ANY_SOURCE == source) {
        match_bits = (match_bits << MPIDI_CH4U_TAG_SHIFT);
        *mask_bits |= MPIDI_CH4U_SOURCE_MASK;
    }
    else {
        match_bits |= source;
        match_bits = (match_bits << MPIDI_CH4U_TAG_SHIFT);
    }

    if (MPI_ANY_TAG == tag)
        *mask_bits |= MPIDI_CH4U_TAG_MASK;
    else
        match_bits |= (MPIDI_CH4U_TAG_MASK & tag);

    return match_bits;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_valid_group_rank
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4I_valid_group_rank(MPID_Comm  *comm,
                                              int         rank,
                                              MPID_Group *grp)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4I_VALID_GROUP_RANK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4I_VALID_GROUP_RANK);

    int lpid;
    int size = grp->size;
    int z;
    int ret;

    if (unlikely(rank == MPI_PROC_NULL)) {
        /* Treat PROC_NULL as always valid */
        ret = 1;
        goto fn_exit;
    }

    MPIDI_CH4_NM_comm_get_lpid(comm, rank, &lpid, FALSE);

    for(z = 0; z < size &&lpid != grp->lrank_to_lpid[z].lpid; ++z) {}

    ret = (z < size);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4I_VALID_GROUP_RANK);
fn_exit:
    return ret;
}

#define MPIDI_CH4R_PROGRESS()                                   \
    do {							\
	mpi_errno = MPIDI_Progress_test();			\
	if (mpi_errno != MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);	\
    } while (0)

#define MPIDI_CH4R_PROGRESS_WHILE(cond)         \
    do {					\
	while (cond)				\
	    MPIDI_CH4R_PROGRESS();              \
    } while (0)

#ifdef HAVE_ERROR_CHECKING
#define MPIDI_CH4U_EPOCH_CHECK_SYNC(win, mpi_errno, stmt)               \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIDI_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4U_WIN(win, sync).target_epoch_type && \
           MPIDI_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4U_EPOTYPE_REFENCE) \
        {                                                               \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4U_EPOTYPE_FENCE; \
            MPIDI_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4U_EPOTYPE_FENCE; \
        }                                                               \
        if(MPIDI_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4U_EPOTYPE_NONE || \
           MPIDI_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4U_EPOTYPE_POST) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                stmt, "**rmasync");                     \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#define MPIDI_CH4U_EPOCH_CHECK_TYPE(win,mpi_errno,stmt)            \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_NONE && \
           MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_REFENCE) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                stmt, "**rmasync");                     \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#define MPIDI_CH4U_EPOCH_START_CHECK(win,mpi_errno,stmt)                \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if (MPIDI_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4U_EPOTYPE_START && \
            !MPIDI_CH4I_valid_group_rank(win->comm_ptr, target_rank,    \
                                         MPIDI_CH4U_WIN(win, sync).sc.group)) \
            MPIR_ERR_SETANDSTMT(mpi_errno,                              \
                                MPI_ERR_RMA_SYNC,                       \
                                stmt,                                   \
                                "**rmasync");                           \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#define MPIDI_CH4U_EPOCH_START_CHECK2(win,mpi_errno,stmt)               \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if (MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_START) { \
            MPIR_ERR_SETANDSTMT(mpi_errno,                              \
                                MPI_ERR_RMA_SYNC,                       \
                                stmt,                                   \
                                "**rmasync");                           \
        }                                                               \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#define MPIDI_CH4U_EPOCH_FENCE_CHECK(win,mpi_errno,stmt)                \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_WIN(win, sync).target_epoch_type) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                stmt, "**rmasync");                     \
        if (!(massert & MPI_MODE_NOPRECEDE) &&                          \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_FENCE && \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_REFENCE && \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_NONE) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                stmt, "**rmasync");                     \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#define MPIDI_CH4U_EPOCH_POST_CHECK(win,mpi_errno,stmt)            \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIDI_CH4U_WIN(win, sync).target_epoch_type != MPIDI_CH4U_EPOTYPE_NONE && \
           MPIDI_CH4U_WIN(win, sync).target_epoch_type != MPIDI_CH4U_EPOTYPE_REFENCE) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                stmt, "**rmasync");                     \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#define MPIDI_CH4U_EPOCH_LOCK_CHECK(win,mpi_errno,stmt)               \
do {                                                                  \
    MPID_BEGIN_ERROR_CHECKS;                                      \
    if((MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_LOCK) && \
       (MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_LOCK_ALL)) \
        MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                \
                            stmt, "**rmasync");                 \
    MPID_END_ERROR_CHECKS;                                              \
} while (0)

#define MPIDI_CH4U_EPOCH_FREE_CHECK(win,mpi_errno,stmt)                 \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_WIN(win, sync).target_epoch_type || \
           (MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_NONE && \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type != MPIDI_CH4U_EPOTYPE_REFENCE)) \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC, stmt, "**rmasync"); \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#define MPIDI_CH4U_EPOCH_ORIGIN_CHECK(win, epoch_type, mpi_errno, stmt) \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIDI_CH4U_WIN(win, sync).origin_epoch_type != epoch_type)    \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                stmt, "**rmasync");                     \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#define MPIDI_CH4U_EPOCH_TARGET_CHECK(win, epoch_type, mpi_errno, stmt) \
    do {                                                                \
        MPID_BEGIN_ERROR_CHECKS;                                        \
        if(MPIDI_CH4U_WIN(win, sync).target_epoch_type != epoch_type)    \
            MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,            \
                                stmt, "**rmasync");         \
        MPID_END_ERROR_CHECKS;                                          \
    } while (0)

#else /* HAVE_ERROR_CHECKING */
#define MPIDI_CH4U_EPOCH_CHECK_SYNC(win, mpi_errno, stmt)               if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_CHECK_TYPE(win, mpi_errno, stmt)               if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_START_CHECK(win, mpi_errno, stmt)              if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_START_CHECK2(win, mpi_errno, stmt)             if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_FENCE_CHECK(win, mpi_errno, stmt)              if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_POST_CHECK(win, mpi_errno, stmt)               if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_LOCK_CHECK(win, mpi_errno, stmt)               if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_FREE_CHECK(win, mpi_errno, stmt)               if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_ORIGIN_CHECK(win, epoch_type, mpi_errno, stmt) if(0) goto fn_fail;
#define MPIDI_CH4U_EPOCH_TARGET_CHECK(win, epoch_type, mpi_errno, stmt) if(0) goto fn_fail;
#endif /* HAVE_ERROR_CHECKING */

#define MPIDI_CH4U_EPOCH_FENCE_EVENT(win, massert)                 \
    do {                                                                \
        if(massert & MPI_MODE_NOSUCCEED)                           \
        {                                                               \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4U_EPOTYPE_NONE; \
            MPIDI_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4U_EPOTYPE_NONE; \
        }                                                               \
        else                                                            \
        {                                                               \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4U_EPOTYPE_REFENCE; \
            MPIDI_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4U_EPOTYPE_REFENCE; \
        }                                                               \
    } while (0)

#define MPIDI_CH4U_EPOCH_TARGET_EVENT(win)                              \
    do {                                                            \
        if(MPIDI_CH4U_WIN(win, sync).target_epoch_type == MPIDI_CH4U_EPOTYPE_REFENCE) \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4U_EPOTYPE_REFENCE; \
        else                                                            \
            MPIDI_CH4U_WIN(win, sync).origin_epoch_type = MPIDI_CH4U_EPOTYPE_NONE; \
    } while (0)

#define MPIDI_CH4U_EPOCH_ORIGIN_EVENT(Win)                              \
    do {                                                                \
        if(MPIDI_CH4U_WIN(win, sync).origin_epoch_type == MPIDI_CH4U_EPOTYPE_REFENCE) \
            MPIDI_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4U_EPOTYPE_REFENCE; \
        else                                                            \
            MPIDI_CH4U_WIN(win, sync).target_epoch_type = MPIDI_CH4U_EPOTYPE_NONE; \
    } while (0)

/*
  Calculate base address of the target window at the origin side
  Return zero to let the target side calculate the actual address
  (only offset from window base is given to the target in this case)
*/
static inline uintptr_t MPIDI_CH4I_win_base_at_origin(const MPID_Win *win, int target_rank)
{
    /* TODO: In future we may want to calculate the full virtual address
       in the target at the origin side. It can be done by looking at
       MPIDI_CH4U_WINFO(win, target_rank)->base_addr */
    return 0;
}

/*
  Calculate base address of the window at the target side
  If MPIDI_CH4I_win_base_at_origin calculates the full virtual address
  this function must return zero
*/
static inline uintptr_t MPIDI_CH4I_win_base_at_target(const MPID_Win *win)
{
    return (uintptr_t) win->base;
}

#endif /* CH4_IMPL_H_INCLUDED */
