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
#ifndef NETMOD_OFI_IMPL_H_INCLUDED
#define NETMOD_OFI_IMPL_H_INCLUDED

#include <mpidimpl.h>
#include "types.h"
#include "mpidch4r.h"
#include "ch4_impl.h"
#include "iovec_util.h"

/* The purpose of this hacky #ifdef is to tag                */
/* select MPI util functions with always inline.  A better   */
/* approach would be to declare them inline in a header file */
/* In a static library with the "always_inline" attribute    */
/* This allows an application with ipo/pgo enabled to inline */
/* the function directly into the application despite        */
/* mpi header files not using attribute inline               */
/* Trick doesn't work with gcc or clang                      */
#if defined(__INTEL_COMPILER)
#ifndef __cplusplus
struct MPIU_Object_alloc_t;
#define ILU(ret,fcname,...) inline __attribute__((always_inline)) ret MPIU_##fcname(__VA_ARGS__)
ILU(void *, Handle_obj_alloc_unsafe, struct MPIU_Object_alloc_t *);
ILU(void *, Handle_obj_alloc, struct MPIU_Object_alloc_t *);
ILU(void *, Handle_direct_init, void *, int, int, int);
ILU(void *, Handle_indirect_init, void *(* *)[], int *, int, int, int, int);
ILU(void, Handle_obj_alloc_complete, struct MPIU_Object_alloc_t *, int);
ILU(void, Handle_obj_free, struct MPIU_Object_alloc_t *objmem, void *object);
ILU(void *, Handle_get_ptr_indirect, int, struct MPIU_Object_alloc_t *);
#undef ILU
#endif /* __cplusplus */
#endif /* __clang__ || __INTEL_COMPILER */

#define MPIDI_OFI_COMM_TO_INDEX(comm,rank) \
    MPIDI_OFI_COMM(comm).vcrt->vcr_table[rank].addr_idx
#ifdef MPIDI_OFI_CONFIG_USE_AV_TABLE
#define MPIDI_OFI_COMM_TO_PHYS(comm,rank) \
    ((fi_addr_t)(uintptr_t)MPIDI_OFI_COMM_TO_INDEX(comm,rank))
#define MPIDI_OFI_TO_PHYS(rank)            ((fi_addr_t)(uintptr_t)rank)
#else
#define MPIDI_OFI_COMM_TO_PHYS(comm,rank)                       \
    MPIDI_Addr_table->table[MPIDI_OFI_COMM_TO_INDEX(comm,rank)].dest
#define MPIDI_OFI_TO_PHYS(rank)            MPIDI_Addr_table->table[rank].dest
#endif

#define MPIDI_OFI_WIN(win)     ((win)->dev.netmod.ofi)
/*
 * Helper routines and macros for request completion
 */
#define MPIDI_OFI_ssendack_request_t_tls_alloc(req)             \
    do {                                                                \
        (req) = (MPIDI_OFI_ssendack_request_t*)                 \
            MPIU_Handle_obj_alloc(&MPIR_Request_mem);                  \
        if (req == NULL)                                                \
            MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1,                      \
                       "Cannot allocate Ssendack Request");             \
    } while (0)

#define MPIDI_OFI_ssendack_request_t_tls_free(req) \
  MPIU_Handle_obj_free(&MPIR_Request_mem, (req))

#define MPIDI_OFI_ssendack_request_t_alloc_and_init(req)        \
    do {                                                                \
        MPIDI_OFI_ssendack_request_t_tls_alloc(req);            \
        MPIU_Assert(req != NULL);                                       \
        MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)                    \
                    == MPID_SSENDACK_REQUEST);                          \
    } while (0)

#define MPIDI_OFI_request_create_null_rreq(rreq_, mpi_errno_, FAIL_) \
  do {                                                                  \
    (rreq_) = MPIDI_OFI_request_alloc_and_init(1);                          \
    if ((rreq_) != NULL) {                                              \
      MPIR_cc_set(&(rreq_)->cc, 0);                                     \
      (rreq_)->kind = MPIR_REQUEST_KIND__RECV;                                \
      MPIR_Status_set_procnull(&(rreq_)->status);                       \
    }                                                                   \
    else {                                                              \
      MPIR_ERR_SETANDJUMP(mpi_errno_,MPI_ERR_OTHER,"**nomemreq");       \
    }                                                                   \
  } while (0)


#define MPIDI_OFI_PROGRESS()                              \
    do {                                                          \
        mpi_errno = MPIDI_Progress_test();                        \
        if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);      \
    } while (0)

#define MPIDI_OFI_PROGRESS_WHILE(cond)                 \
    while (cond) MPIDI_OFI_PROGRESS()

#define MPIDI_OFI_ERR  MPIR_ERR_CHKANDJUMP4
#define MPIDI_OFI_CALL(FUNC,STR)                                     \
    do {                                                    \
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);   \
        ssize_t _ret = FUNC;                                \
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);    \
        MPIDI_OFI_ERR(_ret<0,                       \
                              mpi_errno,                    \
                              MPI_ERR_OTHER,                \
                              "**ofid_"#STR,                \
                              "**ofid_"#STR" %s %d %s %s",  \
                              __SHORT_FILE__,               \
                              __LINE__,                     \
                              FCNAME,                       \
                              fi_strerror(-_ret));          \
    } while (0)

#define MPIDI_OFI_CALL_NOLOCK(FUNC,STR)                              \
    do {                                                    \
        ssize_t _ret = FUNC;                                \
        MPIDI_OFI_ERR(_ret<0,                       \
                              mpi_errno,                    \
                              MPI_ERR_OTHER,                \
                              "**ofid_"#STR,                \
                              "**ofid_"#STR" %s %d %s %s",  \
                              __SHORT_FILE__,               \
                              __LINE__,                     \
                              FCNAME,                       \
                              fi_strerror(-_ret));          \
    } while (0)

#define MPIDI_OFI_CALL_RETRY(FUNC,STR)                               \
    do {                                                    \
    ssize_t _ret;                                           \
    do {                                                    \
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);   \
        _ret = FUNC;                                        \
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);    \
        if(likely(_ret==0)) break;                          \
        MPIDI_OFI_ERR(_ret!=-FI_EAGAIN,             \
                              mpi_errno,                    \
                              MPI_ERR_OTHER,                \
                              "**ofid_"#STR,                \
                              "**ofid_"#STR" %s %d %s %s",  \
                              __SHORT_FILE__,               \
                              __LINE__,                     \
                              FCNAME,                       \
                              fi_strerror(-_ret));          \
        MPIDI_OFI_PROGRESS();                                \
    } while (_ret == -FI_EAGAIN);                           \
    } while (0)

#define MPIDI_OFI_CALL_RETRY2(FUNC1,FUNC2,STR)                       \
    do {                                                    \
    ssize_t _ret;                                           \
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);       \
    FUNC1;                                                  \
    do {                                                    \
        _ret = FUNC2;                                       \
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);    \
        if(likely(_ret==0)) break;                          \
        MPIDI_OFI_ERR(_ret!=-FI_EAGAIN,             \
                              mpi_errno,                    \
                              MPI_ERR_OTHER,                \
                              "**ofid_"#STR,                \
                              "**ofid_"#STR" %s %d %s %s",  \
                              __SHORT_FILE__,               \
                              __LINE__,                     \
                              FCNAME,                       \
                              fi_strerror(-_ret));          \
        MPIDI_OFI_PROGRESS();                                \
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);   \
    } while (_ret == -FI_EAGAIN);                           \
    } while (0)


#define MPIDI_OFI_CALL_RETRY_NOLOCK(FUNC,STR)                          \
  do                                                          \
    {                                                         \
     ssize_t _ret;                                            \
     do {                                                     \
         _ret = FUNC;                                         \
         if (likely(_ret==0)) break;                          \
         MPIDI_OFI_ERR(_ret!=-FI_EAGAIN,              \
                               mpi_errno,                     \
                               MPI_ERR_OTHER,                 \
                               "**ofid_"#STR,                 \
                               "**ofid_"#STR" %s %d %s %s",   \
                               __SHORT_FILE__,                \
                               __LINE__,                      \
                               FCNAME,                        \
                               fi_strerror(-_ret));           \
         MPID_THREAD_CS_EXIT(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);     \
         MPIDI_OFI_PROGRESS();                                 \
         MPID_THREAD_CS_ENTER(POBJ,MPIDI_OFI_THREAD_FI_MUTEX);    \
     } while (_ret == -FI_EAGAIN);                            \
    } while (0)

#define MPIDI_OFI_PMI_CALL_POP(FUNC,STR)                    \
  do                                                          \
    {                                                         \
      pmi_errno  = FUNC;                                      \
      MPIDI_OFI_ERR(pmi_errno!=PMI_SUCCESS,           \
                            mpi_errno,                        \
                            MPI_ERR_OTHER,                    \
                            "**ofid_"#STR,                    \
                            "**ofid_"#STR" %s %d %s %s",      \
                            __SHORT_FILE__,                   \
                            __LINE__,                         \
                            FCNAME,                           \
                            #STR);                            \
    } while (0)

#define MPIDI_OFI_MPI_CALL_POP(FUNC)                               \
  do                                                                 \
    {                                                                \
      mpi_errno = FUNC;                                              \
      if (unlikely(mpi_errno!=MPI_SUCCESS)) MPIR_ERR_POP(mpi_errno); \
    } while (0)

#define MPIDI_OFI_STR_CALL(FUNC,STR)                                   \
  do                                                            \
    {                                                           \
      str_errno = FUNC;                                         \
      MPIDI_OFI_ERR(str_errno!=MPL_STR_SUCCESS,        \
                            mpi_errno,                          \
                            MPI_ERR_OTHER,                      \
                            "**"#STR,                           \
                            "**"#STR" %s %d %s %s",             \
                            __SHORT_FILE__,                     \
                            __LINE__,                           \
                            FCNAME,                             \
                            #STR);                              \
    } while (0)

#define MPIDI_OFI_REQUEST_CREATE(req)                 \
    do {                                                      \
        (req) = MPIDI_OFI_request_alloc_and_init(2);  \
    } while (0)

#define MPIDI_OFI_SEND_REQUEST_CREATE_LW(req)                   \
    do {                                                                \
        (req) = MPIDI_OFI_request_alloc_and_init_send_lw(1);    \
    } while (0)

#define MPIDI_OFI_SSEND_ACKREQUEST_CREATE(req)            \
    do {                                                          \
        MPIDI_OFI_ssendack_request_t_tls_alloc(req);      \
    } while (0)

#define WINFO(w,rank) MPIDI_CH4U_WINFO(w,rank)

static inline void *MPIDI_OFI_winfo_base(MPIR_Win *w, int rank)
{
    return NULL;
}

#define MPIDI_OFI_WINFO_DISP_UNIT(w,rank) MPIDI_CH4U_WINFO_DISP_UNIT(w,rank)

static inline uint64_t MPIDI_OFI_winfo_mr_key(MPIR_Win *w, int rank)
{
    return MPIDI_OFI_WIN(w).mr_key;
}

/*  Prototypes */

/* Common Utility functions used by the
 * C and C++ components
 */
__ALWAYS_INLINE__ MPIR_Request *MPIDI_OFI_request_alloc_and_init(int count)
{
    MPIR_Request *req;
    req = (MPIR_Request *) MPIU_Handle_obj_alloc(&MPIR_Request_mem);

    if(req == NULL)
        MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request");

    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPIR_REQUEST);
    MPIDI_CH4U_REQUEST(req, req) = NULL;
    MPIR_cc_set(&req->cc, 1);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, count);
    req->u.ureq.greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG = MPI_UNDEFINED;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->comm = NULL;
    req->u.nbc.errflag = MPIR_ERR_NONE;
#ifdef MPIDI_BUILD_CH4_SHM
    MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req) = NULL;
#endif
    MPIR_REQUEST_CLEAR_DBG(req);
    return req;
}

__ALWAYS_INLINE__ MPIR_Request *MPIDI_OFI_request_alloc_and_init_send_lw(int count)
{
    MPIR_Request *req;
    req = (MPIR_Request *) MPIU_Handle_obj_alloc(&MPIR_Request_mem);
    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPIR_REQUEST);
    MPIDI_CH4U_REQUEST(req, req) = NULL;
    MPIR_cc_set(&req->cc, 0);
    req->cc_ptr  = &req->cc;
    MPIU_Object_set_ref(req, count);
    req->u.ureq.greq_fns          = NULL;
    req->status.MPI_ERROR  = MPI_SUCCESS;
    req->kind              = MPIR_REQUEST_KIND__SEND;
    req->comm              = NULL;
    req->u.nbc.errflag           = MPIR_ERR_NONE;
    MPIR_REQUEST_CLEAR_DBG(req);
    return req;
}

__ALWAYS_INLINE__ MPIDI_OFI_win_request_t *MPIDI_OFI_win_request_alloc_and_init(int count,int extra)
{
    MPIDI_OFI_win_request_t *req;
    req = (MPIDI_OFI_win_request_t*)MPIU_Handle_obj_alloc(&MPIR_Request_mem);
    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPIR_REQUEST);
    MPIU_Object_set_ref(req, count);
    memset((char*)req+MPIDI_REQUEST_HDR_SIZE, 0,
           sizeof(MPIDI_OFI_win_request_t)-
           MPIDI_REQUEST_HDR_SIZE);
    req->noncontig = (MPIDI_OFI_win_noncontig_t*)MPL_calloc(1,(extra)+sizeof(*(req->noncontig)));
    return req;
}

__ALWAYS_INLINE__ void MPIDI_OFI_win_datatype_unmap(MPIDI_OFI_win_datatype_t *dt)
{
    if(dt->map != &dt->__map)
        MPL_free(dt->map);
}

__ALWAYS_INLINE__ void MPIDI_OFI_win_request_complete(MPIDI_OFI_win_request_t *req)
{
    int count;
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPIR_REQUEST);
    MPIU_Object_release_ref(req, &count);
    MPIU_Assert(count >= 0);
    if (count == 0)
    {
        MPIDI_OFI_win_datatype_unmap(&req->noncontig->target_dt);
        MPIDI_OFI_win_datatype_unmap(&req->noncontig->origin_dt);
        MPIDI_OFI_win_datatype_unmap(&req->noncontig->result_dt);
        MPL_free(req->noncontig);
        MPIU_Handle_obj_free(&MPIR_Request_mem, (req));
    }
}

static inline fi_addr_t MPIDI_OFI_comm_to_phys(MPIR_Comm *comm, int rank, int ep_family)
{
#ifdef MPIDI_OFI_CONFIG_USE_SCALABLE_ENDPOINTS
    int ep_num = MPIDI_OFI_COMM_TO_EP(comm, rank);
    int offset = MPIDI_Global.ctx[ep_num].ctx_offset;
    int rx_idx = offset + ep_family;
    return fi_rx_addr(MPIDI_OFI_COMM_TO_PHYS(comm, rank), rx_idx, MPIDI_OFI_MAX_ENDPOINTS_BITS);
#else
    return MPIDI_OFI_COMM_TO_PHYS(comm, rank);
#endif
}

static inline fi_addr_t MPIDI_OFI_to_phys(int rank, int ep_family)
{
#ifdef MPIDI_OFI_CONFIG_USE_SCALABLE_ENDPOINTS
    int ep_num = 0;
    int offset = MPIDI_Global.ctx[ep_num].ctx_offset;
    int rx_idx = offset + ep_family;
    return fi_rx_addr(MPIDI_OFI_TO_PHYS(rank), rx_idx, MPIDI_OFI_MAX_ENDPOINTS_BITS);
#else
    return MPIDI_OFI_TO_PHYS(rank);
#endif
}

static inline bool MPIDI_OFI_is_tag_sync(uint64_t match_bits)
{
    return (0 != (MPIDI_OFI_SYNC_SEND & match_bits));
}

static inline uint64_t MPIDI_OFI_init_sendtag(MPIU_Context_id_t contextid,
                                                      int               source,
                                                      int               tag,
                                                      uint64_t          type)
{
    uint64_t match_bits;
    match_bits = contextid;
    match_bits = (match_bits << MPIDI_OFI_SOURCE_SHIFT);
    match_bits |= source;
    match_bits = (match_bits << MPIDI_OFI_TAG_SHIFT);
    match_bits |= (MPIDI_OFI_TAG_MASK & tag) | type;
    return match_bits;
}

/* receive posting */
static inline uint64_t MPIDI_OFI_init_recvtag(uint64_t          *mask_bits,
                                                      MPIU_Context_id_t  contextid,
                                                      int                source,
                                                      int                tag)
{
    uint64_t match_bits = 0;
    *mask_bits = MPIDI_OFI_PROTOCOL_MASK;
    match_bits = contextid;
    match_bits = (match_bits << MPIDI_OFI_SOURCE_SHIFT);

    if(MPI_ANY_SOURCE == source) {
        match_bits = (match_bits << MPIDI_OFI_TAG_SHIFT);
        *mask_bits |= MPIDI_OFI_SOURCE_MASK;
    } else {
        match_bits |= source;
        match_bits = (match_bits << MPIDI_OFI_TAG_SHIFT);
    }

    if(MPI_ANY_TAG == tag)
        *mask_bits |= MPIDI_OFI_TAG_MASK;
    else
        match_bits |= (MPIDI_OFI_TAG_MASK & tag);

    return match_bits;
}

static inline int MPIDI_OFI_init_get_tag(uint64_t match_bits)
{
    return ((int)(match_bits & MPIDI_OFI_TAG_MASK));
}

static inline int MPIDI_OFI_init_get_source(uint64_t match_bits)
{
    return ((int)((match_bits & MPIDI_OFI_SOURCE_MASK) >> MPIDI_OFI_TAG_SHIFT));
}

static inline MPIR_Request *MPIDI_OFI_context_to_request(void *context)
{
    char *base = (char *) context;
    return (MPIR_Request *) container_of(base, MPIR_Request, dev.ch4.netmod);
}

/* Utility functions */
extern int   MPIDI_OFI_handle_cq_error_util(ssize_t ret);
extern int   MPIDI_OFI_control_handler(void *am_hdr,
                                               void **data,size_t *data_sz,int *is_contig,
                                               MPIDI_CH4_NM_am_completion_handler_fn *cmpl_handler_fn,
                                               MPIR_Request **req);
extern int   MPIDI_OFI_vcrt_create(int size, struct MPIDI_OFI_VCRT **vcrt_ptr);
extern int   MPIDI_OFI_vcrt_release(struct MPIDI_OFI_VCRT *vcrt);
extern void  MPIDI_OFI_map_create(void **map);
extern void  MPIDI_OFI_map_destroy(void *map);
extern void  MPIDI_OFI_map_set(void *_map, uint64_t id, void *val);
extern void  MPIDI_OFI_map_erase(void *_map, uint64_t id);
extern void *MPIDI_OFI_map_lookup(void *_map, uint64_t id);
extern int   MPIDI_OFI_control_dispatch(void *buf);
extern void  MPIDI_OFI_index_datatypes();
extern void  MPIDI_OFI_index_allocator_create(void **_indexmap, int start);
extern int   MPIDI_OFI_index_allocator_alloc(void *_indexmap);
extern void  MPIDI_OFI_index_allocator_free(void *_indexmap, int index);
extern void  MPIDI_OFI_index_allocator_destroy(void *_indexmap);

#endif
