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
#ifndef MPIDCH4_IMPL_H_INCLUDED
#define MPIDCH4_IMPL_H_INCLUDED

#include "ch4_types.h"

static inline int MPIDI_CH4U_Get_source(uint64_t match_bits)
{
    return ((int) ((match_bits & MPIDI_CH4U_SOURCE_MASK) >> MPIDI_CH4U_TAG_SHIFT));
}

static inline int MPIDI_CH4U_Get_tag(uint64_t match_bits)
{
    return ((int) (match_bits & MPIDI_CH4U_TAG_MASK));
}

static inline int MPIDI_CH4U_Get_context(uint64_t match_bits)
{
    return ((int) ((match_bits & MPIDI_CH4U_CONTEXT_MASK) >> \
		   (MPIDI_CH4U_TAG_SHIFT + MPIDI_CH4U_SOURCE_SHIFT)));
}

static inline int MPIDI_CH4U_Get_context_index(uint64_t context_id)
{
    int raw_prefix, idx, bitpos, gen_id;
    raw_prefix = MPID_CONTEXT_READ_FIELD(PREFIX,context_id);
    idx        = raw_prefix / MPIR_CONTEXT_INT_BITS;
    bitpos     = raw_prefix % MPIR_CONTEXT_INT_BITS;
    gen_id     = (idx*MPIR_CONTEXT_INT_BITS) + (31-bitpos);
    return gen_id;
}

static inline MPID_Request *MPIDI_CH4U_Create_req()
{
    MPID_Request *req = MPIDI_netmod_request_create();
    MPIU_CH4U_REQUEST(req, status) = 0;
    return req;
}

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

#define MPIDI_Datatype_get_info(_count, _datatype,                      \
                                _dt_contig_out, _data_sz_out,           \
                                _dt_ptr, _dt_true_lb)                   \
    ({									\
	if (IS_BUILTIN(_datatype))					\
	{								\
	    (_dt_ptr)        = NULL;					\
	    (_dt_contig_out) = TRUE;					\
	    (_dt_true_lb)    = 0;					\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *		\
		MPID_Datatype_get_basic_size(_datatype);		\
	}								\
	else								\
	{								\
	    MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
	    (_dt_contig_out) = (_dt_ptr)->is_contig;			\
	    (_dt_true_lb)    = (_dt_ptr)->true_lb;			\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size; \
	}								\
    })

#define MPIDI_Datatype_check_contig(_datatype,_dt_contig_out)	\
    ({								\
      if (IS_BUILTIN(_datatype))				\
      {								\
       (_dt_contig_out) = TRUE;					\
       }							\
      else							\
      {								\
       MPID_Datatype *_dt_ptr;					\
       MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
       (_dt_contig_out) = (_dt_ptr)->is_contig;			\
       }							\
      })

#define MPIDI_Datatype_check_contig_size(_datatype,_count,              \
                                         _dt_contig_out,                \
                                         _data_sz_out)                  \
    ({									\
      if (IS_BUILTIN(_datatype))					\
      {                                                                 \
	  (_dt_contig_out) = TRUE;					\
	  (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *			\
	      MPID_Datatype_get_basic_size(_datatype);			\
      }                                                                 \
      else								\
      {                                                                 \
	  MPID_Datatype *_dt_ptr;					\
	  MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
	  (_dt_contig_out) = (_dt_ptr)->is_contig;			\
	  (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size; \
      }                                                                 \
    })

#define MPIDI_Datatype_check_contig_size_lb(_datatype,_count,           \
                                            _dt_contig_out,             \
                                            _data_sz_out,               \
                                            _dt_true_lb)                \
    ({									\
	if (IS_BUILTIN(_datatype))					\
	{								\
	    (_dt_contig_out) = TRUE;					\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *		\
		MPID_Datatype_get_basic_size(_datatype);		\
	    (_dt_true_lb)    = 0;					\
	}								\
	else								\
	{								\
	    MPID_Datatype *_dt_ptr;					\
	    MPID_Datatype_get_ptr((_datatype), (_dt_ptr));		\
	    (_dt_contig_out) = (_dt_ptr)->is_contig;			\
	    (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size; \
	    (_dt_true_lb)    = (_dt_ptr)->true_lb;			\
	}								\
    })

#define MPIDI_Request_create_null_rreq(rreq_, mpi_errno_, FAIL_)        \
  do {                                                                  \
    (rreq_) = MPIDI_Request_create();                                   \
    if ((rreq_) != NULL) {                                              \
      MPIU_Object_set_ref((rreq_), 1);                                  \
      MPID_cc_set(&(rreq_)->cc, 0);                                     \
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

static inline uint64_t MPID_CH4U_init_send_tag(MPIU_Context_id_t contextid,
				     int source, int tag)
{
    uint64_t match_bits;
    match_bits = contextid;
    match_bits = (match_bits << MPIDI_CH4U_SOURCE_SHIFT);
    match_bits |= source;
    match_bits = (match_bits << MPIDI_CH4U_TAG_SHIFT);
    match_bits |= (MPIDI_CH4U_TAG_MASK & tag);
    return match_bits;
}

static inline uint64_t MPID_CH4U_init_recvtag(uint64_t * mask_bits,
                                    MPIU_Context_id_t contextid, 
				    int source, int tag)
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

#define MPIU_RC_POP(FUNC)					\
    do {							\
	mpi_errno = FUNC;					\
	if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);	\
	} while (0)

#endif /* MPIDCH4_IMPL_H_INCLUDED */
