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
#ifndef NETMOD_OFI_IMPL_H_INCLUDED
#define NETMOD_OFI_IMPL_H_INCLUDED

#include <mpidimpl.h>
#include "types.h"
#include "mpidch4r.h"
#include "ch4_impl.h"
#include "iovec_util.h"
EXTERN_C_BEGIN
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
ILU(void *, Handle_indirect_init, void *(**)[], int *, int, int, int, int);
ILU(void, Handle_obj_alloc_complete, struct MPIU_Object_alloc_t *, int);
ILU(void, Handle_obj_free, struct MPIU_Object_alloc_t *objmem, void *object);
ILU(void *, Handle_get_ptr_indirect, int, struct MPIU_Object_alloc_t *);
#undef ILU
#endif /* __cplusplus */
#endif /* __clang__ || __INTEL_COMPILER */


#define COMM_TO_INDEX(comm,rank) COMM_OFI(comm).vcrt->vcr_table[rank].addr_idx
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
#define COMM_TO_EP(comm,rank)  COMM_OFI(comm).vcrt->vcr_table[rank].ep_idx
#define MPIDI_MAX_ENDPOINTS 256
#define MPIDI_MAX_ENDPOINTS_BITS 8
#define G_TXC_TAG(x) MPIDI_Global.ctx[x].tx_tag
#define G_TXC_RMA(x) MPIDI_Global.ctx[x].tx_rma
#define G_TXC_MSG(x) MPIDI_Global.ctx[x].tx_msg
#define G_TXC_CTR(x) MPIDI_Global.ctx[x].tx_ctr
#define G_RXC_TAG(x) MPIDI_Global.ctx[x].rx_tag
#define G_RXC_RMA(x) MPIDI_Global.ctx[x].rx_rma
#define G_RXC_MSG(x) MPIDI_Global.ctx[x].rx_msg
#define G_RXC_CTR(x) MPIDI_Global.ctx[x].rx_ctr
#else
#define COMM_TO_EP(comm,rank) 0
#define MPIDI_MAX_ENDPOINTS 0
#define MPIDI_MAX_ENDPOINTS_BITS 0
#define G_TXC_TAG(x) MPIDI_Global.ep
#define G_TXC_RMA(x) MPIDI_Global.ep
#define G_TXC_MSG(x) MPIDI_Global.ep
#define G_TXC_CTR(x) MPIDI_Global.ep
#define G_RXC_TAG(x) MPIDI_Global.ep
#define G_RXC_RMA(x) MPIDI_Global.ep
#define G_RXC_MSG(x) MPIDI_Global.ep
#define G_RXC_CTR(x) MPIDI_Global.ep
#endif
#ifdef MPIDI_USE_AV_TABLE
#define COMM_TO_PHYS(comm,rank)  ((fi_addr_t)(uintptr_t)COMM_TO_INDEX(comm,rank))
#define TO_PHYS(rank)            ((fi_addr_t)(uintptr_t)rank)
#else
#define COMM_TO_PHYS(comm,rank)  MPIDI_Addr_table->table[COMM_TO_INDEX(comm,rank)].dest
#define TO_PHYS(rank)            MPIDI_Addr_table->table[rank].dest
#endif
#define REQ_OFI(req,field) ((req)->dev.ch4.netmod.ofi.field)
#define COMM_OFI(comm)     ((comm)->dev.ch4.netmod.ofi)

/*
 * Helper routines and macros for request completion
 */
#define MPIDI_Win_request_tls_alloc(req)                                \
  ({                                                                           \
  (req) = (MPIDI_Win_request*)MPIU_Handle_obj_alloc(&MPIDI_Request_mem); \
  if (req == NULL)                                                           \
    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Win Request");   \
  })

#define MPIDI_Win_request_tls_free(req) \
  MPIU_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Win_request_complete(req)                 \
  ({                                                    \
  int count;                                          \
  MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
              == MPID_REQUEST);                       \
  MPIU_Object_release_ref(req, &count);               \
  MPIU_Assert(count >= 0);                            \
  if (count == 0)                                     \
    {                                                 \
      MPIDI_Win_datatype_unmap(&req->noncontig->target_dt); \
      MPIDI_Win_datatype_unmap(&req->noncontig->origin_dt); \
      MPIDI_Win_datatype_unmap(&req->noncontig->result_dt); \
      MPIU_Free(req->noncontig);                      \
      MPIDI_Win_request_tls_free(req);                \
    }                                                 \
  })


#define MPIDI_Ssendack_request_tls_alloc(req)                           \
  ({                                                                    \
  (req) = (MPIDI_Ssendack_request*)                                     \
    MPIU_Handle_obj_alloc(&MPIDI_Request_mem);                          \
  if (req == NULL)                                                      \
    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1,                              \
               "Cannot allocate Ssendack Request");                     \
  })

#define MPIDI_Ssendack_request_tls_free(req) \
  MPIU_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Ssendack_request_alloc_and_init(req)      \
  ({                                                    \
    MPIDI_Ssendack_request_tls_alloc(req);              \
    MPIU_Assert(req != NULL);                           \
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
                == MPID_SSENDACK_REQUEST);              \
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


#define MPIDI_NM_PROGRESS()                              \
  ({                                                     \
    mpi_errno = MPIDI_Progress_test();                   \
    if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno); \
  })

#define MPIDI_NM_PROGRESS_WHILE(cond)                 \
  ({                                                  \
  while (cond)                                        \
       MPIDI_NM_PROGRESS();                           \
  })

#define MPIDI_CH4_NMI_OFI_ERR  MPIR_ERR_CHKANDJUMP4
#define FI_RC(FUNC,STR)                                     \
    do {                                                    \
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);   \
        ssize_t _ret = FUNC;                                \
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);    \
        MPIDI_CH4_NMI_OFI_ERR(_ret<0,                       \
                              mpi_errno,                    \
                              MPI_ERR_OTHER,                \
                              "**ofid_"#STR,                \
                              "**ofid_"#STR" %s %d %s %s",  \
                              __SHORT_FILE__,               \
                              __LINE__,                     \
                              FCNAME,                       \
                              fi_strerror(-_ret));          \
    } while (0)

#define FI_RC_NOLOCK(FUNC,STR)                              \
    do {                                                    \
        ssize_t _ret = FUNC;                                \
        MPIDI_CH4_NMI_OFI_ERR(_ret<0,                       \
                              mpi_errno,                    \
                              MPI_ERR_OTHER,                \
                              "**ofid_"#STR,                \
                              "**ofid_"#STR" %s %d %s %s",  \
                              __SHORT_FILE__,               \
                              __LINE__,                     \
                              FCNAME,                       \
                              fi_strerror(-_ret));          \
    } while (0)

#define FI_RC_RETRY(FUNC,STR)                               \
    do {                                                    \
    ssize_t _ret;                                           \
    do {                                                    \
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);   \
        _ret = FUNC;                                        \
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);    \
        if(likely(_ret==0)) break;                          \
        MPIDI_CH4_NMI_OFI_ERR(_ret!=-FI_EAGAIN,             \
                              mpi_errno,                    \
                              MPI_ERR_OTHER,                \
                              "**ofid_"#STR,                \
                              "**ofid_"#STR" %s %d %s %s",  \
                              __SHORT_FILE__,               \
                              __LINE__,                     \
                              FCNAME,                       \
                              fi_strerror(-_ret));          \
        MPIDI_NM_PROGRESS();                                \
    } while (_ret == -FI_EAGAIN);                           \
    } while (0)

#define FI_RC_RETRY2(FUNC1,FUNC2,STR)                       \
    do {                                                    \
    ssize_t _ret;                                           \
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);       \
    FUNC1;                                                  \
    do {                                                    \
        _ret = FUNC2;                                       \
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);    \
        if(likely(_ret==0)) break;                          \
        MPIDI_CH4_NMI_OFI_ERR(_ret!=-FI_EAGAIN,             \
                              mpi_errno,                    \
                              MPI_ERR_OTHER,                \
                              "**ofid_"#STR,                \
                              "**ofid_"#STR" %s %d %s %s",  \
                              __SHORT_FILE__,               \
                              __LINE__,                     \
                              FCNAME,                       \
                              fi_strerror(-_ret));          \
        MPIDI_NM_PROGRESS();                                \
        MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);   \
    } while (_ret == -FI_EAGAIN);                           \
    } while (0)


#define FI_RC_RETRY_NOLOCK(FUNC,STR)                          \
  do                                                          \
    {                                                         \
     ssize_t _ret;                                            \
     do {                                                     \
         _ret = FUNC;                                         \
         if (likely(_ret==0)) break;                          \
         MPIDI_CH4_NMI_OFI_ERR(_ret!=-FI_EAGAIN,              \
                               mpi_errno,                     \
                               MPI_ERR_OTHER,                 \
                               "**ofid_"#STR,                 \
                               "**ofid_"#STR" %s %d %s %s",   \
                               __SHORT_FILE__,                \
                               __LINE__,                      \
                               FCNAME,                        \
                               fi_strerror(-_ret));           \
         MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_FI_MUTEX);     \
         MPIDI_NM_PROGRESS();                                 \
         MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_FI_MUTEX);    \
     } while (_ret == -FI_EAGAIN);                            \
    } while (0)

#define MPIDI_CH4_NMI_PMI_RC_POP(FUNC,STR)                    \
  do                                                          \
    {                                                         \
      pmi_errno  = FUNC;                                      \
      MPIDI_CH4_NMI_OFI_ERR(pmi_errno!=PMI_SUCCESS,           \
                            mpi_errno,                        \
                            MPI_ERR_OTHER,                    \
                            "**ofid_"#STR,                    \
                            "**ofid_"#STR" %s %d %s %s",      \
                            __SHORT_FILE__,                   \
                            __LINE__,                         \
                            FCNAME,                           \
                            #STR);                            \
    } while (0)

#define MPIDI_CH4_NMI_MPI_RC_POP(FUNC)                               \
  do                                                                 \
    {                                                                \
      mpi_errno = FUNC;                                              \
      if (unlikely(mpi_errno!=MPI_SUCCESS)) MPIR_ERR_POP(mpi_errno); \
    } while (0)

#define MPIU_STR_RC(FUNC,STR)                                   \
  do                                                            \
    {                                                           \
      str_errno = FUNC;                                         \
      MPIDI_CH4_NMI_OFI_ERR(str_errno!=MPIU_STR_SUCCESS,        \
                            mpi_errno,                          \
                            MPI_ERR_OTHER,                      \
                            "**"#STR,                           \
                            "**"#STR" %s %d %s %s",             \
                            __SHORT_FILE__,                     \
                            __LINE__,                           \
                            FCNAME,                             \
                            #STR);                              \
    } while (0)

#define REQ_CREATE(req)                           \
  ({                                              \
    req = MPIDI_Request_alloc_and_init(2);        \
  })

#define WINREQ_CREATE(req)				\
  ({						\
    req = MPIDI_Win_request_alloc_and_init(1);	\
  })

#define SENDREQ_CREATE_LW(req)                     \
  ({                                               \
    req = MPIDI_Request_alloc_and_init_send_lw(1); \
  })

#define SSENDACKREQ_CREATE(req)            \
  ({                                       \
    MPIDI_Ssendack_request_tls_alloc(req); \
  })

#ifdef HAVE_ERROR_CHECKING
#define MPIDI_EPOCH_CHECK_SYNC(win, mpi_errno, stmt)                                 \
({                                                                                   \
  MPID_BEGIN_ERROR_CHECKS;                                                           \
  if(WIN_OFI(win)->sync.origin_epoch_type == WIN_OFI(win)->sync.target_epoch_type && \
     WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_REFENCE)                   \
    {                                                                                \
      WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_FENCE;                     \
      WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_FENCE;                     \
    }                                                                                \
  if(WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_NONE ||                    \
     WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_POST)                      \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,stmt,"**rmasync")              \
  MPID_END_ERROR_CHECKS;                                                             \
})

#define MPIDI_EPOCH_CHECK_TYPE(win,mpi_errno,stmt)                 \
({                                                                 \
  MPID_BEGIN_ERROR_CHECKS;                                         \
  if(WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_NONE &&  \
     WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_REFENCE) \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,             \
                          stmt, "**rmasync");                      \
})

#define MPIDI_EPOCH_CHECK_START(win,mpi_errno,stmt)                                  \
({                                                                                   \
  MPID_BEGIN_ERROR_CHECKS;                                                           \
  if (WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_START &&                  \
      !MPIDI_valid_group_rank(COMM_TO_INDEX(win->comm_ptr,target_rank),              \
                              WIN_OFI(win)->sync.sc.group))                          \
      MPIR_ERR_SETANDSTMT(mpi_errno,                                                 \
                          MPI_ERR_RMA_SYNC,                                          \
                          stmt,                                                      \
                          "**rmasync");                                              \
  MPID_END_ERROR_CHECKS;                                                             \
})

#define MPIDI_EPOCH_FENCE_CHECK(win,mpi_errno,stmt)                                \
({                                                                                 \
  MPID_BEGIN_ERROR_CHECKS;                                                         \
  if(WIN_OFI(win)->sync.origin_epoch_type != WIN_OFI(win)->sync.target_epoch_type) \
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                               \
                        stmt, "**rmasync");                                        \
  if (!(massert & MPI_MODE_NOPRECEDE) &&                                           \
      WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_FENCE &&                \
      WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_REFENCE &&              \
      WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_NONE)                   \
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                               \
                        stmt, "**rmasync");                                        \
  MPID_END_ERROR_CHECKS;                                                           \
})

#define MPIDI_EPOCH_POST_CHECK(win,mpi_errno,stmt)                 \
({                                                                 \
  MPID_BEGIN_ERROR_CHECKS;                                         \
  if(WIN_OFI(win)->sync.target_epoch_type != MPID_EPOTYPE_NONE &&  \
     WIN_OFI(win)->sync.target_epoch_type != MPID_EPOTYPE_REFENCE) \
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,               \
                        stmt, "**rmasync");                        \
  MPID_END_ERROR_CHECKS;                                           \
})

#define MPIDI_EPOCH_LOCK_CHECK(win,mpi_errno,stmt)                    \
({                                                                    \
  MPID_BEGIN_ERROR_CHECKS;                                            \
  if((WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_LOCK) &&   \
     (WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_LOCK_ALL)) \
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                  \
                        stmt, "**rmasync");                           \
  MPID_END_ERROR_CHECKS;                                              \
})

#define MPIDI_EPOCH_FREE_CHECK(win,mpi_errno,stmt)                                   \
({                                                                                   \
  MPID_BEGIN_ERROR_CHECKS;                                                           \
  if(WIN_OFI(win)->sync.origin_epoch_type != WIN_OFI(win)->sync.target_epoch_type || \
     (WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_NONE &&                   \
      WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_REFENCE))                 \
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC, stmt, "**rmasync"); \
  MPID_END_ERROR_CHECKS;                                                             \
})

#define MPIDI_EPOCH_ORIGIN_CHECK(win, epoch_type, mpi_errno, stmt)  \
({                                                                  \
  MPID_BEGIN_ERROR_CHECKS;                                          \
  if(WIN_OFI(win)->sync.origin_epoch_type != epoch_type)            \
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                \
                        stmt, "**rmasync");                         \
  MPID_END_ERROR_CHECKS;                                            \
})

#define MPIDI_EPOCH_TARGET_CHECK(win, epoch_type, mpi_errno, stmt) \
({                                                                 \
  MPID_BEGIN_ERROR_CHECKS;                                         \
  if(WIN_OFI(win)->sync.target_epoch_type != epoch_type)           \
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,               \
                        stmt, "**rmasync");                        \
  MPID_END_ERROR_CHECKS;                                           \
})

#else /* HAVE_ERROR_CHECKING */
#define MPIDI_EPOCH_CHECK_SYNC(win_ofi, mpi_errno, stmt)           if(0) goto fn_fail;
#define MPIDI_EPOCH_CHECK_TYPE(win,mpi_errno,stmt)                 if(0) goto fn_fail;
#define MPIDI_EPOCH_CHECK_START(win,mpi_errno,stmt)                if(0) goto fn_fail;
#define MPIDI_EPOCH_FENCE_CHECK(win_ofi, mpi_errno, stmt)          if(0) goto fn_fail;
#define MPIDI_EPOCH_POST_CHECK(win_ofi, mpi_errno, stmt)           if(0) goto fn_fail;
#define MPIDI_EPOCH_LOCK_CHECK(win_ofi, mpi_errno, stmt)           if(0) goto fn_fail;
#define MPIDI_EPOCH_FREE_CHECK(win_ofi, mpi_errno, stmt)           if(0) goto fn_fail;
#define MPIDI_EPOCH_ORIGIN_CHECK(win, epoch_type, mpi_errno, stmt) if(0) goto fn_fail;
#define MPIDI_EPOCH_TARGET_CHECK(win, epoch_type, mpi_errno, stmt) if(0) goto fn_fail;
#endif /* HAVE_ERROR_CHECKING */

#define MPIDI_EPOCH_FENCE_EVENT(win,massert)                       \
({                                                                 \
  if(massert & MPI_MODE_NOSUCCEED)                                 \
    {                                                              \
      WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_NONE;    \
      WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_NONE;    \
    }                                                              \
  else                                                             \
    {                                                              \
      WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_REFENCE; \
      WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_REFENCE; \
    }                                                              \
})

#define MPIDI_EPOCH_TARGET_EVENT(win)                              \
({                                                                 \
  if(WIN_OFI(win)->sync.target_epoch_type == MPID_EPOTYPE_REFENCE) \
    WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_REFENCE;   \
  else                                                             \
    WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_NONE;      \
})

#define MPIDI_EPOCH_ORIGIN_EVENT(win)                                       \
({                                                                 \
  if(WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_REFENCE) \
    WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_REFENCE;   \
  else                                                             \
    WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_NONE;      \
})


#define WINFO(w,rank)                                                   \
({                                                                      \
  void *_p;                                                             \
  _p = &(((MPIDI_Win_info*) WIN_OFI(w)->winfo)[rank]);                  \
  _p;                                                                   \
})

#define WINFO_BASE(w,rank)                                              \
({                                                                      \
  void *_p;                                                             \
  _p = NULL;                                                            \
  _p;                                                                   \
})

#define WINFO_BASE_FORCE(w,rank)                                        \
({                                                                      \
  void *_p;                                                             \
  _p = (((MPIDI_Win_info*) WIN_OFI(w)->winfo)[rank]).base_addr;         \
  _p;                                                                   \
})

#define WINFO_DISP_UNIT(w,rank)                                         \
({                                                                      \
  uint32_t _v;                                                          \
  if(WIN_OFI(w)->winfo) {                                               \
      _v = (((MPIDI_Win_info*) WIN_OFI(w)->winfo)[rank]).disp_unit;     \
  }                                                                     \
  else {                                                                \
      _v = w->disp_unit;                                                \
  }                                                                     \
  _v;                                                                   \
})

#define WINFO_MR_KEY(w,rank)                                            \
({                                                                      \
  uint64_t _v;                                                          \
  _v = WIN_OFI(w)->mr_key;                                              \
  _v;                                                                   \
})

#define WINFO_MR(w,target)                                              \
({                                                                      \
  void* _v;                                                             \
  _v = fi_mr_desc(WIN_OFI(w)->mr);                                      \
  _v;                                                                   \
})

/*  Prototypes */

/* Common Utility functions used by the
 * C and C++ components
 */
__ALWAYS_INLINE__ MPID_Request *MPIDI_Request_alloc_and_init(int count)
{
    MPID_Request *req;
    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);

    if (req == NULL)
        MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request");

    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIDI_CH4R_REQUEST(req, req) = NULL;
    MPID_cc_set(&req->cc, 1);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, count);
    req->greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG = MPI_UNDEFINED;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->comm = NULL;
    req->errflag = MPIR_ERR_NONE;
    MPIR_REQUEST_CLEAR_DBG(req);
    return req;
}

__ALWAYS_INLINE__ MPID_Request *MPIDI_Request_alloc_and_init_send_lw(int count)
{
    MPID_Request *req;
    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
    if (req == NULL)
        MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request");
    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIDI_CH4R_REQUEST(req, req) = NULL;
    MPID_cc_set(&req->cc, 0);
    req->cc_ptr  = &req->cc;
    MPIU_Object_set_ref(req, count);
    req->greq_fns          = NULL;
    req->status.MPI_ERROR  = MPI_SUCCESS;
    req->kind              = MPID_REQUEST_SEND;
    req->comm              = NULL;
    req->errflag           = MPIR_ERR_NONE;
    MPIR_REQUEST_CLEAR_DBG(req);
    return req;
}

static inline void MPIDI_CH4_NMI_OFI_request_release(MPID_Request * req)
{
    int count;
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIU_Object_release_ref(req, &count);
    MPIU_Assert(count >= 0);

    if (count == 0) {
        MPIU_Assert(MPID_cc_is_complete(&req->cc));

        if (req->comm)
            MPIR_Comm_release(req->comm);

        if (req->greq_fns)
            MPIU_Free(req->greq_fns);

        MPIU_Handle_obj_free(&MPIDI_Request_mem, req);
    }
    return;
}

static inline void MPIDI_CH4_NMI_OFI_request_complete(MPID_Request *req)
{
    int count;
    MPID_cc_decr(req->cc_ptr, &count);
    MPIU_Assert(count >= 0);
    MPIDI_CH4_NMI_OFI_request_release(req);
}

static inline fi_addr_t _comm_to_phys(MPID_Comm * comm, int rank, int ep_family)
{
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
    int ep_num = COMM_TO_EP(comm, rank);
    int offset = MPIDI_Global.ctx[ep_num].ctx_offset;
    int rx_idx = offset + ep_family;
    return fi_rx_addr(COMM_TO_PHYS(comm, rank), rx_idx, MPIDI_MAX_ENDPOINTS_BITS);
#else
    return COMM_TO_PHYS(comm, rank);
#endif
}

static inline fi_addr_t _to_phys(int rank, int ep_family)
{
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
    int ep_num = 0;
    int offset = MPIDI_Global.ctx[ep_num].ctx_offset;
    int rx_idx = offset + ep_family;
    return fi_rx_addr(TO_PHYS(rank), rx_idx, MPIDI_MAX_ENDPOINTS_BITS);
#else
    return TO_PHYS(rank);
#endif
}

static inline bool is_tag_sync(uint64_t match_bits)
{
    return (0 != (MPID_SYNC_SEND & match_bits));
}

static inline uint64_t init_sendtag(MPIU_Context_id_t contextid, int source, int tag, uint64_t type)
{
    uint64_t match_bits;
    match_bits = contextid;
    match_bits = (match_bits << MPID_SOURCE_SHIFT);
    match_bits |= source;
    match_bits = (match_bits << MPID_TAG_SHIFT);
    match_bits |= (MPID_TAG_MASK & tag) | type;
    return match_bits;
}

/* receive posting */
static inline uint64_t init_recvtag(uint64_t * mask_bits,
                                    MPIU_Context_id_t contextid, int source, int tag)
{
    uint64_t match_bits = 0;
    *mask_bits = MPID_PROTOCOL_MASK;
    match_bits = contextid;
    match_bits = (match_bits << MPID_SOURCE_SHIFT);

    if (MPI_ANY_SOURCE == source) {
        match_bits = (match_bits << MPID_TAG_SHIFT);
        *mask_bits |= MPID_SOURCE_MASK;
    }
    else {
        match_bits |= source;
        match_bits = (match_bits << MPID_TAG_SHIFT);
    }

    if (MPI_ANY_TAG == tag)
        *mask_bits |= MPID_TAG_MASK;
    else
        match_bits |= (MPID_TAG_MASK & tag);

    return match_bits;
}

static inline int get_tag(uint64_t match_bits)
{
    return ((int) (match_bits & MPID_TAG_MASK));
}

static inline int get_source(uint64_t match_bits)
{
    return ((int) ((match_bits & MPID_SOURCE_MASK) >> MPID_TAG_SHIFT));
}


static inline void MPIDI_Win_datatype_unmap(MPIDI_Win_dt *dt)
{
  if(dt->map != &dt->__map)
    MPIU_Free(dt->map);
}
/* Utility functions */
extern int   MPIDI_OFI_Control_handler(void *am_hdr,size_t am_hdr_sz,uint64_t reply_token,
                                       void **data,size_t * data_sz,int *is_contig,
                                       MPIDI_CH4_NM_am_completion_handler_fn *cmpl_handler_fn,
                                       MPID_Request ** req);
extern int   MPIDI_OFI_VCRT_Create(int size, struct MPIDI_VCRT **vcrt_ptr);
extern int   MPIDI_OFI_VCRT_Release(struct MPIDI_VCRT *vcrt);
extern void  MPIDI_OFI_Map_create(void **map);
extern void  MPIDI_OFI_Map_destroy(void *map);
extern void  MPIDI_OFI_Map_set(void *_map, uint64_t id, void *val);
extern void  MPIDI_OFI_Map_erase(void *_map, uint64_t id);
extern void *MPIDI_OFI_Map_lookup(void *_map, uint64_t id);
extern int   MPIDI_OFI_control_dispatch(void *buf);
extern void  MPIDI_OFI_Index_datatypes();
extern void  MPIDI_OFI_Index_allocator_create(void **_indexmap, int start);
extern int   MPIDI_OFI_Index_allocator_alloc(void *_indexmap);
extern void  MPIDI_OFI_Index_allocator_free(void *_indexmap, int index);
extern void  MPIDI_OFI_Index_allocator_destroy(void *_indexmap);

EXTERN_C_END
#endif
