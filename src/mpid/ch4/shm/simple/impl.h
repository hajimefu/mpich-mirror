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
#ifndef SHM_SIMPLE_IMPL_H_INCLUDED
#define SHM_SIMPLE_IMPL_H_INCLUDED

#include <mpidimpl.h>
#include "mpidch4r.h"

#include "mpihandlemem.h"
#include "mpiu_os_wrappers_pre.h"
#include "mpiu_shm_wrappers.h"
#include "pmi.h"

#include "mpidu_shm.h"

/* ---------------------------------------------------- */
/* temp headers                                         */
/* ---------------------------------------------------- */
#include "datatypes.h"  /* MPID_nem datatypes like cell, fastbox defined here */
#include "defs.h"       /* MPID_nem objects like shared memory region defined here */
#include "queue.h"      /* MPIDI_SIMPLE_queue functions defined here */

/* ---------------------------------------------------- */
/* constants                                            */
/* ---------------------------------------------------- */
#define MPIDI_SIMPLE_EAGER_THRESHOLD MPIDI_SIMPLE_DATA_LEN
#define MPIDI_SIMPLE_TYPESTANDARD    0
#define MPIDI_SIMPLE_TYPEEAGER       1
#define MPIDI_SIMPLE_TYPELMT         2
#define MPIDI_SIMPLE_TYPESYNC        3
#define MPIDI_SIMPLE_TYPEBUFFERED    4
#define MPIDI_SIMPLE_TYPEREADY       5
#define MPIDI_SIMPLE_TYPEACK         6
#define MPIDI_SIMPLE_REQUEST(req)    (&(req)->dev.ch4.shm.simple)

/* ---------------------------------------------------- */
/* shm specific object data                             */
/* ---------------------------------------------------- */
/* VCR Table Data */
typedef struct {
    unsigned int avt_rank;
} MPIDI_SIMPLE_vcr_t;

struct MPIDI_SIMPLE_vcrt_t {
    MPIU_OBJECT_HEADER;
    unsigned                    size;         /**< Number of entries in the table */
    MPIDI_SIMPLE_vcr_t vcr_table[0]; /**< Array of virtual connection references */
};
/* ---------------------------------------------------- */
/* general send/recv queue types, macros and objects    */
/* ---------------------------------------------------- */
typedef struct {
    MPIR_Request *head;
    MPIR_Request *tail;
} MPIDI_SIMPLE_request_queue_t;

#define MPIDI_SIMPLE_REQUEST_COMPLETE(req_)    \
{ \
    int incomplete__; \
    MPIR_cc_decr((req_)->cc_ptr, &incomplete__); \
    dtype_release_if_not_builtin(MPIDI_SIMPLE_REQUEST(req_)->datatype); \
    if (!incomplete__) \
        MPIDI_CH4U_request_release(req_);    \
}

#define MPIDI_SIMPLE_REQUEST_ENQUEUE(req,queue) \
{ \
    if ((queue).tail != NULL) \
        MPIDI_SIMPLE_REQUEST((queue).tail)->next = req; \
    else \
        (queue).head = req; \
    (queue).tail = req; \
}

#define MPIDI_SIMPLE_REQUEST_DEQUEUE(req_p,prev_req,queue) \
{ \
    MPIR_Request *next = MPIDI_SIMPLE_REQUEST(*(req_p))->next; \
    if ((queue).head == *(req_p)) \
        (queue).head = next; \
    else \
        MPIDI_SIMPLE_REQUEST(prev_req)->next = next; \
    if ((queue).tail == *(req_p)) \
        (queue).tail = prev_req; \
    MPIDI_SIMPLE_REQUEST(*(req_p))->next = NULL; \
}

#define MPIDI_SIMPLE_REQUEST_DEQUEUE_AND_SET_ERROR(req_p,prev_req,queue,err) \
{ \
    MPIR_Request *next = MPIDI_SIMPLE_REQUEST(*(req_p))->next; \
    if ((queue).head == *(req_p)) \
        (queue).head = next; \
    else \
        MPIDI_SIMPLE_REQUEST(prev_req)->next = next; \
    if ((queue).tail == *(req_p)) \
        (queue).tail = prev_req; \
    (*(req_p))->status.MPI_ERROR = err; \
    MPIDI_SIMPLE_REQUEST_COMPLETE(*(req_p)); \
    *(req_p) = next; \
}

#define MPIDI_SIMPLE_REQUEST_CREATE_SREQ(sreq_)	\
{								\
    MPIDI_SIMPLE_REQUEST_ALLOC_AND_INIT(sreq_,2);      \
    (sreq_)->kind = MPIR_REQUEST_KIND__SEND;				\
    (sreq_)->u.persist.real_request   = NULL;                          \
}

#define MPIDI_SIMPLE_REQUEST_CREATE_RREQ(rreq_)	\
{								\
    MPIDI_SIMPLE_REQUEST_ALLOC_AND_INIT(rreq_,2);      \
    (rreq_)->kind = MPIR_REQUEST_KIND__RECV;				\
    (rreq_)->u.persist.real_request   = NULL;                          \
}

/* ---------------------------------------------------- */
/* matching macros                                      */
/* ---------------------------------------------------- */
#define MPIDI_SIMPLE_ENVELOPE_SET(ptr_,rank_,tag_,context_id_) \
{ \
    (ptr_)->rank = rank_; \
    (ptr_)->tag = tag_; \
    (ptr_)->context_id = context_id_; \
}

#define MPIDI_SIMPLE_ENVELOPE_GET(ptr_,rank_,tag_,context_id_) \
{ \
    rank_ = (ptr_)->rank; \
    tag_ = (ptr_)->tag; \
    context_id_ = (ptr_)->context_id; \
}

#define MPIDI_SIMPLE_ENVELOPE_MATCH(ptr_,rank_,tag_,context_id_) \
    (((ptr_)->rank == (rank_) || (rank_) == MPI_ANY_SOURCE) && \
     ((ptr_)->tag == (tag_) || (tag_) == MPI_ANY_TAG) && \
     (ptr_)->context_id == (context_id_))

/*
 * Helper routines and macros for request completion
 */
#define MPIDI_SIMPLE_REQUEST_ALLOC_AND_INIT(req,count)     \
  do {                                                              \
    (req) = (MPIR_Request*)MPIU_Handle_obj_alloc(&MPIR_Request_mem);      \
    if (req == NULL)                                                       \
        MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request"); \
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
                == MPIR_REQUEST);                       \
    MPIR_cc_set(&req->cc, 1);                           \
    req->cc_ptr = &req->cc;                             \
    MPIU_Object_set_ref(req, count);                    \
    req->u.ureq.greq_fns          = NULL;                      \
    MPIR_STATUS_SET_COUNT(req->status, 0);              \
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);     \
    req->status.MPI_SOURCE    = MPI_UNDEFINED;          \
    req->status.MPI_TAG       = MPI_UNDEFINED;          \
    req->status.MPI_ERROR     = MPI_SUCCESS;            \
    req->comm                 = NULL;                   \
    MPIR_REQUEST_CLEAR_DBG(req);                        \
  } while (0)

#define DECL_FUNC(FUNCNAME)  MPL_QUOTE(FUNCNAME)

#undef FUNCNAME
#define FUNCNAME nothing
#define BEGIN_FUNC(FUNCNAME)                    \
  MPIDI_STATE_DECL(FUNCNAME);                   \
  MPIDI_FUNC_ENTER(FUNCNAME);
#define END_FUNC(FUNCNAME)                      \
  MPIDI_FUNC_EXIT(FUNCNAME);
#define END_FUNC_RC(FUNCNAME) \
  fn_exit:                    \
  MPIDI_FUNC_EXIT(FUNCNAME);  \
  return mpi_errno;           \
fn_fail:                      \
  goto fn_exit;

#define __SHORT_FILE__                          \
  (strrchr(__FILE__,'/')                        \
   ? strrchr(__FILE__,'/')+1                    \
   : __FILE__                                   \
)

int MPIDI_SIMPLE_barrier_vars_init(MPIDI_SIMPLE_barrier_vars_t *barrier_region);
extern MPIDI_SIMPLE_request_queue_t MPIDI_SIMPLE_sendq;
extern MPIDI_SIMPLE_request_queue_t MPIDI_SIMPLE_recvq_unexpected;
extern MPIDI_SIMPLE_request_queue_t MPIDI_SIMPLE_recvq_posted;



#endif /* SHM_SIMPLE_IMPL_H_INCLUDED */
