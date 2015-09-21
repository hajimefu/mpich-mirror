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
#ifndef SHM_IMPL_H_INCLUDED
#define SHM_IMPL_H_INCLUDED

/* for some reason these variables can't be set with configure */
#define USE_MMAP_SHM 1
#define MPID_NEM_USE_LOCK_FREE_QUEUES 1

#include <mpidimpl.h>
#include "mpidch4u.h"

#include "mpihandlemem.h"
#include "mpiu_os_wrappers_pre.h"
#include "mpiu_shm_wrappers.h"
#include "mpiu_process_wrappers.h"
#include "mpidbg.h"
#include "pmi.h"

/* ---------------------------------------------------- */
/* temp headers                                         */
/* ---------------------------------------------------- */
#include "ch4_shm_datatypes.h"  /* MPID_nem datatypes like cell, fastbox defined here */
#include "ch4_shm_defs.h"       /* MPID_nem objects like shared memory region defined here */
#include "ch4_shm_queue.h"      /* MPID_nem_queue functions defined here */

/* ---------------------------------------------------- */
/* shm specific object data                      */
/* ---------------------------------------------------- */
/* VCR Table Data */
typedef struct MPIDI_SHM_VCR {
    unsigned int pg_rank;
} MPIDI_SHM_VCR;

struct MPIDI_SHM_VCRT {
    MPIU_OBJECT_HEADER;
    unsigned size;                /**< Number of entries in the table */
    MPIDI_SHM_VCR vcr_table[0];       /**< Array of virtual connection references */
};
typedef struct MPIDI_SHM_VCRT *MPID_SHM_VCRT;

#define REQ_SHM(req) (&(req)->dev.ch4.shm.simple)
/* ---------------------------------------------------- */
/* general send/recv queue types, macros and objects    */
/* ---------------------------------------------------- */
typedef struct {
    MPID_Request *head;
    MPID_Request *tail;
} MPIDI_shm_queue_t;
extern MPIDI_shm_queue_t MPIDI_shm_sendq;       /* defined in send.h */
extern MPIDI_shm_queue_t MPIDI_shm_recvq_posted;        /* defined in recv.h */
extern MPIDI_shm_queue_t MPIDI_shm_recvq_unexpected;    /* defined in recv.h */

#define REQ_SHM_COMPLETE(req_) \
{ \
    int incomplete__; \
    MPID_cc_decr((req_)->cc_ptr, &incomplete__); \
    if (!incomplete__) \
        MPIDI_Request_release(req_);	\
}

#define REQ_SHM_ENQUEUE(req,queue) \
{ \
    if ((queue).tail != NULL) \
        REQ_SHM((queue).tail)->next = req; \
    else \
        (queue).head = req; \
    (queue).tail = req; \
}

#define REQ_SHM_DEQUEUE_AND_SET_ERROR(req_p,prev_req,queue,err) \
{ \
    MPID_Request *next = REQ_SHM(*(req_p))->next; \
    if ((queue).head == *(req_p)) \
        (queue).head = next; \
    else \
        REQ_SHM(prev_req)->next = next; \
    if ((queue).tail == *(req_p)) \
        (queue).tail = prev_req; \
    (*(req_p))->status.MPI_ERROR = err; \
    REQ_SHM_COMPLETE(*(req_p)); \
    *(req_p) = next; \
}

#define MPIDI_Request_create_sreq(sreq_)	\
{								\
    (sreq_) = MPIDI_Request_create();                            \
    MPIU_Object_set_ref((sreq_), 2);				\
    (sreq_)->kind = MPID_REQUEST_SEND;				\
    (sreq_)->partner_request   = NULL;                          \
}

#define MPIDI_Request_create_rreq(rreq_)	\
{								\
    (rreq_) = MPIDI_Request_create();                            \
    MPIU_Object_set_ref((rreq_), 2);				\
    (rreq_)->kind = MPID_REQUEST_RECV;				\
    (rreq_)->partner_request   = NULL;                          \
}

/* ---------------------------------------------------- */
/* matching macros                                      */
/* ---------------------------------------------------- */
#define ENVELOPE_SET(ptr_,rank_,tag_,context_id_) \
{ \
    (ptr_)->rank = rank_; \
    (ptr_)->tag = tag_; \
    (ptr_)->context_id = context_id_; \
}

#define ENVELOPE_GET(ptr_,rank_,tag_,context_id_) \
{ \
    rank_ = (ptr_)->rank; \
    tag_ = (ptr_)->tag; \
    context_id_ = (ptr_)->context_id; \
}

#define ENVELOPE_MATCH(ptr_,rank_,tag_,context_id_) \
    (((ptr_)->rank == (rank_) || (rank_) == MPI_ANY_SOURCE) && \
     ((ptr_)->tag == (tag_) || (tag_) == MPI_ANY_TAG) && \
     (ptr_)->context_id == (context_id_))

/* ---------------------------------------------------- */
/* genral macros and constants */
/* ---------------------------------------------------- */
#define EAGER_THRESHOLD MPID_NEM_MPICH_DATA_LEN
#define TYPE_STANDARD 0
#define TYPE_EAGER 1
#define TYPE_LMT 2
#define TYPE_SYNC 3
#define TYPE_BUFFERED 4
#define TYPE_READY 5
#define TYPE_ACK 6

/*
 * Helper routines and macros for request completion
 */
extern MPIU_Object_alloc_t MPIDI_Request_mem;
#define MPIDI_Request_tls_alloc(req)                                     \
  ({                                                                     \
    (req) = (MPID_Request*)MPIU_Handle_obj_alloc(&MPIDI_Request_mem);    \
    if (req == NULL)                                                     \
      MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request"); \
  })

#define MPIDI_Request_tls_free(req) \
  MPIU_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Request_shm_alloc_and_init(req,count)     \
  ({                                                    \
    MPIDI_Request_tls_alloc(req);                       \
    MPIU_Assert(req != NULL);                           \
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
                == MPID_REQUEST);                       \
    MPID_cc_set(&req->cc, 1);                           \
    req->cc_ptr = &req->cc;                             \
    MPIU_Object_set_ref(req, count);                    \
    req->greq_fns          = NULL;                      \
    MPIR_STATUS_SET_COUNT(req->status, 0);              \
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);     \
    req->status.MPI_SOURCE    = MPI_UNDEFINED;          \
    req->status.MPI_TAG       = MPI_UNDEFINED;          \
    req->status.MPI_ERROR     = MPI_SUCCESS;            \
    req->comm                 = NULL;                   \
  })

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


int MPIDI_CH3I_Seg_alloc(size_t len, void **ptr_p);
int MPIDI_CH3I_Seg_commit(MPID_nem_seg_ptr_t memory, int num_local, int local_rank);
int MPID_nem_barrier_vars_init(MPID_nem_barrier_vars_t * barrier_region);
int MPID_nem_barrier_init(MPID_nem_barrier_t * barrier_region, int init_values);
int MPID_nem_barrier(void);
int MPIDI_CH3I_Seg_destroy(void);

#endif /* NETMOD_SHM_IMPL_H_INCLUDED */
